#include "CServer.h"

namespace nsNetwork
{
    CServer::CServer(int lPort)
        : m_lPort(lPort)
    {
        // 清空容器
        m_hashClientSocket.clear();
        m_listSocketID.clear();

        m_bLoginCert = false;
        m_lOverTime = 0;

        //! @todo 暂时设定最大连接数为100
        setMaxPendingConnections(100);

        // 开启服务端心跳帧处理线程
        m_pHeartBeatThread = new CServerHeartBeatThread;
        connect(m_pHeartBeatThread, SIGNAL(sgHeartBreak(int)), this, SLOT(stHeartBreak(int)), Qt::UniqueConnection);
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

        //foreach(int key, m_hashClientSocket.keys())
        //{
        //    CTcpSocket *socket = m_hashClientSocket.value(key);
        //    if( NULL != socket )
        //    {
        //        delete socket;
        //        socket = NULL;
        //        m_hashClientSocket.remove(key);
        //    }
        //}

        // 清空容器
        m_listSocketID.clear();

        qDeleteAll(m_hashClientSocket);
        m_hashClientSocket.clear();

        // 调用该函数会把连接该服务器的QTcpSocket对象资源释放
        QTcpServer::close();
    }

    bool CServer::send(QByteArray baData, int socketDescriptor)
    {
        m_mutex.lock();

        CTcpSocket *tcpClient = m_hashClientSocket.value(socketDescriptor);

        bool bRet = tcpClient->send(baData);

        m_mutex.unlock();

        return bRet;
    }

    QByteArray CServer::read(int length, int socketDescriptor)
    {
        CTcpSocket *tcpClient = m_hashClientSocket.value(socketDescriptor);

        return tcpClient->read(length);
    }

    QByteArray CServer::readAll(int socketDescriptor)
    {
        CTcpSocket *tcpClient = m_hashClientSocket.value(socketDescriptor);

        return tcpClient->readAll();
    }

    quint64 CServer::bytesAvailable(int socketDescriptor)
    {
        return m_hashClientSocket.value(socketDescriptor)->bytesAvailable();
    }

    void CServer::clearHeartBeatCount(int socketDescriptor)
    {
        m_pHeartBeatThread->clearHeartBeatCount(socketDescriptor);
    }

    bool CServer::hasPendingConnections() const
    {
        return !m_listSocketID.isEmpty();
    }

    CTcpSocket *CServer::nextPendingConnection()
    {
        int socketDescriptor = m_listSocketID.takeFirst();

        return m_hashClientSocket.take(socketDescriptor);
    }

    void CServer::setLoginCertification(bool bLogin, int msecs)
    {
        m_bLoginCert = bLogin;
        m_lOverTime = msecs;
    }

    void CServer::acceptConnection(int socketDescriptor)
    {
        CTcpSocket *tcpClient = m_hashClientSocket.value(socketDescriptor);
        acceptConnection(tcpClient);
    }

    void CServer::rejectConnection(int socketDescriptor)
    {
        stDisConnected(socketDescriptor);
    }

    bool CServer::waitForReadyRead(int socketDescriptor, int msecs)
    {
        return m_hashClientSocket.value(socketDescriptor)->waitForReadyRead(msecs);
    }

    void CServer::incomingConnection(int handle)
    {
        CTcpSocket *tcpClient = new CTcpSocket;
        tcpClient->setSocketDescriptor(handle);

        addPendingConnection(tcpClient);

        if( m_bLoginCert )
        {
            if( tcpClient->waitForReadyRead(m_lOverTime) )
            {
                QByteArray baLogin = tcpClient->readAll();
                emit sgLoginCertInfo(tcpClient->socketDescriptor(), baLogin);
            }
            else
            {
                stDisConnected(tcpClient->socketDescriptor());
            }
        }
        else
        {
            acceptConnection(tcpClient);
        }
    }

    void CServer::addPendingConnection(CTcpSocket *tcpClient)
    {
        m_hashClientSocket.insert(tcpClient->socketDescriptor(), tcpClient);
        m_listSocketID.append(tcpClient->socketDescriptor());
        m_pHeartBeatThread->pendingTcpSocket(tcpClient);
    }

    void CServer::stHeartBreak(int socketDescriptor)
    {
        // 当心跳包接受异常，说明客户端处于不正常状态，断连并释放资源
        CTcpSocket *tcpClient = m_hashClientSocket.value(socketDescriptor);

        m_pHeartBeatThread->removeTcpSocket(socketDescriptor);

        if( NULL != tcpClient )
        {
            delete tcpClient;
            tcpClient = NULL;
        }
        m_hashClientSocket.remove(socketDescriptor);
    }

    void CServer::stDisConnected(int socketDescriptor)
    {
        stHeartBreak(socketDescriptor);

        emit sgDisConnected(socketDescriptor);
    }

    void CServer::acceptConnection(CTcpSocket *tcpClient)
    {
        connect(tcpClient, SIGNAL(sgReadyRead(int)),  this, SIGNAL(sgReadyRead(int)));
        connect(tcpClient, SIGNAL(sgConnected(int)), this, SIGNAL(sgConnected(int)));
        // 设置为队列形式触发，防止同一时间多个客户端断连
        connect(tcpClient, SIGNAL(sgDisConnected(int)),
                this, SLOT(stDisConnected(int)), Qt::QueuedConnection);
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

