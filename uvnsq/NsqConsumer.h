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
 
struct TopicAndChannel
{
    std::string topic;
    std::string channel;
};

class NsqConsumer
{
public:
    NsqConsumer(uv::EventLoop* loop);
    virtual ~NsqConsumer();

    void start(uv::SocketAddr& addr);
    void sub(std::string topic, std::string channel);
    void rdy(int count);
    void fin(std::string& id);

    void appendSub(std::string topic, std::string channel);
    void setRdy(int count);

    void setOnNsqMessage(OnNsqMessage callback);
    void setOnNsqResp(OnNsqResp callback);
    void setOnNsqError(OnNsqError callback);
    void onConnect(uv::TcpClient::ConnectStatus status);
    
private:
    NsqClient client_;
    std::vector<TopicAndChannel> subs_;
    int rdy_;
};

}
#endif