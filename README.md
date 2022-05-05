# TDDE35-project-14

Ad hoc routing simulator for DSDV, DSR and GPSR in C++.

Simulator Code:
- C++-+
- done at the latest by April 27
- Specify topology in txt file, one line represents an entry where entry values are separated by space
- Super fancy function that loads and creates the topology from a given file ("Wow!")
- Visual representation of the topology (maybe with animations?)
- Links as objects probably => greater control over jitter and links breaking
- Packets as objects, contains (size of) data, type (routing or data transfer), source and destination node

Topologies:
- Homogeneous (for now)
- Sparse
- Crowded

Metrics:
- Throughput
- Routing overhead
- Average packet delay
- Packet Delivery Ratio

Events:
- host P send A bytes to host Q              :   0 <P> <Q> <bytes>
- host join at (x, y)                        :   1 <x> <y>
- host P disconnect                          :   2 <P>
- host P move to (x, y)                      :   3 <P> <x> <y>

Implemented Features:
- Drawing packets (time left on packet relative to length of link)
- Links (and hosts?) keep track of the time stamp they were last ticked (global clock). When a link is ticked, it is given the
  current time stamp and calculates a time delta compared to its own time stamp. If the delta is greater than 0
  then the link updates its time stamp and does the ticky tick thingy ٩(◕‿◕)۶  (๑˃ᴗ˂)ﻭ (o˘◡˘o) ヽ(°〇°)ﾉ
- Unique host ID (incremental?)

Feature Backlog:
- Destructors
- Deallocate allocated memory where needed (link gets broken, end of simulation) => NO MEMORY LEAKS >:(
- DSDV: Keep track of no. changes in routing table => determine when to send table (hosts)
- TTL
- GPSR handle isolated host case (RHREdge is null -> put into buffer and try again later)
- Link buffer "pair" also keeps track of originating source position in order to handle both source and destination mobility. And counter counts upward instead of down to 0.
