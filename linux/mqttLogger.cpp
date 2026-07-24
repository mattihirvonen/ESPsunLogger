
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <mosquitto.h>
#include <pthread.h>
#include <cstring>

#define UNUSED  __attribute__((unused))

#define BUFFER_SIZE    1024                // Max UDP packet size to receive
#define NaN            ((int)0x80000000)   // Not a Number
#define SCALE_SECOND   1.0
#define SCALE_MINUTE   60.0
#define SCALE_HOUR     3600.0
#define SCALE_DAY      86400.0
#define SCALE_MONTH    2592000.0           // Seconds of 30 days
#define LOGSTART       1768374260
#define MQTT_BROKER    "localhost"         // Default MQTT message broker "127.0.0.1"
#define MQTT_PORT      1883
#define TOPIC          "#"
#define UDP_PORT       8080                // Default UDP packet listening port

typedef struct
{
    time_t  start_time;
    char    mqtt_broker[64];
    int     Naverage;
    float   time_scale;
    char    topic[64];
} conf_t;


conf_t conf = {
      .start_time  = 0,
      .mqtt_broker = MQTT_BROKER,
      .Naverage    = 1,
      .time_scale  = SCALE_HOUR,
      .topic       = TOPIC
};

// -----------------------------------------------------------------------------

/**
 * diff_timespec - Calculate the difference between two timespecs
 * @start: start time
 * @end:   end time
 * @result: pointer to timespec where the difference will be stored
 *
 * Returns:
 *   0 on success
 *  -1 if end < start (negative difference)
 */
int diff_timespec(const struct timespec *start,
                  const struct timespec *end,
                        struct timespec *result)
{
    if (!start || !end || !result) {
        return -1; // Null pointer check
    }

    // Check if end is before start
    if ((end->tv_sec <  start->tv_sec) ||
        (end->tv_sec == start->tv_sec && end->tv_nsec < start->tv_nsec)) {
        return -1; // Negative difference not supported here
    }

    result->tv_sec  = end->tv_sec  - start->tv_sec;
    result->tv_nsec = end->tv_nsec - start->tv_nsec;

    // Adjust if nanoseconds are negative
    if (result->tv_nsec < 0) {
        result->tv_sec -= 1;
        result->tv_nsec += 1000000000L;
    }

    return 0;
}

int64_t time_ms( void )
{
    struct  timeval tv;
    int64_t ms;

    gettimeofday( &tv, NULL );
    ms   = tv.tv_usec / 1000;
    ms  += tv.tv_sec  * 1000;
    return ms;
}

// -----------------------------------------------------------------------------
#if 0

int udp_recv_open( const int port )
{
    int sockfd;
    struct sockaddr_in server_addr;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return EXIT_FAILURE;
    }

    // Zero out the server address structure
    memset(&server_addr, 0, sizeof(server_addr));

    // Configure server address
    server_addr.sin_family = AF_INET;         // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(port);       // Port in network byte order

    // Bind socket to the specified port
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return EXIT_FAILURE;
    }
    return sockfd;
}


// Result should be zero with valid shecksum
char calcChecksum( const char *buffer, const int count )
{
    char checksum = 0;

    for (int i = 0; i < count; i++) {
        checksum += *buffer++;
    }
    return checksum;
}


char *findTopic( const char *buffer, const char *topic )
{
    char  find[16], *item;

    strcpy( find, "\n" );     // Add leading  '\n' character
    strcat( find, topic );
    strcat( find, "\t" );     // Add training '\t' character

    item = const_cast<char*>( strstr(buffer, find) );
    if ( item ) {
         item += 1;  // Skip leading '\n' character
    }
    return item;
}


int getValue1( const char *buffer, const char *topic )
{
    int   value = NaN;
    char *item  = findTopic( buffer, topic );

    if ( item ) {
        item += strlen( topic ) + 1;    // Skip trailing '\t'
        value = atoi( item );
    }
    return value;
}


void getValues( shunt_t *value, const char *buffer )
{
    value->V   = getValue1( buffer, "V"   );
    value->VS  = getValue1( buffer, "VS"  );
    value->I   = getValue1( buffer, "I"   );
    value->P   = getValue1( buffer, "P"   );
    value->CE  = getValue1( buffer, "CE"  );
    value->SOC = getValue1( buffer, "SOC" );
}


void addValues( shunt_t *sum, shunt_t *value )
{
    sum->V   += value->V;
    sum->VS  += value->VS;
    sum->I   += value->I;
    sum->P   += value->P;
    sum->CE  += value->CE;
    sum->SOC += value->SOC;
}

#endif
// -----------------------------------------------------------------------------

void printTime( time_t seconds, float time_scale )
{
    float timestamp = seconds;

    printf("%9.5f", timestamp / time_scale);
}


#if 0
void printValue1( int value, UNUSED const char *topic, float scale )
{
    float data  = value;

    if ( value == NaN ) {
        return;
    }
 // printf("- %s\t%d\n", topic, value );
    printf("   %8.3f", data * scale );
}


void printValues( shunt_t *shunt, int Naverage )
{
    printValue1( shunt->V,   "V",   0.001 / Naverage );
    printValue1( shunt->VS,  "VS" , 0.001 / Naverage );
    printValue1( shunt->I,   "I",   0.001 / Naverage );
    printValue1( shunt->P,   "P",   0.001 / Naverage );
    printValue1( shunt->CE,  "CE",  0.001 / Naverage );
    printValue1( shunt->SOC, "SOC", 0.100 / Naverage );
}


void printStatistics( int messages, int cserr )
{
    printf(" -  msg=%d  cserr=%d", messages, cserr);
}


int handleShuntMessage( char *buffer, int bytes_received )
{
    static shunt_t  sum;
    static int      messages = 0, cserr = 0, count = 0;

    // Null-terminate the received data for safe printing and string operations
    if ( bytes_received > 0 ) {  buffer[ bytes_received ] = 0;  }
    else                      {  return -1;                     }

    uint8_t  checksum = calcChecksum( buffer, bytes_received );
    shunt_t  shunt;

    messages += 1;
    if ( checksum ) {  // Checksum error ?
         cserr += 1;
         return -1;
    }
    if ( !findTopic(buffer, "PID") ) {  // Message contains? PID, V, VS, I, ...
         return -1;
    }
    getValues( &shunt, buffer );
    addValues( &sum,   &shunt );
    if ( ++count >= conf.Naverage )
    {
        time_t seconds = time(NULL) - conf.start_time;

        printTime( seconds, conf.time_scale );
        printValues( &sum, count );
        printStatistics( messages, cserr );
        printf("\n");
        fflush( NULL );
        sync();
        memset( &sum, 0, sizeof(sum) );
        count = 0;
    }
    return 0;
}

// -----------------------------------------------------------------------------
// UDP

void printMessage( const char *buffer, int bytes_received, uint8_t checksum, struct sockaddr_in *client_addr )
{
        // Print client info and message
        printf("Received %d bytes from %s:%d -- csum=%d\n",
               bytes_received,
               inet_ntoa(client_addr->sin_addr),
               ntohs(client_addr->sin_port),
               checksum);
        printf("Data: %s\n", buffer);
}


int udp_recvfrom( int sockfd )
{
    struct sockaddr_in  client_addr;
    socklen_t           addr_len = sizeof(client_addr);
    char                buffer[BUFFER_SIZE];

    while (1)
    {
        ssize_t bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                          (struct sockaddr *)&client_addr, &addr_len);
        if (bytes_received < 0) {
            perror("recvfrom failed");
            continue; // Keep running even if one receive fails
        }
        buffer[bytes_received ] = 0;
        #if 1
        handleShuntMessage( buffer, bytes_received );
        #else
        printMessage( buffer, bytes_received, checksum, &client_addr );
        #endif
    }
    return 1;
}

#endif
// -----------------------------------------------------------------------------
// MQTT


int handleMQTTmessage( char *buffer, int bytes_received )
{
    static int  messages = 0, cserr = 0, count = 0;

    // Null-terminate the received data for safe printing and string operations
    if ( bytes_received > 0 ) {  buffer[ bytes_received ] = 0;  }
    else                      {  return -1;                     }

    messages += 1;

    if ( ++count >= conf.Naverage )
    {
        time_t seconds = time(NULL) - conf.start_time;

        printTime( seconds, conf.time_scale );
        printf(",  %s", buffer);
//      printf("\n");
        fflush( NULL );
        sync();
        count = 0;
    }
    return 0;
}


void on_connect(UNUSED struct mosquitto *mosq, UNUSED void *obj, int rc)
{
    if (rc == 0) {
        printf("# Connected to broker.\n");
        mosquitto_subscribe(mosq, NULL, "test/topic", 0);
    } else {
        fprintf(stderr, "# Connection failed: %d\n", rc);
    }
}


void on_message(UNUSED struct mosquitto *mosq, UNUSED void *obj, const struct mosquitto_message *msg)
{
    char *payload = reinterpret_cast<char*>( msg->payload );

    payload[ msg->payloadlen ] = 0;
    #if 1
//  handleShuntMessage( reinterpret_cast<char*>( msg->payload ), msg->payloadlen );
    handleMQTTmessage( reinterpret_cast<char*>( msg->payload ), msg->payloadlen );
    #else
    printf("Topic: %s | Message(%d bytes): %s\n", msg->topic, msg->payloadlen, (char *)msg->payload);
    #endif // 1
}


void* thread_mqtt( void* arg )
{
    struct mosquitto *mosq;

    printf("# MQTT client connect to %s\n", reinterpret_cast<char*>( arg ) );

    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Failed to create mosquitto instance.\n");
        return NULL;
    }

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    if (mosquitto_connect(mosq, conf.mqtt_broker, MQTT_PORT, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to connect.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return NULL;
    }

    //Subscribe to any channel that ends with ambient_data
//  mosquitto_subscribe(mosq, NULL, "home/+/ambient_data", 0);
//  mosquitto_subscribe(mosq, NULL, "#", 0);
    mosquitto_subscribe(mosq, NULL, conf.topic, 0);

    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return NULL;
}

// -----------------------------------------------------------------------------

void parse_args( int argc, char *argv[] )
{
    for ( int ix = 0; ix < argc; ix++ )
    {
             if ( !strcmp(argv[ix], "-l") ) {
                 conf.start_time = 1768374260;            // 2026-1-14 9:05
                 conf.time_scale = SCALE_DAY;
                 conf.Naverage   = 60;
             }
        else if ( !strcmp(argv[ix], "-a") )  { conf.Naverage   = atoi( argv[++ix] );    }   // N sample average (one sample per sec)
        else if ( !strcmp(argv[ix], "-t") )  { strcpy( conf.topic, argv[++ix] );        }
    }
}


int main(int argc, char *argv[])
{
    pthread_t thread_id;
    int       sockfd;

    /* Determine port from arguments or use default
    if (argc == 2) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number. Must be 1-65535.\n");
            return EXIT_FAILURE;
        }
    }
    */
    conf.start_time = time(NULL);
    parse_args(argc, argv);
    printf("# time %ld\n", time(NULL) );

 // if ( conf.mode_mqtt )
    {
        // Create a thread
        if ( pthread_create(&thread_id, NULL, thread_mqtt, conf.mqtt_broker) != 0 )  {
            perror("Failed to create thread");
            return 1;
        }
    }
    // Wait for the thread to finish
    pthread_join(thread_id, NULL);
    return EXIT_SUCCESS;
}

//==================================================================================
#if 0

#include <stdio.h>
#include <time.h>


int main(void) {
    struct timespec t1 = { .tv_sec = 5, .tv_nsec = 500000000 }; // 5.5 sec
    struct timespec t2 = { .tv_sec = 8, .tv_nsec = 200000000 }; // 8.2 sec
    struct timespec diff;

    if (diff_timespec(&t1, &t2, &diff) == 0) {
        printf("Difference: %ld.%09ld seconds\n",
               diff.tv_sec, diff.tv_nsec);
    } else {
        printf("Error: end time is before start time or invalid input.\n");
    }

    return 0;
}

#endif
