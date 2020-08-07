#include "CNetwork.h"
#include "CNetwork_p.h"

namespace nsNetwork
{
    CNetwork::CNetwork()
        : d_ptr(new CNetworkPrivate)
    {
        d_ptr->q_ptr = this;
    }

    CNetwork::~CNetwork()
    {

    }

    bool CNetwork::runAs(NetType emType, int lPort, QString Str_IP)
    {
        Q_D(CNetwork);

        if( NULL != d->m_client &&
                NULL != d->m_server )
        {
            return false;
        }

        d->m_type = emType;
        if( EM_SERVICE == emType )
        {
            if( NULL == d->m_server )
            {
                d->m_server = new CServer(lPort);
            }

            connect(d->m_server, SIGNAL(sgReadyRead(int)), this, SIGNAL(sgReadyRead(int)));
            connect(d->m_server, SIGNAL(sgConnected(int)), this, SIGNAL(sgConnected(int)));
            connect(d->m_server, SIGNAL(sgDisConnected(int)), this, SIGNAL(sgDisConnected(int)));
            connect(d->m_server, SIGNAL(sgLoginCertInfo(int,QByteArray)),
                    this, SIGNAL(sgLoginCertInfo(int,QByteArray)));
        }
        else if( EM_CLIENT == emType )
        {
            if( NULL == d->m_client )
            {
                d->m_client = new CClient(Str_IP, lPort);
            }

            connect(d->m_client, SIGNAL(sgConnected(int)), this, SIGNAL(sgConnected(int)));
            connect(d->m_client, SIGNAL(sgDisConnected(int)), this, SIGNAL(sgDisConnected(int)));
            connect(d->m_client, SIGNAL(sgReadyRead(int)), this, SIGNAL(sgReadyRead(int)));
            connect(d->m_client, SIGNAL(sgSendHeartBeat()), this, SIGNAL(sgSendHeartBeat()));
        }

        return true;
    }

    bool CNetwork::start(int msecs)
    {
        Q_D(CNetwork);
        if( EM_SERVICE == d->m_type && NULL != d->m_server )
        {
            return d->m_server->start();
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            return d->m_client->start(msecs);
        }

        return false;
    }

    bool CNetwork::close()
    {
        Q_D(CNetwork);
        if( EM_SERVICE == d->m_type && NULL != d->m_server )
        {
            d->m_server->close();
            return true;
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            d->m_client->close();
            return true;
        }

        return false;
    }

    bool CNetwork::isValid()
    {
        Q_D(CNetwork);
        if( EM_SERVICE == d->m_type && NULL != d->m_server )
        {
            return d->m_server->isListening();
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            return d->m_client->isOpen();
        }

        return false;
    }

    bool CNetwork::sendData(QByteArray baData, int socketDescriptor)
    {
        Q_D(CNetwork);

        if( EM_SERVICE == d->m_type
                && NULL != d->m_server && socketDescriptor != 0)
        {
            return d->m_server->send(baData, socketDescriptor);
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            return d->m_client->send(baData);
        }

        return false;
    }

    QByteArray CNetwork::readData(int length, int socketDescriptor)
    {
        Q_D(CNetwork);

        if( EM_SERVICE == d->m_type
                && NULL != d->m_server && socketDescriptor != 0 )
        {
            return d->m_server->read(length, socketDescriptor);
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            return d->m_client->read(length);
        }

        return QByteArray();
    }

    QByteArray CNetwork::readAllData(int socketDescriptor)
    {
        Q_D(CNetwork);

        if( EM_SERVICE == d->m_type
                && NULL != d->m_server && socketDescriptor != 0 )
        {
            return d->m_server->readAll(socketDescriptor);
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            return d->m_client->readAll();
        }

        return QByteArray();
    }

    quint64 CNetwork::bytesAvailable(int socketDescriptor)
    {
        Q_D(CNetwork);

        if( EM_SERVICE == d->m_type
                && NULL != d->m_server && socketDescriptor != 0 )
        {
            return d->m_server->bytesAvailable(socketDescriptor);
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            return d->m_client->bytesAvailable();
        }

        return 0;
    }

    void CNetwork::clearHeartBeatCount(int socketDescriptor)
    {
        Q_D(CNetwork);

        if( EM_SERVICE == d->m_type
                && NULL != d->m_server && socketDescriptor != 0 )
        {
            return d->m_server->clearHeartBeatCount(socketDescriptor);
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            return d->m_client->clearHeartBeatCount();
        }
    }

    bool CNetwork::waitForReadyRead(int msecs, int socketDescriptor)
    {
        Q_D(CNetwork);

        if( EM_SERVICE == d->m_type
                && NULL != d->m_server && socketDescriptor != 0 )
        {
            return d->m_server->waitForReadyRead(msecs, socketDescriptor);
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            return d->m_client->waitForReadyRead(msecs);
        }

        return false;
    }

    void CNetwork::setLoginCertification(bool bLogin, int msecs, QByteArray baLoginCert)
    {
        Q_D(CNetwork);

        if( EM_SERVICE == d->m_type && NULL != d->m_server  )
        {
            d->m_server->setLoginCertification(bLogin, msecs);
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            d->m_client->setLoginCertification(bLogin, msecs, baLoginCert);
        }
    }

    void CNetwork::acceptConnection(int socketDescriptor)
    {
        Q_D(CNetwork);

        if( EM_SERVICE == d->m_type
                && NULL != d->m_server && socketDescriptor != 0 )
        {
            d->m_server->acceptConnection(socketDescriptor);
        }
    }

    void CNetwork::rejectConnection(int socketDescriptor)
    {
        Q_D(CNetwork);

        if( EM_SERVICE == d->m_type
                && NULL != d->m_server && socketDescriptor != 0 )
        {
            d->m_server->rejectConnection(socketDescriptor);
        }
    }

    QTcpSocket *CNetwork::getTcpSocket(int socketDescriptor)
    {
        Q_D(CNetwork);

        if( EM_SERVICE == d->m_type
                && NULL != d->m_server && socketDescriptor != 0 )
        {
            return d->m_server->getTcpSocket(socketDescriptor);
        }
        else if( EM_CLIENT == d->m_type && NULL != d->m_client )
        {
            return (QTcpSocket*)d->m_client;
        }

        return NULL;
    }

    CNetworkPrivate::~CNetworkPrivate()
    {
        if( NULL != m_client )
        {
            delete m_client;
            m_client = NULL;
        }

        if( NULL != m_server )
        {
            delete m_server;
            m_server = NULL;
        }
    }

    CNetworkPrivate::CNetworkPrivate()
    {
        q_ptr = NULL;
        m_client = NULL;
        m_server = NULL;
    }

}
