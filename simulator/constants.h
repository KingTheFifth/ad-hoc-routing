#define WINDOW_SCALE 1.3 // scaling of the simulation window
#define PACKET_SIZE 4096
#define TICK_STEP 3 // time units incremented every tick
#define TICK_SPEED 5 // ms
#define CLOSE_THRESHOLD 3 // length units within which a point is considered close enough
#define PI 3.141592653589793238462643383279502884L

#define HOST_BUFFER_SIZE 40960 // enough for 10 packets
#define HOST_MOVEMENT_STEP 5 // how much a moving host steps each tick towards its target location
#define HOST_TRANSMISSION_DELAY 0
#define HOST_PROCESSING_DELAY 10


// DEBUG
#define ONLY_ONE_PACKET 0