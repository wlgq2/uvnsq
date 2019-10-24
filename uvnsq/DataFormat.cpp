/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-10

   Description: uv-nsq
*/

#include "DataFormat.h"

using namespace nsq;

const uint32_t DataFormat::FrameTypeResponse;
const uint32_t DataFormat::FrameTypeError;
const uint32_t DataFormat::FrameTypeMessage;

DataFormat::DataFormat()
{
}


DataFormat::~DataFormat()
{
}

uint32_t nsq::DataFormat::Size()
{
    return static_cast<uint32_t>( MinMessageSize+ messageBody_.size());
}

int nsq::DataFormat::decodePacketBuf(uv::PacketBuffer* buf, std::string& out)
{
    auto size = buf->readSize();
    if (size <= MinMessageSize)
        return -1;
    std::string data;
    buf->readBufferN(data, MinMessageSize);
    UnpackNum(data.c_str(), size_);
    UnpackNum(data.c_str()+sizeof(size_), frameType_);
    //未知的消息格式错误。情况buf数据重新接受
    if (frameType_ != FrameTypeResponse
        && frameType_ != FrameTypeError
        && frameType_ != FrameTypeMessage)
    {
        uv::LogWriter::Instance()->error("err parse buffer.");
        buf->clear();
        return -1;
    }
    
    auto msgSize = size_ + sizeof(size_);
    if (msgSize > size)
    {
        //包数据长度不够
        return 0;
    }
    //清空8位已读字节头部
    buf->clearBufferN(MinMessageSize);
    uint32_t bodysize = (uint32_t)msgSize - MinMessageSize;
    //读消息体
    messageBody_.clear();
    buf->readBufferN(messageBody_, bodysize);
    buf->clearBufferN(bodysize);
    return msgSize;
}

int nsq::DataFormat::decode(const char* data, uint32_t size)
{
    if (size < MinMessageSize)
    {
        return 0;
    }
    const char* index = data;
    index += UnpackNum(index, size_);

    auto msgSize = size_ + sizeof(size_);
    if (msgSize > size)
    {
        return 0;
    }
    index += UnpackNum(index, frameType_);

    uint32_t bodysize = (uint32_t)msgSize - 8;
    std::string body(index, bodysize);
    messageBody_.swap(body);
    return static_cast<int>(msgSize);
}

int nsq::DataFormat::encode(char* data, uint32_t size)
{
    if (size < Size())
    {
        return -1;
    }
    if (size_ + sizeof(uint32_t) != Size())
    {
        return -1;
    }

    char* index = data;
    index += PackNum(index, size_);
    index += PackNum(index, frameType_);

    std::copy(messageBody_.c_str(), messageBody_.c_str() + messageBody_.size(), index);
    return Size();
}

uint32_t  DataFormat::DataSize()
{
    return size_;
}

uint32_t  DataFormat::FrameType()
{
    return  frameType_;
}

std::string&  DataFormat::MessageBody()
{
    return messageBody_;
}
