
function LHA = sun_LHA( longitude_deg, year, month, day, UTC_hour, varargin )

  if nargin < 5
    error("Usage: sun_LHA(longitude_deg, year, month, day, UTC_hour [, minute, second])");
  end

  % Optional parameters
  minute = 0;
  second = 0;
  if length(varargin) >= 1, minute = varargin{1}; endif
  if length(varargin) >= 2, second = varargin{2}; endif

  ## Validate inputs
  if any([month < 1, month > 12, day < 1, day > 31, ...
          UTC_hour < 0, UTC_hour >= 24, minute < 0, minute >= 60, ...
          second < 0, second >= 60])
    error("Invalid date/time values.");
  end

  [GHA, GMST] = sun_GHA( year, month, day, UTC_hour, minute, second );

  % Normalize GHA and longitude to [0, 360)
  GHA = mod(GHA, 360);
  longitude_deg = mod(longitude_deg, 360);

  % Compute LHA
  LHA = GHA + longitude_deg;

  % Normalize LHA to [0, 360)
  LHA = mod(LHA, 360);

end

