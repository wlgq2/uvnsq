/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-10

   Description: uv-nsq
*/


#ifndef   NSQ_DATA_FORMAT_H
#define   NSQ_DATA_FORMAT_H

#include <string>

namespace nsq
{ 

class DataFormat
{
public:
    DataFormat();
    virtual ~DataFormat();

    uint32_t Size();
    int decode(const char* data, uint32_t size);
    int  encode(char* data, uint32_t size);

    uint32_t DataSize();
    uint32_t FrameType();
    std::string& MessageBody();


public:
    const uint32_t MinMessageSize = 8;

    template<typename NumType>
    static  uint64_t UnpackNum(const char* data, NumType& num);
    template<typename NumType>
    static uint64_t PackNum(char* data, NumType num);

    static  const uint32_t FrameTypeResponse =0;
    static  const uint32_t FrameTypeError =1;
    static  const uint32_t FrameTypeMessage =2;

private:
    uint32_t size_;
    uint32_t frameType_;
    std::string messageBody_;
};


template<typename NumType>
inline uint64_t DataFormat::UnpackNum(const char* data, NumType& num)
{
    num = 0;
    auto size = static_cast<int>(sizeof(NumType));

    for (int i = 0; i < size; i++)
    {
        num <<= 8;
        num |= data[i];
    }
    return size;
}

template<typename NumType>
inline uint64_t DataFormat::PackNum(char* data, NumType num)
{
    int size = static_cast<int>(sizeof(NumType));
    for (int i = size - 1; i >= 0; i--)
    {
        data[i] = num & 0xff;
        num >>= 8;
    }
    return size;
}

}
#endif // ! NSQ_MESSAGE_H