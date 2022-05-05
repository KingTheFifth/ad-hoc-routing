#define WINDOW_SCALE 1 // scaling of the simulation window
#define PACKET_SIZE 4096
#define TICK_STEP 3 // time units incremented every tick
#define TICK_SPEED 1 // ms usually 1
#define LINK_SPEED 1.0 // usually 1.0
#define CLOSE_THRESHOLD 3 // length units within which a point is considered close enough
#define PI 3.141592653589793238462643383279502884L

#define HOST_BUFFER_SIZE 40960 // enough for 10 packets
#define HOST_MOVEMENT_STEP 5 // how much a moving host steps each tick towards its target location
#define HOST_MOVEMENT_SPEED 10 // host moves HOST_MOVEMENT_STEP length units every HOST_MOVEMENT_SPEEDth tick
#define HOST_TRANSMISSION_DELAY 10
#define HOST_PROCESSING_DELAY 60

#define HOST_MOBILITY 0


// DEBUG
#define ONLY_ONE_PACKET 0