/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-11

   Description: uv-nsq
*/

#ifndef  NSQ_PROTOCOL_H
#define  NSQ_PROTOCOL_H

#include  <string>
#include  <iostream>

namespace nsq
{

const std::string ProtocolVerV2 = "  V2";
const std::string ProtocolNOP = "NOP";
const std::string ProtocolPUB = "PUB";
const std::string ProtocolSUB = "SUB";
const std::string ProtocolRDY = "RDY";
const std::string ProtocolFIN = "FIN";

const std::string ProtocolRespHeartbeat = "_heartbeat_";

template<typename NumType>
uint64_t PackNumToString(std::string& out, NumType num)
{
    int size = static_cast<int>(sizeof(NumType));
    for (int i = size - 1; i >= 0; i--)
    {
        uint8_t byte = (num>>(i << 3)) & 0xff;
        out.push_back(byte);
    }
    return size;
}

struct CommandVer
{
    const std::string Command = ProtocolVerV2;
    void pack(std::string& out)
    {
        out = Command;
    }
};


struct CommandNOP
{
    const std::string Command = ProtocolNOP;

    void pack(std::string& out)
    {
        out = Command;
        out.push_back('\n');
    }
};

struct CommandPUB
{
    const std::string Command = ProtocolPUB;
    std::string topic;
    std::string body;

    void pack(std::string& out)
    {
        out = Command;
        out.push_back(' ');
        out.append(topic);
        out.push_back('\n');
        uint32_t size = static_cast<uint32_t>(body.size());
        PackNumToString(out, size);
        out.append(body);
    }
};

struct CommandSUB
{
    const std::string Command = ProtocolSUB;
    std::string topic;
    std::string channel;

    void pack(std::string& out)
    {
        out = Command;
        out.push_back(' ');
        out.append(topic);
        out.push_back(' ');
        out.append(channel);
        out.push_back('\n');
    }
};

struct CommandRDY
{
    const std::string Command = ProtocolRDY;
    int count;

    void pack(std::string& out)
    {
        out = Command;
        out.push_back(' ');
        out.append(std::to_string(count));
        out.push_back('\n');
    }
};

struct CommandFIN
{
    const std::string Command = ProtocolFIN;
    std::string msgid;

    void pack(std::string& out)
    {
        out = Command;
        out.push_back(' ');
        out.append(msgid);
        out.push_back('\n');
    }
};
}
#endif // ! NSQ_PROTOCOL_H

