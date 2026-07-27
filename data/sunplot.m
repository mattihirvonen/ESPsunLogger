%---------------------------------------------------------------
% Example script of Octave data handling commands
% This script plot measurement results from named matrix: "sundata"
%
% Octave can use '#' and '%' for comment
% (but my editor show only '%' lines coloured as comment)
%---------------------------------------------------------------

%  Add value to first column
% (for example to change plot's x axis to show wall clock time)
timeshift = 6;
%
sundata(:, 1) = sundata(:, 1) + timeshift;

% Pick matrix columns to named vectors
time       = sundata(:, 1);   % [h]
sample     = sundata(:, 2);   % sample number (increment by 1 per second)
Udiff      = sundata(:, 3);   % [mV] Udiff = U(Rshunt) = Upanel - Udiode
intensity  = sundata(:, 4);   % [%]  solar intensity of 950 W/m2 (Finland summer time)
cumulative = sundata(:, 5);   %
Upanel     = sundata(:, 6);   % [mV]
Udiode     = sundata(:, 7);   % [mV]
Debug      = sundata(:, 8);   % filtered(Udiff) - analogReadMilliVolts()


# plotyy( time, intensity, time, cumulative );
# plotyy( time, Udiff,     time, intensity );
  plotyy( time, Udiff,     time, cumulative );

grid on
title('Solar Intensity Measurement');
xlabel('Time [h]');
ylabel('[mV]');
% ylabel(ax(1), '[mV]');               % not supported
% ylabel(ax(2), 'Cumulative');         % not supported

hold on;
plot( time, Upanel );
plot( time, Udiode );
% plotyy( time, Upanel, time, Udiode );
hold off

legend('Ushunt', 'Cumulative', 'Udiode', 'Upanel');