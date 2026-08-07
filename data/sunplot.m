%---------------------------------------------------------------
% Example function of Octave data handling commands
% This script plot measurement results from matrix: "sundata"
%
% Octave can use '#' and '%' for comment
% (but my editor show only '%' lines coloured as comment)
%---------------------------------------------------------------


function sunplot( varargin )

    % varargin is a cell array containing all extra arguments
    numArgs   = nargin;  % Number of input arguments
    timeshift = 0;

    if numArgs < 1
        fprintf('ERROR: sunplot() function requires at leat one argument (sun data matrix)\n');
        return;
    end;

    if numArgs > 1
        timeshift = varargin{2};
    end;

    sundata = varargin{1};

    % Pick matrix columns to named vectors
    time       = sundata(:, 1);   % [h]
    intensity  = sundata(:, 2);   % [%]  solar intensity of 950 W/m2 (Finland summer time)
    cumulative = sundata(:, 3);   %
    sample     = sundata(:, 4);   % sample number (increment by 1 per second)
    Udiff      = sundata(:, 5);   % [mV] Udiff = U(Rshunt) = Upanel - Udiode
    Upanel     = sundata(:, 6);   % [mV]
    Udiode     = sundata(:, 7);   % [mV]
    Debug      = sundata(:, 8);   % filtered(Udiff) - analogReadMilliVolts()

    %  Add timeshift value (to X axis)
    % (for example to change plot's X axis to show wall clock time)
    time = time + timeshift;

%   plotyy( time, intensity, time, cumulative );
%   plotyy( time, Udiff,     time, intensity );
    plotyy( time, Udiff,     time, cumulative );

    grid on
    title('Solar Intensity Measurement');
    xlabel('Time [h]');
    ylabel('[mV]');
    % ylabel(ax(1), '[mV]');               % not supported
    % ylabel(ax(2), 'Cumulative');         % not supported

    hold on;
    plot(   time, Upanel );
    plot(   time, Udiode );
%   plotyy( time, Upanel, time, Udiode );
    hold off

    legend('Ushunt', 'Cumulative', 'Udiode', 'Upanel');

endfunction
