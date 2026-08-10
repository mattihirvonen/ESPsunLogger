## ==================================================

## File: solar_irridance.m
## Usage:
##   [I_direct, I_diffuse, I_reflected, I_total] = ...
##       solar_irradiance(lat_deg, lon_deg, tz_offset, day_of_year, local_time [, albedo]);
##
## lat_deg     : Latitude in degrees (-90 to 90)
## lon_deg     : Longitude in degrees (-180 to 180, East positive)
## tz_offset   : Time zone offset from UTC in hours
## day_of_year : Day of year (1–365 or 366)
## local_time  : Local clock time in decimal hours (0–24)
## albedo      : (Optional) Ground reflectivity (0–1, default 0.20)
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
##  tz     = +11;        % AEDT
##  day    = 355;        % Dec 21
##  time   = 13;         % 1 PM local time
##  albedo = 0.2;        % slightly reflective ground
##
##  [I_direct, I_diffuse, I_reflected, I_total] = ...
##      solar_irradiance(lat, lon, tz, day, time, tilt, az, albedo );

## Irradiance  tells you how bright the sun is right now.
## Irradiation tells you how much total energy the sun delivered over a given stretch of time.

## ==================================================

function [I_direct, I_diffuse, I_reflected, I_total, altitude, azimuth, LHA, GHA, declination] = ...
    solar_irradiance( lat_deg, lon_deg, tz_offset, day_of_year, local_time ,varargin )

  % Irradiance calibration fix by MH (Finland summer time):
  % - Suncalc.org web site 944 W/m2, unmodified code 1099 W/m2
  irradiation_calibration_fix = 944 / 1085;

  % --- Input validation ---
  if nargin < 5
    error("Usage: solar_irradiance(lat, lon, tz_offset, day_of_year, local_time [, albedo])");
  endif
  if any(~isnumeric([lat_deg, lon_deg, tz_offset, day_of_year])) || ...
     abs(lat_deg) > 90 || abs(lon_deg) > 180 || abs(tz_offset) > 14 || ...
     day_of_year < 1 || day_of_year > 366
    error("Invalid geographic or date parameters.");
  endif
  if ~isnumeric(local_time) || any(local_time < 0) || any(local_time > 24)
    error("Time must be between 0 and 24 hours.");
  endif

  % Optional parameters
  albedo = 0.20;   % default ground reflectivity
  if length(varargin) >= 1, albedo = varargin{1}; endif

  %--------------------------------------------------------------------
  % --- Constants ---
% G_sc = 1367;                                % Solar constant [W/m²]
  G_sc = 1367 * irradiation_calibration_fix;  % Solar constant [W/m²], fix by MH
  lat_rad = deg2rad(lat_deg);

  %--------------------------------------------------------------------
  % Get sun position on the sky

  [altitude, azimuth, LHA, GHA, declination] = ...
      solar_position( lat_deg, lon_deg, tz_offset, day_of_year, local_time );

  elev_rad = deg2rad( altitude );

  %--------------------------------------------------------------------
  % --- Earth–Sun distance factor ---
  dist_factor = 1 + 0.033 * cos(deg2rad(360 * day_of_year / 365));

  % --- Air mass ---
  m = zeros(size(elev_rad));
  idx = elev_rad > 0;
  m(idx) = 1 ./ (sin(elev_rad(idx)) + 0.50572 .* (rad2deg(elev_rad(idx)) + 6.07995).^(-1.6364));

  % --- Direct Normal Irradiance (DNI) ---
  DNI = zeros(size(elev_rad));
  DNI(idx) =  G_sc * dist_factor .* exp(-0.14 .* m(idx));

  % --- Diffuse Horizontal Irradiance (empirical fraction) ---
  DHI = zeros(size(elev_rad));
  DHI(idx) = 0.1 * DNI(idx) + 0.2 * G_sc * dist_factor .* sin(elev_rad(idx));

  % --- Direct component ---
  I_direct = DNI;

  % --- Diffuse component (isotropic model) ---
% I_diffuse = DHI .* (1 + cos(tilt_rad)) / 2;
  I_diffuse = DHI;

  % --- Ground-reflected component ---
% I_reflected = (DNI .* sin(elev_rad) + DHI) .* albedo .* (1 - cos(tilt_rad)) / 2;
  I_reflected = (DNI .* sin(elev_rad) + DHI) .* albedo;

  % --- Total irradiance ---
  I_total = I_direct + I_diffuse + I_reflected;

endfunction


