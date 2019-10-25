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

NsqClient::NsqClient(EventLoop* loop)
    :TcpClient(loop),
    onMessage_(nullptr),
    onResp_(nullptr),
    onError_(nullptr)
{
    UvConfig::RunOnce();
    setConnectStatusCallback(std::bind(&NsqClient::onConnectStatus, this, std::placeholders::_1));
    setMessageCallback(std::bind(&NsqClient::onMessage,this ,placeholders::_1, placeholders::_2));
}

NsqClient::~NsqClient()
{

}

void nsq::NsqClient::connectToNsq(uv::SocketAddr& addr)
{
    addr_ = std::make_shared<uv::SocketAddr>(addr);
    connect(*addr_);
}

void NsqClient::onMessage(const char* data, ssize_t size)
{
    std::string logInfo("receive message :");
    logInfo.append(data, size);
    logInfo.append(" hex :");
    uv::LogWriter::ToHex(logInfo, data, (unsigned)size);
    uv::LogWriter::Instance()->debug(logInfo);
    
    auto packbuf = getCurrentBuf();
    if (packbuf != nullptr)
    {
        packbuf->append(data, (int)size);
        std::string nsqData;
        DataFormat message;
        uv::GlobalConfig::ReadBufCallback = std::bind(&DataFormat::decodePacketBuf, &message, placeholders::_1, placeholders::_2);
        while (0 == packbuf->readPacket(nsqData))
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

void NsqClient::setOnNsqConnect(ConnectStatusCallback callback)
{
    nextCallback_ = callback;
}

void NsqClient::onConnectStatus(TcpClient::ConnectStatus status)
{
    if (status != TcpClient::ConnectStatus::OnConnectSuccess)
    {
        uv::LogWriter::Instance()->warn("disconnect from sever and retry...");
        Timer* ptr = new Timer(Loop(), 1500, 0, [this](Timer* ptr)
        {
            connect(*addr_);
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


