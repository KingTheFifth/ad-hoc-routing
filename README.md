# TDDE35-project-14

Ad hoc routing simulator for DSDV, DSR and GPSR in C++.

Routing protocol, as well as event file is set in main.cpp. There exists 5 event files, A, B, C, D, and
E for each of the four existing topologies seen below. Each event file consists of a combination of event
types, where all hosts join the network through join events. The different event types can be seen below.
A simulation ends when all events have been processed and all data packets sent through events have either
arrived at their destination, or have been dropped for any reason. After the simulation ends, the metrics
seen below will be printed in order after the line "Actual metrics, in order:". Additional, raw metrics are
also printed above this line.

Topologies:
- Small, sparse
- Small, dense
- Large, sparse
- Large, dense

Metrics:
- Packet Delivery Ratio
- Throughput
- Routing overhead
- Average packet delay

Event types:
- 0: host P sends a packet to host Q of size A bytes
  - Input for this would look like: 0 P Q A
- 1: host join at (x, y)
  - Input for this would look like: 1 x y
- 2: host P disconnect 
  - Input for this would look like: 2 P
- 3: host P move to (x, y) 
  - Input for this would look like: 3 P x y
