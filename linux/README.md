
# mqttLogger 
**mqttLogger** command line option(s)
- __*-t topic*__ set message topic filter (default is all by wild card  __*#*__)

Start mqttLogger with command line comman and redirect output into file
- mqttLogger > sundata.log &

Use __*make*__ command to build Linux version of mqttLogger application

### Convert UNIX Timestamp (seconds) to Date
Example commands:
- date -ud @1784863249
- *Fri Jul 24 03:20:49 UTC 2026*
- date -d @1784863249
- *Fri Jul 24 06:20:49 EST+3 2026*


### Screen Command to Manage  mqttLogger in Backround
Use Screen command to put logger in background.
- https://www.tecmint.com/screen-command-examples-to-manage-linux-terminals/

Start new named screen session
- screen -S logger

Detach screen
- Ctrl-a+k

List screens
- screen -ls
- *There is a screen on:*
- *1135.logger     (07/24/26 03:57:50)     (Detached)*

Resume sceen
- screen -r

Resume screen by ID
- screen -r 1135

