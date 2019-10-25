/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-10

   Description: uv-nsq
*/

#ifndef   NSQ_CLIENT_H
#define   NSQ_CLIENT_H

#include  <uv/uv11.h>
#include  "NsqMessage.h"
#include  "DataFormat.h"
#include  "NsqProtocol.h"

namespace nsq 
{
using OnNsqMessage = std::function<void(NsqMessage&)>;
using OnNsqResp = std::function<void(std::string&)>;
using OnNsqError = std::function<void(std::string&)>;

class UvConfig
{
public:
    static UvConfig& RunOnce()
    {
        static UvConfig single;
        return single;
    }
private:
    UvConfig()
    {
        uv::GlobalConfig::BufferModeStatus = uv::GlobalConfig::CycleBuffer;
        uv::GlobalConfig::CycleBufferSize = 1024 * 1024;
    }
};
class NsqClient  : public uv::TcpClient
{
public:
    NsqClient(uv::EventLoop* loop);
    virtual ~NsqClient();
    
    void connectToNsq(uv::SocketAddr& addr);
    void onMessage(const char* data, ssize_t size);

    void setOnNsqMessage(OnNsqMessage callback);
    void setOnNsqResp(OnNsqResp callback);
    void setOnNsqError(OnNsqError callback);
    void setOnNsqConnect(ConnectStatusCallback callback);

    template<typename Protocol>
    void sendProtocol(Protocol& msg);

    template<typename Protocol>
    void sendProtocolDefault();

private:
    void onConnectStatus(uv::TcpClient::ConnectStatus status);

    bool ifOnHeartbeat(std::string& body);

    std::shared_ptr<uv::SocketAddr> addr_;
    OnNsqMessage onMessage_;
    OnNsqResp onResp_;
    OnNsqError onError_;

    ConnectStatusCallback nextCallback_;
};

template<typename Protocol>
inline void NsqClient::sendProtocol(Protocol& msg)
{
    std::string data;
    msg.pack(data);
    write(data.c_str(), (unsigned)(data.size()));
}

template<typename Protocol>
inline void NsqClient::sendProtocolDefault()
{
    Protocol msg;
    sendProtocol(msg);
}

}
#endif
