# uvnsq
Async C++ client library for [NSQ][1]

## Dependencies

 * [uv-cpp][2]
 * [libcurl][3]
 * [json][4]

[1]: https://github.com/nsqio/nsq
[2]: https://github.com/wlgq2/uv-cpp
[3]: https://curl.haxx.se/libcurl
[4]: https://github.com/nlohmann/json

```C++
#include <iostream>
#include <nsq/NsqProducer.h>
#include <nsq/NsqConsumer.h>
#include <nsq/NsqLookupd.h>

using namespace nsq;
using namespace uv;

void runProducer(std::string ip,uint16_t port)
{
    uv::EventLoop loop;
    uv::SocketAddr addr(ip, port);
    NsqProducer producer(&loop, addr);
    uv::Timer timer(&loop, 1000, 2000, [&producer](uv::Timer* timer)
    {
        producer.pub("test", "test message.");
    });
    timer.start();
    loop.run();
}


void runConsumers(std::string ip, uint16_t port,std::vector<std::string>& channels)
{
    uv::EventLoop loop;
    uv::SocketAddr addr(ip, port);
    for (auto& channel : channels)
    {
        std::shared_ptr<NsqConsumer> consumer(new NsqConsumer(&loop));
        consumer->appendSub("test", channel);
        consumer->setRdy(64);
        consumer->setOnNsqMessage(
            [consumer, channel](NsqMessage& message)
        {
            std::cout<<channel<< " receive" <<" attempts * " << message.Attempts() << " :" << message.MsgBody() << std::endl;
            std::string info("hex: ");
            uv::LogWriter::ToHex(info, message.MsgID().c_str(), message.MsgID().size());
            std::cout << info<<"\n" << std::endl;
            consumer->fin(message.MsgID());
        });
        consumer->start(addr);
    }

    loop.run();
}

int main(int argc, char** args)
{
    uv::LogWriter::Instance()->setLevel(uv::LogWriter::Info);
    
    std::vector<NsqNode> nodes;
    auto code = NsqLookupd::GetNodes("http://127.0.0.1:4161/nodes", nodes);   

    if (!nodes.empty())
    {
        std::string serverip("127.0.0.1");
        uint16_t port = nodes.front().tcpport;
        std::vector<std::string> channels{ "ch1","ch2","ch3" };
        std::thread t1(std::bind(std::bind(&runConsumers, serverip, port, channels)));
        std::thread t2(std::bind(std::bind(&runProducer, serverip, port)));
        t1.join();
        t2.join();
    }
}




```

