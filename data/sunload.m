% Example script of Octave data handling commands.
% This script load two mqttLogger measurement results
% log files and combine files to result data matrix "sundata"
% for ploting using next scrip (sunplot.m)
%
% Octave can use '#' and '%' for comment
% (but my editor show only '%' lines coloured as comment)

% Load measurement data files (skip first 5 lines garbage)
M1 = load('aurinko_2026-07-26_1.log', '-ascii', 'skiprows',    '5');
M2 = load('aurinko_2026-07-26_3.log', '-ascii', 'headerlines', '5');

% Calculate time shift in seconds to hours
% Read sample number from second column of M2
% Update sample number manually here:
samplenumber = 6017
timeshift    = samplenumber / 3600;

%  Add "timeshift" value to first column
% (for example to change plot's X axis to show wall clock time)
M2(:, 1) = M2(:, 1) + timeshift;

# disp(M2);

% Vertical concatenation (append rows) - Append M2 below M1
M3 = [M1; M2];

% Debug test command to verify result matrix using text editor
# save('matrix.txt', 'M3', '-ascii');

%Copy measurement data matrix to standard name "sundata" for next script
sundata = M3;

% Call next script to plot measured sun data from matrix "sundata"
sunplot
