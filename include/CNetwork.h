#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QString>
#include <QSharedPointer>

namespace nsNetwork
{
    enum NetType
    {
        EM_SERVICE = 0,
        EM_CLIENT,
    };

    class CNetworkPrivate;
    class CNetwork : public QObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(CNetwork)
        Q_DECLARE_PRIVATE(CNetwork)

    public:
        explicit CNetwork();

        virtual ~CNetwork();

        /*!
         * \brief runAs     设置网络运行状态
         * \param emType    [in]        网络类型(服务器 or 客户端)
         * \param lPort     [in]        端口号
         * \param Str_IP    [in]        IP地址(作为服务器时缺省输入)
         * \return          成功/失败
         */
        bool runAs(NetType emType, int lPort, QString Str_IP = "");//作为服务器/客户端运行

        /*!
         * \brief start     启动网络服务
         * \return          成功/失败
         */
        bool start();//启动

        /*!
         * \brief close     关闭网络服务
         * \return          成功/失败
         */
        bool close();//关闭

        /*!
         * \brief isValid   网络服务是否有效
         * \return          是/否
         */
        bool isValid();//是否有效

        /*!
         * \brief sendData          网络信息发送
         * \param baData            [in]            网络数据信息
         * \param socketDiescriptor [in]            socket描述符(作为客户端时缺省)
         * \return                  成功/失败
         */
        bool sendData(QByteArray baData, int socketDiescriptor = 0);

        /*!
         * \brief readData          按数据长度获取网络数据
         * \param length            [in]            数据长度
         * \param socketDiescriptor [in]            socket描述符(作为客户端时缺省)
         * \return                  网络数据信息
         */
        QByteArray readData(int length, int socketDiescriptor = 0);

        /*!
         * \brief readAllData       获取缓存区所有网络数据
         * \param socketDiescriptor [in]            socket描述符(作为客户端时缺省)
         * \return                  网络数据信息
         */
        QByteArray readAllData(int socketDiescriptor = 0);

        /*!
         * \brief bytesAvailable    可读位数获取
         * \param socketDiescriptor [in]            socket描述符(作为客户端时缺省)
         * \return                  网络数据可读长度
         */
        quint64 bytesAvailable(int socketDiescriptor = 0);

        /*!
         * \brief clearHeartBeatCount   清空心跳帧超时计数值
         * \param socketDiescriptor     [in]            socket描述符(作为客户端时缺省)
         */
        void clearHeartBeatCount(int socketDiescriptor = 0);

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

        // For Client
        /*!
         * \brief sgSendHeartBeat       客户端需要发送心跳帧(作为客户端必须实现)
         */
        void sgSendHeartBeat();

    private:
        QSharedPointer<CNetworkPrivate>  d_ptr;
    };
}

#endif // NETWORK_H
