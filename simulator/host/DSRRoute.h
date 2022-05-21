#ifndef DSRROUTE_H
#define DSRROUTE_H

#include "host.h"
#include <vector>

struct DSRRoute {
    vector<const Host*> route;

    DSRRoute() {
        route = {};
    }

    ~DSRRoute() {};

    /**
     * Copy constructor of DSRRoute, make this a copy of 'other'. The 'reverse' boolean changes direction of the route
     */
    DSRRoute(const DSRRoute& other, bool reverse) {
        copyOther(other, reverse);
    }

    /**
     * Make this a copy of 'other'. The 'reverse' boolean changes direction of the route
     */
    void copyOther(const DSRRoute& other, bool reverse) {
        for (const Host* h : other.route) {
            if (reverse) {
                route.insert(route.begin(), h);
            }
            else {
                route.push_back(h);
            }
        }
    }

    /**
     * Returns true if the route is empty
     */
    bool isEmpty() {
        return route.empty();
    }

    /**
     * Empty the route
     */
    void empty() {
        route.clear();
    }

    /**
     * Return the size of the route
     */
    unsigned size() {
        return route.size();
    }

    /**
     * Adds a node to the end of the route
     */
    void addNode(const Host* node) {
        route.push_back(node);
    }

    /**
     * Adds the route of another DSRRoute to the end of this route
     */
    void addRoute(const DSRRoute* other) {
        for (const Host* h : other->route) {
            route.push_back(h);
        }
    }

    /**
     * Returns true if the given host is somehow reachable through this route. Target 
     * does not necessarily have to be the last node of the route
     */
    bool hasTarget(const Host* target) const {
        for (const Host* host : route) {
            if (host == target) return true;
        }
        return false;
    }

    /** Returns the next hop host from the given source host according to this route.
     * Works under the assumption that a route does not necessarily start with the 
     * node that is forwarding a packet along a route.
     */
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

    /**
     * Remove hosts up until 'first' at the front of the route
     */
    void trimFront(const Host* first) {
        vector<const Host*>::iterator it = route.begin();
        while (it != route.end() && *it != first) {
            it = route.erase(it);
        }
    }

    /**
     * Remove hosts up until 'last' at the back of the route
     */
    void trimBack(const Host* last) {
        vector<const Host*>::iterator it = route.begin();
        bool hasPassed = false;
        while (it != route.end()) {
            if (*it == last) {
                hasPassed = true;
                it++;
            }
            else if (hasPassed)
                it = route.erase(it);
            else it++;
        }
    }
};

#endif