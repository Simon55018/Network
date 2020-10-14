#include "CServer.h"

/*! @todo 因为CNetwok.h, CClient.cpp, CServer.cpp   */
/*!       均有以下定义, 应该新建一个文件进行存储"         */
#define STRING_LOGIN_SUCCESS        "LOGIN_SUCCESS"
#define STRING_LOGIN_FAILURE        "LOGIN_FAILURE"

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
        connect(m_pHeartBeatThread, SIGNAL(sgHeartBreak(int)), this, SLOT(stDisConnected(int)));
        m_pHeartBeatThread->start();
    }

    CServer::~CServer()
    {
        close();
        if( NULL != m_pHeartBeatThread )
        {
            delete m_pHeartBeatThread;
            m_pHeartBeatThread = NULL;
        }
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

    QTcpSocket* CServer::getTcpSocket(int socketDescriptor)
    {
        return m_hashClientSocket.value(socketDescriptor);
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
        emit sgConnected(handle);

        if( m_bLoginCert )  //登录验证
        {
            if( tcpClient->waitForReadyRead(m_lOverTime) )  // 等待发送登录信息
            {
                // 先读登录信息数据长度
                int length = *(tcpClient->read(1).data());
                // 获取登录系信息
                QByteArray baLogin = tcpClient->read(length);
                emit sgLoginCertInfo(handle, baLogin);
            }
            else
            {
                this->send(QByteArray(STRING_LOGIN_FAILURE), handle);
                stDisConnected(handle);
            }
        }
        else
        {
            acceptConnection(tcpClient);
            this->send(QByteArray(STRING_LOGIN_SUCCESS), handle);
        }
    }

    void CServer::addPendingConnection(CTcpSocket *tcpClient)
    {
        m_hashClientSocket.insert(tcpClient->socketDescriptor(), tcpClient);
        m_listSocketID.append(tcpClient->socketDescriptor());
        m_pHeartBeatThread->pendingTcpSocket(tcpClient);
    }

    void CServer::stDisConnected(int socketDescriptor)
    {
        emit sgDisConnected(socketDescriptor);

        // 当心跳包接受异常，说明客户端处于不正常状态，断连并释放资源
        CTcpSocket *tcpClient = m_hashClientSocket.value(socketDescriptor);

        m_pHeartBeatThread->removeTcpSocket(socketDescriptor);

        disconnect(tcpClient, SIGNAL(sgReadyRead(int)), this, SIGNAL(sgReadyRead(int)));
        // 设置为队列形式触发，防止同一时间多个客户端断连
        disconnect(tcpClient, SIGNAL(sgDisConnected(int)), this, SLOT(stDisConnected(int)));

        // deleteLater防止该对象为完成操作后直接退出
        tcpClient->deleteLater();
        m_hashClientSocket.remove(socketDescriptor);
    }

    void CServer::acceptConnection(CTcpSocket *tcpClient)
    {
        connect(tcpClient, SIGNAL(sgReadyRead(int)), this, SIGNAL(sgReadyRead(int)));
        connect(tcpClient, SIGNAL(sgDisConnected(int)), this, SLOT(stDisConnected(int)));
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

