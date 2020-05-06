/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-10

   Description: uv-nsq
*/

#include <string>

#include "NsqClient.h"

using namespace uv;
using namespace std;
using namespace nsq;

NsqClient::NsqClient(EventLoop* loop, uv::SocketAddr& addr)
    :addr_(std::make_shared<uv::SocketAddr>(addr)),
    isRun_(false),
    client_(nullptr),
    onMessage_(nullptr),
    onResp_(nullptr),
    onError_(nullptr)
{
    UvConfig::RunOnce();
    client_ = new uv::TcpClient(loop);
    client_->setConnectStatusCallback(std::bind(&NsqClient::onConnectStatus, this, std::placeholders::_1));
    client_->setMessageCallback(std::bind(&NsqClient::onMessage,this ,placeholders::_1, placeholders::_2));
}

NsqClient::~NsqClient()
{
    isRun_ = false;
    auto client = client_;
    client->close([this, client](std::string&)
    {
        delete client;
    });
}

void nsq::NsqClient::connectToNsq()
{
    client_->connect(*addr_);
}

void NsqClient::onMessage(const char* data, ssize_t size)
{
    std::string logInfo("receive message ");
    logInfo += std::to_string(size);
    logInfo += " :";
    logInfo.append(data, size);
    uv::LogWriter::Instance()->debug(logInfo);
    
    logInfo = "hex :";
    uv::LogWriter::ToHex(logInfo, data, (unsigned)size);
    uv::LogWriter::Instance()->debug(logInfo);
    
    auto packbuf = client_->getCurrentBuf();
    if (packbuf != nullptr)
    {
        if (packbuf->append(data, (int)size) < 0)
        {
            std::string info("buffer not enough :");
            info += std::to_string(size);
            uv::LogWriter::Instance()->error(info);
            return;
        }
        DataFormat message;
        while (0 == packbuf->readGeneric(&message))
        {
            switch (message.FrameType())
            {
            case DataFormat::FrameTypeResponse:
                if (ifOnHeartbeat(message.MessageBody()))
                {
                    break;
                }
                if (onResp_)
                {
                    onResp_(message.MessageBody());
                }
                break;
            case DataFormat::FrameTypeError:
                if (onError_)
                {
                    onError_(message.MessageBody());
                }
                break;
            case DataFormat::FrameTypeMessage:
                if (onMessage_)
                {
                    NsqMessage nMsg;
                    auto rst = nMsg.decode(message.MessageBody());
                    if (rst == 0)
                    {
                        onMessage_(nMsg);
                    }
                    else
                    {
                        string str("decode message error :");
                        uv::LogWriter::ToHex(str, message.MessageBody());
                        uv::LogWriter::Instance()->error(str);
                    }
                }
                break;
            default:
                string str("undefined message type :");
                uv::LogWriter::ToHex(str, data, (unsigned)size);
                uv::LogWriter::Instance()->error(str);
                break;
            }
        }
    }

}

void NsqClient::setOnNsqMessage(OnNsqMessage callback)
{
    onMessage_ = callback;
}

void NsqClient::setOnNsqResp(OnNsqResp callback)
{
    onResp_ = callback;
}

void nsq::NsqClient::setOnNsqError(OnNsqError callback)
{
    onError_ = callback;
}

void NsqClient::setOnNsqConnect(uv::TcpClient::ConnectStatusCallback callback)
{
    nextCallback_ = callback;
}

void NsqClient::onConnectStatus(TcpClient::ConnectStatus status)
{
    if (status != TcpClient::ConnectStatus::OnConnectSuccess)
    {
        uv::LogWriter::Instance()->warn("disconnect from sever and retry...");
        Timer* ptr = new Timer(client_->Loop(), 1500, 0, [this](Timer* ptr)
        {
            client_->connect(*addr_);
            ptr->close([this](Timer* ptr)
            {
                delete ptr;
            });
        });
        ptr->start();
    }
    else
    {
        sendProtocolDefault<CommandVer>();
    }
    if (nextCallback_)
        nextCallback_(status);
}

bool nsq::NsqClient::ifOnHeartbeat(std::string& body)
{
    if (body == ProtocolRespHeartbeat)
    {
        sendProtocolDefault<CommandNOP>();
        return true;
    }
    return false;
}


