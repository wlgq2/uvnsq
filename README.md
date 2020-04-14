# uvnsq
Async C++ client library for [NSQ][1]

## Dependencies

 * [uv-cpp][2]
 * [json][3]

## Example
```C++
#include <iostream>
#include <uvnsq/NsqProducer.h>
#include <uvnsq/NsqConsumer.h>
#include <uvnsq/NsqLookupd.h>

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
            uv::LogWriter::ToHex(info, message.MsgID());
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
    
    uv::EventLoop loop;
    nsq::NsqLookupd lookup(&loop);

    lookup.getNodes("127.0.0.1", 4161, [](nsq::NsqNodesPtr ptr) 
    {
        if (nullptr != ptr && !ptr->empty())
        {
            std::string serverip("127.0.0.1");
            uint16_t port = ptr->front().tcpport;
            std::vector<std::string> channels{ "ch1","ch2","ch3" };
            std::thread t1(std::bind(std::bind(&runConsumers, serverip, port, channels)));
            std::thread t2(std::bind(std::bind(&runProducer, serverip, port)));
            t1.join();
            t2.join();
        }
    });
    loop.run();


}







```
[1]: https://github.com/nsqio/nsq
[2]: https://github.com/wlgq2/uv-cpp
[3]: https://github.com/nlohmann/json
