#ifndef CCLIENT_H
#define CCLIENT_H

#include <QObject>
#include <QThread>
#include "CTcpSocket.h"

namespace nsNetwork
{
    class CClientHeartBeatThread;
    class CClient : public CTcpSocket
    {
        Q_OBJECT
    public:
        /*!
         * \brief CClient       Client构造函数
         * \param sAddr         IP地址
         * \param lPort         端口号
         */
        CClient(const QString &sAddr, const int lPort);
        ~CClient();

        /*!
         * \brief start     网络服务开启
         * \return          成功/失败
         */
        bool start();

        /*!
         * \brief close     关闭网络服务
         */
        void close();

        /*!
         * \brief clearHeartBeatCount   清空心跳帧超时计数
         */
        void clearHeartBeatCount();

    signals:
        /*!
         * \brief sgSendHeartBeat       需要发送心跳帧
         */
        void sgSendHeartBeat();

    private:
        QString                 m_sIPAddr;
        int                     m_lPort;
        CClientHeartBeatThread  *m_pHeartThread;
    };

    class CClientHeartBeatThread : public IHeartBeatThread
    {
        Q_OBJECT
    public:
        explicit CClientHeartBeatThread(CTcpSocket *pTcpClient);
        ~CClientHeartBeatThread();

    signals:
        /*!
         * \brief sgSendHeartBeat       需要发送心跳帧
         */
        void sgSendHeartBeat();

    protected:
        void run();
    };
} // namespace nsNetwork

#endif // CCLIENT_H
