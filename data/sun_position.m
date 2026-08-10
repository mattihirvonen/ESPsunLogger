##
## File:   sun_position.m
##
## WARNING:
## This function is too inaccurate for celestial navigation!
## Celestial navigation require iterative algorithm for earth's
## elliptic radius integration of traverse round sun.
##

function [sun_altitude, sun_azimuth, LHA, GHA, GMST, declination] = ...
    sun_position( lat_deg, lon_deg, tz_offset, year, month, day, local_hour, varargin )

  % Optional parameters
  minute = 0;
  second = 0;
  if length(varargin) >= 1, minute = varargin{1}; endif
  if length(varargin) >= 2, second = varargin{2}; endif

% --- Input validation ---
  if nargin < 5
    error("Usage: sun_position(lat, lon, tz_offset, day_of_year, local_hour [, minute, second])");
  endif
  if any(~isnumeric([lat_deg, lon_deg, tz_offset, ])) || ...
     abs(lat_deg) > 90 || abs(lon_deg) > 180 || abs(tz_offset) > 14
    error("Invalid geographic or time zone parameters.");
  endif
  if ~isnumeric(local_hour) || any(local_hour < 0) || any(local_hour > 24)
    error("Time must be between tz_offset and 24 hours.");
  endif

  lat_rad = deg2rad(lat_deg);
  lon_rad = deg2rad(lon_deg);

  % --- Solar declination ---
  declination = sun_declination(year, month, day);
  decl_rad    = deg2rad( declination );

  % --- Local time to UTC ---
  UTC_hour = local_hour - tz_offset;

  % --- Local solar time ---
  [GHA, GMST] = sun_GHA( year, month, day, UTC_hour, minute, second );

  % --- Hour angle ---
  LHA     = GHA + lon_deg;
  LHA     = mod(LHA, 360);    % Normalize LHA to [0, 360)
  LHA_rad = deg2rad( LHA );

  % --- Solar elevation ---
  % "Piloting/Navigation with the Pocket Calculator" page 171
  sinHc = ( sin(decl_rad) * sin(lat_rad) ) + ...
          ( cos(decl_rad) * cos(lat_rad)   * cos(LHA_rad) );

  Hc_rad = asin(max(sinHc, 0));

  % --- Sun altitude over horizon ---
  sun_altitude = rad2deg( Hc_rad );

  % --- Sun azimuth ---
  % "Piloting/Navigation with the Pocket Calculator" page 171
  cosZ = ( sin(decl_rad) - sin(lat_rad) * sin(Hc_rad) ) / ...
         ( cos(Hc_rad) * cos(lat_rad) );

  azimuth_rad = acos(cosZ);

  if sin(LHA_rad) < 0
     sun_azimuth = rad2deg(azimuth_rad);
  else
     sun_azimuth = 360 - rad2deg(azimuth_rad);
  endif

endfunction

