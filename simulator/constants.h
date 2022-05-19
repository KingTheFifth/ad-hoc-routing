#define WINDOW_SCALE 1 // scaling of the simulation window
#define PACKET_SIZE 4096
#define TICK_STEP 3 // time units incremented every tick
#define TICK_INTERVAL 15 // ms usually 50
#define LINK_SPEED 1.0 // usually 1.0
#define CLOSE_THRESHOLD 3 // length units within which a point is considered close enough
#define PI 3.141592653589793238462643383279502884L
#define MAX_OVERTIME 550
#define DSR_TIMEOUT 1000 // value is very experimental
#define MAX_RETRY_COUNT 3

#define HOST_BUFFER_SIZE 40960 // enough for 10 packets, probably very small. TODO: Look for a better value?
#define HOST_MOVEMENT_STEP 5 // how much a moving host steps each tick towards its target location
#define HOST_MOVEMENT_SPEED 10 // host moves HOST_MOVEMENT_STEP length units every HOST_MOVEMENT_SPEEDth tick
#define HOST_TRANSMISSION_DELAY 10 // measurements done at 10
#define HOST_PROCESSING_DELAY 60 // measurements done at 60

#define HOST_MOBILITY 1 // on / off

#define BROADCASTDELAY 1600 //1600 has been found to be a suitable value for BROADCASTDELAY with a trade-off between delivery ratio and average delay.
#define FULLBROADCASTDELAY 6400 //9600 was found to be "better", but it does not detect broken links in a timely manner.

#define EVENT_DURATION_DEFAULT 40
#define EVENT_DURATION_JOIN 10

#define TTL 300
