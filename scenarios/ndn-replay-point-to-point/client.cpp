#include <memory>

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"
#include "utils/ndn-ns3-packet-tag.hpp"

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

namespace ConsumerNS
{
    NS_LOG_COMPONENT_DEFINE("ndn.ConsumerWithScheduler");

    class ConsumerWithScheduler
    {
    public:
        ConsumerWithScheduler()
            : m_face(m_ioService) // Create face with io_service object
              ,
              m_scheduler(m_ioService)

        {
        }

        void
        run()
        {
            // Create interests and producer store
            std::ifstream file("ndn6dump.vps1.json");
            std::vector<json> interests;
            std::map<ndn::Name, json> store;
            std::string line;
            while (std::getline(file, line))
            {
                json record = json::parse(line);
                if (record["t"] == ">I")
                {
                    interests.push_back(record);
                }
                else if (record["t"] == ">D" or record["t"] == "<D")
                {
                    ndn::Name ndnName = ndn::Name(record["name"].dump());
                    store.insert({ndnName, record});
                }
            }

            // Schedule interests
            ndn::Interest interest;
            auto initialTimestamp = ndn::time::nanoseconds(interests[0]["ts"]);
            auto delay = ndn::time::seconds(1);
            auto timestampOffset = delay - initialTimestamp;
            for (json interestRecord : interests)
            {
                auto intName = interestRecord["name"].dump();
                auto ndnName = ndn::Name(intName);
                interest.setName(ndnName);
                if (interestRecord["lifetime"].is_number())
                {
                    interest.setInterestLifetime(ndn::time::seconds(interestRecord["lifetime"]));
                }
                interest.setInterestLifetime(ndn::time::seconds(1000));
                if (interestRecord["mbf"].is_boolean())
                {
                    interest.setMustBeFresh(interestRecord["mbf"]);
                }
                interest.setMustBeFresh(false);
                // interest.setCanBePrefix(interestRecord["cbp"].dump());
                auto schedule = ndn::time::nanoseconds(interestRecord["ts"]) + timestampOffset;
                // NS_LOG_INFO("Scheduling " << interest.getName() << " at time " << schedule);
                m_scheduler.scheduleEvent(schedule, bind(&ConsumerWithScheduler::delayedInterest, this, interest));
            }
            m_face.processEvents();
        }

    private:
        void
        onData(const ndn::Interest &interest, const ndn::Data &data)
        {
            std::cout << "received data!!" << interest.getName() << std::endl;
        }

        void
        onNack(const ndn::Interest &interest, const ndn::lp::Nack &nack)
        {
            NS_LOG_INFO("Received Nack with reason " << nack.getReason()
                                                     << " for interest " << interest.getName());
        }

        void
        onTimeout(const ndn::Interest &interest)
        {
            NS_LOG_INFO("Timeout " << interest);

            auto newInterest = ndn::Interest(interest.getName());
            newInterest.setInterestLifetime(ndn::time::seconds(1));
            newInterest.setMustBeFresh(true);
            // newInterest.refreshNonce();

            m_scheduler.scheduleEvent(ndn::time::seconds(1),
                                      bind(&ConsumerWithScheduler::delayedInterest, this, newInterest));
        }

        void
        delayedInterest(const ndn::Interest &interest)
        {
            m_face.expressInterest(interest,
                                   bind(&ConsumerWithScheduler::onData, this, _1, _2),
                                   bind(&ConsumerWithScheduler::onNack, this, _1, _2),
                                   bind(&ConsumerWithScheduler::onTimeout, this, _1));
        }

    private:
        // Explicitly create io_service object, which can be shared between Face and Scheduler
        boost::asio::io_service m_ioService;
        ndn::Face m_face;
        ndn::Scheduler m_scheduler;
    };
} // app