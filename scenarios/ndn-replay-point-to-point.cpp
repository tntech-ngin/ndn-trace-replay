#include <fstream>
#include <nlohmann/json.hpp>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
// #include "ns3/ndnSIM-module.h"

#include "ndn-replay-point-to-point/client-app.hpp"
#include "ndn-replay-point-to-point/producer-app.hpp"

namespace ns3
{
    NS_OBJECT_ENSURE_REGISTERED(ProducerApp);
    NS_OBJECT_ENSURE_REGISTERED(ClientApp);

    int
    main(int argc, char *argv[])
    {

        // setting default parameters for PointToPoint links and channels
        Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
        Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
        Config::SetDefault("ns3::QueueBase::MaxPackets", UintegerValue(20));

        // Creating nodes
        NodeContainer nodes;
        nodes.Create(3);

        // Connecting nodes using two links
        PointToPointHelper p2p;
        p2p.Install(nodes.Get(0), nodes.Get(1));
        p2p.Install(nodes.Get(1), nodes.Get(2));

        // Install NDN stack on all nodes
        ndn::StackHelper ndnHelper;
        ndnHelper.SetDefaultRoutes(true);
        ndnHelper.InstallAll();

        // Choosing forwarding strategy
        ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/multicast");

        // Installing applications
        // Producer
        ndn::AppHelper producerHelper("ProducerApp");
        // ndn::AppHelper producerHelper("ns3::ndn::Producer");
        // Producer will reply to all requests starting with /prefix
        // producerHelper.SetPrefix("/prefix");
        // producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
        producerHelper.Install(nodes.Get(2)).Start(Seconds(3)); // last node

        // Consumer
        ndn::AppHelper consumerHelper("ClientApp");
        // Consumer will request /prefix/0, /prefix/1, ...
        // consumerHelper.SetPrefix("/prefix");
        // consumerHelper.SetAttribute("Frequency", StringValue("10")); // 10 interests a second
        consumerHelper.Install(nodes.Get(0)).Start(Seconds(3)); // first node

        Simulator::Stop(Seconds(200));
        Simulator::Run();
        Simulator::Destroy();
        return 0;
    }

} // namespace ns3

int main(int argc, char *argv[])
{
    return ns3::main(argc, argv);
}