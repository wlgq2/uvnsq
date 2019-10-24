/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-11

   Description: uv-nsq
*/

#ifndef   NSQ_PRODUCER_H
#define   NSQ_PRODUCER_H

#include  "NsqClient.h"

namespace nsq
{ 

class NsqProducer
{
public:
    NsqProducer(uv::EventLoop* loop,uv::SocketAddr& addr);
    virtual ~NsqProducer();

    void pub(std::string& topic, std::string& body);
    void pub(std::string&& topic, std::string&& body);
    void onMessage(NsqMessage& message);

private:
    NsqClient client_;
};

}
#endif