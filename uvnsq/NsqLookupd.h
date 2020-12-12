/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-16

   Description: uv-nsq
*/

#ifndef   NSQ_LOOKUPD_H
#define   NSQ_LOOKUPD_H

#include <string>
#include <vector>
#include <memory>
#include <json/json.hpp>
#include <uv/include/uv11.hpp>

namespace nsq 
{

struct NsqNode
{
    std::string remoteaddr;
    uint16_t tcpport;
    uint16_t httpport;
};

using JsonPtr = std::shared_ptr<nlohmann::json>;
using NsqNodesPtr = std::shared_ptr<std::vector<NsqNode>>;
using OnLookupCallback = std::function<void(JsonPtr)>;
using OnGetNodesCallback = std::function<void(NsqNodesPtr)>;
class NsqLookupd 
{
public:
    NsqLookupd(uv::EventLoop* loop);
    void get(uv::SocketAddr& addr, std::string path, OnLookupCallback callback);
    void get(std::string ip,uint16_t port, std::string path, OnLookupCallback callback, uv::SocketAddr::IPV ipv = uv::SocketAddr::Ipv4);
    void getNodes(uv::SocketAddr& addr,  OnGetNodesCallback callback);
    void getNodes(std::string ip, uint16_t port, OnGetNodesCallback callback, uv::SocketAddr::IPV ipv = uv::SocketAddr::Ipv4);


private:
    uv::EventLoop* loop_;
};

}
#endif
