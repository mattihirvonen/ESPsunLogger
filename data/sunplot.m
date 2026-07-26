
# Example script of Octave data handling commands

M1 = load('aurinko_2026-07-26_1.log');
M2 = load('aurinko_2026-07-26_3.log');

# calculate time shift in seconds to hours
timeshift = 6017 / 3600;

# Add value to first column
M2(:, 1) = M2(:, 1) + timeshift;

# disp(M2);

# Vertical concatenation (append rows) - Append M2 below M1
M3 = [M1; M2];

save('matrix.txt', 'M3', '-ascii');

# plot( M3(:, 1), M3(:, 3) );
plotyy( M3(:, 1), M3(:, 3), M3(:, 1), M3(:, 5) );
xlabel('Time [h]');
ylabel('[mV]');
% ylabel(ax(1), '[mV]');               % not supported
% ylabel(ax(2), 'Cumulative');         % not supported
title('Solar Intensity Measurement');
grid on

hold on;
plot( M3(:, 1), M3(:, 6) );
plot( M3(:, 1), M3(:, 7) );
% plotyy( M3(:, 1), M3(:, 6), M3(:, 1), M3(:, 7) );
hold off;
legend('Ushunt', 'Cumulative', 'Udiode', 'Upanel');