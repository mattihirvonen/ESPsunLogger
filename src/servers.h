#ifndef TELNET_CONFIG_H
#define TELNET_CONFIG_H

// Choose your server's name - this is how Telnet server would introduce itself to the clients
#define HOSTNAME  "esp32server"


// Choose which built-in Telnet commands will be included

#define TELNET_CLEAR_COMMAND    1   // 0=exclude, 1=include, clear is included by default
#define TELNET_UNAME_COMMAND    1   // 0=exclude, 1=include, uname is included by default
#define TELNET_FREE_COMMAND     1   // 0=exclude, 1=include, free is included by default
#define TELNET_NOHUP_COMMAND    1   // 0=exclude, 1=include, nohup is included by default
#define TELNET_REBOOT_COMMAND   1   // 0=exclude, 1=include, reboot is included by default
#define TELNET_DMESG_COMMAND    1   // 0=exclude, 1=include, dmesg is included by default
#define TELNET_QUIT_COMMAND     1   // 0=exclude, 1=include, quit is included by default
#define TELNET_UPTIME_COMMAND   1   // 0=exclude, 1=include, date is included by default
#define TELNET_DATE_COMMAND     1   // 0=exclude, 1=include, date is included by default
#define TELNET_NTPDATE_COMMAND  1   // 0=exclude, 1=include, ntpdate is included by default
#define TELNET_PING_COMMAND     1   // 0=exclude, 1=include, ping is included by default
#define TELNET_IFCONFIG_COMMAND 1   // 0=exclude, 1=include, ifconfig is included by default
#define TELNET_IW_COMMAND       1   // 0=exclude, 1=include, iw is included by default
#define TELNET_NETSTAT_COMMAND  1   // 0=exclude, 1=include, netstat is included by default
#define TELNET_KILL_COMMAND     1   // 0=exclude, 1=include, kill is included by default
#define TELNET_CURL_COMMAND     1   // 0=exclude, 1=include, curl is included by default
#define TELNET_SENDMAIL_COMMAND 1   // 0=exclude, 1=include, sendmail is included by default
#define TELNET_LS_COMMAND       1   // 0=exclude, 1=include, ls is included by default
#define TELNET_TREE_COMMAND     1   // 0=exclude, 1=include, tree is included by default
#define TELNET_MKDIR_COMMAND    1   // 0=exclude, 1=include, mkdir is included by default
#define TELNET_RMDIR_COMMAND    1   // 0=exclude, 1=include, rmdir is included by default
#define TELNET_CD_COMMAND       1   // 0=exclude, 1=include, cd is included by default
#define TELNET_PWD_COMMAND      1   // 0=exclude, 1=include, pwd is included by default
#define TELNET_CAT_COMMAND      1   // 0=exclude, 1=include, cat is included by default
#define TELNET_VI_COMMAND       1   // 0=exclude, 1=include, vi is included by default
#define TELNET_CP_COMMAND       1   // 0=exclude, 1=include, cp is included by default
#define TELNET_RM_COMMAND       1   // 0=exclude, 1=include, rm is included by default
#define TELNET_LSOF_COMMAND     1   // 0=exclude, 1=include, lsof is included by default

#define SWAP_DEL_AND_BACKSPACE  0   // seto to 1 to swap the meaning of these keys - this would be suitable for Putty and Linux Telnet clients

// Provide help text for user-defined commands
#define USER_DEFINED_TELNET_HELP_TEXT   "\r\n  LED commands:" \
                                        "\r\n      turn led on" \
                                        "\r\n      turn led off" \
                                        "\r\n      led  state" \
                                        "\r\n  LOG commands:" \
                                        "\r\n      log period [ms]" \
                                        "\r\n      log print" \
                                        "\r\n      log start  [seconds]" \
                                        "\r\n      log save   filename  [float]" \
                                        "\r\n  INA commands:" \
                                        "\r\n      ina read  register" \
                                        "\r\n      ina write register value"

void setup_servers( int wifi_accesspoint );
void setup_telnetServer( void );
void setup_ntpClient( int wifi_accesspoint );
void setup_ftpServer( void );

#endif //  TELNET_CONFIG_H
