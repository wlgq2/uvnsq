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
    client_(nullptr),
    topic_(topic),
    channel_(channel),
    rdy_(1)
{
    
}

NsqConsumer::~NsqConsumer()
{
    isRun_ = false;
    client_ =nullptr;
}

void nsq::NsqConsumer::setNsqd(uv::SocketAddr& addr)
{
    client_ = std::make_shared<NsqClient>(loop_, addr);
    client_->setOnNsqConnect(std::bind(&NsqConsumer::onConnect, this, client_.get(),std::placeholders::_1));
}

void nsq::NsqConsumer::start()
{
    //re connent while disconnect...
    client_->setOnNsqMessage([this](NsqMessage& message)
    {
        if (nullptr != onNsqMessage_)
        {
            onNsqMessage_(message);
        }
        fin(message.MsgID());
    });
    client_->setOnNsqResp(onNsqResp_);
    client_->setOnNsqError(onNsqError_);
    client_->connectToNsq();
    isRun_ = true;
}

void  NsqConsumer::sub(std::string topic, std::string channel)
{
    CommandSUB msg;
    msg.topic = topic;
    msg.channel = channel;
    client_->sendProtocol(msg);
}

void  NsqConsumer::rdy( int count)
{
    CommandRDY msg;
    msg.count = count;
    client_->sendProtocol(msg);
}

void nsq::NsqConsumer::fin(std::string& id)
{
    CommandFIN command;
    command.msgid = id;
    client_->sendProtocol(command);
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
        sub(topic_, channel_);
        rdy(rdy_);
    }
}

