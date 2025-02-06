/*
<pre>
	When	Who		What
	080731	L.Deffenbaugh	Original Declaration
</pre>
*/

#ifndef GOT_APRS_APRSDEF
#define GOT_APRS_APRSDEF

typedef unsigned long INTEGER_ID_F;	/* Shorthand index to records */

typedef char STATION_ID_F[12];
typedef char VERSION_F[32];
typedef short APRS_SYMBOL_F;	/* high bit = page, next 7 = overlay, low byte = selector */
typedef char APRS_DATA_TYPE_F;	/* straight from the APRS spec (char after :) */

typedef char IP_ADDRESS_F[16];	/* nnn.nnn.nnn.nnn */
typedef char DNS_NAME_F[64];	/* 250.202.204.68.cfl.res.rr.com mobile-032-170-086-237.mycingular.net */

typedef double DISTANCE_F;
typedef double ALTITUDE_F;
typedef double COORDINATE_F;
typedef double BEARING_F;
typedef double SPEED_F;

typedef struct COORDINATE_S
{	COORDINATE_F Latitude;
	COORDINATE_F Longitude;
} COORDINATE_S;

#endif /* GOT_APRS_APRSDEF */

