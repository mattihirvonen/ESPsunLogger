
## File: solar_intensity.m
## Usage:
##   [I_total, I_direct, I_diffuse, I_reflected] = ...
##       solar_panel_intensity(lat_deg, lon_deg, tz_offset, day_of_year, time_hours [, tilt_deg, azimuth_deg, albedo])
##
## lat_deg     : Latitude in degrees (-90 to 90)
## lon_deg     : Longitude in degrees (-180 to 180, East positive)
## tz_offset   : Time zone offset from UTC in hours
## day_of_year : Day of year (1–365 or 366)
## time_hours  : Vector of local clock times in decimal hours (0–24)
## tilt_deg    : (Optional) Surface tilt from horizontal (0 = flat, 90 = vertical)
## azimuth_deg : (Optional) Surface azimuth (0 = North, 90 = East, 180 = South)
## albedo      : (Optional) Ground reflectivity (0–1, default 0.2)
##
## Returns:
##   I_total    : Total irradiance on surface [W/m²]
##   I_direct   : Direct beam component [W/m²]
##   I_diffuse  : Diffuse sky component [W/m²]
##   I_reflected: Ground-reflected component [W/m²]

function [I_total, I_direct, I_diffuse, I_reflected] = ...
    solar_panel_intensity(lat_deg, lon_deg, tz_offset, day_of_year, time_hours, varargin)

  % --- Input validation ---
  if nargin < 5
    error("Usage: solar_intensity(lat, lon, tz_offset, day_of_year, time_hours [, tilt, azimuth, albedo])");
  endif
  if any(~isnumeric([lat_deg, lon_deg, tz_offset, day_of_year])) || ...
     abs(lat_deg) > 90 || abs(lon_deg) > 180 || abs(tz_offset) > 14 || ...
     day_of_year < 1 || day_of_year > 366
    error("Invalid geographic or date parameters.");
  endif
  if ~isnumeric(time_hours) || any(time_hours < 0) || any(time_hours > 24)
    error("Time must be between 0 and 24 hours.");
  endif

  % Optional parameters
  tilt_deg = 0;   % default horizontal
  azimuth_deg = 180; % default facing south
  albedo = 0.2;   % default ground reflectivity
  if length(varargin) >= 1, tilt_deg = varargin{1}; endif
  if length(varargin) >= 2, azimuth_deg = varargin{2}; endif
  if length(varargin) >= 3, albedo = varargin{3}; endif

  % --- Constants ---
  G_sc = 1367; % Solar constant [W/m²]
  lat_rad = deg2rad(lat_deg);
  tilt_rad = deg2rad(tilt_deg);
  surf_az_rad = deg2rad(azimuth_deg);

  % --- Solar declination ---
  decl_rad = deg2rad(23.45) * sin(deg2rad(360 * (284 + day_of_year) / 365));

  % --- Equation of time ---
  B = deg2rad(360 * (day_of_year - 81) / 364);
  eq_time_min = 9.87*sin(2*B) - 7.53*cos(B) - 1.5*sin(B);

  % --- Time correction factor ---
  time_corr_min = 4 * (lon_deg - tz_offset * 15) + eq_time_min;

  % --- Local solar time ---
  lst_hours = time_hours + time_corr_min / 60;

  % --- Hour angle ---
  hour_angle_rad = deg2rad(15 * (lst_hours - 12));

  % --- Solar elevation ---
  sin_elev = sin(lat_rad).*sin(decl_rad) + cos(lat_rad).*cos(decl_rad).*cos(hour_angle_rad);
  elev_rad = asin(max(sin_elev, 0));

  % --- Earth–Sun distance factor ---
  dist_factor = 1 + 0.033 * cos(deg2rad(360 * day_of_year / 365));

  % --- Air mass ---
  m = zeros(size(elev_rad));
  idx = elev_rad > 0;
  m(idx) = 1 ./ (sin(elev_rad(idx)) + 0.50572 .* (rad2deg(elev_rad(idx)) + 6.07995).^(-1.6364));

  % --- Direct Normal Irradiance (DNI) ---
  DNI = zeros(size(elev_rad));
  DNI(idx) = G_sc * dist_factor .* exp(-0.14 .* m(idx));

  % --- Diffuse Horizontal Irradiance (empirical fraction) ---
  DHI = zeros(size(elev_rad));
  DHI(idx) = 0.1 * DNI(idx) + 0.2 * G_sc * dist_factor .* sin(elev_rad(idx));

  % --- Direct component on tilted surface ---
  cos_inc = sin(decl_rad).*sin(lat_rad).*cos(tilt_rad) ...
          - sin(decl_rad).*cos(lat_rad).*sin(tilt_rad).*cos(surf_az_rad) ...
          + cos(decl_rad).*cos(lat_rad).*cos(tilt_rad).*cos(hour_angle_rad) ...
          + cos(decl_rad).*sin(lat_rad).*sin(tilt_rad).*cos(surf_az_rad).*cos(hour_angle_rad) ...
          + cos(decl_rad).*sin(tilt_rad).*sin(surf_az_rad).*sin(hour_angle_rad);
  cos_inc = max(cos_inc, 0);
  I_direct = DNI .* cos_inc;

  % --- Diffuse component on tilted surface (isotropic model) ---
  I_diffuse = DHI .* (1 + cos(tilt_rad)) / 2;

  % --- Ground-reflected component ---
  I_reflected = (DNI .* sin(elev_rad) + DHI) .* albedo .* (1 - cos(tilt_rad)) / 2;

  % --- Total irradiance ---
  I_total = I_direct + I_diffuse + I_reflected;
endfunction
