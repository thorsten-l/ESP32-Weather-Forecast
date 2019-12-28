/*
Entfernen Sie folgende Informationen auf keinen Fall: / Do not remove following text:
Source code based on the javascript by Arnold Barmettler, www.astronomie.info / www.CalSky.com
based on algorithms by Peter Duffett-Smith's great and easy book
'Practical Astronomy with your Calculator'.
*/
#ifndef _SUNMOON_H_
#define _SUNMOON_H_

#include <time.h>

enum SIGN
{
	SIGN_ARIES,			//!< Widder
	SIGN_TAURUS,		//!< Stier
	SIGN_GEMINI,		//!< Zwillinge
	SIGN_CANCER,		//!< Krebs
	SIGN_LEO,			//!< Löwe
	SIGN_VIRGO,			//!< Jungfrau
	SIGN_LIBRA,			//!< Waage
	SIGN_SCORPIO,		//!< Skorpion
	SIGN_SAGITTARIUS,	//!< Schütze
	SIGN_CAPRICORNUS,	//!< Steinbock
	SIGN_AQUARIUS,		//!< Wassermann
	SIGN_PISCES			//!< Fische
};

enum LUNARPHASE
{
	LP_NEW_MOON,                //!< Neumond
	LP_WAXING_CRESCENT_MOON,    //!< Zunehmende Sichel
	LP_FIRST_QUARTER_MOON,      //!< Erstes Viertel
	LP_WAXING_GIBBOUS_MOON,     //!< Zunehmender Mond
	LP_FULL_MOON,               //!< Vollmond
	LP_WANING_GIBBOUS_MOON,     //!< Abnehmender Mond
	LP_LAST_QUARTER_MOON,       //!< Letztes Viertel
	LP_WANING_CRESCENT_MOON,    //!< Abnehmende Sichel
};

struct TIMESPAN
{
	int hh;			//!< Stunden
	int mm;			//!< Minuten
	int ss;			//!< Sekunden

	double real;	//!< Zeit als Fließkommazahl
};

struct SUN_MOON
{
	double Lat;										//!< Östl. geografische Länge [Grad]
	double Lon;										//!< Geografische Breite [Grad]
	struct tm DateTime;								//!< Datum und Uhrzeit
	int Zone;										//!< Zeitdifferenz zu Weltzeit [h]
	double DeltaT;									//!< deltaT [sek]
	double JD;										//!< Julianisches Datum [Tage]
	struct TIMESPAN GMST;							//!< Greenwich Sternzeit GMST [h]
	struct TIMESPAN LMST;							//!< Lokale Sternzeit LMST [h]
	double SunDistance;								//!< Entfernung der Sonne (Erdmittelpunkt) [km]
	double SunDistanceObserver;						//!< Entfernung der Sonne (vom Beobachter) [km]
	double SunLon;									//!< Eklipt. Länge der Sonne [Grad]
	struct TIMESPAN SunRA;							//!< Rektaszension der Sonne [h]
	double SunDec;									//!< Deklination der Sonne [Grad]
	double SunAz;									//!< Azimut der Sonne [Grad]
	double SunAlt;									//!< Höhe der Sonne über Horizont [Grad]
	double SunDiameter;								//!< Durchmesser der Sonne [']
	struct TIMESPAN SunAstronomicalTwilightMorning;	//!< Astronomische Morgendämmerung [h]
	struct TIMESPAN SunNauticalTwilightMorning;		//!< Nautische Morgendämmerung [h]
	struct TIMESPAN SunCivilTwilightMorning;		//!< Bürgerliche Morgendämmerung [h]
	struct TIMESPAN SunRise;						//!< Sonnenaufgang [h]
	struct TIMESPAN SunTransit;						//!< Sonnenkulmination [h]
	struct TIMESPAN SunSet;							//!< Sonnenuntergang [h]
	struct TIMESPAN SunCivilTwilightEvening;		//!< Bürgerliche Abenddämmerung [h]
	struct TIMESPAN SunNauticalTwilightEvening;		//!< Nautische Abenddämmerung [h]
	struct TIMESPAN SunAstronomicalTwilightEvening;	//!< Astronomische Abenddämmerung [h]
	enum SIGN SunSign;								//!< Tierkreiszeichen
	double MoonDistance;							//!< Entfernung des Mondes (Erdmittelpunkt) [km]
	double MoonDistanceObserver;					//!< Entfernung des Mondes (vom Beobachter) [km]
	double MoonLon;									//!< Eklipt. Länge des Mondes [Grad]
	double MoonLat;									//!< Eklipt. Breite des Mondes [Grad]
	struct TIMESPAN MoonRA;							//!< Rektaszension des Mondes [h]
	double MoonDec;									//!< Deklination des Mondes [Grad]
	double MoonAz;									//!< Azimut des Mondes [Grad]
	double MoonAlt;									//!< Höhe des Mondes über Horizont [Grad]
	double MoonDiameter;							//!< Durchmesser des Mondes [']
	struct TIMESPAN MoonRise;						//!< Mondaufgang [h]
	struct TIMESPAN MoonTransit;					//!< Mondkulmination [h]
	struct TIMESPAN MoonSet;						//!< Monduntergang [h]
	double MoonPhaseNumber;							//!< Mondphase
	double MoonAge;									//!< Mondalter [Grad]
	enum LUNARPHASE MoonPhase;						//!< Mondphase
	enum SIGN MoonSign;								//!< Mondzeichen
};

extern int sun_moon(double latitude, double longitude, struct tm* datetime, double deltaT, int zone, struct SUN_MOON* output);
#endif
