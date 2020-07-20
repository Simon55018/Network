#ifndef CSERVER_H
#define CSERVER_H

#include <QObject>
#include <QTcpServer>
#include "CTcpSocket.h"

namespace nsNetwork
{
    class CServerHeartBeatThread;
    class CServer : public QTcpServer
    {
        Q_OBJECT
    public:
        /*!
         * \brief CServer   CServer构造函数
         * \param lPort     端口号
         */
        explicit CServer(int lPort);
        ~CServer();

        /*!
         * \brief start     开启服务端
         * \return          成功/失败
         */
        bool start();

        /*!
         * \brief close     关闭服务端
         */
        void close();

        /*!
         * \brief send              网络数据发送
         * \param baData            [in]        网络数据
         * \param socketDiescriptor [in]        socket描述符
         * \return                  成功/失败
         */
        bool send(QByteArray baData, int socketDiescriptor);

        /*!
         * \brief read              按长度读取网络数据
         * \param length            [in]        数据长度
         * \param socketDescriptor  [in]        socket描述符
         * \return                  网络数据信息
         */
        QByteArray read(int length, int socketDescriptor);

        /*!
         * \brief readAll           读取缓存区所有网络数据
         * \param socketDescriptor  [in]        socket描述符
         * \return                  网络数据信息
         */
        QByteArray readAll(int socketDescriptor);

        /*!
         * \brief bytesAvailable    可读位数获取
         * \param socketDiescriptor [in]            socket描述符
         * \return                  网络数据可读长度
         */
        quint64 bytesAvailable(int socketDescriptor);

        /*!
         * \brief clearHeartBeatCount   清空心跳帧超时计数值
         * \param socketDiescriptor     [in]            socket描述符
         */
        void clearHeartBeatCount(int socketDescriptor);

        /*!
         * \brief hasPendingConnections 判断是否有挂起的连接
         * \return 是/否
         */
        virtual bool hasPendingConnections() const;
        /*!
         * \brief nextPendingConnection 根据socket描述符获取CTcpSocket对象
         * \param socketDescriptor      [in]            socket描述符
         * \return CTcpSocket对象指针
         */
        virtual CTcpSocket *nextPendingConnection(int socketDescriptor);

    protected:
        /*!
         * \brief incomingConnection    处理新网络连接信号
         * \param handle                [in]            socket描述符
         */
        virtual void incomingConnection(int handle);

        /*!
         * \brief addPendingConnection  增加挂起的连接
         * \param socket                [in]            CTcpSocket指针
         */
        void addPendingConnection(CTcpSocket* tcpClient);

    protected slots:
        /*!
         * \brief stHeartBreak          处理心跳帧终止信号
         * \param socketDiescriptor     socket描述符
         */
        void stHeartBreak(int socketDiescriptor);
        /*!
         * \brief stDisConnected        处理断开连接信号
         * \param socketDiescriptor     socket描述符
         */
        void stDisConnected(int socketDiescriptor);

    signals:
        /*!
         * \brief sgConnected           成功连接信号
         * \param socketDiescriptor     socket描述符
         */
        void sgConnected(int socketDiescriptor);

        /*!
         * \brief sgDisConnected        连接中断信号
         * \param socketDiescriptor     socket描述符
         */
        void sgDisConnected(int socketDiescriptor);
        /*!
         * \brief sgReadyRead           网络数据读准备信号
         * \param socketDiescriptor     socket描述符
         */
        void sgReadyRead(int socketDiescriptor);

    private:
        QMutex      m_mutex;
        int         m_lPort;
        QMap<int, CTcpSocket*>              m_mapClientSocket;
        QMap<int, CServerHeartBeatThread*>  m_mapHeartBeatThread;
    };

    class CServerHeartBeatThread : public IHeartBeatThread
    {
        Q_OBJECT
    public:
        explicit CServerHeartBeatThread(CTcpSocket* pTcpClient);
        ~CServerHeartBeatThread();

    signals:
        void sgHeartBreak(int);

    protected:
        void run();
    };
} // namespace nsNetwork

#endif // CSERVER_H
