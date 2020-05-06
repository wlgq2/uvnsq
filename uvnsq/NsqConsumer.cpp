/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-11

   Description: uv-nsq
*/

#include "NsqConsumer.h"

using namespace nsq;

NsqConsumer::NsqConsumer(uv::EventLoop* loop, std::string topic, std::string channel)
    :loop_(loop),
    isRun_(false),
    topic_(topic),
    channel_(channel),
    rdy_(1)
{
    
}

NsqConsumer::~NsqConsumer()
{
    isRun_ = false;
    clients_.clear();
}

void nsq::NsqConsumer::appendNsqd(uv::SocketAddr& addr)
{
    auto client = std::make_shared<NsqClient>(loop_, addr);
    client->setOnNsqConnect(std::bind(&NsqConsumer::onConnect, this, client.get(),std::placeholders::_1));
    clients_.push_back(client);
}

void nsq::NsqConsumer::start()
{
    //re connent while disconnect...
    for (auto client : clients_)
    {
        auto ptr = client.get();
        client->setOnNsqMessage([this, ptr](NsqMessage& message)
        {
            if (nullptr != onNsqMessage_)
            {
                onNsqMessage_(message);
            }
            fin(ptr,message.MsgID());
        });
        client->setOnNsqResp(onNsqResp_);
        client->setOnNsqError(onNsqError_);
        client->connectToNsq();
    }
    isRun_ = true;
}

void  NsqConsumer::sub(NsqClient* client, std::string topic, std::string channel)
{
    CommandSUB msg;
    msg.topic = topic;
    msg.channel = channel;
    client->sendProtocol(msg);
}

void  NsqConsumer::rdy(NsqClient* client, int count)
{
    CommandRDY msg;
    msg.count = count;
    client->sendProtocol(msg);
}

void nsq::NsqConsumer::fin(NsqClient* client, std::string& id)
{
    CommandFIN command;
    command.msgid = id;
    client->sendProtocol(command);
}

void NsqConsumer::setRdy(int count)
{
    rdy_ = count;
}

void nsq::NsqConsumer::setOnNsqMessage(OnNsqMessage callback)
{
    onNsqMessage_ = callback;
}

void nsq::NsqConsumer::setOnNsqResp(OnNsqResp callback)
{
    onNsqResp_ = callback;
}

void nsq::NsqConsumer::setOnNsqError(OnNsqError callback)
{
    onNsqError_ = callback;
}

void nsq::NsqConsumer::onConnect(NsqClient* client,uv::TcpClient::ConnectStatus status)
{
    if (status == uv::TcpClient::OnConnectSuccess)
    {
        sub(client,topic_, channel_);
        rdy(client,rdy_);
    }
}

