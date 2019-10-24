/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-12

   Description: uv-nsq
*/

#include "NsqMessage.h"
#include "DataFormat.h"

using namespace nsq;

NsqMessage::NsqMessage()
{
}


NsqMessage::~NsqMessage()
{
}

int NsqMessage::decode(std::string& message)
{
    auto size = message.size();
    if (size < MinSize)
    {
        return -1;
    }
    auto ptr = message.c_str();
    ptr += DataFormat::UnpackNum(ptr, timestamp_);
    ptr += DataFormat::UnpackNum(ptr, attempts_);
    std::string id(ptr, 16);
    msgID_.swap(id);
    ptr += 16;
    
    std::string body(ptr, size - MinSize);
    body_.swap(body);
    return 0;
}

uint64_t NsqMessage::Timestamp()
{
    return timestamp_;
}

uint16_t NsqMessage::Attempts()
{
    return attempts_;
}

std::string& NsqMessage::MsgID()
{
    return  msgID_;
}

std::string& NsqMessage::MsgBody()
{
    return body_;
}
