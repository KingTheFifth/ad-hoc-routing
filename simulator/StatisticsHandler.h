#ifndef STATISTICSHANDLER_H
#define STATISTICSHANDLER_H

#include <sstream>

using namespace std;

struct StatisticsHandler {
    unsigned packetsSent;
    unsigned dataPacketsSent;
    unsigned dataPacketsArrived;
    unsigned routingPacketsSent; // Will be 0 for GPSR

    string toString() {
        stringstream string;
        string << "Packets sent: " << packetsSent << "\n";
        string << "Data packets sent: " << dataPacketsSent << "\n";
        string << "Routing packets sent: " << routingPacketsSent << "\n";
        string << "Data packets arrived: " << dataPacketsArrived << "\n";
        string << "Data packets delivery ratio: " << ((double) dataPacketsArrived) / ((double) dataPacketsSent) << "\n";
        return string.str();
    }
};


#endif