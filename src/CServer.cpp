#include "CServer.h"

namespace nsNetwork
{
    CServer::CServer(int lPort)
        : m_lPort(lPort)
    {
        //! @todo 暂时设定最大连接数为100
        setMaxPendingConnections(100);
    }

    CServer::~CServer()
    {
        close();
    }

    bool CServer::start()
    {
        if( this->isListening() )
        {
            return true;
        }

        return this->listen(QHostAddress::Any, m_lPort);
    }

    void CServer::close()
    {
        foreach(int key, m_mapClientSocket.keys())
        {
            CServerHeartBeatThread *thread = m_mapHeartBeatThread.value(key);
            if( NULL != thread )
            {
                thread->quit();
                thread->wait();

                delete thread;
                thread = NULL;
                m_mapClientSocket.remove(key);
            }
        }

        // 调用该函数会把连接该服务器的QTcpSocket对象资源释放
        QTcpServer::close();

        //! @todo QTcpServer和CTcpServer释放顺序有待商榷
        // QTcpSocket与CTcpSocket并不相同，所以还需将CTcpSocket资源释放
        foreach(int key, m_mapClientSocket.keys())
        {
            CServerHeartBeatThread *thread = m_mapHeartBeatThread.value(key);
            if( NULL != thread )
            {
                thread->quit();
                thread->wait();

                delete thread;
                thread = NULL;
            }
            m_mapClientSocket.remove(key);
        }
    }

    bool CServer::send(QByteArray baData, int socketDiescriptor)
    {
        m_mutex.lock();

        CTcpSocket *tcpClient = m_mapClientSocket.value(socketDiescriptor);

        bool bRet = tcpClient->send(baData);

        m_mutex.unlock();

        return bRet;
    }

    QByteArray CServer::read(int length, int socketDescriptor)
    {
        CTcpSocket *tcpClient = m_mapClientSocket.value(socketDescriptor);

        return tcpClient->read(length);
    }

    QByteArray CServer::readAll(int socketDescriptor)
    {
        CTcpSocket *tcpClient = m_mapClientSocket.value(socketDescriptor);

        return tcpClient->readAll();
    }

    quint64 CServer::bytesAvailable(int socketDescriptor)
    {
        return m_mapClientSocket.value(socketDescriptor)->bytesAvailable();
    }

    void CServer::clearHeartBeatCount(int socketDescriptor)
    {
        m_mapHeartBeatThread.value(socketDescriptor)->clearHeartBeatCount();
    }

    bool CServer::hasPendingConnections() const
    {
        return !m_mapClientSocket.isEmpty();
    }

    CTcpSocket *CServer::nextPendingConnection(int socketDescriptor)
    {
        if( m_mapClientSocket.isEmpty() )
        {
            return NULL;
        }
        return m_mapClientSocket.take(socketDescriptor);
    }

    void CServer::incomingConnection(int handle)
    {
        CTcpSocket *tcpClient = new CTcpSocket;
        tcpClient->setSocketDescriptor(handle);

        addPendingConnection(tcpClient);
    }

    void CServer::addPendingConnection(CTcpSocket *tcpClient)
    {
        m_mapClientSocket.insert(tcpClient->socketDescriptor(), tcpClient);

        connect(tcpClient, SIGNAL(sgReadyRead(int)),  this, SIGNAL(sgReadyRead(int)));
        connect(tcpClient, SIGNAL(sgConnected(int)), this, SIGNAL(sgConnected(int)));
        // 设置为队列形式触发，防止同一时间多个客户端断连
        connect(tcpClient, SIGNAL(sgDisConnected(int)),
                this, SLOT(stDisConnected(int)), Qt::QueuedConnection);

        CServerHeartBeatThread* thread = new CServerHeartBeatThread(tcpClient);
        connect(thread, SIGNAL(sgHeartBreak(int)), this, SLOT(stHeartBreak(int)));
        m_mapHeartBeatThread.insert(tcpClient->socketDescriptor(), thread);
        thread->start();
    }

    void CServer::stHeartBreak(int socketDiescriptor)
    {
        // 当心跳包接受异常，说明客户端处于不正常状态，断连并释放资源
        CTcpSocket *tcpClient = m_mapClientSocket.value(socketDiescriptor);
        if( NULL != tcpClient )
        {
            delete tcpClient;
            tcpClient = NULL;
        }
        m_mapClientSocket.remove(socketDiescriptor);

        CServerHeartBeatThread *thread = m_mapHeartBeatThread.value(socketDiescriptor);
        thread->stop();
        if( NULL != thread )
        {
            delete thread;
            thread = NULL;
        }
        m_mapHeartBeatThread.remove(socketDiescriptor);
    }

    void CServer::stDisConnected(int socketDiescriptor)
    {
        stHeartBreak(socketDiescriptor);

        emit sgDisConnected(socketDiescriptor);
    }

    CServerHeartBeatThread::CServerHeartBeatThread(CTcpSocket *pTcpClient)
        : IHeartBeatThread(pTcpClient)
    {

    }

    CServerHeartBeatThread::~CServerHeartBeatThread()
    {

    }

    void CServerHeartBeatThread::run()
    {
        while (1)
        {
            countHeartBeat();//加一
            if ( 4 <= getHeartBeatCount() )
            {
                clearHeartBeatCount();
                break;
            }
            sleep(30);
        }
        emit sgHeartBreak(m_pTcpClient->socketDescriptor());
    }

} // namespace nsNetwork

