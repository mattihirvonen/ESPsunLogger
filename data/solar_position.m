## ==================================================

## File: solar_position.m
## Usage:
##   [altitude, azimuth, LHA, GHA, declaration] = ...
##       solar_position(lat_deg, lon_deg, tz_offset, day_of_year, local_time);
##
## lat_deg     : Latitude in degrees (-90 to 90)
## lon_deg     : Longitude in degrees (-180 to 180, East positive)
## tz_offset   : Time zone offset from UTC in hours
## day_of_year : Day of year (1–365 or 366)
## local_time  : Local clock time in decimal hours (0–24)
##
## Returns:
##   altitude     : Sun altitude over horizon [degrees]
##   azimuth      : Sun direction from north [degrees]
##   LHA          : Local Hour Angle of sun [degrees]
##   GHA          : Greewitch Hour Angle of sun [degrees]
##   declination  : Sun declination north/south of latitude=0 [degrees]
##
##  Example: Sydney, Australia, December 21 at 1 PM
##  lat    = -33.8688;
##  lon    = 151.2093;
##  tz     = +11;        % AEDT
##  day    = 355;        % Dec 21
##  time   = 13;         % 1 PM local time
##
##  [altitude, azimuth, LHA, GHA, declination] = ...
##      solar_position(lat, lon, tz, day, time );

## Irradiance  tells you how bright the sun is right now.
## Irradiation tells you how much total energy the sun delivered over a given stretch of time.

## ==================================================

function [altitude, azimuth, LHA, GHA, declination] = ...
    solar_position( lat_deg, lon_deg, tz_offset, day_of_year, local_time )

  % --- Input validation ---
  if nargin < 5
    error("Usage: solar_position(lat, lon, tz_offset, day_of_year, local_time )");
  endif
  if any(~isnumeric([lat_deg, lon_deg, tz_offset, day_of_year])) || ...
     abs(lat_deg) > 90 || abs(lon_deg) > 180 || abs(tz_offset) > 14 || ...
     day_of_year < 1 || day_of_year > 366
    error("Invalid geographic or date parameters.");
  endif
  if ~isnumeric(local_time) || any(local_time < 0) || any(local_time > 24)
    error("Time must be between 0 and 24 hours.");
  endif

  %--------------------------------------------------------------------

  % --- Constants ---
  lat_rad = deg2rad(lat_deg);

  % --- Constants ---
  axial_tilt = 23.44; % Earth's axial tilt in degrees
  days_in_year = 365; % Approximation (ignores leap year effect on declination)

  % --- Sun declination ---
  % Formula: decl = -23.44 * cos( (360/365) * (n + 10) )
  decl_deg = -axial_tilt * cosd((360 / days_in_year) * (day_of_year + 10));
  decl_rad = deg2rad( decl_deg );

  % --- Equation of time ---
  B = deg2rad(360 * (day_of_year - 81) / 364);
  eq_time_min = 9.87*sin(2*B) - 7.53*cos(B) - 1.5*sin(B);

  % --- GHA ---
  HA      = local_time - tz_offset + eq_time_min / 60;
  GHA_deg = 15 * (HA + 12);
  GHA_deg = mod( GHA_deg + 360, 360);
  GHA_rad = deg2rad( GHA_deg );

  % --- LHA ---
  LHA_deg = GHA_deg + lon_deg;
  LHA_deg = mod( LHA_deg + 360, 360);
  LHA_rad = deg2rad( LHA_deg );

  % --- Sun altitude ---
  % "Piloting/Navigation with the Pocket Calculator" page 171
  sinHc = ( sin(decl_rad) * sin(lat_rad) ) + ...
          ( cos(decl_rad) * cos(lat_rad)   * cos(LHA_rad) );

  Hc_rad = asin(max(sinHc, 0));

  % --- Sun altitude over horizon ---
  altitude_deg = rad2deg( Hc_rad );

  % --- Sun azimuth ---
  % "Piloting/Navigation with the Pocket Calculator" page 171
  cosZ = ( sin(decl_rad) - sin(lat_rad) * sin(Hc_rad) ) / ...
         ( cos(Hc_rad) * cos(lat_rad) );

  azimuth_rad = acos(cosZ);
  if sin(LHA_rad) < 0
     azimuth_deg = rad2deg( azimuth_rad );
  else
     azimuth_deg = 360 - rad2deg(azimuth_rad);
  endif

  altitude    = altitude_deg;
  azimuth     = azimuth_deg;
  LHA         = LHA_deg;
  GHA         = GHA_deg;
  declination = decl_deg;

endfunction


