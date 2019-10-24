/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-11

   Description: uv-nsq
*/

#include "NsqConsumer.h"

using namespace nsq;

NsqConsumer::NsqConsumer(uv::EventLoop* loop)
    :client_(loop),
    rdy_(1)
{
    client_.setOnNsqConnect(std::bind(&NsqConsumer::onConnect,this,std::placeholders::_1));
}

NsqConsumer::~NsqConsumer()
{

}

void nsq::NsqConsumer::start(uv::SocketAddr& addr)
{
    //re connent while disconnect...
    client_.connectToNsq(addr);
}

void  NsqConsumer::sub(std::string topic, std::string channel)
{
    CommandSUB msg;
    msg.topic = topic;
    msg.channel = channel;
    client_.sendProtocol(msg);
}

void  NsqConsumer::rdy(int count)
{
    CommandRDY msg;
    msg.count = count;
    client_.sendProtocol(msg);
}

void nsq::NsqConsumer::fin(std::string& id)
{
    CommandFIN command;
    command.msgid = id;
    client_.sendProtocol(command);
}

void NsqConsumer::appendSub(std::string topic, std::string channel)
{
    subs_.push_back({ topic,channel });
}

void NsqConsumer::setRdy(int count)
{
    rdy_ = count;
}

void nsq::NsqConsumer::setOnNsqMessage(OnNsqMessage callback)
{
    client_.setOnNsqMessage(callback);
}

void nsq::NsqConsumer::setOnNsqResp(OnNsqResp callback)
{
    client_.setOnNsqResp(callback);
}

void nsq::NsqConsumer::setOnNsqError(OnNsqError callback)
{
    client_.setOnNsqError(callback);
}

void nsq::NsqConsumer::onConnect(uv::TcpClient::ConnectStatus status)
{
    if (status == uv::TcpClient::OnConnectSuccess)
    {
        for (auto& subinfo : subs_)
        {
            sub(subinfo.topic, subinfo.channel);
        }
        rdy(rdy_);
    }
}

