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

    class CClientHeartBeatThread : public QThread
    {
        Q_OBJECT
    public:
        explicit CClientHeartBeatThread(CTcpSocket *pTcpSocket, QObject *parent = 0);
        ~CClientHeartBeatThread();

        /*!
         * \brief stop  停止心跳帧处理线程
         */
        void stop();

        /*!
         * \brief clearHeartBeatCount   清空心跳帧超时计数值
         */
        void clearHeartBeatCount();
        /*!
         * \brief setHeartBeatEnable    设置心跳帧处理线程工作状态
         * \param bIsEnable             [in]        开始/停止
         */
        void setHeartBeatEnable(const bool bIsEnable);

    signals:
        /*!
         * \brief sgSendHeartBeat       需要发送心跳帧
         */
        void sgSendHeartBeat();

        /*!
         * \brief sgDisconnected        心跳帧异常,认为已经断开连接
         */
        void sgDisconnected(int);

    protected:
        void run();

    private:
        CTcpSocket      *m_pTcpSocket;
        int     m_lHeartBeatCount;//心跳计数
        bool    m_bIsHeartBeatEnable;//心跳工作标志
    };
} // namespace nsNetwork

#endif // CCLIENT_H
