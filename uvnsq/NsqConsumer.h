/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-11

   Description: uv-nsq
*/

#ifndef   NSQ_CONSUMER_H
#define   NSQ_CONSUMER_H

#include  "NsqClient.h"
#include  <string>
#include  <vector>

namespace nsq
{ 

class NsqConsumer
{
public:
    NsqConsumer(uv::EventLoop* loop, std::string topic, std::string channel);
    virtual ~NsqConsumer();

    void setNsqd(uv::SocketAddr& addr);

    void start();
    void sub(std::string topic, std::string channel);
    void rdy(int count);
    void fin( std::string& id);

    void setRdy(int count);

    void setOnNsqMessage(OnNsqMessage callback);
    void setOnNsqResp(OnNsqResp callback);
    void setOnNsqError(OnNsqError callback);
    void onConnect(NsqClient* client,uv::TcpClient::ConnectStatus status);
    
private:
    uv::EventLoop* loop_;
    bool isRun_;
    NsqClientPtr client_;
    std::string topic_;
    std::string channel_;
    int rdy_;
    OnNsqMessage onNsqMessage_;
    OnNsqResp onNsqResp_;
    OnNsqError onNsqError_;
};

using NsqConsumerPtr = std::shared_ptr<NsqConsumer>;
}
#endif