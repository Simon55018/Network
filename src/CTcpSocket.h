#ifndef CTCPSOCKET_H
#define CTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QMutex>
#include <QThread>

namespace nsNetwork
{
    class CTcpSocket : public QTcpSocket
    {
        Q_OBJECT
    public:
        CTcpSocket()
        {
            connect(this, SIGNAL(readyRead()), this, SLOT(stRecieve()));
            connect(this, SIGNAL(sgSendData(QByteArray)), this, SLOT(stSend(QByteArray)));
            connect(this, SIGNAL(connected()), this, SLOT(stConnected()));
            connect(this, SIGNAL(disconnected()), this, SLOT(stDisConnected()));

            clearHeartBeatCount();
        }

        ~CTcpSocket()
        {
            close();
        }

        /*!
         * \brief resetTcpSocket    重置socket
         * \return
         */
        bool resetTcpSocket()
        {
            bool bIsValid = this->isValid();

            if( bIsValid )
            {
                return this->reset();
            }

            return false;
        }

        /*!
         * \brief countHeartBeat    心跳帧超时计数
         */
        void countHeartBeat()
        {
            m_lHeartBeatCount++;
        }

        /*!
         * \brief getHeartBeatCount 获取心跳帧超时计数值
         * \return                  心跳帧超时计数值
         */
        int getHeartBeatCount()
        {
            return m_lHeartBeatCount;
        }

        /*!
         * \brief clearHeartBeatCount   清空心跳帧超时计数值
         */
        void clearHeartBeatCount()
        {
            m_lHeartBeatCount = 0;
        }

        /*!
         * \brief send      发送网络数据
         * \param baData    [in]        网络数据信息
         * \return
         */
        virtual bool send(QByteArray baData)
        {
            if( QAbstractSocket::ConnectedState != this->state() )
            {
                qWarning("CTcpSocket::send: Socket %d state is disconnected", m_lSocketDescriptor);
                return false;
            }
            emit sgSendData(baData);

            return true;
        }

        virtual bool setSocketDescriptor(int socketDescriptor, SocketState state = ConnectedState,
                                         OpenMode openMode = ReadWrite)
        {
            m_lSocketDescriptor = socketDescriptor;

            return QAbstractSocket::setSocketDescriptor(socketDescriptor, state, openMode);
        }

    signals:
        /*!
         * \brief sgSendData    数据发送信号
         * \param baData        网络数据信息
         */
        void sgSendData(QByteArray baData);
        /*!
         * \brief sgReadyRead           网络数据读准备信号
         * \param socketDescriptor      socket描述符
         */
        void sgReadyRead(int socketDescriptor);
        /*!
         * \brief sgHeartBeat           心跳请求发送帧信号
         * \param socketDescriptor      socket描述符
         */
        void sgHeartBeat(int socketDescriptor);
        /*!
         * \brief sgConnected           成功连接信号
         * \param socketDescriptor      socket描述符
         */
        void sgConnected(int socketDescriptor);
        /*!
         * \brief sgDisConnected        断开连接信号
         * \param socketDescriptor      socket描述符
         */
        void sgDisConnected(int socketDescriptor);

    protected slots:
        /*!
         * \brief stRecieve     数据可读准备信号处理,转发网络数据到上层
         * \return              成功/失败
         */
        bool stRecieve()
        {
            if( QAbstractSocket::ConnectedState != this->state() )
            {
                qWarning("CTcpSocket::stRecieve: Socket %d state is disconnected", m_lSocketDescriptor);
                return false;
            }

            sgReadyRead(this->socketDescriptor());

            return true;
        }

        /*!
         * \brief stSend        数据发送信号处理
         * \param baData        [in]        网络数据
         * \return              成功/失败
         */
        bool stSend(QByteArray baData)
        {
            if( QAbstractSocket::UnconnectedState == this->state() )
            {
                return false;
            }

            m_Mutex.lock();
            this->write(baData);
            this->flush();

            m_Mutex.unlock();

            return true;
        }

    private slots:
        /*!
         * \brief stConnected       成功连接信号转发
         */
        void stConnected()
        {
            m_lSocketDescriptor = this->socketDescriptor();
            emit sgConnected(m_lSocketDescriptor);
        }

        /*!
         * \brief stDisConnected    断开连接信号转发
         */
        void stDisConnected()
        {
            // 由于QAbstractSocket在发送disconnect之前已经把this->socketDescriptor()对应的变量设为-1
            // 所以用类成员变量发送断连的socket描述符
            // emit sgDisConnected(this->socketDescriptor());
            emit sgDisConnected(m_lSocketDescriptor);
        }

    private:
        QMutex      m_Mutex;
        int         m_lHeartBeatCount;//心跳计数
        int         m_lSocketDescriptor;
    };
} // namespace nsNetwork


#endif // INETWORK_H
