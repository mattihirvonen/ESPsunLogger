
# mqttLogger
**mqttLogger** receive MQTT messages from (mosquitto) MQTT message broker and print
message contents to stdout with time stamp. Typically these message contents
are formated as numerical data rows of matrix. Redirect this print out to file.
Later file data can visualize and manipulate using GnuPlot or Octave
(free Matlab "look like").

Mosquitto (or similar) message brpoker and mqttLogger application can run in Rasberry PI,
Linux virtual machine, Proxmox LXC container or as in my case QNAP NAS ContainerStation
LXD container (Ubuntu 24.04).Even oldest first Raspberry Pi model can run Mosquitto
and mqttLogger applications. Typically this kind server is easier to setup in Linux
environment than in Windows (of course it is possible setup also in windows,
but I have not tested).

**mqttLogger** command line option(s)
- __*-t topic*__   set message topic filter (default is all by wild card  __*#*__)
- __*-h host*__    set MQTT broker host name or IP address (default is localhost)
- __*-W offset*__  set time column as wall clock time  UTC + "offset" hours


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

### Using Octave to Plot Data
Subdirectory **data** contain some Octave scripts to help ploting captured *mqttLogger* data.
- *sunload.m*  &emsp; main function to read captured mqttLogger data (function calls sunplot)
- *sunplot.m*  &emsp; helper function to plot sun data matrix

Example command(s) to use functin from Octave's command line
- octave:1> &emsp; sunload("datafile.txt")    &emsp; &emsp; % Plot "datafile.txt"
- octave:2> &emsp; sunload("datafile.txt", 8) &emsp; &emsp; % Add 8h time shift to X axis

### Octave Function and GnuPlot Scripts
**data** directory contains some handy function script for Octave an GnuPlot.
- _solar_intensity.m_         &emsp;  Octave function to calculate solar irridance versus time
- _solar_panel_intensity.m_   &emsp;  Octave function to calculate solar panel irridance versus time with panels orientation (under construction)
- _sunload.m_                 &emsp;  Octave script to load and plot sunLogger/mqttlogger captured data
- _sunplot.m_                 &emsp;  Octave function to plot sunLogger/mqttlogger captured data matrix
- _gnuplot.bat_               &emsp;  Windows command line script to GnuPlot sunLogger/mqttlogger captured data
- _gnuplot_script.gp_         &emsp;  GnuPlot command scrip (used by "gnuplot.bat")
- _swapcols.c_                &emsp;  Helper application to swap numerical data text file's columns

### Octave Command Examples
Load numerical matrix data (text file) into memory matrix "*__M1__*"
- M1 = load('sundata1.log');
- M2 = load('sundata2.log');

Add time shift value to the first column of matrix
- timeshift = 6017 / 3600; &emsp; *% calculate time shift in seconds to hours*
- M2(:, 1) = M2(:, 1) + timeshift;

Display result
- disp(M2);

Vertical concatenation (append rows)
- M3 = \[M1; M2\]; &emsp; *% Append M2 below M1*

Save matrix to a text file in ASCII format
- save('matrix.txt', 'M3', '-ascii');
- save('matrix.txt', 'M3', '-ascii', '-append');
- save('matrix.txt', 'M3', '-ascii', '-double');
- dlmwrite('matrix.txt', M3, 'delimiter', '\t', 'precision', 6);

Some plot commands
- plot(M3(:, 1), M3(:, 3));                  &emsp; *% Plot the curve*
- xlabel('Time \[h\]');                      &emsp; *% Label X-axis*
- ylabel('\[mV\]');                          &emsp; *% Label Y-axis*
- title('Solar Intensity Measurement');
- grid on;                                   &emsp; *% Add grid*
- hold on;                                   &emsp; *% Add / hold multiple plots*
- plot(M3(:, 1), M3(:, 6));                  &emsp; *% Plot the curve*
- plot(M3(:, 1), M3(:, 7));                  &emsp; *% Plot the curve*
- hold off;
- legend('Ushunt', 'Upanel', 'Udiode');
