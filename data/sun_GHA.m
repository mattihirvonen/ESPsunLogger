## sun_GHA.m
## Calculate the Sun's Greenwich Hour Angle (GHA) for a given UTC date/time
## Usage: gha = gha_sun(year, month, day, UTC_hour, minute, second)

## Accurate to ~0.01° for most navigation purposes.
## Works for any date/time from 1900–2100.
## Handles leap years and fractional days.
## Returns GHA in degrees (0–360°).

## Starting point for AI:
## create octave function to calculate GHA vs daytime

## ---------------------------------------------------------------

function [GHA, GMST] = sun_GHA(year, month, day, UTC_hour, varargin )

  if nargin < 4
    error("Usage: sun_GHA(year, month, day, UTC_hour [, minute, second])");
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

  ## Convert date/time to Julian Date (UTC)
  if month <= 2
    year = year - 1;
    month = month + 12;
  end
  A  = floor(year / 100);
  B  = 2 - A + floor(A / 4);
  JD = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + ...
       day + B - 1524.5 + (UTC_hour + minute/60 + second/3600) / 24;

  ## Days since J2000.0
  D = JD - 2451545.0;

  ## Mean longitude of the Sun (deg)
  L = mod(280.460 + 0.9856474 * D, 360);

  ## Mean anomaly of the Sun (deg)
  g = mod(357.528 + 0.9856003 * D, 360);

  ## Ecliptic longitude of the Sun (deg)
  lambda = L + 1.915 * sind(g) + 0.020 * sind(2 * g);

  ## Obliquity of the ecliptic (deg)
  epsilon = 23.439 - 0.0000004 * D;

  ## Right ascension (deg)
  alpha = atan2d(cosd(epsilon) * sind(lambda), cosd(lambda));
  if alpha < 0
    alpha = alpha + 360;
  end

  ## Greenwich Mean Sidereal Time (hours)
  GMST = mod(18.697374558 + 24.06570982441908 * D, 24);

  ## Convert GMST to degrees and compute GHA
  GHA = mod(GMST * 15 - alpha, 360);

end

