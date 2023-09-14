# NDN Trace Replay

This project hosts an NDN application written using [ndnSIM](https://ndnsim.net/current/) library. It is a simple producer-consumer application that is able to replay NDN traffic traces by reading a dumped NDN trace from [ndntdump](https://github.com/usnistgov/ndntdump) application. The application assumes that the provided trace is in NDJSON format.

## Prerequisites

Custom version of NS-3 and specified version of ndnSIM needs to be installed. The code should also work with the latest version of ndnSIM, but it is not guaranteed.

1. ndnSIM -b ndnSIM-2.5 (https://github.com/named-data-ndnSIM/ndnSIM)
2. NS-3 -b ndnSIM-2.5 (https://github.com/named-data-ndnSIM/ns-3-dev.git)
3. pybindgen -b 0.18.0 (https://github.com/named-data-ndnSIM/pybindgen.git)

## Running

Assuming NS-3 is installed in a standard location, the application can be run using the following command:

    ./waf --run ndn-replay-point-to-point