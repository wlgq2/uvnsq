/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-16

   Description: easy curl interface
*/

#include <string>

#include "EasyCurl.h"

using namespace base;

EasyCurl::EasyCurl()
    :curl_(curl_easy_init())
{

}

EasyCurl::~EasyCurl()
{
    curl_easy_cleanup(curl_);
    curl_ = nullptr;
}


CURLcode EasyCurl::perform()
{
    return curl_easy_perform(curl_);
}

size_t EasyCurl::WriteCallback(void* ptr, size_t size, size_t nmemb, std::string* data)
{
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}
