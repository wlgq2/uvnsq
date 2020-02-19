/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-16

   Description: uv-nsq
*/

#include <string>
#include <uv/include/uv11.h>

#include "NsqLookupd.h"
#include "http/EasyCurl.h"

using namespace std;
using namespace nsq;

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