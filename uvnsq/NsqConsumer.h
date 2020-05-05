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

    void appendNsqd(uv::SocketAddr& addr);

    void start();
    void sub(NsqClientPtr client, std::string topic, std::string channel);
    void rdy(NsqClientPtr client, int count);
    void fin(NsqClientPtr client, std::string& id);

    void setRdy(int count);

    void setOnNsqMessage(OnNsqMessage callback);
    void setOnNsqResp(OnNsqResp callback);
    void setOnNsqError(OnNsqError callback);
    void onConnect(NsqClientPtr client,uv::TcpClient::ConnectStatus status);
    
private:
    uv::EventLoop* loop_;
    std::vector<NsqClientPtr> clients_;
    std::string topic_;
    std::string channel_;
    int rdy_;
    OnNsqMessage onNsqMessage_;
    OnNsqResp onNsqResp_;
    OnNsqError onNsqError_;
};

}
#endif