#ifndef STATISTICSHANDLER_H
#define STATISTICSHANDLER_H

#include <sstream>
#include <iomanip>

using namespace std;

struct StatisticsHandler {
    unsigned packetsSent;
    unsigned dataPacketsSent;
    unsigned dataPacketsArrived;
    unsigned routingPacketsSent; // Will be 0 for GPSR
    unsigned avgDelay;

    string toString() {
        stringstream string;
        string << "Packets sent: " << packetsSent << "\n";
        string << "Data packets sent: " << dataPacketsSent << "\n";
        string << "Routing packets sent: " << routingPacketsSent << "\n";
        string << "Data packets arrived: " << dataPacketsArrived << "\n";
        string << "Data packets delivery ratio: " << fixed << setprecision(2)<< 100 * ((double) dataPacketsArrived) / ((double) dataPacketsSent) << "%\n";
        string << "Average end-to-end packet delay: " << avgDelay << " \n";
        return string.str();
    }
};


#endif