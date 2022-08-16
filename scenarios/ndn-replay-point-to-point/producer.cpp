#include <memory>

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"
#include "utils/ndn-ns3-packet-tag.hpp"
#include "helper/ndn-stack-helper.hpp"

#include <string>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>

#include <iostream>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/scheduler.hpp>

#include <ndn-cxx/lp/tags.hpp>

#include <ns3/node.h>

#include <typeinfo>

using json = nlohmann::json;

namespace ProducerNS
{
    NS_LOG_COMPONENT_DEFINE("ndn.ProducerForSchedules");

    class ProducerForSchedules
    {
    public:
        ProducerForSchedules()
            : m_face(m_ioService) // Create face with io_service object
        {
        }

        void
        run()
        {
            // Create interests and producer store
            std::ifstream file("ndn6dump.vps1.json");
            std::string line;
            while (std::getline(file, line))
            {
                json record = json::parse(line);
                if (record["t"] == ">D" or record["t"] == "<D")
                {
                    ndn::Name ndnName = ndn::Name(record["name"].dump());
                    m_store.insert({ndnName, record});
                }
            }
            m_face.setInterestFilter("/",
                                     bind(&ProducerForSchedules::onInterest, this, _1, _2),
                                     nullptr,
                                     bind(&ProducerForSchedules::onRegisterFailed, this, _1, _2));

            m_face.processEvents();
        }

    private:
        void
        onInterest(const ndn::InterestFilter &, const ndn::Interest &interest)
        {
            std::cout << "On Interest" << std::endl;
            // Search for name in store and reply if found
            auto it = m_store.find(interest.getName());
            if (it != m_store.end())
            {
                static const std::string content("Random");
                auto data = std::make_shared<ndn::Data>(interest.getName());
                data->setFreshnessPeriod(ndn::time::milliseconds(1000));
                data->setContent(reinterpret_cast<const uint8_t *>(content.data()), content.size());
                ns3::ndn::StackHelper::getKeyChain().sign(*data);
                NS_LOG_INFO("Replying to interest " << interest.getName());
                m_face.put(*data);
            }
        }

        void
        onRegisterFailed(const ndn::Name &prefix, const std::string &reason)
        {
            std::cerr << "ERROR: Failed to register prefix '" << prefix << "' with the local forwarder (" << reason << ")" << std::endl;
            m_face.shutdown();
        }

    private:
        // Explicitly create io_service object, which can be shared between Face and Scheduler
        boost::asio::io_service m_ioService;
        ndn::Face m_face;
        std::map<ndn::Name, json> m_store;
    };
} // ProducerNS