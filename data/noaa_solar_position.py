
import math
from datetime import datetime, timezone

def noaa_solar_position(dt, latitude, longitude):
    # Convert time to UTC decimal hours
    dt_utc = dt.astimezone(timezone.utc)
    total_minutes = dt_utc.hour * 60 + dt_utc.minute + dt_utc.second / 60
    time_utc = total_minutes / 60

    # Julian day
    year, month, day = dt_utc.year, dt_utc.month, dt_utc.day
    if month <= 2:
        year -= 1
        month += 12
    A = math.floor(year / 100)
    B = 2 - A + math.floor(A / 4)
    jd = math.floor(365.25 * (year + 4716)) + math.floor(30.6001 * (month + 1)) + day + B - 1524.5 + time_utc / 24

    # Julian century
    jc = (jd - 2451545) / 36525

    # Geometric mean longitude of the sun (deg)
    gml_sun = (280.46646 + jc * (36000.76983 + jc * 0.0003032)) % 360

    # Geometric mean anomaly of the sun (deg)
    gma_sun = 357.52911 + jc * (35999.05029 - 0.0001537 * jc)

    # Eccentricity of Earth's orbit
    ecc_earth = 0.016708634 - jc * (0.000042037 + 0.0000001267 * jc)

    # Sun equation of center
    sun_eq_center = (math.sin(math.radians(gma_sun)) * (1.914602 - jc * (0.004817 + 0.000014 * jc)) +
                     math.sin(math.radians(2 * gma_sun)) * (0.019993 - 0.000101 * jc) +
                     math.sin(math.radians(3 * gma_sun)) * 0.000289)

    # Sun true longitude
    sun_true_long = gml_sun + sun_eq_center

    # Apparent longitude
    omega = 125.04 - 1934.136 * jc
    sun_app_long = sun_true_long - 0.00569 - 0.00478 * math.sin(math.radians(omega))

    # Mean obliquity of ecliptic
    mean_obliq = 23 + (26 + ((21.448 - jc * (46.815 + jc * (0.00059 - jc * 0.001813)))) / 60) / 60

    # Corrected obliquity
    obliq_corr = mean_obliq + 0.00256 * math.cos(math.radians(omega))

    # Sun declination
    sun_decl = math.degrees(math.asin(math.sin(math.radians(obliq_corr)) * math.sin(math.radians(sun_app_long))))

    # Equation of time (minutes)
    y = math.tan(math.radians(obliq_corr / 2)) ** 2
    eq_time = 4 * math.degrees(
        y * math.sin(2 * math.radians(gml_sun)) -
        2 * ecc_earth * math.sin(math.radians(gma_sun)) +
        4 * ecc_earth * y * math.sin(math.radians(gma_sun)) * math.cos(2 * math.radians(gml_sun)) -
        0.5 * y ** 2 * math.sin(4 * math.radians(gml_sun)) -
        1.25 * ecc_earth ** 2 * math.sin(2 * math.radians(gma_sun))
    )

    # True solar time (minutes)
    tst = (time_utc * 60 + eq_time + 4 * longitude) % 1440

    # Hour angle
    ha = tst / 4 - 180
    if ha < -180:
        ha += 360

    # Solar zenith angle
    lat_rad = math.radians(latitude)
    decl_rad = math.radians(sun_decl)
    ha_rad = math.radians(ha)

