%---------------------------------------------------------------
% Example function of Octave data handling commands
% This script plot measurement results from matrix: "sundata"
%
% Octave can use '#' and '%' for comment
% (but my editor show only '%' lines coloured as comment)
%---------------------------------------------------------------


function sunplot2( varargin )

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
    sample     = sundata(:, 2);   % sample number (increment by 1 per second)
    Udiff      = sundata(:, 3);   % [mV] Udiff = U(Rshunt) = Upanel - Udiode
    intensity  = sundata(:, 4);   % [%]  solar intensity of 950 W/m2 (Finland summer time)
    cumulative = sundata(:, 5);   %
    Upanel     = sundata(:, 6);   % [mV]
    Udiode     = sundata(:, 7);   % [mV]
    Debug      = sundata(:, 8);   % filtered(Udiff) - analogReadMilliVolts()

    %  Add timeshift value (to X axis)
    % (for example to change plot's X axis to show wall clock time)
    time = time + timeshift;

    plotyy( time, intensity, time, cumulative );
%   plotyy( time, Udiff,     time, intensity );
%   plotyy( time, Udiff,     time, cumulative );

    grid on
    title('Solar Measurement - Cumulative Scale: 1.0 = 1h * 950 W/m2');
    xlabel('Time [h]');
    ylabel('[%]');
    % ylabel(ax(1), '[mV]');               % not supported
    % ylabel(ax(2), 'Cumulative');         % not supported

    hold on;
%    plot(   time, Upanel );
%    plot(   time, Udiode );
%%   plotyy( time, Upanel, time, Udiode );
    hold off

    legend('Intensity [%]', 'Cumulative');

endfunction
