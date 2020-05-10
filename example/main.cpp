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
    std::vector<NsqProducerPtr> producers;
    std::vector<std::string> messages;
    for (auto& node : *nodes)
    {
        uv::SocketAddr addr(serverip, node.tcpport);
        producers.push_back(std::make_shared<NsqProducer>(&loop, addr));
        messages.push_back(std::string("a message from ") + addr.toStr());
    }

    uv::Timer timer(&loop, 1500, 3000, [&producers, messages](uv::Timer* timer)
    {
        std::string topic("test");
        for (size_t i = 0;i < producers.size();i++)
        {
            std::string& str = const_cast<std::string&>(messages[i]);
            producers[i]->pub(topic, str);
        }
    });
    timer.start();
    loop.run();
}


void runConsumers(nsq::NsqNodesPtr nodes,std::vector<std::string> channels)
{
    uv::EventLoop loop;
    std::string serverip("127.0.0.1");

    std::vector<NsqConsumerPtr> consumers;
    for (auto& channel : channels)
    {

        for (auto& node : *nodes)
        {
            NsqConsumerPtr consumer(new NsqConsumer(&loop, "test", channel));
            consumers.push_back(consumer);
            uv::SocketAddr addr(serverip, node.tcpport);
            consumer->setNsqd(addr);
            consumer->setRdy(64);
            consumer->setOnNsqMessage(
                [channel](NsqMessage& message)
            {
                std::cout << channel << " receive" << " attempts * " << message.Attempts() << " :" << message.MsgBody() << std::endl;
                std::string info("hex: ");
                uv::LogWriter::ToHex(info, message.MsgID());
                std::cout << info << "\n" << std::endl;
            });
            consumer->start();
        }
    }

    uv::Timer timer(&loop, 15000, 0, [&consumers](uv::Timer* timer)
    {
        //delete all consumers.
        std::cout << "delete and close tcp connection." << std::endl;
        consumers.front() = nullptr;;
    });
    timer.start();

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

            std::vector<std::string> channels{ "ch1" , "ch2"};
            std::thread t1(std::bind(std::bind(&runConsumers, ptr, std::ref(channels))));
            std::thread t2(std::bind(std::bind(&runProducer, ptr)));
            t1.detach();
            t2.detach();
        }
    });
    uv::Idle idle(&loop);
    loop.run();


}

