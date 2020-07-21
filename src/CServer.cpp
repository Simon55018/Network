#include "CServer.h"

namespace nsNetwork
{
    CServer::CServer(int lPort)
        : m_lPort(lPort)
    {
        //! @todo 暂时设定最大连接数为100
        setMaxPendingConnections(100);

        // 开启服务端心跳帧处理线程
        m_pHeartBeatThread = new CServerHeartBeatThread;
        connect(m_pHeartBeatThread, SIGNAL(sgHeartBreak(int)), this, SLOT(stHeartBreak(int)));
        m_pHeartBeatThread->start();
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
        if( NULL != m_pHeartBeatThread )
        {
            m_pHeartBeatThread->stop();
            m_pHeartBeatThread->clearPendingTcpSocket();
        }

        //foreach(int key, m_mapClientSocket.keys())
        //{
        //    CTcpSocket *socket = m_mapClientSocket.value(key);
        //    if( NULL != socket )
        //    {
        //        delete socket;
        //        socket = NULL;
        //        m_mapClientSocket.remove(key);
        //    }
        //}
        qDeleteAll(m_mapClientSocket);
        m_mapClientSocket.clear();

        // 调用该函数会把连接该服务器的QTcpSocket对象资源释放
        QTcpServer::close();
    }

    bool CServer::send(QByteArray baData, int socketDescriptor)
    {
        m_mutex.lock();

        CTcpSocket *tcpClient = m_mapClientSocket.value(socketDescriptor);

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
        m_pHeartBeatThread->clearHeartBeatCount(socketDescriptor);
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
        m_pHeartBeatThread->pendingTcpSocket(tcpClient);
    }

    void CServer::addPendingConnection(CTcpSocket *tcpClient)
    {
        m_mapClientSocket.insert(tcpClient->socketDescriptor(), tcpClient);

        connect(tcpClient, SIGNAL(sgReadyRead(int)),  this, SIGNAL(sgReadyRead(int)));
        connect(tcpClient, SIGNAL(sgConnected(int)), this, SIGNAL(sgConnected(int)));
        // 设置为队列形式触发，防止同一时间多个客户端断连
        connect(tcpClient, SIGNAL(sgDisConnected(int)),
                this, SLOT(stDisConnected(int)), Qt::QueuedConnection);
    }

    void CServer::stHeartBreak(int socketDescriptor)
    {
        // 当心跳包接受异常，说明客户端处于不正常状态，断连并释放资源
        CTcpSocket *tcpClient = m_mapClientSocket.value(socketDescriptor);

        m_pHeartBeatThread->removeTcpSocket(socketDescriptor);

        if( NULL != tcpClient )
        {
            delete tcpClient;
            tcpClient = NULL;
        }
        m_mapClientSocket.remove(socketDescriptor);
    }

    void CServer::stDisConnected(int socketDescriptor)
    {
        stHeartBreak(socketDescriptor);

        emit sgDisConnected(socketDescriptor);
    }

    CServerHeartBeatThread::CServerHeartBeatThread(QObject *parent)
        : QThread(parent)
    {
        m_mapTcpSocket.clear();
        setHeartBeatEnable(true);
    }

    CServerHeartBeatThread::~CServerHeartBeatThread()
    {

    }

    void CServerHeartBeatThread::pendingTcpSocket(CTcpSocket *pTcpClient)
    {
        m_mapTcpSocket.insert(pTcpClient->socketDescriptor(), pTcpClient);
    }

    void CServerHeartBeatThread::stop()
    {
        this->setHeartBeatEnable(false);
        if( this->isRunning() )
        {
            this->quit();
            this->wait();
        }
    }

    void CServerHeartBeatThread::clearHeartBeatCount(int socketDescriptor)
    {
        m_mapTcpSocket.value(socketDescriptor)->clearHeartBeatCount();
    }

    void CServerHeartBeatThread::clearPendingTcpSocket()
    {
        m_mapTcpSocket.clear();
    }

    void CServerHeartBeatThread::removeTcpSocket(int socketDescriptor)
    {
        m_mapTcpSocket.remove(socketDescriptor);
    }

    void CServerHeartBeatThread::setHeartBeatEnable(const bool bIsEnable)
    {
        m_bIsHeartBeatEnable = bIsEnable;
    }

    void CServerHeartBeatThread::run()
    {
        while ( m_bIsHeartBeatEnable )
        {
            foreach(CTcpSocket *socket, m_mapTcpSocket)
            {
                socket->countHeartBeat();
                if( 4 <= socket->getHeartBeatCount() )
                {
                    socket->clearHeartBeatCount();
                    emit sgHeartBreak(socket->socketDescriptor());
                }
            }
            sleep(30);
        }
    }

} // namespace nsNetwork

