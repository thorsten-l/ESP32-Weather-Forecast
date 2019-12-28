/*
Entfernen Sie folgende Informationen auf keinen Fall: / Do not remove following text:
Source code based on the javascript by Arnold Barmettler, www.astronomie.info / www.CalSky.com
based on algorithms by Peter Duffett-Smith's great and easy book
'Practical Astronomy with your Calculator'.
*/
#include "sunmoon.h"
#include <stdlib.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

#define M_PI 3.14159265358979323846
#define DEG (M_PI / 180.0)
#define RAD (180.0 / M_PI)

#ifndef isnan
inline bool isnan(double x) {
	return x != x;
}
inline bool isnan(int x) {
	return x != x;
}
#endif

int sun_moon_int(double x) { return (x < 0) ? (int)ceil(x) : (int)floor(x); }
double sun_moon_sqr(double x) { return x * x; }
double sun_moon_frac(double x) { return (x - floor(x)); }
double sun_moon_mod(double a, double b) { return (a - floor(a / b) * b); }
double sun_moon_mod2pi(double x) { return sun_moon_mod(x, 2.0 * M_PI); }
double sun_moon_round100000(double x) { return (round(100000.0 * x) / 100000.0); }
double sun_moon_round10000(double x) { return (round(10000.0 * x) / 10000.0); }
double sun_moon_round1000(double x) { return (round(1000.0 * x) / 1000.0); }
double sun_moon_round100(double x) { return (round(100.0 * x) / 100.0); }
double sun_moon_round10(double x) { return (round(10.0 * x) / 10.0); }
enum SIGN sun_moon_sign(double lon) { return (enum SIGN)((int)floor(lon * RAD / 30.0)); }

// Calculate Julian date: valid only from 1.3.1901 to 28.2.2100
double sun_moon_calcjd(int day, int month, int year)
{
	double jd = 2415020.5 - 64; // 1.1.1900 - correction of algorithm
	if (month <= 2) { year--; month += 12; }
	jd += sun_moon_int(((year - 1900)) * 365.25);
	jd += sun_moon_int(30.6001 * ((1 + month)));
	return jd + day;
}

// Julian Date to Greenwich Mean Sidereal Time
double sun_moon_calcgmst(double JD)
{
	double UT, T, T0;
	UT = sun_moon_frac(JD - 0.5) * 24.0; // UT in hours
	JD = floor(JD - 0.5) + 0.5;   // JD at 0 hours UT
	T = (JD - 2451545.0) / 36525.0;
	T0 = 6.697374558 + T * (2400.051336 + T * 0.000025862);
	return (sun_moon_mod(T0 + UT * 1.002737909, 24.0));
}

// Convert Greenweek mean sidereal time to UT
double sun_moon_gmst2ut(double JD, double gmst)
{
	double T, T0;
	JD = floor(JD - 0.5) + 0.5;   // JD at 0 hours UT
	T = (JD - 2451545.0) / 36525.0;
	T0 = sun_moon_mod(6.697374558 + T * (2400.051336 + T * 0.000025862), 24.0);
	return 0.9972695663 * ((gmst - T0));
}

// Local Mean Sidereal Time, geographical longitude in radians, East is positive
double sun_moon_gmst2lmst(double gmst, double lon) { return sun_moon_mod(gmst + RAD * lon / 15, 24.0); }

struct SUN_MOON_STORAGE
{
	double lat;
	double lon;
	double ra;
	double dec;
	double az;
	double alt;
	double radius;
	double distance;
	double rise;
	double transit;
	double set;
	double x;
	double y;
	double z;
	double distanceTopocentric;
	double decTopocentric;
	double raTopocentric;
	double anomalyMean;
	double diameter;
	double parallax;
	double orbitLon;
	double raGeocentric;
	double decGeocentric;
	double moonAge;
	double phase;
	int moonPhase;
	double cicilTwilightMorning, cicilTwilightEvening;
	double nauticalTwilightMorning, nauticalTwilightEvening;
	double astronomicalTwilightMorning, astronomicalTwilightEvening;
};

// Transform ecliptical coordinates (lon/lat) to equatorial coordinates (RA/dec)
struct SUN_MOON_STORAGE sun_moon_ecl2equ(struct SUN_MOON_STORAGE coor, double TDT)
{
	double T = (TDT - 2451545.0) / 36525.0; // Epoch 2000 January 1.5
	double eps = (23.0 + (26 + 21.45 / 60.0) / 60.0 + T * (-46.815 + T * (-0.0006 + T * 0.00181)) / 3600.0) * DEG;
	double coseps = cos(eps);
	double sineps = sin(eps);
	double sinlon = sin(coor.lon);
	coor.ra = sun_moon_mod2pi(atan2((sinlon * coseps - tan(coor.lat) * sineps), cos(coor.lon)));
	coor.dec = asin(sin(coor.lat) * coseps + cos(coor.lat) * sineps * sinlon);
	return coor;
}

// Transform equatorial coordinates (RA/Dec) to horizonal coordinates (azimuth/altitude)
// Refraction is ignored
struct SUN_MOON_STORAGE sun_moon_equ2altaz(struct SUN_MOON_STORAGE coor, double TDT, double geolat, double lmst)
{
	double cosdec = cos(coor.dec);
	double sindec = sin(coor.dec);
	double lha = lmst - coor.ra;
	double coslha = cos(lha);
	double sinlha = sin(lha);
	double coslat = cos(geolat);
	double sinlat = sin(geolat);
	double N = -cosdec * sinlha;
	double D = sindec * coslat - cosdec * coslha * sinlat;
	coor.az = sun_moon_mod2pi(atan2(N, D));
	coor.alt = asin(sindec * sinlat + cosdec * coslha * coslat);
	return coor;
}

// Transform geocentric equatorial coordinates (RA/Dec) to topocentric equatorial coordinates
struct SUN_MOON_STORAGE sun_moon_geoequ2topoequ(struct SUN_MOON_STORAGE coor, struct SUN_MOON_STORAGE observer, double lmst)
{
	double cosdec = cos(coor.dec);
	double sindec = sin(coor.dec);
	double coslst = cos(lmst);
	double sinlst = sin(lmst);
	double coslat = cos(observer.lat); // we should use geocentric latitude, not geodetic latitude
	double sinlat = sin(observer.lat);
	double rho = observer.radius; // observer-geocenter in Kilometer
	double x = coor.distance * cosdec * cos(coor.ra) - rho * coslat * coslst;
	double y = coor.distance * cosdec * sin(coor.ra) - rho * coslat * sinlst;
	double z = coor.distance * sindec - rho * sinlat;
	coor.distanceTopocentric = sqrt(x * x + y * y + z * z);
	coor.decTopocentric = asin(z / coor.distanceTopocentric);
	coor.raTopocentric = sun_moon_mod2pi(atan2(y, x));
	return coor;
}

// Calculate cartesian from polar coordinates
struct SUN_MOON_STORAGE sun_moon_equpolar2cart(double lon, double lat, double distance)
{
	struct SUN_MOON_STORAGE cart;
	double rcd = cos(lat) * distance;
	memset(&cart, 0, sizeof(struct SUN_MOON_STORAGE));
	cart.x = rcd * cos(lon);
	cart.y = rcd * sin(lon);
	cart.z = distance * sin(lat);
	return cart;
}

// Calculate observers cartesian equatorial coordinates (x,y,z in celestial frame) 
// from geodetic coordinates (longitude, latitude, height above WGS84 ellipsoid)
// Currently only used to calculate distance of a body from the observer
struct SUN_MOON_STORAGE sun_moon_observer2equcart(double lon, double lat, double height, double gmst)
{
	double co, si, fl, u, a, b, radius, x, y, rotangle;
	double flat = 298.257223563;        // WGS84 flatening of earth
	double aearth = 6378.137;           // GRS80/WGS84 semi major axis of earth ellipsoid
	struct SUN_MOON_STORAGE cart;
	memset(&cart, 0, sizeof(struct SUN_MOON_STORAGE));
	// Calculate geocentric latitude from geodetic latitude
	co = cos(lat);
	si = sin(lat);
	fl = 1.0 - 1.0 / flat;
	fl = fl * fl;
	si = si * si;
	u = 1.0 / sqrt(co * co + fl * si);
	a = aearth * u + height;
	b = aearth * fl * u + height;
	radius = sqrt(a * a * co * co + b * b * si); // geocentric distance from earth center
	cart.y = acos(a * co / radius); // geocentric latitude, rad
	cart.x = lon; // longitude stays the same
	if (lat < 0.0) { cart.y = -cart.y; } // adjust sign
	cart = sun_moon_equpolar2cart(cart.x, cart.y, radius); // convert from geocentric polar to geocentric cartesian, with regard to Greenwich
	// rotate around earth's polar axis to align coordinate system from Greenwich to vernal equinox
	x = cart.x;
	y = cart.y;
	rotangle = gmst / 24.0 * 2.0 * M_PI; // sideral time gmst given in hours. Convert to radians
	cart.x = x * cos(rotangle) - y * sin(rotangle);
	cart.y = x * sin(rotangle) + y * cos(rotangle);
	cart.radius = radius;
	cart.lon = lon;
	cart.lat = lat;
	return cart;
}

// Calculate coordinates for Sun
// Coordinates are accurate to about 10s (right ascension) 
// and a few minutes of arc (declination)
struct SUN_MOON_STORAGE sun_moon_sunposition(double TDT, double geolat, double lmst)
{
	struct SUN_MOON_STORAGE sunCoor;
	double D = TDT - 2447891.5;
	double eg = 279.403303 * DEG;
	double wg = 282.768422 * DEG;
	double e = 0.016713;
	double a = 149598500; // km
	double diameter0 = 0.533128 * DEG; // angular diameter of Moon at a distance
	double MSun = 360 * DEG / 365.242191 * D + eg - wg;
	double nu = MSun + 360.0 * DEG / M_PI * e * sin(MSun);
	memset(&sunCoor, 0, sizeof(struct SUN_MOON_STORAGE));
	sunCoor.lon = sun_moon_mod2pi(nu + wg);
	sunCoor.lat = 0;
	sunCoor.anomalyMean = MSun;

	sunCoor.distance = (1 - sun_moon_sqr(e)) / (1 + e * cos(nu)); // distance in astronomical units
	sunCoor.diameter = diameter0 / sunCoor.distance; // angular diameter in radians
	sunCoor.distance *= a;                         // distance in km
	sunCoor.parallax = 6378.137 / sunCoor.distance;  // horizonal parallax

	sunCoor = sun_moon_ecl2equ(sunCoor, TDT);

	// Calculate horizonal coordinates of sun, if geographic positions is given
	if (!isnan(geolat) && !isnan(lmst))
	{
		sunCoor = sun_moon_equ2altaz(sunCoor, TDT, geolat, lmst);
	}
	return sunCoor;
}

// Calculate data and coordinates for the Moon
// Coordinates are accurate to about 1/5 degree (in ecliptic coordinates)
struct SUN_MOON_STORAGE sun_moon_moonposition(struct SUN_MOON_STORAGE sunCoor, double TDT, struct SUN_MOON_STORAGE* observer, double lmst)
{
	struct SUN_MOON_STORAGE moonCoor;
	double mainPhase, p;

	double D = TDT - 2447891.5;

	// Mean Moon orbit elements as of 1990.0
	double l0 = 318.351648 * DEG;
	double P0 = 36.340410 * DEG;
	double N0 = 318.510107 * DEG;
	double i = 5.145396 * DEG;
	double e = 0.054900;
	double a = 384401; // km
	double diameter0 = 0.5181 * DEG; // angular diameter of Moon at a distance
	double parallax0 = 0.9507 * DEG; // parallax at distance a

	double l = 13.1763966 * DEG * D + l0;
	double MMoon = l - 0.1114041 * DEG * D - P0; // Moon's mean anomaly M
	double N = N0 - 0.0529539 * DEG * D;       // Moon's mean ascending node longitude
	double C = l - sunCoor.lon;
	double Ev = 1.2739 * DEG * sin(2 * C - MMoon);
	double Ae = 0.1858 * DEG * sin(sunCoor.anomalyMean);
	double A3 = 0.37 * DEG * sin(sunCoor.anomalyMean);
	double MMoon2 = MMoon + Ev - Ae - A3;  // corrected Moon anomaly
	double Ec = 6.2886 * DEG * sin(MMoon2);  // equation of centre
	double A4 = 0.214 * DEG * sin(2 * MMoon2);
	double l2 = l + Ev + Ec - Ae + A4; // corrected Moon's longitude
	double V = 0.6583 * DEG * sin(2 * (l2 - sunCoor.lon));
	double l3 = l2 + V; // true orbital longitude;

	double N2 = N - 0.16 * DEG * sin(sunCoor.anomalyMean);
	memset(&moonCoor, 0, sizeof(struct SUN_MOON_STORAGE));
	moonCoor.lon = sun_moon_mod2pi(N2 + atan2(sin(l3 - N2) * cos(i), cos(l3 - N2)));
	moonCoor.lat = asin(sin(l3 - N2) * sin(i));
	moonCoor.orbitLon = l3;

	moonCoor = sun_moon_ecl2equ(moonCoor, TDT);
	// relative distance to semi mayor axis of lunar oribt
	moonCoor.distance = (1 - sun_moon_sqr(e)) / (1 + e * cos(MMoon2 + Ec));
	moonCoor.diameter = diameter0 / moonCoor.distance; // angular diameter in radians
	moonCoor.parallax = parallax0 / moonCoor.distance; // horizontal parallax in radians
	moonCoor.distance = moonCoor.distance * a; // distance in km

	// Calculate horizonal coordinates of sun, if geographic positions is given
	if (observer != NULL && !isnan(lmst))
	{
		// transform geocentric coordinates into topocentric (==observer based) coordinates
		moonCoor = sun_moon_geoequ2topoequ(moonCoor, *observer, lmst);
		moonCoor.raGeocentric = moonCoor.ra; // backup geocentric coordinates
		moonCoor.decGeocentric = moonCoor.dec;
		moonCoor.ra = moonCoor.raTopocentric;
		moonCoor.dec = moonCoor.decTopocentric;
		moonCoor = sun_moon_equ2altaz(moonCoor, TDT, observer->lat, lmst); // now ra and dec are topocentric
	}

	// Age of Moon in radians since New Moon (0) - Full Moon (pi)
	moonCoor.moonAge = sun_moon_mod2pi(l3 - sunCoor.lon);
	moonCoor.phase = 0.5 * (1 - cos(moonCoor.moonAge)); // Moon phase, 0-1

	mainPhase = 1.0 / 29.53 * 360 * DEG; // show 'Newmoon, 'Quarter' for +/-1 day arond the actual event
	p = sun_moon_mod(moonCoor.moonAge, 90.0 * DEG);
	if (p < mainPhase || p > 90 * DEG - mainPhase) p = 2 * round(moonCoor.moonAge / (90.0 * DEG));
	else p = 2 * floor(moonCoor.moonAge / (90.0 * DEG)) + 1;
	moonCoor.moonPhase = (int)p;

	return moonCoor;
}

// Rough refraction formula using standard atmosphere: 1015 mbar and 10°C
// Input true altitude in radians, Output: increase in altitude in degrees
double sun_moon_refraction(double alt)
{
	int i;
	double pressure, temperature, y, D, P, Q, y0, D0, N;
	double altdeg = alt * RAD;
	if (altdeg < -2 || altdeg >= 90) return 0.0;

	pressure = 1015;
	temperature = 10;
	if (altdeg > 15) return (0.00452 * pressure / ((273 + temperature) * tan(alt)));

	y = alt;
	D = 0.0;
	P = (pressure - 80.0) / 930.0;
	Q = 0.0048 * (temperature - 10.0);
	y0 = y;
	D0 = D;

	for (i = 0; i < 3; i++)
	{
		N = y + (7.31 / (y + 4.4));
		N = 1.0 / tan(N * DEG);
		D = N * P / (60.0 + Q * (N + 39.0));
		N = y - y0;
		y0 = D - D0 - N;
		if ((N != 0.0) && (y0 != 0.0)) { N = y - N * (alt + D - y) / y0; }
		else { N = alt + D; }
		y0 = y;
		D0 = D;
		y = N;
	}
	return D; // Hebung durch Refraktion in radians
}

// returns Greenwich sidereal time (hours) of time of rise 
// and set of object with coordinates coor.ra/coor.dec
// at geographic position lon/lat (all values in radians)
// Correction for refraction and semi-diameter/parallax of body is taken care of in function RiseSet
// h is used to calculate the twilights. It gives the required elevation of the disk center of the sun
struct SUN_MOON_STORAGE sun_moon_gmstriseset(struct SUN_MOON_STORAGE coor, double lon, double lat, double hn)
{
	struct SUN_MOON_STORAGE riseset;
	double h = isnan(hn) ? 0.0 : hn; // set default value
	double tagbogen = acos((sin(h) - sin(lat) * sin(coor.dec)) / (cos(lat) * cos(coor.dec)));
	memset(&riseset, 0, sizeof(struct SUN_MOON_STORAGE));
	riseset.transit = RAD / 15 * (+coor.ra - lon);
	riseset.rise = 24.0 + RAD / 15 * (-tagbogen + coor.ra - lon); // calculate GMST of rise of object
	riseset.set = RAD / 15 * (+tagbogen + coor.ra - lon); // calculate GMST of set of object

	// using the modulo function Mod, the day number goes missing. This may get a problem for the moon
	riseset.transit = sun_moon_mod(riseset.transit, 24);
	riseset.rise = sun_moon_mod(riseset.rise, 24);
	riseset.set = sun_moon_mod(riseset.set, 24);

	return riseset;
}

// Find GMST of rise/set of object from the two calculates 
// (start)points (day 1 and 2) and at midnight UT(0)
double sun_moon_interpolategmst(double gmst0, double gmst1, double gmst2, double timefactor)
{
	return ((timefactor * 24.07 * gmst1 - gmst0 * (gmst2 - gmst1)) / (timefactor * 24.07 + gmst1 - gmst2));
}

// JD is the Julian Date of 0h UTC time (midnight)
struct SUN_MOON_STORAGE sun_moon_riseset(double jd0UT, struct SUN_MOON_STORAGE coor1, struct SUN_MOON_STORAGE coor2, double lon, double lat, double timeinterval, double naltitude)
{
	struct SUN_MOON_STORAGE rise1, rise2, rise;
	double T0, T02, decMean, psi, y, dt;

	// altitude of sun center: semi-diameter, horizontal parallax and (standard) refraction of 34'
	double alt = 0.0; // calculate 
	double altitude = isnan(naltitude) ? 0.0 : naltitude; // set default value

	// true height of sun center for sunrise and set calculation. Is kept 0 for twilight (ie. altitude given):
	if (altitude == 0.0) alt = 0.5 * coor1.diameter - coor1.parallax + 34.0 / 60 * DEG;

	rise1 = sun_moon_gmstriseset(coor1, lon, lat, altitude);
	rise2 = sun_moon_gmstriseset(coor2, lon, lat, altitude);

	memset(&rise, 0, sizeof(struct SUN_MOON_STORAGE));

	// unwrap GMST in case we move across 24h -> 0h
	if (rise1.transit > rise2.transit && abs(rise1.transit - rise2.transit) > 18) rise2.transit += 24.0;
	if (rise1.rise > rise2.rise && abs(rise1.rise - rise2.rise) > 18) rise2.rise += 24.0;
	if (rise1.set > rise2.set && abs(rise1.set - rise2.set) > 18) rise2.set += 24.0;
	T0 = sun_moon_calcgmst(jd0UT);
	//  var T02 = T0-zone*1.002738; // Greenwich sidereal time at 0h time zone (zone: hours)

	// Greenwich sidereal time for 0h at selected longitude
	T02 = T0 - lon * RAD / 15 * 1.002738;
	if (T02 < 0) T02 += 24;

	if (rise1.transit < T02) { rise1.transit += 24.0; rise2.transit += 24.0; }
	if (rise1.rise < T02) { rise1.rise += 24.0; rise2.rise += 24.0; }
	if (rise1.set < T02) { rise1.set += 24.0; rise2.set += 24.0; }

	// Refraction and Parallax correction
	decMean = 0.5 * (coor1.dec + coor2.dec);
	psi = acos(sin(lat) / cos(decMean));
	y = asin(sin(alt) / sin(psi));
	dt = 240 * RAD * y / cos(decMean) / 3600; // time correction due to refraction, parallax

	rise.transit = sun_moon_gmst2ut(jd0UT, sun_moon_interpolategmst(T0, rise1.transit, rise2.transit, timeinterval));
	rise.rise = sun_moon_gmst2ut(jd0UT, sun_moon_interpolategmst(T0, rise1.rise, rise2.rise, timeinterval) - dt);
	rise.set = sun_moon_gmst2ut(jd0UT, sun_moon_interpolategmst(T0, rise1.set, rise2.set, timeinterval) + dt);

	return rise;
}

// Find (local) time of sunrise and sunset, and twilights
// JD is the Julian Date of 0h local time (midnight)
// Accurate to about 1-2 minutes
// recursive: 1 - calculate rise/set in UTC in a second run
// recursive: 0 - find rise/set on the current local day. This is set when doing the first call to this function
struct SUN_MOON_STORAGE sun_moon_calcsunrise(double JD, double deltaT, double lon, double lat, int zone, int recursive)
{
	struct SUN_MOON_STORAGE risetemp;
	double jd0UT = floor(JD - 0.5) + 0.5;   // JD at 0 hours UT
	struct SUN_MOON_STORAGE coor1 = sun_moon_sunposition(jd0UT + deltaT / 24.0 / 3600.0, NAN, NAN);
	struct SUN_MOON_STORAGE coor2 = sun_moon_sunposition(jd0UT + 1.0 + deltaT / 24.0 / 3600.0, NAN, NAN); // calculations for next day's UTC midnight

	// rise/set time in UTC. 
	struct SUN_MOON_STORAGE rise = sun_moon_riseset(jd0UT, coor1, coor2, lon, lat, 1, NAN);
	if (recursive == 0)
	{ // check and adjust to have rise/set time on local calendar day
		if (zone > 0)
		{
			// rise time was yesterday local time -> calculate rise time for next UTC day
			if (rise.rise >= 24 - zone || rise.transit >= 24 - zone || rise.set >= 24 - zone)
			{
				risetemp = sun_moon_calcsunrise(JD + 1, deltaT, lon, lat, zone, 1);
				if (rise.rise >= 24 - zone) rise.rise = risetemp.rise;
				if (rise.transit >= 24 - zone) rise.transit = risetemp.transit;
				if (rise.set >= 24 - zone) rise.set = risetemp.set;
			}
		}
		else if (zone < 0)
		{
			// rise time was yesterday local time -> calculate rise time for next UTC day
			if (rise.rise < -zone || rise.transit < -zone || rise.set < -zone)
			{
				risetemp = sun_moon_calcsunrise(JD - 1, deltaT, lon, lat, zone, 1);
				if (rise.rise < -zone) rise.rise = risetemp.rise;
				if (rise.transit < -zone) rise.transit = risetemp.transit;
				if (rise.set < -zone) rise.set = risetemp.set;
			}
		}

		rise.transit = sun_moon_mod(rise.transit + zone, 24.0);
		rise.rise = sun_moon_mod(rise.rise + zone, 24.0);
		rise.set = sun_moon_mod(rise.set + zone, 24.0);

		// Twilight calculation
		// civil twilight time in UTC. 
		risetemp = sun_moon_riseset(jd0UT, coor1, coor2, lon, lat, 1, -6.0 * DEG);
		rise.cicilTwilightMorning = sun_moon_mod(risetemp.rise + zone, 24.0);
		rise.cicilTwilightEvening = sun_moon_mod(risetemp.set + zone, 24.0);

		// nautical twilight time in UTC. 
		risetemp = sun_moon_riseset(jd0UT, coor1, coor2, lon, lat, 1, -12.0 * DEG);
		rise.nauticalTwilightMorning = sun_moon_mod(risetemp.rise + zone, 24.0);
		rise.nauticalTwilightEvening = sun_moon_mod(risetemp.set + zone, 24.0);

		// astronomical twilight time in UTC. 
		risetemp = sun_moon_riseset(jd0UT, coor1, coor2, lon, lat, 1, -18.0 * DEG);
		rise.astronomicalTwilightMorning = sun_moon_mod(risetemp.rise + zone, 24.0);
		rise.astronomicalTwilightEvening = sun_moon_mod(risetemp.set + zone, 24.0);
	}
	return rise;
}

// Find local time of moonrise and moonset
// JD is the Julian Date of 0h local time (midnight)
// Accurate to about 5 minutes or better
// recursive: 1 - calculate rise/set in UTC
// recursive: 0 - find rise/set on the current local day (set could also be first)
// returns '' for moonrise/set does not occur on selected day
struct SUN_MOON_STORAGE sun_moon_calcmoonrise(double JD, double deltaT, double lon, double lat, int zone, int recursive)
{
	struct SUN_MOON_STORAGE risetemp;
	double timeinterval = 0.5;
	double jd0UT = floor(JD - 0.5) + 0.5;   // JD at 0 hours UT
	struct SUN_MOON_STORAGE suncoor1 = sun_moon_sunposition(jd0UT + deltaT / 24.0 / 3600.0, NAN, NAN);
	struct SUN_MOON_STORAGE coor1 = sun_moon_moonposition(suncoor1, jd0UT + deltaT / 24.0 / 3600.0, NULL, NAN);

	struct SUN_MOON_STORAGE suncoor2 = sun_moon_sunposition(jd0UT + timeinterval + deltaT / 24.0 / 3600.0, NAN, NAN); // calculations for noon
	// calculations for next day's midnight
	struct SUN_MOON_STORAGE coor2 = sun_moon_moonposition(suncoor2, jd0UT + timeinterval + deltaT / 24.0 / 3600.0, NULL, NAN);


	// rise/set time in UTC, time zone corrected later.
	// Taking into account refraction, semi-diameter and parallax
	struct SUN_MOON_STORAGE rise = sun_moon_riseset(jd0UT, coor1, coor2, lon, lat, timeinterval, NAN);

	if (recursive == 0)
	{ // check and adjust to have rise/set time on local calendar day
		if (zone > 0)
		{
			// recursive call to MoonRise returns events in UTC
			risetemp = sun_moon_calcmoonrise(JD - 1.0, deltaT, lon, lat, zone, 1);
			if (rise.transit >= 24.0 - zone || rise.transit < -zone)
			{ // transit time is tomorrow local time
				if (risetemp.transit < 24.0 - zone) rise.transit = NAN; // there is no moontransit today
				else rise.transit = risetemp.transit;
			}

			if (rise.rise >= 24.0 - zone || rise.rise < -zone)
			{ // rise time is tomorrow local time
				if (risetemp.rise < 24.0 - zone) rise.rise = NAN; // there is no moontransit today
				else rise.rise = risetemp.rise;
			}

			if (rise.set >= 24.0 - zone || rise.set < -zone)
			{ // set time is tomorrow local time
				if (risetemp.set < 24.0 - zone) rise.set = NAN; // there is no moontransit today
				else rise.set = risetemp.set;
			}

		}
		else if (zone < 0)
		{
			// rise/set time was tomorrow local time -> calculate rise time for former UTC day
			if (rise.rise < -zone || rise.set < -zone || rise.transit < -zone)
			{
				risetemp = sun_moon_calcmoonrise(JD + 1.0, deltaT, lon, lat, zone, 1);

				if (rise.rise < -zone)
				{
					if (risetemp.rise > -zone) rise.rise = NAN; // there is no moonrise today
					else rise.rise = risetemp.rise;
				}

				if (rise.transit < -zone)
				{
					if (risetemp.transit > -zone) rise.transit = NAN; // there is no moonset today
					else rise.transit = risetemp.transit;
				}

				if (rise.set < -zone)
				{
					if (risetemp.set > -zone) rise.set = NAN; // there is no moonset today
					else rise.set = risetemp.set;
				}

			}
		}

		if (rise.rise != NAN) rise.rise = sun_moon_mod(rise.rise + zone, 24.0);    // correct for time zone, if time is valid
		if (rise.transit != NAN) rise.transit = sun_moon_mod(rise.transit + zone, 24.0); // correct for time zone, if time is valid
		if (rise.set != NAN) rise.set = sun_moon_mod(rise.set + zone, 24.0);    // correct for time zone, if time is valid
	}
	return rise;
}

struct TIMESPAN sun_moon_calctimespan(double tm)
{
	struct TIMESPAN ts;
	double m, s;

	ts.hh = ts.mm = ts.ss = (int)NAN;
	ts.real = NAN;

	if (tm == 0.0 || isnan(tm)) return ts;
	ts.real = tm;
	m = (tm - floor(tm)) * 60.0;
	ts.hh = sun_moon_int(tm);
	s = (m - floor(m)) * 60.0;
	ts.mm = sun_moon_int(m);
	if (s >= 59.5) { ts.mm++; s -= 60.0; }
	if (ts.mm >= 60) { ts.hh++; ts.mm -= 60; }
	ts.ss = (int)round(s);
	return ts;
}


int sun_moon(double latitude, double longitude, struct tm* datetime, double deltaT, int zone, struct SUN_MOON* output)
{
	struct SUN_MOON_STORAGE observerCart, sunCoor, moonCoor, sunCart, sunRise, moonCart, moonRise;

	if (output == NULL) return EXIT_FAILURE;
	memset(output, 0, sizeof(struct SUN_MOON));

	double JD0 = sun_moon_calcjd(datetime->tm_mday, datetime->tm_mon + 1, datetime->tm_year + 1900);
	double jd = JD0 + (datetime->tm_hour - zone + datetime->tm_min / 60.0 + datetime->tm_sec / 3600.0) / 24.0;
	double TDT = jd + deltaT / 24.0 / 3600.0;
	double lat = latitude * DEG; // geodetic latitude of observer on WGS84
	double lon = longitude * DEG; // latitude of observer
	double height = 0 * 0.001; // altiude of observer in meters above WGS84 ellipsoid (and converted to kilometers)

	double gmst = sun_moon_calcgmst(jd);
	double lmst = sun_moon_gmst2lmst(gmst, lon);

	output->Lat = latitude;
	output->Lon = longitude;
	memcpy(&output->DateTime, datetime, sizeof(struct tm));
	output->DeltaT = deltaT;
	output->Zone = zone;

	observerCart = sun_moon_observer2equcart(lon, lat, height, gmst); // geocentric cartesian coordinates of observer

	sunCoor = sun_moon_sunposition(TDT, lat, lmst * 15.0 * DEG);   // Calculate data for the Sun at given time
	moonCoor = sun_moon_moonposition(sunCoor, TDT, &observerCart, lmst * 15.0 * DEG);    // Calculate data for the Moon at given time

	output->JD = sun_moon_round100000(jd);
	output->GMST = sun_moon_calctimespan(gmst);
	output->LMST = sun_moon_calctimespan(lmst);

	output->SunLon = sun_moon_round1000(sunCoor.lon * RAD);
	output->SunRA = sun_moon_calctimespan(sunCoor.ra * RAD / 15);
	output->SunDec = sun_moon_round1000(sunCoor.dec * RAD);
	output->SunAz = sun_moon_round100(sunCoor.az * RAD);
	output->SunAlt = sun_moon_round10(sunCoor.alt * RAD + sun_moon_refraction(sunCoor.alt));  // including refraction

	output->SunSign = sun_moon_sign(sunCoor.lon);
	output->SunDiameter = sun_moon_round100(sunCoor.diameter * RAD * 60.0); // angular diameter in arc seconds
	output->SunDistance = sun_moon_round10(sunCoor.distance);

	// Calculate distance from the observer (on the surface of earth) to the center of the sun
	sunCart = sun_moon_equpolar2cart(sunCoor.ra, sunCoor.dec, sunCoor.distance);
	output->SunDistanceObserver = sun_moon_round10(sqrt(sun_moon_sqr(sunCart.x - observerCart.x) + sun_moon_sqr(sunCart.y - observerCart.y) + sun_moon_sqr(sunCart.z - observerCart.z)));

	// JD0: JD of 0h UTC time
	sunRise = sun_moon_calcsunrise(JD0, output->DeltaT, lon, lat, output->Zone, 0);

	output->SunTransit = sun_moon_calctimespan(sunRise.transit);
	output->SunRise = sun_moon_calctimespan(sunRise.rise);
	output->SunSet = sun_moon_calctimespan(sunRise.set);

	output->SunCivilTwilightMorning = sun_moon_calctimespan(sunRise.cicilTwilightMorning);
	output->SunCivilTwilightEvening = sun_moon_calctimespan(sunRise.cicilTwilightEvening);
	output->SunNauticalTwilightMorning = sun_moon_calctimespan(sunRise.nauticalTwilightMorning);
	output->SunNauticalTwilightEvening = sun_moon_calctimespan(sunRise.nauticalTwilightEvening);
	output->SunAstronomicalTwilightMorning = sun_moon_calctimespan(sunRise.astronomicalTwilightMorning);
	output->SunAstronomicalTwilightEvening = sun_moon_calctimespan(sunRise.astronomicalTwilightEvening);

	output->MoonLon = sun_moon_round1000(moonCoor.lon * RAD);
	output->MoonLat = sun_moon_round1000(moonCoor.lat * RAD);
	output->MoonRA = sun_moon_calctimespan(moonCoor.ra * RAD / 15.0);
	output->MoonDec = sun_moon_round1000(moonCoor.dec * RAD);
	output->MoonAz = sun_moon_round100(moonCoor.az * RAD);
	output->MoonAlt = sun_moon_round10(moonCoor.alt * RAD + sun_moon_refraction(moonCoor.alt));  // including refraction
	output->MoonAge = sun_moon_round1000(moonCoor.moonAge * RAD);
	output->MoonPhaseNumber = sun_moon_round1000(moonCoor.phase);
	//char* phases[] = { "Neumond", "Zunehmende Sichel", "Erstes Viertel", "Zunehmender Mond", "Vollmond", "Abnehmender Mond", "Letztes Viertel", "Abnehmende Sichel", "Neumond" };
	if (moonCoor.moonPhase == 8) moonCoor.moonPhase = 0;
	output->MoonPhase = (enum LUNARPHASE)moonCoor.moonPhase;

	output->MoonSign = sun_moon_sign(moonCoor.lon);
	output->MoonDistance = sun_moon_round10(moonCoor.distance);
	output->MoonDiameter = sun_moon_round100(moonCoor.diameter * RAD * 60.0); // angular diameter in arc seconds

	// Calculate distance from the observer (on the surface of earth) to the center of the moon
	moonCart = sun_moon_equpolar2cart(moonCoor.raGeocentric, moonCoor.decGeocentric, moonCoor.distance);
	output->MoonDistanceObserver = sun_moon_round10(sqrt(sun_moon_sqr(moonCart.x - observerCart.x) + sun_moon_sqr(moonCart.y - observerCart.y) + sun_moon_sqr(moonCart.z - observerCart.z)));

	moonRise = sun_moon_calcmoonrise(JD0, output->DeltaT, lon, lat, output->Zone, 0);

	output->MoonTransit = sun_moon_calctimespan(moonRise.transit);
	output->MoonRise = sun_moon_calctimespan(moonRise.rise);
	output->MoonSet = sun_moon_calctimespan(moonRise.set);
	
	return EXIT_SUCCESS;
}
