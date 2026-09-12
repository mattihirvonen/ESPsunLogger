
#include <LittleFS.h>             // Or FFat.h or/and SD.h
#include <threadSafeFS.h>         // Include thread-safe wrapper since LittleFS, FFat and SD file systems are not thread safe
#include "serversConfig.h"        // Local config enable/disable telnet server's built in command set
#include <telnetServer.h>
#include <ntpClient.h>            // NTP client is needed only for time commands
#include <ftpServer.h>
#include "pinMap.h"               // LED, BUTTON, AIN0, AIN1, ...
#include "measure.h"              // measure_start()
#include "INA219.h"

extern threadSafeFS::FS TSFS;

telnetServer_t  *telnetServer = NULL;
ftpServer_t     *ftpServer    = NULL;

extern logger_t  logger;
extern INA219    INA();

static int print_logdata( telnetServer_t::telnetConnection_t *tcn );
static int write_logfile( char *filename, telnetServer_t::telnetConnection_t *tcn, int float32 );


// Provide callback function that would handle user-defined commands
String telnetCommandHandlerCallback (int argc, char *argv [], telnetServer_t::telnetConnection_t *tcn)
{
    #undef  LED_BUILTIN
    #define LED_BUILTIN  LED

    // Must be reentrant !!!

    #define argv0is(X) (argc > 0 && !strcmp (argv[0], X))  
    #define argv1is(X) (argc > 1 && !strcmp (argv[1], X))
    #define argv2is(X) (argc > 2 && !strcmp (argv[2], X))   
    #define argv3is(X) (argc > 3 && !strcmp (argv[3], X))   

    // Short-running functions should return the text the Telnet server will send to the client as a response to the command
    if (argv0is ("turn") && argv1is ("led") && argv2is ("on")) {
            digitalWrite (LED_BUILTIN, LED_ON);
            return "Led is on";
    } else if (argv0is ("turn") && argv1is ("led") && argv2is ("off")) {
            digitalWrite (LED_BUILTIN, LED_OFF);
            return "Led is off";
    }

    // Long-running functions should provide a mechanism to break the loop 
    else if (argv0is ("led") && argv1is ("state")) {
            for (int i = 0; i < 1000; i++)
            {
                char buf [6];
                sprintf (buf, "%s", digitalRead (LED_BUILTIN) ? "on\r\n" : "off\r\n");
                if (tcn->sendString (buf) <= 0) {
                    return "\r";
                }
                delay (250); 

                if (tcn->peekChar ()) {
                    tcn->recvChar ();
                    return "\r"; // break the loop and return something different than "" to let the telnet server function know that the command has been processed
                }
            }
            return "\r"; // return something different than "" to let the telnet server function know that the command has been processed
    }
    else if (argv0is ("log") && argv1is ("period")) {
        char    buf [80];
        uint32_t ms = 0;
        if ( argc >= 3 ) {
            ms = atoi( argv[2] );
            if ( (ms < 10) || (1000 < ms) ) {
                ms = 50;
            }
            measure_period(ms);
        }
        else {
            ms = measure_period( 0 );
            snprintf(buf, sizeof(buf), "%d ms", ms);
            tcn->sendString (buf);
        }
        return "\r";
    }
    else if (argv0is ("log") && argv1is ("start")) {
        uint32_t seconds = 10;
        if ( argc >= 3 ) {
            seconds = atoi( argv[2] );
        }
        measure_start( seconds );
        return "\r";
    }
    else if (argv0is ("log") && argv1is ("print")) {
        print_logdata( tcn );
        // return something different than empty string "" to let the telnet server function know that the command has been processed
        return "\r";
    }
    else if (argv0is ("log") && argv1is ("save")) {
        if ( argc == 3 ) {
            if ( write_logfile(argv[2], tcn, 0) ) {
                 return "\r";   // OK
            }
        }
        if ( (argc >= 4) && argv3is ("float")) {
            if ( write_logfile(argv[2], tcn, 1) ) {
                 return "\r";   // OK
            }
        }
    }
    /*
    else if (argv0is ("ina") && argv1is ("read")) {
        char buf[32];
        if ( argc < 3 ) return "";
        int reg = atoi( argv[2] );
        int value = INA.reg( 1, reg );
        snprintf(buf, sizeof(buf), "%d", value);
        if (tcn->sendString (buf) <= 0) {
            return "\r";
        }
    }
    else if (argv0is ("ina") && argv1is ("write")) {
        if ( argc < 4 ) return "";
        uint8_t reg   = atoi( argv[2] );
        int16_t value = atoi( argv[3] );
        INA.reg( 2, reg, value );
        return " ";
    }
    */

    // Unhandeled - let the Telnet server try to handle the command itself
    return "";
}

//-----------------------------------------------------------------------------------------------------------------------------

// "filename" must be with absolute path (start with '/' character)
static int check_filename( const char *filename, telnetServer_t::telnetConnection_t *tcn )
{
    if ( *filename != '/' ) {
        tcn->sendString ("Error: Filename must be with absolute path! (start with '/' character)\r\n");
        return 0;
    }
    return 1; // OK
}


static int print_logdata(  telnetServer_t::telnetConnection_t *tcn )
{
    int  period_ms = measure_period( 0 );

    for (int i = 0; i < logger.samples; i++)
    {
        char  buf [80];
        float time_s  = (period_ms * i)    / 1000.0;
        float voltage =  logger.data[i].mV / 1000.0;
        float current =  logger.data[i].mA / 1000.0;

        snprintf (buf, sizeof(buf), "%.3f %.3f %.3f\r\n", time_s, voltage, current);
        if (tcn->sendString (buf) <= 0) {
            return 1;
        }
        delay (20); 

        if (tcn->peekChar ()) {
            tcn->recvChar ();
            return 1;           // break the loop
        }
    }
    return 1;
}


// Write binary log file
// Simplify file format to 32 bit integers/floats, which are easy to read into Octave.
static int write_logfile (char *filename, telnetServer_t::telnetConnection_t *tcn, int float32)
{
    #define COLUMNS 3

    if ( ! check_filename(filename, tcn) ) {
        return 0;
    }
    // Use thread-safe wrapper as you would use LittleFS in your code
    File   f = TSFS.open (filename, "w");
    if ( ! f ) {
        return 0;
    }

    int       period_ms = measure_period( 0 );
    int32_t   columns[COLUMNS];
    float    *f32 = (float*)   columns;
    uint8_t  *buf = (uint8_t*) columns;

    for ( int i = 0; i < logger.samples; i++ ) {

        // Write int32 binary log file (little endian)
        // Simplify file format to 32 bit integers, which are easy to read into Octave.
        columns[0] = period_ms * i;
        columns[1] = logger.data[i].mV;
        columns[2] = logger.data[i].mA;

        // Convert int32 to "float" binary log file
        // Simplify file format to 32 bit floats, which are easy to read into Octave.
        if ( float32 ) {
            for ( int j = 0; j < COLUMNS; j++ ) {
                f32[j] = columns[j];
            }
            f32[0] /= 1000.0;   // Scale [ms] to seconds
        }
        f.write ( buf, sizeof(columns) );
    //  f.print ("This is a test file.");
    }
    f.close ();
    return 1;
}

//-----------------------------------------------------------------------------------------------------------------------------

void setup_servers( int wifi_accesspoint )
{
    setup_telnetServer();
    setup_ntpClient( wifi_accesspoint );
    setup_ftpServer();
}


void setup_telnetServer( void )
{
    // Create Telnet server instance that would use thread-safe wrapper arround LittleFS (or FFat or SD)
    telnetServer = new (std::nothrow) telnetServer_t (TSFS, // optional arguments:
                                                      NULL, telnetCommandHandlerCallback, 23, NULL, true);
                                                      // Cstring<255> (*getUserHomeDirectory) (const Cstring<64>& userName, const Cstring<64>& password) = NULL
                                                      // String (*telnetCommandHandlerCallback) (int argc, char *argv [], telnetConnection_t *tcn) = NULL
                                                      // int serverPort = 23
                                                      // bool (*firewallCallback) (char *clientIP, char *serverIP) = NULL
                                                      // bool runListenerInItsOwnTask = true

    // Check if Telnet server instance is created && Telnet server is running
    if (telnetServer && *telnetServer)  Serial.println ("Telnet server started");
    else                                Serial.println ("Telnet server did not start");
}


void setup_ntpClient( int wifi_accesspoint )
{
    if ( ! wifi_accesspoint )
    {
        // Setting the time is only important for time commands
        // Select another (POSIX) time zones: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
        setenv ("TZ", "CET-2CEST,M3.5.0,M10.5.0/3", 1);
        ntpClient_t ntpClient ("1.si.pool.ntp.org", "2.si.pool.ntp.org", "3.si.pool.ntp.org");
        ntpClient.syncTime ();
    }
}


void setup_ftpServer( void )
{
    // Create FTP server instance that would use thread-safe wrapper arround LittleFS (or FFat or SD)
    ftpServer = new (std::nothrow) ftpServer_t (TSFS);  // optional arguments:
                                                        //    Cstring<255> (*getUserHomeDirectory) (const Cstring<64>& userName, const Cstring<64>& password) = NULL
                                                        //    int serverPort = 21
                                                        //    bool (*firewallCallback) (char *clientIP, char *serverIP) = NULL
                                                        //    bool runListenerInItsOwnTask = true

    // Check if FTP server instance is created && FTP server is running
    if (ftpServer && *ftpServer)  Serial.println ("FTP server started");
    else                          Serial.println ("FTP server did not start");
}
