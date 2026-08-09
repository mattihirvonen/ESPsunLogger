
% Example: Sydney, Australia, December 21
lat    = -33.8688;
lon    = 151.2093;
tz     = +11;         % AEDT
day    = 355;         % Dec 21
time   = 0:0.10:24;   % every 15 minutes
albedo = 0.25;        % slightly reflective ground


% Example: Hiekkaharju, Finland, about begin of august
lat    = 60.3000;
lon    = 25.0500;
tz     = +3;          % EET2+summertime
day    = 221;         % about end of july
time   = 0:0.10:24;   % every 15 minutes
albedo = 0.00;        % no reflective ground


% Code	Color	RGB Triplet
% 'r'	Red     [1, 0, 0]
% 'g'	Green   [0, 1, 0]
% 'b'	Blue    [0, 0, 1]
% 'c'	Cyan    [0, 1, 1]
% 'm'	Magenta	[1, 0, 1]
% 'y'	Yellow  [1, 1, 0]
% 'k'	Black   [0, 0, 0]
% 'w'	White   [1, 1, 1]
%
% Plot example in Octave:
%
% Sample data
% x = 0:0.1:2*pi;
%
% Multiple plots with short color codes
% plot(x, sin(x),   'r', ...    % red
%      x, cos(x),   'b--', ...  % blue dashed
%      x, sin(2*x), 'g:');      % green dotted
%
% grid on;
% legend('sin(x)', 'cos(x)', 'sin(2x)');
% title('Octave Plot with MATLAB-style Color Codes');
%
% Notes:
%
% You can combine a color code with a line style (e.g., 'r--' for red dashed).
% For more precise colors, you can use RGB triplets like [0.2, 0.6, 0.8] instead of short codes.
% If you want, I can give you a full Octave color/style cheat sheet that includes markers,
% line styles, and RGB usage. Would you like me to prepare that?


[I_direct, I_diffuse, I_reflected, I_total ] = ...
    solar_irradiance_day(lat, lon, tz, day, time, tilt, az, albedo);

% Plot
figure;
plot(time, I_total,     'r--', 'LineWidth', 1);    % Red
hold on;
plot(time, I_direct,    'k',   'LineWidth', 1);    % Black
plot(time, I_diffuse,   'b..', 'LineWidth', 1);    % Blue
plot(time, I_reflected, 'g:',  'LineWidth', 2);    % Green
hold off;
xlabel('Local Time [hours]');
ylabel('Irradiance [W/m²]');
title(sprintf('Irradiance on Day %d at %.2f°N', day, lat));
grid on;
