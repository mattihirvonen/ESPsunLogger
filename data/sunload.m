%-----------------------------------------------------------------------------------
%
% Call this function from octave CLI's command line like
%
%     octave:1>   sunload("filename.ext")
%     octave:2>   sunload("filename.ext", 2)   % Add 2 hour time shift to X
%
% Function return value:
% - Loaded matrix
%
% https://stackoverflow.com/questions/48022907/how-do-i-pass-a-command-line-argument-to-an-octave-function-when-calling-functio
%
%-----------------------------------------------------------------------------------

function result = sunload( varargin )

    % Some extra (obsolete) code stuff from previous tests...

    % Read incoming arguments
    % args = argv();

    % if numel(args) < 1
    %     fprintf("Error: Missing filename.\n");
    %     fprintf("Usage: sunload.m  filename  [timeshift]\n");
    %     return;
    % end

    % filename  =  args{1};
    % timeshift = 0;

    % if numel(args) > 1
    %     timeshift = args{2};
    % end

    % ---

    % varargin is a cell array containing all extra arguments
    numArgs   = nargin;  % Number of input arguments
    timeshift = 0;

    if numArgs < 1
        fprintf('ERROR: sunload() function requires at leat one argument (data file name)\n');
        return;
    end;

    if numArgs > 1
        timeshift = varargin{2};
    end;

    filename = varargin{1};

    % Check if file exists in current directory or search path
    if exist(filename, "file") == 2
        printf("Load file: '%s'\n", filename);
    else
        printf("File '%s' does not exist.\n", filename);
        return;
    end

    sundata = load(filename, '-ascii', 'headerlines', '5');
    sundata(:, 1) = sundata(:, 1) + timeshift;
    sunplot( sundata );

    result = sundata;

%   disp( sundata  );
%   disp( filename );
%   disp( timeshift);

endfunction
