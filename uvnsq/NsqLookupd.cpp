/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-16

   Description: uv-nsq
*/

#include <string>

#include "NsqLookupd.h"
#include "http/EasyCurl.h"

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
        client->setOnResp([this, callback, client](int status, uv::http::Response* resp)
        {
            if (callback)
            {
                if (status == uv::http::HttpClient::Success)
                {
                    try
                    {
                        JsonPtr ptr = std::make_shared<nlohmann::json>();
                        *ptr = nlohmann::json::parse(resp->getContent());
                        callback(ptr);
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
        client->Req((uv::SocketAddr)addr, req);
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
                nlohmann::json root;
                auto nodesCnt = root["producers"].size();
                auto& producers = root["producers"];
                NsqNodesPtr ptr = std::make_shared<std::vector<NsqNode>>();
                for (auto i = 0; i < nodesCnt; i++)
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

int nsq::NsqLookupd::Get(std::string&& url, nlohmann::json& out)
{
    std::string header;
    std::string context;
    NsqLookupd::CurlGet(std::move(url), header, context);
    try
    {
        out = nlohmann::json::parse(context);
        return 0;
    }
    catch (...)
    {
        uv::LogWriter::Instance()->error("parse node's json fail.");
        return -1;
    }
}



int nsq::NsqLookupd::GetNodes(std::string&& url, std::vector<NsqNode>& nodes)
{
    std::string header;
    std::string context;
    auto code = NsqLookupd::CurlGet(std::move(url), header, context);

    nlohmann::json root;
    auto rst = NsqLookupd::Get(std::move(url), root);
    if (0 == rst)
    {
        try 
        {
            auto nodesCnt = root["producers"].size();
            auto& producers = root["producers"];
            for (auto i = 0; i < nodesCnt; i++)
            {
                NsqNode node;
                node.remoteaddr = producers[i]["remote_address"].get<std::string>();
                node.httpport = producers[i]["http_port"].get<uint16_t>();
                node.tcpport = producers[i]["tcp_port"].get<uint16_t>();
                nodes.push_back(node);
            }
        }
        catch (...)
        {
            return -1;
        }
    }
    return rst;
}

long NsqLookupd::CurlGet(std::string&& url, std::string& header, std::string& resp)
{
    base::EasyCurl http;
    http.setopt(CURLOPT_URL, url.c_str());

    http.setopt(CURLOPT_WRITEFUNCTION, base::EasyCurl::WriteCallback);
    http.setopt(CURLOPT_WRITEDATA, &resp);
    http.setopt(CURLOPT_HEADERDATA, &header);

    long code;
    http.getinfo(CURLINFO_RESPONSE_CODE, &code);

    http.perform();
    return code;
}