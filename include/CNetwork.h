#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QTcpSocket>

#define STRING_LOGIN_SUCCESS        "LOGIN_SUCCESS"
#define STRING_LOGIN_FAILURE        "LOGIN_FAILURE"

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
        bool start(int msecs = 3000);//启动

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
         * \param socketDescriptor  [in]            socket描述符(作为客户端时缺省)
         * \return                  成功/失败
         */
        bool sendData(QByteArray baData, int socketDescriptor = 0);

        /*!
         * \brief readData          按数据长度获取网络数据
         * \param length            [in]            数据长度
         * \param socketDescriptor  [in]            socket描述符(作为客户端时缺省)
         * \return                  网络数据信息
         */
        QByteArray readData(int length, int socketDescriptor = 0);

        /*!
         * \brief readAllData       获取缓存区所有网络数据
         * \param socketDescriptor  [in]            socket描述符(作为客户端时缺省)
         * \return                  网络数据信息
         */
        QByteArray readAllData(int socketDescriptor = 0);

        /*!
         * \brief bytesAvailable    可读位数获取
         * \param socketDescriptor  [in]            socket描述符(作为客户端时缺省)
         * \return                  网络数据可读长度
         */
        quint64 bytesAvailable(int socketDescriptor = 0);

        /*!
         * \brief clearHeartBeatCount   清空心跳帧超时计数值
         * \param socketDescriptor      [in]            socket描述符(作为客户端时缺省)
         */
        void clearHeartBeatCount(int socketDescriptor = 0);

        /*!
         * \brief waitForReadyRead      等待可读
         * \param msecs                 [in]            毫秒(默认30000毫秒,即30秒)
         * \param socketDescriptor      [in]            socket描述符(作为客户端时缺省)
         * \return
         */
        bool waitForReadyRead(int msecs = 30000, int socketDescriptor = 0);

        /*!
         * \brief setLoginCertification 设置是否需要登录验证(若需要验证,请在创建CNetwork对象后,进行登录验证设置)
         * \param bLogin                [in]            是/否
         * \param msecs                 [in]            验证超时时间
         * \param baLoginCert           [in]            验证信息(用户客户端传入自身验证信息)
         */
        void setLoginCertification(bool bLogin, int msecs = 30000, QByteArray baLoginCert = 0);

        /*!
         * \brief acceptConnection      接受连接(当需要登录验证时,需要在验证完毕执行是否接受连接,作为客户端时无效)
         * \param socketDescriptor      [in]            socket描述符
         */
        void acceptConnection(int socketDescriptor);

        /*!
         * \brief rejectConnection      拒绝连接(当需要登录验证时,需要在验证完毕执行是否拒绝连接,作为客户端时无效)
         * \param socketDescriptor      [in]            socket描述符
         */
        void rejectConnection(int socketDescriptor);

        /*!
         * \brief getSocket             获取socket对象
         * \param socketDescriptor      [in]            socket描述符(作为客户端时缺省)
         * \return socket对象
         */
        QTcpSocket* getTcpSocket(int socketDescriptor = 0);

    signals:
        /*!
         * \brief sgConnected           成功连接信号
         * \param socketDescriptor     socket描述符
         */
        void sgConnected(int socketDescriptor);

        /*!
         * \brief sgDisConnected        连接中断信号
         * \param socketDescriptor     socket描述符
         */
        void sgDisConnected(int socketDescriptor);
        /*!
         * \brief sgReadyRead           网络数据读准备信号
         * \param socketDescriptor     socket描述符
         */
        void sgReadyRead(int socketDescriptor);

        // For Client
        /*!
         * \brief sgSendHeartBeat       客户端需要发送心跳帧(作为客户端必须实现)
         */
        void sgSendHeartBeat();

        // For Server
        /*!
         * \brief sgLoginCertInfo       登录验证信息信号
         * \param socketDescriptor      socket描述符
         * \param baLoginCertInfo       验证信息
         */
        void sgLoginCertInfo(int socketDescriptor, QByteArray baLoginCertInfo);

    private:
        QScopedPointer<CNetworkPrivate>  d_ptr;
    };
}

#endif // NETWORK_H
