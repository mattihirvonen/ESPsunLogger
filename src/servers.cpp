
#include <LittleFS.h>             // Or FFat.h or/and SD.h
#include <threadSafeFS.h>         // Include thread-safe wrapper since LittleFS, FFat and SD file systems are not thread safe
#include "serversConfig.h"        // Local config enable/disable telnet server's built in command set
#include <telnetServer.h>
#include <ntpClient.h>            // NTP client is needed only for time commands
#include <ftpServer.h>
#include "pinMap.h"               // LED, BUTTON, AIN0, AIN1, ...

extern threadSafeFS::FS TSFS;

telnetServer_t  *telnetServer = NULL;
ftpServer_t     *ftpServer    = NULL;


// Provide callback function that would handle user-defined commands
String telnetCommandHandlerCallback (int argc, char *argv [], telnetServer_t::telnetConnection_t *tcn)
{
    #undef  LED_BUILTIN
    #define LED_BUILTIN  LED

    // Must be reentrant !!!

    #define argv0is(X) (argc > 0 && !strcmp (argv[0], X))  
    #define argv1is(X) (argc > 1 && !strcmp (argv[1], X))
    #define argv2is(X) (argc > 2 && !strcmp (argv[2], X))   

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

    // Unhandeled - let the Telnet server try to handle the command itself
    return "";
}

//-----------------------------------------------------------------------------------------------------------------------------

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


void blink_led( int32_t now, int wifi_accesspoint )
{
    if ( wifi_accesspoint )
    {
        #define BLINK   1000L //   [ms]
        static int      ledstate = 0;
        static int32_t  blink    = 0;

        if ( (int32_t)(now - blink) >= BLINK ) {
            blink    += BLINK;
            ledstate ^= 1;
            digitalWrite( LED, ledstate );    // Toggle the LED on/off
        }
    }
}
