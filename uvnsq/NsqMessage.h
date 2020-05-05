/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-12

   Description: uv-nsq
*/

#ifndef NSQ_MESSAGE_H
#define NSQ_MESSAGE_H

#include  <string>

namespace nsq
{ 
class NsqMessage
{
public:
    NsqMessage();
    virtual ~NsqMessage();

    int decode(std::string& message);

public:
    const uint32_t MinSize = 26;

    uint64_t Timestamp();
    uint16_t Attempts();
    std::string& MsgID();
    std::string& MsgBody();

private:
    uint64_t  timestamp_;
    uint16_t  attempts_;
    std::string msgID_;
    std::string body_;
};

}
#endif

