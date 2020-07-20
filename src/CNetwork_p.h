#ifndef CNETWORK_P_H
#define CNETWORK_P_H

#include "CNetwork.h"
#include "CServer.h"
#include "CClient.h"

namespace nsNetwork
{
    class CNetworkPrivate : public QObject
    {
        Q_OBJECT
        Q_DECLARE_PUBLIC(CNetwork)

    public:
        CNetworkPrivate();

        virtual ~CNetworkPrivate();

    public:
        CClient *m_client;
        CServer *m_server;
        NetType m_type;

    private:
        CNetwork *q_ptr;
    };
}

#endif // CNETWORK_P_H
