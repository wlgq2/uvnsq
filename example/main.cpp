#include <iostream>
#include <uvnsq/NsqProducer.h>
#include <uvnsq/NsqConsumer.h>
#include <uvnsq/NsqLookupd.h>

using namespace nsq;
using namespace uv;

void runProducer(nsq::NsqNodesPtr nodes)
{
    uv::EventLoop loop;
    std::string serverip("127.0.0.1");
    uint16_t port = nodes->front().tcpport;
    uv::SocketAddr addr(serverip, port);
    NsqProducer producer(&loop, addr);
    uv::Timer timer(&loop, 1000, 2000, [&producer](uv::Timer* timer)
    {
        producer.pub("test", "test message.");
    });
    timer.start();
    loop.run();
}


void runConsumers(nsq::NsqNodesPtr nodes,std::vector<std::string> channels)
{
    uv::EventLoop loop;
    std::string serverip("127.0.0.1");
    
    for (auto& channel : channels)
    {
        std::shared_ptr<NsqConsumer> consumer(new NsqConsumer(&loop,"test", channel));
        for (auto& node : *nodes)
        {
            uv::SocketAddr addr(serverip, node.tcpport);
            consumer->appendNsqd(addr);
        }
        consumer->setRdy(64);
        consumer->setOnNsqMessage(
            [consumer, channel](NsqMessage& message)
        {
            std::cout<<channel<< " receive" <<" attempts * " << message.Attempts() << " :" << message.MsgBody() << std::endl;
            std::string info("hex: ");
            uv::LogWriter::ToHex(info, message.MsgID());
            std::cout << info<<"\n" << std::endl;
        });
        consumer->start();
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
            
            std::vector<std::string> channels{ "ch1" , "ch2", "ch3"};
            std::thread t1(std::bind(std::bind(&runConsumers, ptr, std::ref(channels))));
            std::thread t2(std::bind(std::bind(&runProducer, ptr)));
            t1.detach();
            t2.detach();
        }
    });
    uv::Idle idle(&loop);
    loop.run();


}

