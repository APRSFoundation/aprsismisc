#ifndef GOT_PARSEDEF
#define GOT_PARSEDEF

#define MAX_SYMBOL_TABLE (6*16)
#define MAX_SYMBOL_INDEX (MAX_SYMBOL_TABLE*MAX_SYMBOL_TABLE)	/* 9,216 */

#define APRS_DATE_VALID			(1<<0)
#define APRS_TIME_VALID			(1<<1)
#define APRS_LATLON_VALID		(1<<2)
#define APRS_ALTITUDE_VALID		(1<<3)
#define APRS_SYMBOL_VALID		(1<<4)
#define APRS_DATATYPE_VALID		(1<<5)
#define APRS_HOPS_VALID			(1<<6)
#define APRS_CRSSPD_VALID		(1<<7)
#define APRS_PHG_VALID			(1<<8)
#define APRS_PHG_RANGE_VALID	(1<<9)
#define APRS_WEATHER_VALID		(1<<10)
#define APRS_FREQUENCY_VALID	(1<<11)	/* Object or comment or status */
#define APRS_BULLETIN_VALID		(1<<12)
#define APRS_MESSAGE_VALID		(1<<13)
#define APRS_OBJECT_VALID		(1<<14)
#define APRS_ITEM_VALID			(1<<15)
#define APRS_PLATFORM_VALID		(1<<16)
#define APRS_MICE_MESSAGE_VALID	(1<<17)
#define APRS_TELEMETRY_VALID	(1<<18)
#define APRS_TELEMETRYDEF_VALID	(1<<19)
#define APRS_STORM_VALID		(1<<20)
#define APRS_NWS_VALID			(1<<21)
#define APRS_DFS_VALID			(1<<22)	/* In PHG structure */
#define APRS_BRGNRQ_VALID		(1<<23) /* /BRG/NRQ that follows the CSE/SPD */
#define APRS_AREA_OBJECT_VALID	(1<<24)	/* \l (alternate el) is Tyy/Cxx */
#define APRS_GRIDSQUARE_VALID	(1<<25)	/* From Status Report */

#define APRS_SYMBOL_DEFAULTED	(1<<31)

typedef enum APRS_PLATFORM_V
{	PLATFORM_UNKNOWN=0,		/* Platform completely unrecognized */

/* If you change this, update GetPlatformString() */
	PLATFORM_EXPERIMENTAL,

/* If you change this, update GetPlatformString() */
	PLATFORM_APRS_DOS,		/* Weather Platforms - per Spec */
	PLATFORM_MAC_APRS,
	PLATFORM_POCKET_APRS,
	PLATFORM_APRS_SA,
	PLATFORM_WIN_APRS,
	PLATFORM_X_APRS,

/* If you change this, update GetPlatformString() */
	PLATFORM_AGW_TRACKER,	/* Popular software platforms */
	PLATFORM_APRS4R,
	PLATFORM_APRSBB,
	PLATFORM_APRSD,
	PLATFORM_APRSDROID,
	PLATFORM_APRSISCE,
	PLATFORM_APRSISDR,
	PLATFORM_APRSIS32,
	PLATFORM_APRS_MESSENGER,
	PLATFORM_APRSTT,
	PLATFORM_APRX,
	PLATFORM_AVRT,
	PLATFORM_DIGI_NED,
	PLATFORM_DIXPRS,
	PLATFORM_JAPRS_IGATE,
	PLATFORM_JAVAPRSSRVR,
	PLATFORM_OPEN_APRS,
	PLATFORM_SARTRACK,
	PLATFORM_SQ3FYK,
	PLATFORM_U2APRS,
	PLATFORM_UI_VIEW_32N,
	PLATFORM_UI_VIEW_32,
	PLATFORM_UI_VIEW_23N,
	PLATFORM_UI_VIEW_23,
	PLATFORM_UI_VIEW_22,
	PLATFORM_UI_VIEW_1xx,
	PLATFORM_UI_VIEW_2xx,
	PLATFORM_UI_VIEW_3xx,
	PLATFORM_UI_VIEW_OTHER,
	PLATFORM_UISS,
	PLATFORM_XASTIR,
	PLATFORM_YAAC,

/* If you change this, update GetPlatformString() */
	PLATFORM_KAM,	/* Popular hardware platforms */
	PLATFORM_KISSOZ,
	PLATFORM_KPC_3,
	PLATFORM_KPC_9612,
	PLATFORM_ALLINONE,
	PLATFORM_TINYTRAK,
	PLATFORM_TINYTRAK2,
	PLATFORM_TINYTRAK3,
	PLATFORM_TINYTRAK4,
	PLATFORM_WXTRAC,
	PLATFORM_OPEN_TRACK,
	PLATFORM_HAMHUD,
	PLATFORM_HINZTEC,
	PLATFORM_BIGREDBEE,
	PLATFORM_UI_DIGI,
	PLATFORM_TNC_X,
	PLATFORM_KENWOOD_D7,
	PLATFORM_KENWOOD_D72,
	PLATFORM_KENWOOD_D74,
	PLATFORM_KENWOOD_D700,
	PLATFORM_KENWOOD_D710,
	PLATFORM_YAESU_FT1D,
	PLATFORM_YAESU_FT2D,
	PLATFORM_YAESU_VX8R,
	PLATFORM_YAESU_VX8G,
	PLATFORM_YAESU_FTM100D,
	PLATFORM_YAESU_FTM350,
	PLATFORM_YAESU_FTM400DR,
	PLATFORM_YAGTRACKER,

/* If you change this, update GetPlatformString() */
	PLATFORM_APRS,
	PLATFORM_BEACON,
	PLATFORM_GENERIC,
	PLATFORM_ID,
	PLATFORM_MICE,
	PLATFORM_NMEA,
	PLATFORM_OBSOLETE,

/* If you change this, update GetPlatformString() */
	PLATFORM_OTHER,			/* NOT a detailed category */

	PLATFORM_MAX = PLATFORM_OTHER	/* Must point to the last one */

} APRS_PLATFORM_V;

typedef struct TIME_INFO_S
{	char month, day;	/* 0 if not populated */
	char hour, minute, second;	/* 0 if not populated */
	char type;	/* ddhhmm z = zulu, hhmmss h=zulu ddhhmm /=local */
				/* mmddhhmm = Positionless Weather */
} TIME_INFO_S;

typedef struct PHG_INFO_S
{	short power;	/* For PHG */
	short gain;
	short dir;
	short rate;		/* 0 if not specified */
	short sunits;	/* For DFS */
	long height;	/* Can get BIG for balloons! */
	double range;	/* Calculated from PHG or directly specified in RNG */
} PHG_INFO_S;

typedef struct BRGNRQ_INFO_S
{	short bearing;
	short quality;
	short number;
	long range;
} BRGNRQ_INFO_S;

typedef struct FREQUENCY_INFO_S
{	char *Issues;	/* Pointer to parser issue string */
	double freq;
	double altfreq;	/* if second specified (FFF.FFFrx or Object w/FFF.FFFMHz) */
	double range;	/* Largest of NEWS ranges if specified, in miles (kilometer?) */
	short tone;		/* 0=off or not specified */
	short offset;	/* +/- in 10khz units (60=600khz) */
	char tonetype;	/* upper=wide, lower=narrow T/t, C/c, D/d, 1=1750 */
	char standardoffset;	/* +/- if no digits */
} FREQUENCY_INFO_S;

typedef struct TELEMETRY_INFO_S
{	unsigned short Sequence;	/* 0 for MIC */
	unsigned short Analog[5];
	unsigned char Digital;
} TELEMETRY_INFO_S;

#define VALID_WX_RAIN_HOUR			(1<<0)
#define VALID_WX_RAIN_24HOUR		(1<<1)
#define VALID_WX_RAIN_MIDNIGHT		(1<<2)
#define VALID_WX_BAROMETER			(1<<3)
#define VALID_WX_HUMIDITY			(1<<4)
#define VALID_WX_TEMPERATURE		(1<<5)
#define VALID_WX_GUST				(1<<6)
#define VALID_WX_WINDSPEED			(1<<7)
#define VALID_WX_DIRECTION			(1<<8)
#define VALID_WX_LUMINOSITY			(1<<9)
#define VALID_WX_WATER_HEIGHT		(1<<10)
#define VALID_WX_UNIT				(1<<11)
#define VALID_WX_BATTERY_VOLTS		(1<<12)
#define VALID_WX_RADIATION			(1<<13)
#define VALID_WX_PLATFORM			(1<<14)

typedef struct WEATHER_INFO_S
{	long Valid;
	int rainHour;
	int rain24Hour;
	int rainMidnight;
//	int snow;
	double barometer;
	int humidity;
	int temperature;
	double gust;
	double windspeed;
	int direction;
	int luminosity;		/* In watt/square meter */
	double water_height;	/* In feet */
	double battery_volts;	/* In volts to the tenths */
	double radiation;	/* In nanosieverts */
	APRS_PLATFORM_V tPlatform;	/* enum of weather platform */
	char Platform[32];	/* weather source Platform */
	char Unit[32];	/* If supplied and interpreted */
} WEATHER_INFO_S;

#define VALID_STORM_TYPE			(1<<0)
#define VALID_STORM_WINDSPEED		(1<<1)
#define VALID_STORM_GUST			(1<<2)
#define VALID_STORM_PRESSURE		(1<<3)
#define VALID_STORM_RADIUS_HURR		(1<<4)
#define VALID_STORM_RADIUS_TS		(1<<5)
#define VALID_STORM_RADIUS_GALE		(1<<6)

typedef struct STORM_INFO_S
{	long valid;
	char type[2];
	double windspeed;
	double gust;
	double pressure;
	struct
	{	int hurricane;
		int tropical_storm;
		int gale;
	} radius;
} STORM_INFO_S;

#define MAX_HOPS 16	/* with src & dst & AX.25 limit of 8 */
#define STATION_SIZE 10	/* Fits XXXXXX-NN\0 */

typedef struct APRS_HOP_INFO_S
{	int hopCount, hopUnused;	/* Count of hops and Index of first unused Hop */
	int TCPIP;					/* TRUE if TCPIP found in path */
	char Hops[MAX_HOPS][STATION_SIZE+1];	/* 11 is room for ABCDEF-NN* (and null) */
} APRS_HOP_INFO_S;

typedef struct APRS_PARSED_INFO_S
{	long Valid;			/* 0 if a completely invalid packet, see APRS_*_VALID bits */
	char *ParseError;	/* Pointer to parse error on failure */
	char srcCall[STATION_SIZE];	/* Always valid */
	char dstCall[STATION_SIZE];	/* Always valid */
	char thirdCall[STATION_SIZE];	/* Third-party sender (may be "") */
	char objCall[STATION_SIZE];	/* if APRS_OBJECT_VALID Or APRS_ITEM_VALID */
	char msgCall[STATION_SIZE];	/* if APRS_MESSAGE_VALID Text in comment (CleanComment?) */
	char msgAck[8];		/* ack from Message */
	char entryCall[STATION_SIZE+1];	/* From Hops (may be "") */
	char relayCall[STATION_SIZE+1];	/* From Hops (may be "") */
	char qCode[STATION_SIZE+1];		/* From Hops (may be "") */
	APRS_HOP_INFO_S Path;		/* Path is the actual packet path (old Hops) */
	APRS_HOP_INFO_S ThirdPath;	/* ThirdPath is the originally received wrapper path from thirdCall */
	double lat, lon, alt;
	int latlonExtended;	/* True if !DAO! parsed */
	int latAmbiguity, lonAmbiguity;	/* In powers of 10, 0=all 1=.nx 2=.xx 3=x.xx */
	int symbol;
	char datatype;
	unsigned long CRC32;	/* CRC32 of data payload for faster duplicate detection */
	int ObjectKilled;	/* TRUE if object had the kill flag set */
	int MessageCapable;	/* TRUE if message capable */
#ifdef SUPPORT_SPOTTER_NETWORK
	int SkyWarn;		/* TRUE if !SN! was found in comment */
#endif
	APRS_PLATFORM_V tPlatform;	/* enum of general platform */
	char Platform[32];	/* source Platform */
	short course;
	short speed;
	TIME_INFO_S Time;
	PHG_INFO_S PHG;	/* Also DFS */
	BRGNRQ_INFO_S BRGNRQ;
	WEATHER_INFO_S Weather;
	STORM_INFO_S Storm;
	FREQUENCY_INFO_S Frequency;
	TELEMETRY_INFO_S Telemetry;
	char Comment[512];	/* Raw, uncleaned comment */
	char CleanComment[512];	/* "Recognized" text removed */
	char Capabilities[512];	/* From capabilities packet */
	char StatusReport[512];	/* From status packet */
	char GridSquare[16];	/* If valid */
	char *MicEMessage;
} APRS_PARSED_INFO_S;

#endif

