#ifndef DSRROUTE_H
#define DSRROUTE_H

#include "host.h"
#include <vector>

struct DSRRoute {
    vector<const Host*> route;

    // Adds a node to the end of the route
    void addNode(const Host* node) {
        route.push_back(node);
    }

    // Returns true if the given host is somehow reachable through this route. Target 
    // does not necessarily have to be the last node of the route
    bool hasTarget(const Host* target) const {
        for (const Host* host : route) {
            if (host == target) return true;
        }
        return false;
    }

    // Returns the next hop host from the given source host according to this route.
    // Works under the assumption that a route does not necessarily start with the 
    // node that is forwarding a packet along a route.
    const Host* getNextHop(const Host* source, bool reverse) {
        for (vector<const Host*>::const_iterator it = route.begin(); it != route.end(); it++) {
            if(*it == source) {
                if (reverse) {
                    if (*it == *route.begin()) return nullptr;
                    return *(it - 1);
                }
                if (*it == (*route.end() - 1)) return nullptr;
                return *(it + 1);
            }
        }
        return nullptr;
    }

    void trim(const Host* last) {
        vector<const Host*>::iterator it = route.begin();
        while (it != route.end() && *it != last) {
            it = route.erase(it);
        }
    }
};

#endif