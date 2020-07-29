#include "CClient.h"

#include <QDateTime>

#define STRING_LOGIN_SUCCESS        "LOGIN_SUCCESS"
#define STRING_LOGIN_FAILURE        "LOGIN_FAILURE"

namespace nsNetwork
{
    CClient::CClient(const QString &sAddr, const int lPort)
        : m_sIPAddr(sAddr), m_lPort(lPort)
    {
        m_pHeartThread = new CClientHeartBeatThread(this);
        connect(m_pHeartThread, SIGNAL(sgSendHeartBeat()), this, SIGNAL(sgSendHeartBeat()));
        connect(m_pHeartThread, SIGNAL(sgDisconnected(int)), this, SIGNAL(sgDisConnected(int)));
    }

    CClient::~CClient()
    {
        //! 先释放心跳帧线程资源,防止CTcpSocket先释放导致段错误
        if( NULL != m_pHeartThread )
        {
            m_pHeartThread->stop();

            delete m_pHeartThread;
            m_pHeartThread = NULL;
        }

        CTcpSocket::close();
    }

    bool CClient::start(int msecs)
    {
        if( m_pHeartThread != NULL &&
                QAbstractSocket::ConnectedState != this->state() )
        {
            this->connectToHost(m_sIPAddr, m_lPort);

            // 等待时间3秒
            if( this->waitForConnected(msecs) )
            {
                if( m_bLoginCert )
                {
                    this->send(m_baLoginCertification);
                    // 等待验证时间
                    if( this->waitForReadyRead(m_lLoginOverTime) )
                    {
                        // 验证失败
                        if( this->readAll().contains(STRING_LOGIN_FAILURE) )
                        {
                            return false;
                        }
                    }
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        if( !m_pHeartThread->isRunning() )
        {
            m_pHeartThread->start();
        }

        return true;
    }

    void CClient::close()
    {
        if ( NULL != m_pHeartThread )
        {
            //终止检测线程中的计数动作
            m_pHeartThread->stop();
        }

        CTcpSocket::close();
    }

    void CClient::clearHeartBeatCount()
    {
        m_pHeartThread->clearHeartBeatCount();
    }

    void CClient::setLoginCertification(bool bLogin, int msecs, QByteArray baLoginCert)
    {
        m_bLoginCert = bLogin;
        m_lLoginOverTime = msecs;
        m_baLoginCertification = baLoginCert;
    }

    CClientHeartBeatThread::CClientHeartBeatThread(CTcpSocket *pTcpSocket, QObject *parent)
        : QThread(parent), m_pTcpSocket(pTcpSocket)
    {
        clearHeartBeatCount();
        setHeartBeatEnable(true);
    }

    CClientHeartBeatThread::~CClientHeartBeatThread()
    {
        stop();
    }

    void CClientHeartBeatThread::stop()
    {
        this->setHeartBeatEnable(false);
        if( this->isRunning() )
        {
            this->quit();
            this->wait();
        }
    }

    void CClientHeartBeatThread::clearHeartBeatCount()
    {
        m_pTcpSocket->clearHeartBeatCount();
    }

    void CClientHeartBeatThread::setHeartBeatEnable(const bool bIsEnable)
    {
        m_bIsHeartBeatEnable = bIsEnable;
    }

    void CClientHeartBeatThread::run()
    {
        //-------------------------------------------//
        //------------每隔 30s 发送一帧心跳帧-----------//
        //------------------------------------------//
        while ( m_bIsHeartBeatEnable )
        {
            m_pTcpSocket->countHeartBeat();
            if( 4 == m_pTcpSocket->getHeartBeatCount() )
            {
                // 重新连接
                if( m_pTcpSocket->resetTcpSocket() )
                {
                    this->clearHeartBeatCount();
                    continue;
                }
                break;
            }
            //-------------------------------------------//
            //-----注意：由于 QTcpSocket 不能跨线程操作------//
            //------故此处采用发送信号方式进行触发操作--------//
            emit sgSendHeartBeat();
            sleep(30);
        }
        emit sgDisconnected(m_pTcpSocket->socketDescriptor());
    }

}

