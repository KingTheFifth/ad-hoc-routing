#ifndef STATISTICSHANDLER_H
#define STATISTICSHANDLER_H

#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

struct StatisticsHandler {
    unsigned packetsSent;
    unsigned dataPacketsSent;
    unsigned dataPacketsArrived;
    unsigned routingPacketsSent; // Will be 0 for GPSR
    unsigned dataPacketsDropped;
    unsigned avgDelay;
    double avgThroughput;
    bool handledPackets [200];

    string toString() {
        stringstream string;
        string << "Packets sent: " << packetsSent << "\n";
        string << "Data packets sent: " << dataPacketsSent << "\n";
        string << "Routing packets sent: " << routingPacketsSent << "\n";
        string << "Data packets arrived: " << dataPacketsArrived << "\n";
        string << "Data packets delivery ratio: " << fixed << setprecision(2)<< 100 * ((double) dataPacketsArrived) / ((double) dataPacketsSent) << "%\n";
        string << "Average end-to-end packet delay: " << avgDelay << "\n";
        string << "Average throughput: " << fixed << setprecision(10) << avgThroughput << "\n";
        return string.str();
    }

    void addPacketArrival(int delay) {
        unsigned prevDelaySum = avgDelay * dataPacketsArrived;
        double prevThroughputSum = avgThroughput * dataPacketsArrived;
        dataPacketsArrived++;
        avgDelay = (double) (prevDelaySum + delay) / (double) dataPacketsArrived;
        avgThroughput = (prevThroughputSum + 1.0 / (double) delay) / (double) dataPacketsArrived;
    }

    void addRoutingPackets(int amount) {
        packetsSent += amount;
        routingPacketsSent += amount;
    }

    void countPacket(int id){
        handledPackets[id] = true;
    }

    void printHandledPackets(){
        for (int i = 0; i < 200; i++)
            cout << i << ": " << handledPackets[i] << endl;
    }

    void dropDataPacket() {
        dataPacketsDropped++;
    }
};

#endif
