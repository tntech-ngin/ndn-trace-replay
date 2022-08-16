#include "ns3/application.h"
#include "producer.cpp"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-app-helper.hpp"

#include "ns3/random-variable-stream.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <ndn-cxx/face.hpp>
#include "ns3/core-module.h"

namespace ns3
{
    // Class inheriting from ns3::Application
    class ProducerApp : public Application
    {
    public:
        static TypeId
        GetTypeId()
        {
            static TypeId tid = TypeId("ProducerApp")
                                    .SetParent<Application>()
                                    .AddConstructor<ProducerApp>();
            return tid;
        }

    protected:
        // inherited from Application base class.
        virtual void
        StartApplication()
        {

            // Create an instance of the app, and passing the dummy version of KeyChain (no real signing)
            m_instance.reset(new ProducerNS::ProducerForSchedules());
            m_instance->run(); // can be omitted
        }

        virtual void
        StopApplication()
        {
            // Stop and destroy the instance of the app
            m_instance.reset();
        }

    private:
        std::unique_ptr<ProducerNS::ProducerForSchedules> m_instance;
    };

} 