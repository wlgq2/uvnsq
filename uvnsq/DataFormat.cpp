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
