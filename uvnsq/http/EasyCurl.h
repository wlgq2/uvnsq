/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-16

   Description: easy curl interface
*/

#ifndef   NSQ_EASY_CURL_H
#define   NSQ_EASY_CURL_H

#include <curl/curl.h>

namespace base 
{

class EasyCurl  
{
public:
    EasyCurl();
    ~EasyCurl();
    
    template<typename Type>
    CURLcode setopt(CURLoption opt1, Type opt2);
    template<typename Type>
    CURLcode getinfo( CURLINFO info1, Type info2);
    CURLcode perform();

    static size_t WriteCallback(void *ptr, size_t size, size_t nmemb, std::string* data);

private:
    CURL* curl_;

};

template<typename Type>
inline CURLcode EasyCurl::setopt(CURLoption opt1, Type opt2)
{
    return curl_easy_setopt(curl_, opt1, opt2);
}

template<typename Type>
inline CURLcode EasyCurl::getinfo(CURLINFO info1, Type info2)
{
    return curl_easy_getinfo(curl_, info1, info2);
}

}
#endif
