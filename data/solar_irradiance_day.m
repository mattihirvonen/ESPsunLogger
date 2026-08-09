## ==================================================

## File: solar_panel_intensity.m
## Usage:
##   [I_total, I_direct, I_diffuse, I_reflected] = ...
##       solar_irridance_day(lat_deg, lon_deg, tz_offset, day_of_year[, time_hours [, albedo]]);
##
## lat_deg     : Latitude in degrees (-90 to 90)
## lon_deg     : Longitude in degrees (-180 to 180, East positive)
## tz_offset   : Time zone offset from UTC in hours
## day_of_year : Day of year (1–365 or 366)
## time_hours  : (optional) Vector of local clock times in decimal hours (0–24)
## albedo      : (Optional) Ground reflectivity (0–1, default 0.2)
##
## Returns:
##   I_total    : Total irradiance on surface [W/m²]
##   I_direct   : Direct beam component [W/m²]
##   I_diffuse  : Diffuse sky component [W/m²]
##   I_reflected: Ground-reflected component [W/m²]
##
##  Example: Sydney, Australia, December 21 at 1 PM
##  lat    = -33.8688;
##  lon    = 151.2093;
##  tz     = +11;           % AEDT
##  day    = 355;           % Dec 21
##  time   = 0:0.10:24;     % every 6 minutes
##  albedo = 0.25;          % slightly reflective ground
##
##  [I_total, I_direct, I_diffuse, I_reflected] = ...
##      solar_irridance_day(lat, lon, tz, day, time, albedo);

## ==================================================


function [I_direct, I_diffuse, I_reflected, I_total] = ...
    solar_irradiance_day(lat_deg, lon_deg, tz_offset, day_of_year, varargin)

  % Defaults for optional parameters
  time_hours = 0:0.1:24   % default local time vector
  albedo     = 0.0;       % default ground reflectivity (0.25)
  %
  if length(varargin) >= 1, time_hours = varargin{1}; endif
  if length(varargin) >= 2, albedo     = varargin{2}; endif

  % --- Input validation ---
  if nargin < 4
    error("Usage: solar_irradiance_day(lat, lon, tz_offset, day_of_year [, time_hours [, albedo]])");
  endif
  if any(~isnumeric([lat_deg, lon_deg, tz_offset, day_of_year])) || ...
     abs(lat_deg) > 90 || abs(lon_deg) > 180 || abs(tz_offset) > 14 || ...
     day_of_year < 1 || day_of_year > 366
    error("Invalid geographic or date parameters.");
  endif
  if ~isnumeric(time_hours) || any(time_hours < 0) || any(time_hours > 24)
    error("Time must be between 0 and 24 hours.");
  endif

  [I_direct, I_diffuse, I_reflected, I_total] = ...
      solar_irradiance(lat_deg, lon_deg, tz_offset, day_of_year, time_hours, albedo);
	  
endfunction
