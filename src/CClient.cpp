#include "CClient.h"

#include <QDateTime>

namespace nsNetwork
{
    CClient::CClient(const QString &sAddr, const int lPort)
        : m_sIPAddr(sAddr), m_lPort(lPort)
    {
        m_pHeartThread = NULL;
    }

    CClient::~CClient()
    {
        CTcpSocket::close();

        m_pHeartThread->stop();

        if( NULL != m_pHeartThread )
        {
            delete m_pHeartThread;
            m_pHeartThread = NULL;
        }
    }

    bool CClient::start()
    {
        if( QAbstractSocket::ConnectedState != this->state() )
        {
            this->connectToHost(m_sIPAddr, m_lPort);

            if( !this->waitForConnected(3000) )
            {
                return false;
            }
        }

        if( m_pHeartThread == NULL )
        {
            m_pHeartThread = new CClientHeartBeatThread(this);
            connect(m_pHeartThread, SIGNAL(sgSendHeartBeat()), this, SIGNAL(sgSendHeartBeat()));
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
            m_pHeartThread->setHeartBeatEnable(false);

            //关闭线程操作
            m_pHeartThread->quit();
            m_pHeartThread->wait();
        }

        CTcpSocket::close();
    }

    void CClient::clearHeartBeatCount()
    {
        m_pHeartThread->clearHeartBeatCount();
    }

    CClientHeartBeatThread::CClientHeartBeatThread(CTcpSocket *pTcpClient)
        : IHeartBeatThread(pTcpClient)
    {

    }

    CClientHeartBeatThread::~CClientHeartBeatThread()
    {

    }

    void CClientHeartBeatThread::run()
    {
        //-------------------------------------------//
        //------------每隔 30s 发送一帧心跳帧-----------//
        //------------------------------------------//
        while ( isHeartBeatWorking() )
        {
            countHeartBeat();
            if( 4 == getHeartBeatCount() )
            {
                // 重新连接
                if( m_pTcpClient->resetTcpSocket() )
                {
                    clearHeartBeatCount();
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
    }

}

