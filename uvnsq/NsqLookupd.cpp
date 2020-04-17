/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-16

   Description: uv-nsq
*/

#include <string>

#include "NsqLookupd.h"

using namespace std;
using namespace nsq;

NsqLookupd::NsqLookupd(uv::EventLoop* loop)
    :loop_(loop)
{
}

void NsqLookupd::get(uv::SocketAddr& addr, std::string path, OnLookupCallback callback)
{
    loop_->runInThisLoop([this,callback,addr,path]()
    {
        uv::http::HttpClient* client = new uv::http::HttpClient(loop_);
        uv::http::Request req;
        req.setPath((std::string)path);
        req.appendHead("Host", ((uv::SocketAddr)addr).toStr());
        client->setOnResp([this, callback, client](int status, uv::http::Response* resp)
        {
            if (callback)
            {
                if (status == uv::http::HttpClient::Success)
                {
                    try
                    {
                        JsonPtr ptr = std::make_shared<nlohmann::json>();
                        if (resp->getStatusCode() == uv::http::Response::OK)
                        {
                            *ptr = nlohmann::json::parse(resp->getContent());
                            callback(ptr);
                        }
                        else
                        {
                            callback(nullptr);
                        }
                    }
                    catch (...)
                    {
                        uv::LogWriter::Instance()->error("parse json fail.");
                        callback(nullptr);
                    }
                }
                else
                {
                    uv::LogWriter::Instance()->error("get server fail.");
                    callback(nullptr);
                }
            }
            delete client;
        });
        client->Req(const_cast<uv::SocketAddr&>(addr), req);
    });
}

void NsqLookupd::get(std::string ip, uint16_t port, std::string path, OnLookupCallback callback,uv::SocketAddr::IPV ipv)
{
    uv::SocketAddr addr(ip, port, ipv);
    get(addr, path, callback);
}

void NsqLookupd::getNodes(uv::SocketAddr& addr,OnGetNodesCallback callback)
{
    get(addr, "/nodes", [this, callback](JsonPtr jsonData)
    {
        if (jsonData == nullptr)
        {
            callback(nullptr);
        }
        else
        {
            try
            {
                auto nodesCnt = (*jsonData)["producers"].size();
                auto& producers = (*jsonData)["producers"];
                NsqNodesPtr ptr = std::make_shared<std::vector<NsqNode>>();
                for (uint64_t i = 0; i < nodesCnt; i++)
                {
                    NsqNode node;
                    node.remoteaddr = producers[i]["remote_address"].get<std::string>();
                    node.httpport = producers[i]["http_port"].get<uint16_t>();
                    node.tcpport = producers[i]["tcp_port"].get<uint16_t>();
                    ptr->push_back(node);
                }
                callback(ptr);
            }
            catch (...)
            {
                callback(nullptr);
            }
        }
    });
}

void NsqLookupd::getNodes(std::string ip, uint16_t port, OnGetNodesCallback callback, uv::SocketAddr::IPV ipv)
{
    uv::SocketAddr addr(ip, port, ipv);
    getNodes(addr, callback);
}
