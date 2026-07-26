
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

### Octave Command Examples
Load numerical matrix data (text file) into memory matrix "*__M1__*"
- M1 = load('sundata1.log');
- M2 = load('sundata2.log');

Add time shift value to the first column of matrix
- timeshift = 6017 / 3600; &emsp;*% calculate time shift in seconds to hours*
- M2(:, 1) = M2(:, 1) + timeshift;

Display result
- disp(M2);

Vertical concatenation (append rows)
- M3 = \[M1; M2\]; &emsp;*% Append M2 below M1*

Save matrix to a text file in ASCII format
- save('matrix.txt', 'M3', '-ascii');
- save('matrix.txt', 'M3', '-ascii', '-append');
- save('matrix.txt', 'M3', '-ascii', '-double');
- dlmwrite('matrix.txt', M3, 'delimiter', '\t', 'precision', 6);


Some plot commands
- plot(M3(:, 1), M3(:, 3));                  % Plot the curve
- xlabel('Time \[h\]');                      % Label X-axis
- ylabel('\[mV\]');                          % Label Y-axis
- title('Solar Intensity Measurement');
- grid on;                                   % Add grid
- hold on;                                   % Add / hold multiple plots
- plot(M3(:, 1), M3(:, 6));                  % Plot the curve
- plot(M3(:, 1), M3(:, 6));                  % Plot the curve
- hold off;
- legend('Udiff', 'Urshunt', 'Udiode');
