#ifdef _DEBUG
#include <crtdbg.h>
#endif

/* W4GCW>APU25N,WX4MLB-3*,WIDE2-2,qAR,KJ4ERJ-2:@180912h/@5?R:)vEP  z/A=000055 W4GCW@cfl.rr.com {UIV32} */

#define WIN_LEAN_AND_MEAN
#include <windows.h>

#include <math.h>
#include <ctype.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#ifdef UNDER_CE
#define strdup(s) strcpy((char*)malloc(strlen(s)+1),s)
#endif

//unsigned char ack_src[10];
//unsigned char ack_id[6];
//unsigned char ack_rej;

#include "parsedef.h"
#include "parse.h"

#define KphPerKnot 1.852
#define MilePerNM 1.15077945

void cdecl TraceError(HWND hwnd, char *Format, ...);
void cdecl TraceLog(char *Name, BOOL ForceIt, HWND hwnd, char *Format, ...);
void cdecl TraceLogThread(char *Name, BOOL ForceIt, char *Format, ...);
#ifdef UNDER_CE
static void cdecl TraceLogThread(char *Name, BOOL ForceIt, char *Format, ...)
{
}
#endif

#define STD_TABLE 0
#define ALT_TABLE 0x100

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

int IsSameBaseCallsign(char *One, char *Two)
{	char *d;
	if (*One != *Two) return FALSE;	/* cheap 1 character check */
	d = strchr(One,'-');	/* Locate -SSID */
	if (!d) d = strchr(One,'\0');	/* No SSID?  Use the whole thing */
	if (d==One) return *Two==*One;	/* Dash (or null) is FIRST character! */
	if (!_strnicmp(Two, One, d-One)	/* Base matches */
	&& (Two[d-One] == '-' || !Two[d-One]))	/* Up to dash or end of string */
		return TRUE;	/* Matches up to dash or null */
	return FALSE;
}

/* The following CRC code is from:
http://www.cl.cam.ac.uk/research/srg/bluebook/21/crc/crc.html
Fast CRC32 in Software by Richard Black 18th February 1994
And is algorithm 3 which requires no copyright notice

Algorithm three

Algorithm three is the algorithm which I have seen most usually in other literature and implementations. It is a substantial optimisation based on the observation that in algorithm two, (apart from the first thirty two bits) the data to be transmitted is not needed for the control path until some thirty two bits later. This permits the division by subtraction to be done more than one bit at a time using a pre-generated table.

This table is normally indexed by eight bits because in that case the table is 1K in size. Attempting to do more bits than this at once (e.g. 16) is usually slower because at 256K the table has horrific cache performance.
*/

#define QUOTIENT 0x04c11db7

static unsigned long crctab[256];

/* The code to generate the table is: */
static void crc32_init(void)
{
    int i, j;

    unsigned long crc;

    for (i = 0; i < 256; i++)
    {
        crc = i << 24;
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x80000000)
                crc = (crc << 1) ^ QUOTIENT;
            else
                crc = crc << 1;
        }
        crctab[i] = crc;
    }
}

/* The implementation then proceeds byte wise feeding the eight bit subtraction term in at each stage along with the next eight bits of data. The code is: */
unsigned long CRC32(unsigned char *data, int len)
{
    unsigned long       result;
    int                 i;
    
	if (!len) return ~0;
	result = *data++ << 24;
	if (!--len) return ~result;
    result |= *data++ << 16;
	if (!--len) return ~result;
    result |= *data++ << 8;
	if (!--len) return ~result;
    result |= *data++;
	if (!--len) return ~result;
    result = ~ result;
    
    if (!crctab[0]) crc32_init();

    for (i=0; i<len; i++)
    {
        result = (result << 8 | *data++) ^ crctab[result >> 24];
    }
    
    return ~result;
}

int WeatherPart(char *p, APRS_PARSED_INFO_S *Info)
{	char *e;
	int v = 0;
	int neg = 0;
	BOOL Valid = TRUE;	/* Assume it's good */
	static char *Valids = "csgtrpPhlLFfbVX#";	/* All switch/cases must be here */

	if (!isprint(*p&0xff)) return 0;	/* Not valid */
	if (!strchr(Valids,*p))
	{	if (isdigit(p[1]&0xff) || p[1]=='-')	/* Only an issue if it looks like a number after */
			TraceLogThread("Inv(Weather)", FALSE, "%s>%s: Unrecognized Weather Element(%c) from %s\n", Info->srcCall, Info->dstCall, *p, p);
		return 0;	/* Not supported */
	}

	if (!isdigit(p[1]&0xff) && p[1]!='-')	/* Check for ... or spaces */
	{	for (e=p+1; *e; e++)
		{	if (*e!='.' && *e!=' ')
				break;
		}
		return e==p+1?0:e-p;	/* Skip unspecified values */
	}

	for (e=p+1; *e; e++)
	{	if (e==p+1 && *e=='-')
			neg = 1;
		else if (!isdigit(*e&0xff))
			break;
		else v = v*10+*e-'0';	/* Build up the value */
	}
	if (neg) v = -v;	/* Negative */
/*	Need to do something with *p & v here */
	switch (*p)	/* Re-referenced below, check if changed */
	{
case 'c': /* = wind direction (in degrees).*/
	Info->Weather.Valid |= VALID_WX_DIRECTION;
	Info->Weather.direction = v; break;
case 's': /* = sustained one-minute wind speed (in mph). */
	Info->Weather.Valid |= VALID_WX_WINDSPEED;
	Info->Weather.windspeed = v/MilePerNM; break;	// s is mph windspeed is knots
case 'g': /* = gust (peak wind speed in mph in the last 5 minutes). */
	Info->Weather.Valid |= VALID_WX_GUST;
	Info->Weather.gust = v/MilePerNM; break;	// g is mph gust is knots
case 't': /* = temperature (in degrees Fahrenheit). Temperatures below zero are expressed as -01 to -99. */
	Info->Weather.Valid |= VALID_WX_TEMPERATURE;
	Info->Weather.temperature = v; break;
case 'r': /* = rainfall (in hundredths of an inch) in the last hour. */
	Info->Weather.Valid |= VALID_WX_RAIN_HOUR;
	Info->Weather.rainHour = v; break;
case 'p': /* = rainfall (in hundredths of an inch) in the last 24 hours. */
	Info->Weather.Valid |= VALID_WX_RAIN_24HOUR;
	Info->Weather.rain24Hour = v; break;
case 'P': /* = rainfall (in hundredths of an inch) since midnight. */
	Info->Weather.Valid |= VALID_WX_RAIN_MIDNIGHT;
	Info->Weather.rainMidnight = v; break;
case 'h': /* = humidity (in %. 00 = 100%). */
	if (v==0) v=100;
	Info->Weather.Valid |= VALID_WX_HUMIDITY;
	Info->Weather.humidity = v; break;
case 'l': /* = luminosity > 1000 (w/m^2) */
	Info->Weather.Valid |= VALID_WX_LUMINOSITY;
	Info->Weather.luminosity = v + 1000; break;
case 'L': /* = luminosity <= 999 (w/m^2) */
	Info->Weather.Valid |= VALID_WX_LUMINOSITY;
	Info->Weather.luminosity = v; break;
case 'F': /* = water height in feet */
case 'f': /* = water height in meters */
	Info->Weather.Valid |= VALID_WX_WATER_HEIGHT;
	Info->Weather.water_height = v;
	if (*e == '.')	/* Tenths? */
	{	double div = 1;
		while (isdigit(*++e & 0xff))
		{	div *= 10.0;
			Info->Weather.water_height += (*e-'0') / div;
		}
	}
	if (*p == 'f') Info->Weather.water_height *= 3.2808399;
	break;
case 'b': /* = barometric pressure (in tenths of millibars/tenths of hPascal). */
	Info->Weather.Valid |= VALID_WX_BAROMETER;
	Info->Weather.barometer = v; break;
case 'V': /* Battery volts in tenths */
	Info->Weather.Valid |= VALID_WX_BATTERY_VOLTS;
	Info->Weather.battery_volts = (double)v/10.0;
	break;
case 'X': /* = Radiation levels in nanosieverts mms (mm*10^s) */
	Info->Weather.Valid |= VALID_WX_RADIATION;
	Info->Weather.radiation = (double)(v/10) * pow(10,v%10);
	break;
/* Other parameters that are available on some weather station units include: */
//case 's': /* = snowfall (in inches) in the last 24 hours. */
//	Info->Weather.snow = v; break;
case '#': /* = raw rain counter */
	TraceLogThread("Inv(Weather)", FALSE, "%s>%s: Unsupported RawRain(%c) Value(%ld)\n", Info->srcCall, Info->dstCall, *p, (long) v);
	Valid = FALSE;
	break;
default:
	TraceLogThread("Inv(Weather)", TRUE, "%s>%s: Unrecognized Weather(%c) Value(%ld)\n", Info->srcCall, Info->dstCall, *p, (long) v);
	Valid = FALSE;
	return 0;	/* Not valid */
	}
	if (Valid) Info->Valid |= APRS_WEATHER_VALID;

	return e-p;
}

/* c220s004g005t077r000p000P000h50b09900wRSW */
char *ParseAPRSWeather(char *w, APRS_PARSED_INFO_S *Info)
{	int c;
	char *p;

/* Non-standard options by OpenTrackers per Scott 7/16/2012 3:07pm */
//OTW1 - ADS-WS1
//T2WX - OpenTracker USB, Tracker2, Tracker3
//OD1w - OpenTracker+
static struct
{	char *p;	/* Note: Longer non-uniques should appear first */
	char *ps;	/* Platform string */
	APRS_PLATFORM_V tPlatform;	/* Platform enum */
	char *us;	/* Unit string */
} WeatherPUs[] = { { "OD1w", "OpenTracker+", PLATFORM_OPEN_TRACK, "OpenTracker+" },
					{ "OTW1", "ADS-WS1", PLATFORM_OPEN_TRACK, "ADS-WS1" },
					{ "T2WX", "OpenTracker", PLATFORM_OPEN_TRACK, "OT-USB/T2/T3" } };
static struct
{	char p;
	char *s;
	APRS_PLATFORM_V tPlatform;
} WeatherPlatforms[] = { {'d',"APRSdos", PLATFORM_APRS_DOS},
						{'M',"MacAPRS", PLATFORM_MAC_APRS},
						{'P',"pocketAPRS", PLATFORM_POCKET_APRS},
						{'S',"APRS+SA", PLATFORM_APRS_SA},
						{'W',"WinAPRS", PLATFORM_WIN_APRS},
						{'X',"X-APRS (Linux)", PLATFORM_X_APRS} };
static struct
{	char *p;	/* Note: Longer non-uniques should appear first */
	char *s;
} WeatherUnits[] = { {"DsVP","Davis Vantage Pro"},	/* K0GDI */
					{"Dvs","Davis"},
					{"HKT","Heathkit"},
					{"PIC","PIC device"},
					{"RSW","Radio Shack"},
					{"U2kr","Remote Ultimeter logger"},
					{"Upkm","Remote Ultimeter packet mode"},
					{"U-II","Original Ultimeter U-II (auto mode)"},
					{"U2R","Original Ultimeter U-II (remote mode)"},
					{"U2k","Ultimeter 500/2000"},
					{"U5","Ultimeter 500"} };

	/* weather should be xnnn as the first term, short check for comments */
	if ((!isdigit(w[1]&0xff) || !isdigit(w[2]&0xff) || !isdigit(w[3]&0xff))
	&& (w[1]!='.' || w[2]!='.' || w[3]!='.'))
		return w;	/* definitely not weather! */
	else if (*w == 't')	/* Let this one pass, but complain */
		TraceLogThread("Inv(Weather)", FALSE, "%s>%s: Non-Compliant Weather %s\n", Info->srcCall, Info->dstCall, w);
	else if (*w != 'c' && *w != 'g')	/* Must be one of these */
	{	if (*w && *w!=' ')	/* We don't complain about these */
			TraceLogThread("Inv(Weather)", FALSE, "%s>%s: Unrecognized Weather %s\n", Info->srcCall, Info->dstCall, w);
		return w;
	}


	for (p=w; *p; p += c)
	{	c = WeatherPart(p,Info);
		if (!c) break;
	}

	for (c=0; c<ARRAYSIZE(WeatherPUs); c++)
	{	if (!strncmp(p,WeatherPUs[c].p,strlen(WeatherPUs[c].p)))
		{	strncpy(Info->Weather.Platform, WeatherPUs[c].ps, sizeof(Info->Weather.Platform));
			Info->Weather.tPlatform = WeatherPUs[c].tPlatform;
			strncpy(Info->Weather.Unit, WeatherPUs[c].us, sizeof(Info->Weather.Unit));
			Info->Weather.Valid |= VALID_WX_PLATFORM;
			Info->Weather.Valid |= VALID_WX_UNIT;
			p += strlen(WeatherPUs[c].p);	/* For original platform */
			break;
		}
	}

	if (c >= ARRAYSIZE(WeatherPUs))	/* Not a platform/unit, check standards */
	{	for (c=0; c<ARRAYSIZE(WeatherPlatforms); c++)
		{	if (*p == WeatherPlatforms[c].p)
			{	strncpy(Info->Weather.Platform, WeatherPlatforms[c].s, sizeof(Info->Weather.Platform));
				Info->Weather.tPlatform = WeatherPlatforms[c].tPlatform;
				Info->Weather.Valid |= VALID_WX_PLATFORM;
				break;
			}
		}
		if (c >= ARRAYSIZE(WeatherPlatforms))
		{	//TraceLogThread("Inv(Weather)", FALSE, "%s>%s: Unrecognized Platform(%.5s)\n", Info->srcCall, Info->dstCall, p);
			sprintf(Info->Weather.Unit,"UnknownP(%.5s)", p);	/* Unrecognized WeatherPlatform */
			Info->Weather.Valid |= VALID_WX_UNIT;
		}
		if (p[0] && p[1] && p[2])	/* Platform + 2-4 unit */
		{	for (c=0; c<ARRAYSIZE(WeatherUnits); c++)
			{	if (!strncmp(p+1,WeatherUnits[c].p,strlen(WeatherUnits[c].p)))
				{	strncpy(Info->Weather.Unit, WeatherUnits[c].s, sizeof(Info->Weather.Unit));
					Info->Weather.Valid |= VALID_WX_UNIT;
					p += strlen(WeatherUnits[c].p)+1;	/* For original platform */
					break;
				}
				if (c >= ARRAYSIZE(WeatherUnits))
				{	//TraceLogThread("Inv(Weather)", FALSE, "%s>%s: Unrecognized %s Unit(%.5s)\n", Info->srcCall, Info->dstCall, Info->Platform, p);
					sprintf(Info->Weather.Unit,"P:%c UnknownU(%.4s)", isprint(*p&0xff)?*p:'?', p+1);	/* Unrecognized WeatherUnit */
					Info->Weather.Valid |= VALID_WX_UNIT;
				}
			}
		}
	}
	return p;
}

#ifdef FROM_APRS_FI
	while ($s =~ s/^([0-9a-f]{4}|----)//i) {
		if ($1 eq '----') {
			push @vals, undef;
		} else {
			# Signed 16-bit integers in network (big-endian) order
			# encoded in hex, high nybble first.
			# Perl 5.10 unpack supports n! for signed ints, 5.8
			# requires tricks like this:
			my $v = unpack('n', pack('H*', $1));
			
			push @vals, ($v < 32768) ? $v : $v - 65536;
		}
	}
	return 0 if (!@vals);
	
	$t = shift @vals;
	$w{'wind_gust'} = sprintf('%.1f', $t * $kmh_to_ms / 10) if (defined $t);
	$t = shift @vals;
	$w{'wind_direction'} = sprintf('%.0f', ($t& 0xff) * 1.41176) if (defined $t); # 1/255 => 1/360
	$t = shift @vals;
	$w{'temp'} = sprintf('%.1f', _fahrenheit_to_celsius($t / 10)) if (defined $t); # 1/255 => 1/360
	$t = shift @vals;
	$w{'rain_midnight'} = sprintf('%.1f', $t * $hinch_to_mm) if (defined $t);
	$t = shift @vals;
	$w{'pressure'} = sprintf('%.1f', $t / 10) if (defined $t && $t >= 10);
	shift @vals; # Barometer Delta
	shift @vals; # Barometer Corr. Factor (LSW)
	shift @vals; # Barometer Corr. Factor (MSW)
	$t = shift @vals;
	if (defined $t) {
		$w{'humidity'} = sprintf('%.0f', $t / 10); # percentage
		delete $w{'humidity'} if ($w{'humidity'} > 100 || $w{'humidity'} < 1);
	}
	shift @vals; # date
	shift @vals; # time
	$t = shift @vals;
	$w{'rain_midnight'} = sprintf('%.1f', $t * $hinch_to_mm) if (defined $t);
	$t = shift @vals;
	$w{'wind_speed'} = sprintf('%.1f', $t * $kmh_to_ms / 10) if (defined $t);
	
	if (defined $w{'temp'}
	    || (defined $w{'wind_speed'} && defined $w{'wind_direction'})
	    || (defined $w{'pressure'})
	    || (defined $w{'humidity'})
	    	) {
	    		$rh->{'wx'} = \%w;
	    		return 1;
	}
	
#endif

BOOL FromRadix(char *Text, int Len, unsigned long *pResult, int Radix, char *Chars)
{
	unsigned long Result = 0;
	int i;

	for (i=0; i<Len; i++)
	{	char *x = strchr(Chars,toupper(Text[i]));
		if (!Text[i] || !x) return FALSE;
		Result = Result*Radix + (x-Chars);
	}
	*pResult = Result;
	return TRUE;
}
BOOL FromHex(char *Hex, int Len, unsigned long *pResult)
{	return FromRadix(Hex, Len, pResult, 16, "0123456789ABCDEF");
}
BOOL FromDec(char *Dec, int Len, unsigned long *pResult)
{	return FromRadix(Dec, Len, pResult, 10, "0123456789");
}
BOOL dwFromDec(char *Dec, int Len, DWORD *pResult)
{	unsigned long v;
	if (!FromRadix(Dec, Len, &v, 10, "0123456789")) return FALSE;
	*pResult = (DWORD) v;
	return TRUE;
}
BOOL wFromDec(char *Dec, int Len, WORD *pResult)
{	unsigned long v;
	if (!FromRadix(Dec, Len, &v, 10, "0123456789")) return FALSE;
	*pResult = (WORD) v;
	return TRUE;
}
BOOL cFromDec(char *Dec, int Len, unsigned char *pResult)
{	unsigned long v;
	if (!FromRadix(Dec, Len, &v, 10, "0123456789")) return FALSE;
	*pResult = (unsigned char) v;
	return TRUE;
}
BOOL shortFromDec(char *Dec, int Len, short *pResult)
{	unsigned long v;
	char sign = *Dec;
	if (sign == '+' || sign == '-')
	{	Dec++; Len--;
	}
	if (!FromRadix(Dec, Len, &v, 10, "0123456789")) return FALSE;
	if (sign == '-')
		v = -(long)v;
	*pResult = (short) v;
	return TRUE;
}
BOOL doubleFromDec(char *Dec, int Len, double *pResult)
{	BOOL Good;
	char *e, *Temp = malloc(Len+1);
	memcpy(Temp, Dec, Len);
	Temp[Len] = '\0';
	*pResult = strtod(Temp, &e);
	Good = !*e || *e==' ';	/* Allowed to terminate in blank */
	free(Temp);
	return Good;
}

/* $ULTW0000000002EC36E12762001186A9000102EB011200D200000000 */
/* 0123456789012345678901234567890123456789012345678901234567890 */
/*           1         2         3         4         5         6 */
/* $ULTW0000000002EC36E12762001186A9000102EB011200D200000000 */
/* $ULTW00CC00AC028F0000----000000000000----0043001E0050 */
/* $ULTW0000000001AE0000286900039663000103E8011201E5  XXX */
/* $ULTW0000000001AE0000286A00069663000103E8011201EA  xxx */

char *ParseULTWeather(char *w, APRS_PARSED_INFO_S *Info)
{	unsigned long v;

	if (strlen(w) < 57) return w;

	if (FromHex(w+5,4,&v))
	{	Info->Weather.gust = (long) (v/KphPerKnot/10);	/* *kmh_to_ms/10 */
		Info->Weather.Valid |= VALID_WX_GUST;
	}
	if (FromHex(w+9,4,&v))
	{	Info->Weather.direction = (v&0xff)*360/255;	/* 0-255=0-360 */
		Info->Weather.Valid |= VALID_WX_DIRECTION;
	}
	if (FromHex(w+13,4,&v))
	{	Info->Weather.temperature = ((short)v)/10;	/* _fahrenheit_to_celsius($t / 10) */
		Info->Weather.Valid |= VALID_WX_TEMPERATURE;
	}
	if (FromHex(w+17,4,&v))
	{	Info->Weather.rainMidnight = v;	/* *$hinch_to_mm */
		Info->Weather.Valid |= VALID_WX_RAIN_MIDNIGHT;
	}
	if (FromHex(w+21,4,&v))
	{	Info->Weather.barometer = v;	/* /10 */
		Info->Weather.Valid |= VALID_WX_BAROMETER;
	}
	/* 25 Skip 4 for Barometer Delta */
	/* 29 Skip 4 for Barometer Corr. Factor (LSW) */
	/* 33 Skip 4 for Barometer Corr. Factor (MSW) */
	if (FromHex(w+37,4,&v))
	{	Info->Weather.humidity = v/10;	/* /10 */
		Info->Weather.Valid |= VALID_WX_HUMIDITY;
	}
	/* 41 Skip 4 for date */
	/* 45 Skip 4 for time */
	if (FromHex(w+49,4,&v))
	{	Info->Weather.rainMidnight = v;	/* *hinch_to_mm */
		Info->Weather.Valid |= VALID_WX_RAIN_MIDNIGHT;
	}
	if (FromHex(w+53,4,&v))
	{	Info->Weather.windspeed = (long)(v/KphPerKnot/10);	/* *$kmh_to_ms/10 */
		Info->Weather.Valid |= VALID_WX_WINDSPEED;
	}
	if (Info->Weather.Valid)
	{	strncpy(Info->Weather.Unit, "Ultimeter", sizeof(Info->Weather.Unit));
		Info->Weather.Valid |= VALID_WX_UNIT;
		Info->Valid |= APRS_WEATHER_VALID;
	}
	return w+57;
}
/* !!00000039026F0000----02C602EB----0010039500000000 */
/* 0123456789012345678901234567890123456789012345678901234567890 */
/*           1         2         3         4         5         6 */
/* !!00000039026F0000----02C602EB----0010039500000000 */
/* What is THIS? 
	AA0MM-2>APRSW,WIDE,WIDE,qAR,K0SUN-15:!!00000080----0000----02A603CA----00000035 */
char *ParsePEETWeather(char *w, APRS_PARSED_INFO_S *Info)
{	unsigned long v;

	if (strlen(w) < 50) return w;

	if (FromHex(w+2,4,&v))
	{	Info->Weather.windspeed = (long)(v/KphPerKnot/10);	/* *kmh_to_ms/10 */
		Info->Weather.Valid |= VALID_WX_WINDSPEED;
	}
	if (FromHex(w+6,4,&v))
	{	Info->Weather.direction = (v&0xff)*360/255;	/* 0-255=0-360 */
		Info->Weather.Valid |= VALID_WX_DIRECTION;
	}
	if (FromHex(w+10,4,&v))
	{	Info->Weather.temperature = v/10;	/* _fahrenheit_to_celsius($t / 10) */
		Info->Weather.Valid |= VALID_WX_TEMPERATURE;
	}
	if (FromHex(w+14,4,&v))
	{	Info->Weather.rainMidnight = v;	/* *$hinch_to_mm */
		Info->Weather.Valid |= VALID_WX_RAIN_MIDNIGHT;
	}
	if (FromHex(w+18,4,&v))
	{	Info->Weather.barometer = v;	/* /10 */
		Info->Weather.Valid |= VALID_WX_BAROMETER;
	}
	/* 22 Skip 4 for Indoor Temperature */
	if (FromHex(w+26,4,&v))
	{	Info->Weather.humidity = v/10;	/* /10 */
		Info->Weather.Valid |= VALID_WX_HUMIDITY;
	}
	/* 30 Skip 4 for Indoor Humidity */
	/* 34 Skip 4 for date */
	/* 38 Skip 4 for time */
	if (FromHex(w+42,4,&v))
	{	Info->Weather.rainMidnight = v;	/* *hinch_to_mm */
		Info->Weather.Valid |= VALID_WX_RAIN_MIDNIGHT;
	}
	if (FromHex(w+46,4,&v))
	{	Info->Weather.windspeed = (long)(v/KphPerKnot/10);	/* *$kmh_to_ms/10 */
		Info->Weather.Valid |= VALID_WX_WINDSPEED;
	}
	if (Info->Weather.Valid)
	{	strncpy(Info->Weather.Unit, "PEET", sizeof(Info->Weather.Unit));
		Info->Weather.Valid |= VALID_WX_UNIT;
		Info->Valid |= APRS_WEATHER_VALID;
	}
	return w+53;
}

static char *SkipWhite(char *p)
{	while (*p && isspace(*p & 0xff)) p++;
	return p;
}

static BOOL ParseFrequency(char *comment, double *pFrequency)
{
/* Per: http://www.aprs.org/info/freqspec.txt
A96.000MHz would be 1296 MHz
B20.000MHz would be 2320 MHz
C01.000MHz would be 2401 MHz 
D01.000MHz would be 3401 MHz
E51.000MHz would be 5651 MHz 
F60.000MHz would be 5760 MHz 
G30.000MHz would be 5830 MHz
H01.000MHz would be 10,101 MHz
I01.000MHz would be 10,201 MHz
J68.000MHz would be 10,368 MHz 
K01.000MHz would be 10,401 MHz
L01.000MHz would be 10,501 MHz
M48.000MHz would be 24,048 MHz 
N01.000MHz would be 24,101 MHz
O01.000MHz would be 24,201 MHz
*/
static char Alphas[] = "ABCDEFGHIJKLMNO";
static double Adds[] = { 1200.0, 2300.0, 2400.0, 3400.0,	/* A..D */
						5600.0, 5700.0, 5800.0,				/* E..G */
						10100.0, 10200.0, 10300.0, 10400.0,	/* H..L */
						24000.0, 24100.0, 24200.0 };		/* M..O */
	if (strlen(comment)>=7
	&& comment[3]=='.'
	&& isdigit(comment[1]&0xff)
	&& isdigit(comment[2]&0xff)
	&& isdigit(comment[4]&0xff)
	&& isdigit(comment[5]&0xff))	/* optional [6] checked below */
	{	if (isdigit(comment[0]&0xff))
			return doubleFromDec(comment, isdigit(comment[6]&0xff)?7:6, pFrequency);
		else if (strchr(Alphas, comment[0]))
		{	int i = strchr(Alphas,comment[0])-Alphas;
			if (i >= 0 && i <= sizeof(Adds)/sizeof(Adds[0])
			&& doubleFromDec(comment+1,isdigit(comment[6]&0xff)?6:5,pFrequency))
			{	*pFrequency += Adds[i];
//TraceLogThread("FreqSpec-Alpha", TRUE, "Got %.3lf Adds[%ld] %.0lf from %s\n",
//			   (double) *pFrequency, (long) i, (double) Adds[i], comment);
				return TRUE;
			}
		}
	}
	return FALSE;
}

static BOOL ParseFreqSpec(char *Primary, char *comment, APRS_PARSED_INFO_S *Info)
{	BOOL AllDone = FALSE;
	char *orgComment = comment;	/* For moving comment remainder up later */

	if (!ParseFrequency(Primary, &Info->Frequency.freq)) return FALSE;
	if (Primary == comment)	/* Position comment should have MHz */
	{	if (toupper(comment[7]&0xff)=='M'
		&& toupper(comment[8]&0xff)=='H'
		&& tolower(comment[9]&0xff)=='z')
		{	if (comment[7]=='m' || comment[8]=='h' || comment[9]=='Z')
				Info->Frequency.Issues = "*MHzCase*";
			comment += 10;
		} else
		{	Info->Frequency.Issues = "*NoMHz*";
			strtod(comment,&comment);	/* Skip over numeric FFF.FFF */
			if (tolower(comment[0]) == 'g'
			&& tolower(comment[1]) == 'h')	/* Ignore Water Meters */
				return FALSE;
		}
	} else	/* object, should be Yaesu-compatible duplicate frequency here */
	{	if (strlen(comment) >= 10	/* Must fit FFF.FFfMHz */
		&& comment[3]=='.'
		&& toupper(comment[7]&0xff)=='M'
		&& toupper(comment[8]&0xff)=='H'
		&& tolower(comment[9]&0xff)=='z')
		{	double altfreq;
			if (!ParseFrequency(comment, &altfreq))
				Info->Frequency.Issues = "*InvalidFFF.FFF*";	/* MHz but no frequency? */
			else if (altfreq != Info->Frequency.freq)
			{	Info->Frequency.freq = altfreq;			/* comment trumps name */
				Info->Frequency.Issues = "*FreqMisMatch*";	/* Should match for Yaesu's */
			}
			if (comment[7]=='m' || comment[8]=='h' || comment[9]=='Z')
				Info->Frequency.Issues = "*MHzCase*";
			comment += 10;	/* Skip over extra frequency */
		} else Info->Frequency.Issues = "*NoFTM-350*";
	}

#ifndef KmPerMile
#define KmPerMile 1.609344
#endif
	while (!AllDone && *(comment=SkipWhite(comment)))
	{	if (isdigit(comment[0]&0xff)
		&& isdigit(comment[1]&0xff)
		&& isdigit(comment[2]&0xff)
		&& comment[3] == '.'	/* Alternate frequency? */
		&& isdigit(comment[4]&0xff)
		&& isdigit(comment[5]&0xff))
		{	char *e;
			double altfreq = strtod(comment,&e);
			e = SkipWhite(e);
			if (tolower(e[0]&0xff) == 'r' && tolower(e[1]&0xff) == 'x')
			{	Info->Frequency.altfreq = altfreq;	/* rx */
				if (Info->Frequency.altfreq == Info->Frequency.freq)
					Info->Frequency.Issues = "*rxSimplex*";
				else if (comment[7]=='R' || comment[8]=='X')
					Info->Frequency.Issues = "*rxCase*";
				comment = e+2;
			} else	/* Must be an extra frequency */
			{	if (_strnicmp(comment,"MHz",3))
					Info->Frequency.Issues = "*MissingMHz*";
				else Info->Frequency.Issues = "*ExtraFreq*";
				if (altfreq != Info->Frequency.freq)
					Info->Frequency.altfreq = altfreq;
				comment = e;
			}
		} else switch (*comment)
		{
		case '_':	/* Spec uses this for blank */
			Info->Frequency.Issues = "*Underscore*";
			comment++;
			break;
		case '1':	/* 1750 */
			if (!shortFromDec(comment,4,&Info->Frequency.tone)
			|| Info->Frequency.tone != 1750) AllDone = TRUE;
			else
			{	Info->Frequency.tonetype = *comment;
				if (Info->Frequency.tone != 1750)
					Info->Frequency.Issues = "*Non-1750*";
				comment += 4;
			}
			break;
		case 'M': case 'm':	/* Possible misplaced MHz to ignore */
			if (_strnicmp(comment,"MHz",3)) AllDone = TRUE;	/* Not something we can deal with */
			else
			{	if (comment[0]=='M' && comment[1]=='H' && comment[2]=='z')
					Info->Frequency.Issues = "*ExtraMHz*";
				else	Info->Frequency.Issues = "*MHzCase*";
				comment += 3;
			}
			break;
		case 'O': case 'o':	/* Possible extra word "offset" */
			if (_strnicmp(comment,"offset",6)) AllDone = TRUE;	/* Nope, stop now */
			else
			{	Info->Frequency.Issues = "*offset*";
				comment += 6;	/* Skip the word */
			}
			break;
		case 'P': case 'p':	/* Possible PL nnn.nn or PL tone nnn.n */
			if (toupper(comment[1]&0xff)!='L') AllDone = TRUE;	/* P<Other>, need PL */
			else if (comment[2]!=' ' && !isdigit(comment[2]&0xff)) AllDone = TRUE;
			else
			{	char *e;
				double tone = strtod(comment+2,&e);
				if (!tone && !_strnicmp(SkipWhite(e),"tone",4))	/* PL tone, get next number */
				{	tone = strtod(SkipWhite(e)+4,&e);
					if (!tone) AllDone = TRUE;
					else Info->Frequency.Issues = "*PLtone*";
				} else if (!tone) AllDone = TRUE;
				else Info->Frequency.Issues = "*PL*";
				if (!AllDone)
				{	Info->Frequency.tone = (short) tone;
					Info->Frequency.tonetype = isupper(comment[1]&0xff)?'T':'t';
					comment = e;
				}
			}
			break;
		case 'T': case 't':	/* PL tone */
		case 'C': case 'c':	/* CTCSS tone */
		case 'D': case 'd':	/* DCS code */
			if (_strnicmp(comment+1,"off",3)
			&& !shortFromDec(&comment[1],3,&Info->Frequency.tone))
			{	char *e;
				double tone = strtod(comment+1,&e);
				if (tone && (*e=='\0' || *e==' '))
				{	Info->Frequency.tone = (short) tone;
					Info->Frequency.tonetype = *comment;
					Info->Frequency.Issues = "*T/C/Dnot3*";
					comment = e;
				} else AllDone = TRUE;
			} else
			{	Info->Frequency.tonetype = *comment;
#ifdef OFF_IS_SENSITIVE
				if (isupper(comment[1]&0xff)
				|| isupper(comment[2]&0xff)
				|| isupper(comment[3]&0xff))
					Info->Frequency.Issues = TRUE;
#endif
				comment += 4;
				if (*comment == '.')	/* decimals after tone? */
				{	Info->Frequency.Issues = "*DecimalT/D/C*";
					comment++;
					while (isdigit(*comment&0xff)) comment++;
				} else if (isdigit(*comment&0xff))
				{	Info->Frequency.Issues = "*XtraDigT/D/C*";
					while (isdigit(*comment&0xff)) comment++;
				}
			}
			break;
		case '+': case '-':	/* Offsets */
			if (!isdigit(comment[1]&0xff))	/* "standard" offset? */
			{	Info->Frequency.standardoffset = *comment;
				Info->Frequency.Issues = "*Standalone+/-*";
				comment++;	/* Eat +/- */
			} else if (!shortFromDec(comment,4,&Info->Frequency.offset))
			{	double Temp;
				if (comment[2]=='.'
				&& toupper(comment[4]&0xff)=='M'
				&& doubleFromDec(comment,4,&Temp))
				{	Info->Frequency.offset = (short)(Temp * 100);
					Info->Frequency.Issues = "*MOffset*";
					comment += 5;
					if (toupper(comment[0]&0xff)=='H'
					&& tolower(comment[1]&0xff)=='z')
					{	comment += 2;
						Info->Frequency.Issues = "*MHzOffset*";
					}
				} else AllDone = TRUE;
			} else
			{	comment += 4;
				if (isdigit(*comment&0xff))
				{	while (isdigit(*comment&0xff)) comment++;
					if (toupper(comment[0]&0xff)=='K')
					{	comment++;
						Info->Frequency.Issues = "*KOffset*";
						if (toupper(comment[0]&0xff)=='H'
						&& tolower(comment[1]&0xff)=='z')
						{	comment += 2;
							Info->Frequency.Issues = "*KHzOffset*";
						}
					} else Info->Frequency.Issues = "*XtraDigOff*";
				}
			}
			break;
		case 'R':	/* Range */
		case 'N': case 'E': case 'S': case 'W':	/* NEWS range */
		case 'r': case 'n': case 'e': case 's': case 'w':	/* Non-compliant tags */
		{	double range;
			if (strlen(comment) >= 4	/* Gotta have this many */
			&& (tolower(comment[3]&0xff) == 'm' || tolower(comment[3]&0xff) == 'k')
			&& doubleFromDec(&comment[1],2,&range))
			{	if (tolower(comment[3]&0xff) == 'k') range = range / KmPerMile;
				Info->Frequency.range = max(Info->Frequency.range,range);
				if (islower(comment[0]&0xff))
					Info->Frequency.Issues = "*RNESWCase*";
				if (isupper(comment[3]&0xff))
					Info->Frequency.Issues = "*k/mCase*";
				comment += 4;
			} else if (strlen(comment) >= 3	/* Gotta have this many */
			&& (tolower(comment[2]&0xff) == 'm' || tolower(comment[2]&0xff) == 'k')
			&& (isspace(comment[3]&0xff) || !comment[3])	/* Whitespace after */
			&& doubleFromDec(&comment[1],1,&range))
			{	if (tolower(comment[2]&0xff) == 'k') range = range / KmPerMile;
				Info->Frequency.range = max(Info->Frequency.range,range);
				Info->Frequency.Issues = "*1DigitRange*";
				comment += 3;
			} else AllDone = TRUE;
			break;
		}
		default:
			AllDone = TRUE;
		}
	}

/* if we don't have a range yet, sniff around for one */
	if (!Info->Frequency.range)
	{	char *p = comment;
		while (*p)
		{	if (toupper(*p&0xff) == 'R'
			&& isdigit(p[1]&0xff))	/* Possibility */
			{	char *e;
				double range = (double) strtol(&p[1],&e,10);
				if ((tolower(*e&0xff)=='k' || tolower(*e&0xff)=='m')
				&& (isspace(*e&0xff) || !*e))	/* Whitespace after */
				{	if (tolower(*e&0xff) == 'k') range = range / KmPerMile;
					Info->Frequency.range = range;
					break;
				}
				p = e;	/* Skip entire piece */
			} else p++;	/* Check next character */
		}
	}

/* if there's no tone, look for "PL nnnn" */
	if (!Info->Frequency.tonetype)
	{	char *p = comment;
		while (*p)
		{	if (toupper(*p&0xff) == 'P' && toupper(p[1]&0xff) == 'L')
			{	char *e;
				double tone = strtod(p+2,&e);
				if (!tone && !_strnicmp(SkipWhite(e),"tone",4))	/* PL tone, get next number */
				{	tone = strtod(SkipWhite(e)+4,&e);
					if (!tone) break;
					else Info->Frequency.Issues = "*PLtone*";
				} else if (!tone) break;
				else Info->Frequency.Issues = "*PL*";
				if (!AllDone)
				{	Info->Frequency.tone = (short) tone;
					Info->Frequency.tonetype = isupper(*p&0xff)?'T':'t';
				}
				break;
			} else p++;
		}
	}

/* For Kenwoods, check for RT Systems's introduced non-printables */
	if (Info->tPlatform == PLATFORM_KENWOOD_D710
	|| Info->tPlatform == PLATFORM_KENWOOD_D72
	|| Info->tPlatform == PLATFORM_KENWOOD_D74)
	{	char *c;
		for (c=comment; *c; c++)
		{	if (!isprint(*c&0xff))
			{	Info->Frequency.Issues = "*NonPrint*";
				break;
			}
		}
	}
	if (!Info->Frequency.Issues)
	{	while (isspace(*comment&0xff)) comment++;	/* No leading whitespace */
		strcpy(orgComment, comment);	/* "Eat" the frequency components */
	}

	return TRUE;
}

static int IsValidOverlay(char c)
{	if (c == '/') return TRUE;
	if (c == '\\') return TRUE;
	if (c >= '0' && c <= '9') return TRUE;
	if (c >= 'A' && c <= 'Z') return TRUE;
	if (c >= 'a' && c <= 'j') return TRUE;	/* Compressed only! */
	return FALSE;
}

typedef struct OVERLAY_DEFINITION_S
{	char overlay;		/* 0 for end of list */
	char *Description;	/* NULL for end of list */
} OVERLAY_DEFINITION_S;

/*	From: http://www.aprs.org/symbols/symbols-new.txt Dated: 04 Jan 2010 */

/* ATM Machine or CURRENCY:  #$ 
/$ = original primary Phone
\$ = Bank or ATM (generic) */
static OVERLAY_DEFINITION_S OverATM[] = { { 'U', "US Dollars" },
												{ 'L', "Brittish Pound" },
												{ 'Y', "Japanese Yen" },
												{ 0, NULL } };

/* POWER PLANT: #% */
static OVERLAY_DEFINITION_S OverPowerPlant[] = { { 'C', "Coal Power" }, { 'G', "Geothermal" }, 
												{ 'H', "Hydroelectric" }, { 'N', "Nuclear Power" }, 
												{ 'S', "Solar Power" }, { 'T', "Turbine Power" },
												{ 'W', "Wind Power" },
												{ 0, NULL } };

/* GATEWAYS: #& */
static OVERLAY_DEFINITION_S OverGateway[] = { { 'I', "IGate (deprec)" }, { 'R', "RO IGate" }, 
												{ 'T', "1 Hop TX Gate" }, { '2', "2 Hop TX Gate" } };

/* INCIDENT SITES: #' */
static OVERLAY_DEFINITION_S OverIncidentSites[] = { { 'A', "Car Crash" }, { 'H', "Haz Incident" },
												{ 'M', "Multi-Vehicle" }, { 'P', "Pileup" },
												{ 'T', "Truck Wreck" },
												{ 0, NULL } };

static OVERLAY_DEFINITION_S OverFirenet = {0};

static OVERLAY_DEFINITION_S OverPortable[] = { { 'F', "Field Day" }, { 'I', "IOTA" },
											{ 'S', "SOTA" }, { 'W', "WOTA" } };

static OVERLAY_DEFINITION_S OverAdvisory = {0};

/* APRStt or DTMF or RFID gated users: #= (was BOX symbol) */
static OVERLAY_DEFINITION_S OverDTMF[] = { { 'M', "Mobile DTMF" }, { 'H', "HT DTMF" },
											{ 'Q', "QTH DTMF" }, { 'E', "EchoLink DTMF" },
											{ 'I', "IRLP DTMF" }, { 'R', "RFID Report" },
											{ 'E', "Event DTMF" },
											{ 0, NULL } };

/* HAZARDS: #H
/H = hotel
\H = Haze */
static OVERLAY_DEFINITION_S OverHazards[] = { { 'R', "Radiation Detector" },
											{ 'W', "Hazardous Waste" },
											{ 'X', "Skull&Crossbones" },
											{ 0, NULL } };

static OVERLAY_DEFINITION_S OverMARS = {0};

/* HUMAN SYMBOL: #[
/[ = Human
\[ = Wall Cloud (the original definition) */
static OVERLAY_DEFINITION_S OverHuman[] = { { 'B', "Baby on Board" },	/* stroller, pram, etc */
											{ 'S', "Skier" }, { 'R', "Runner" },
											{ 'H', "Hiker" },
											{ 0, NULL } };

/* HOUSE: #-
/- = House
\- = (was HF) */
static OVERLAY_DEFINITION_S OverHome[] = { { '5', "50hz Power" }, { '6', "60hz Power" },
									{ 'B', "Backup Power" }, { 'E', "Emergency Power" },
									{ 'G', "Geothermal" }, { 'H', "HF" },
									{ 'O', "Operator Present" }, { 'S', "Solar Powered" },
									{ 'W', "Wind Powered" },
									{ 0, NULL } };
/* CARS: #>
/> = normal car (side view)
\> = Top view and symbol POINTS in direction of travel */
static OVERLAY_DEFINITION_S OverCar[] = { { 'E', "Electric" }, { 'H', "Hybrid" },
											{ 'S', "Solar" }, { 'V', "GM Volt" },
											{ 0, NULL } };

/* NUMBERED CIRCLES: #0 */
static OVERLAY_DEFINITION_S OverNodes[] = { { 'E', "Echolink Node" },
									{ 'I', "IRLP Repeater" },
									{ 'S', "Staging Area" }, { 'W', "WIRES (Yaesu VOIP)" },
									{ 0, NULL } };

/* NETWORK NODES: #8 */
static OVERLAY_DEFINITION_S OverNetwork[] = { { '8', "802.11 Node" }, { 'G', "802.11G" },
												{ 0, NULL } };


/* BOX SYMBOL: #A
/A = Aid station
\A = numbered box
#A = all others for DTMF or RFID */
static OVERLAY_DEFINITION_S OverBox[] = { { 'X', "OLPC laptop XO" },
									{ 'H', "RFID HotSpot" },
									{ 'R', "RFID Beacon" },
									{ 0, NULL } };

/* RESTAURANTS: #R 
\R = Restaurant (generic) */
static OVERLAY_DEFINITION_S OverRestaurant[] = { { '7', "7/11" }, { 'K', "KFC" },
											{ 'M', "McDonalds" }, { 'T', "Taco Bell" },
											{ 0, NULL } };

/*RADIOS and APRS DEVICES: #Y
/Y = Yacht  <= the original primary symbol
\Y =        <= the original alternate was undefined */
static OVERLAY_DEFINITION_S OverRadios[] = { { 'A', "Alinco" },
											{ 'B', "Byonics" }, 
											{ 'I', "Icom" },
											{ 'K', "Kenwood" },
											{ 'Y', "Yaesu/Standard" },
											{ 0, NULL } };

/* GPS devices: #\
/\ = Triangle DF primary symbol
\\ = was undefined alternate symbol */
static OVERLAY_DEFINITION_S OverGPS[] = { { 'A', "Avmap G5" },
											{ 0, NULL } };

/* ARRL or DIAMOND: #a
/a = Ambulance */
static OVERLAY_DEFINITION_S OverARRL[] = { { 'A', "ARES" }, { 'D', "DARES" },
											{ 'G', "RSGB" }, { 'R', "RACES" },
											{ 'S', "SATERN" }, { 'W', "WinLink" },
											{ 0, NULL } };

/* CIVIL DEFENSE or TRIANGLE: #c
/c = Incident Command Post
\c = Civil Defense */
static OVERLAY_DEFINITION_S OverCivil[] = { { 'R', "RACES" }, { 'S', "SATERN" },
											{ 0, NULL } };

/* BUILDINGS: #h
/h = Hospital
\h = Ham Store       ** <= now used for HAMFESTS */
static OVERLAY_DEFINITION_S OverStore[] = { { 'H', "Home Depot" },
											{ 0, NULL } };

/* SPECIAL VEHICLES: #k
/k = truck
\k = SUV */
static OVERLAY_DEFINITION_S OverVehicle[] = { { '4', "4x4" }, {'A', "ATV" },
											{ 0, NULL } };

/* TRUCKS: #u
/u = Truck (18 wheeler)
\u = truck with overlay */
static OVERLAY_DEFINITION_S OverTruck[] = { { 'G', "Gas" }, { 'T', "Tanker" },
										{ 'C', "Chlorine Tanker" }, { 'H', "Hazardous" },
										{ 0, NULL } };

static char SSIDSymbols[] = "aUfbYX\'s><OjRkv";
#ifdef FOR_INFO_ONLY
{ "-15", 'v' },	/* Van */
{ "-14", 'k' },	/* Truck */
{ "-13", 'R' },	/* Recreational Vehicle */
{ "-12", 'j' },	/* Jeep */
{ "-11", 'O' },	/* Balloon */
{ "-10", '<' },	/* Motorcycle */
{ "-9", '>' },	/* Car */
{ "-8", 's' },	/* Ship (power boat) */
{ "-7", '\'' },	/* Small Aircraft */
{ "-6", 'X' },	/* Helicopter */
{ "-5", 'Y' },	/* Yacht */
{ "-4", 'b' },	/* Bicycle */
{ "-3", 'f' },	/* Fire Truck */
{ "-2", 'U' },	/* Bus */
{ "-1", 'a' },	/* Ambulance */
#endif

static struct
{	char symbol;
	char pxy[2];
	char axy[2];
	char *Primary;
	char *Alternate;
	OVERLAY_DEFINITION_S *Overlays;
} SymbolNames[] = {
{ '!', "BB", "OB", "Police Stn", "Emergency"},
{ '"', "BC", "OC", "No Symbol", "No Symbol"},
{ '#', "BD", "OD", "Digi", "No. Digi" },
{ '$', "BE", "OE", "Phone", "Bank", OverATM },
{ '%', "BF", "OF", "DX Cluster", "Power Plant", OverPowerPlant },
{ '&', "BG", "OG", "HF Gateway", "No. Gateway", OverGateway },
{ '\'',"BH", "OH", "Plane Sm", "Crash site", OverIncidentSites },
{ '(', "BI", "OI", "Mob Sat Stn", "Cloudy"},
{ ')', "BJ", "OJ", "WheelChair", "Firenet MEO", &OverFirenet },
{ '*', "BK", "OK", "Snowmobile", "Snow"},
{ '+', "BL", "OL", "Red Cross", "Church"},
{ ' ', "BM", "OM", "Boy Scout", "Girl Scout"},
{ '-', "BN", "ON", "Home", "Home (HF)", OverHome },
{ '.', "BO", "OO", "X", "UnknownPos"},
{ '/', "BP", "OP", "Red Dot", "Destination"},

{ '0', "P0", "A0", "Circle (0)", "No. Circle", OverNodes },
{ '1', "P1", "A1", "Circle (1)", "No Symbol"},
{ '2', "P2", "A2", "Circle (2)", "No Symbol"},
{ '3', "P3", "A3", "Circle (3)", "No Symbol"},
{ '4', "P4", "A4", "Circle (4)", "No Symbol"},
{ '5', "P5", "A5", "Circle (5)", "No Symbol"},
{ '6', "P6", "A6", "Circle (6)", "No Symbol"},
{ '7', "P7", "A7", "Circle (7)", "No Symbol"},
{ '8', "P8", "A8", "Circle (8)", "No Symbol", OverNetwork },
{ '9', "P9", "A9", "Circle (9)", "Petrol Stn"},

{ ':', "MR", "NR", "Fire", "Hail"},
{ ';', "MS", "NS", "Campground", "Park", OverPortable },
{ '<', "MT", "NT", "Motorcycle", "Advisory", &OverAdvisory },
{ '=', "MU", "NU", "Rail Eng.", "APRStt (DTMF)", OverDTMF },
{ '>', "MV", "NV", "Car", "No. Car", OverCar },
{ '?', "MW", "NW", "File svr", "Info Kiosk"},
{ '@', "MX", "NX", "HC Future", "Hurricane"},

{ 'A', "PA", "AA", "Aid Stn", "No. Box", OverBox },
{ 'B', "PB", "AB", "BBS", "Snow blwng"},
{ 'C', "PC", "AC", "Canoe", "Coast G'rd"},
{ 'D', "PD", "AD", "No Symbol", "Drizzle"},
{ 'E', "PE", "AE", "Eyeball", "Smoke"},
{ 'F', "PF", "AF", "Tractor", "Fr'ze Rain"},
{ 'G', "PG", "AG", "Grid Squ.", "Snow Shwr"},
{ 'H', "PH", "AH", "Hotel", "Haze/Hazard", OverHazards },
{ 'I', "PI", "AI", "Tcp/ip", "Rain Shwr"},
{ 'J', "PJ", "AJ", "No Symbol", "Lightning"},
{ 'K', "PK", "AK", "School", "Kenwood"},
{ 'L', "PL", "AL", "Usr Log-ON", "Lighthouse"},
{ 'M', "PM", "AM", "MacAPRS", "MARS", &OverMARS },
{ 'N', "PN", "AN", "NTS Stn", "Nav Buoy"},
{ 'O', "PO", "AO", "Balloon", "Rocket"},
{ 'P', "PP", "AP", "Police", "Parking"},
{ 'Q', "PQ", "AQ", "TBD", "Quake"},
{ 'R', "PR", "AR", "Rec Veh'le", "Restaurant", OverRestaurant },
{ 'S', "PS", "AS", "Shuttle", "Sat/Pacsat"},
{ 'T', "PT", "AT", "SSTV", "T'storm"},
{ 'U', "PU", "AU", "Bus", "Sunny"},
{ 'V', "PV", "AV", "ATV", "VORTAC"},
{ 'W', "PW", "AW", "WX Station", "No. WXS" },
{ 'X', "PX", "AX", "Helo", "Pharmacy"},
{ 'Y', "PY", "AY", "Yacht", "Radio or GPS", OverRadios },
{ 'Z', "PZ", "AZ", "WinAPRS", "No Symbol"},

{ '[', "HS", "DS", "Jogger", "Wall Cloud", OverHuman },
{ '\\',"HT", "DT", "DF Triangle", "GPS Symbol", OverGPS },
{ ']', "HU", "DU", "PBBS", "No Symbol"},
{ '^', "HV", "DV", "Plane Lrge", "No. Plane"},
{ '_', "HW", "DW", "WX Service", "No. WX Site" },
{ '`', "HX", "DX", "Dish Ant.", "Rain"},

{ 'a', "LA", "SA", "Ambulance", "No. Diamond", OverARRL },
{ 'b', "LB", "SB", "Bike", "Dust blwng"},
{ 'c', "LC", "SC", "ICP", "No. CivDef", OverCivil },
{ 'd', "LD", "SD", "Fire Station", "DX Spot"},
{ 'e', "LE", "SE", "Horse", "Sleet"},
{ 'f', "LF", "SF", "Fire Truck", "Funnel Cld"},
{ 'g', "LG", "SG", "Glider", "Gale"},
{ 'h', "LH", "SH", "Hospital", "(HAM) Store", OverStore },
{ 'i', "LI", "SI", "IOTA", "No. Blk Box"},
{ 'j', "LJ", "SJ", "Jeep", "WorkZone"},
{ 'k', "LK", "SK", "Truck", "Vehicle (SUV)", OverVehicle},
{ 'l', "LL", "SL", "Laptop", "Area Objs"},
{ 'm', "LM", "SM", "Mic-E Rptr", "Milepost"},
{ 'n', "LN", "SN", "Node", "No. Triang"},
{ 'o', "LO", "SO", "EOC", "Circle sm"},
{ 'p', "LP", "SP", "Rover", "Part Cloud"},
{ 'q', "LQ", "SQ", "Grid squ.", "No Symbol"},
{ 'r', "LR", "SR", "Antenna", "Restrooms"},
{ 's', "LS", "SS", "Power Boat", "No. Boat" },
{ 't', "LT", "ST", "Truck Stop", "Tornado"},
{ 'u', "LU", "SU", "Truck 18wh", "No. Truck", OverTruck },
{ 'v', "LV", "SV", "Van", "No. Van"},
{ 'w', "LW", "SW", "Water Stn", "Flooding"},
{ 'x', "LX", "SX", "XAPRS", "No Symbol"},
{ 'y', "LY", "SY", "Yagi", "Sky Warn"},
{ 'z', "LZ", "SZ", "Shelter", "No. Shelter"},

{ '{', "J1", "Q1", "No Symbol", "Fog"},
{ '|', "J2", "Q2", "TNC Stream Sw", "TNC Stream SW"},
{ '}', "J3", "Q3", "No Symbol", "No Symbol"},
{ '~', "J4", "Q4", "Telemetry", "TNC Stream SW"} };

struct
{	int	Table;
	char Symbol;
	char *Desc;
} GPXSymbols[] = {
{ 2, '!', "Airport" },
{ 2, '"', "Amusement Park" },
{ 2, '#', "Ball Park" },
{ 2, '$', "Bank" },
{ 2, '%', "Bar" },
{ 2, '&', "Beach" },
{ 2, '\'', "Bell" },
{ 2, '(', "Boat Ramp" },
{ 2, ')', "Bowling" },
{ 2, '*', "Bridge" },
{ 2, '+', "Building" },
{ 2, ',', "Campground" },
{ 2, '-', "Car" },
{ 2, '.', "Car Rental" },
{ 2, '/', "Car Repair" },
{ 2, '0', "Cemetery" },
{ 2, '1', "Church" },
{ 2, '2', "Circle with X" },
{ 2, '3', "City (Capitol)" },
{ 2, '4', "City (Large)" },
{ 2, '5', "City (Medium)" },
{ 2, '6', "City (Small)" },
{ 2, '7', "Civil" },
{ 2, '8', "Contact, Afro" },
{ 2, '9', "Contact, Alien" },
{ 2, ':', "Contact, Ball Cap" },
{ 2, ';', "Contact, Big Ears" },
{ 2, '<', "Contact, Biker" },
{ 2, '=', "Contact, Bug" },
{ 2, '>', "Contact, Cat" },
{ 2, '?', "Contact, Dog" },
{ 2, '@', "Contact, Dreadlocks" },
{ 2, 'A', "Contact, Female1" },
{ 2, 'B', "Contact, Female2" },
{ 2, 'C', "Contact, Female3" },
{ 3, '!', "Contact, Goatee" },
{ 3, '"', "Contact, Kung-Fu" },
{ 3, '#', "Contact, Pig" },
{ 3, '$', "Contact, Pirate" },
{ 3, '%', "Contact, Ranger" },
{ 3, '&', "Contact, Smiley" },
{ 3, '\'', "Contact, Spike" },
{ 3, '(', "Contact, Sumo" },
{ 3, ')', "Controlled Area" },
{ 3, '*', "Convenience Store" },
{ 3, '+', "Crossing" },
{ 3, ',', "Dam" },
{ 3, '-', "Danger Area" },
{ 3, '.', "Department Store" },
{ 3, '/', "Diver Down Flag 1" },
{ 3, '0', "Diver Down Flag 2" },
{ 3, '1', "Drinking Water" },
{ 3, '2', "Exit" },
{ 3, '3', "Fast Food" },
{ 3, '4', "Fishing Area" },
{ 3, '5', "Fitness Center" },
{ 3, '6', "Flag" },
{ 3, '7', "Forest" },
{ 3, '8', "Gas Station" },
{ 3, '9', "Geocache" },
{ 3, ':', "Geocache Found" },
{ 3, ';', "Ghost Town" },
{ 3, '<', "Glider Area" },
{ 3, '=', "Golf Course" },
{ 3, '>', "Green Diamond" },
{ 3, '?', "Green Square" },
{ 3, '@', "Heliport" },
{ 3, 'A', "Horn" },
{ 3, 'B', "Hunting Area" },
{ 3, 'C', "Information" },
{ 4, '!', "Levee" },
{ 4, '"', "Light" },
{ 4, '#', "Live Theater" },
{ 4, '$', "Lodging" },
{ 4, '%', "Man Overboard" },
{ 4, '&', "Marina" },
{ 4, '\'', "Medical Facility" },
{ 4, '(', "Mile Marker" },
{ 4, ')', "Military" },
{ 4, '*', "Mine" },
{ 4, '+', "Movie Theater" },
{ 4, ',', "Museum" },
{ 4, '-', "Navaid, Amber" },
{ 4, '.', "Navaid, Black" },
{ 4, '/', "Navaid, Blue" },
{ 4, '0', "Navaid, Green" },
{ 4, '1', "Navaid, Green/Red" },
{ 4, '2', "Navaid, Green/White" },
{ 4, '3', "Navaid, Orange" },
{ 4, '4', "Navaid, Red" },
{ 4, '5', "Navaid, Red/Green" },
{ 4, '6', "Navaid, Red/White" },
{ 4, '7', "Navaid, Violet" },
{ 4, '8', "Navaid, White" },
{ 4, '9', "Navaid, White/Green" },
{ 4, ':', "Navaid, White/Red" },
{ 4, ';', "Oil Field" },
{ 4, '<', "Parachute Area" },
{ 4, '=', "Park" },
{ 4, '>', "Parking Area" },
{ 4, '?', "Pharmacy" },
{ 4, '@', "Picnic Area" },
{ 4, 'A', "Pizza" },
{ 4, 'B', "Post Office" },
{ 4, 'C', "Private Field" },
{ 5, '!', "Radio Beacon" },
{ 5, '"', "Red Diamond" },
{ 5, '#', "Red Square" },
{ 5, '$', "Residence" },
{ 5, '%', "Restaurant" },
{ 5, '&', "Restricted Area" },
{ 5, '\'', "Restroom" },
{ 5, '(', "RV Park" },
{ 5, ')', "Scales" },
{ 5, '*', "Scenic Area" },
{ 5, '+', "School" },
{ 5, ',', "Seaplane Base" },
{ 5, '-', "Shipwreck" },
{ 5, '.', "Shopping Center" },
{ 5, '/', "Short Tower" },
{ 5, '0', "Shower" },
{ 5, '1', "Skiing Area" },
{ 5, '2', "Skull and Crossbones" },
{ 5, '3', "Soft Field" },
{ 5, '4', "Stadium" },
{ 5, '5', "Summit" },
{ 5, '6', "Swimming Area" },
{ 5, '7', "Tall Tower" },
{ 5, '8', "Telephone" },
{ 5, '9', "Toll Booth" },
{ 5, ':', "TracBack Point" },
{ 5, ';', "Trail Head" },
{ 5, '<', "Truck Stop" },
{ 5, '=', "Tunnel" },
{ 5, '>', "Ultralight Area" },
{ 5, '?', "Water Hydrant" },
{ 5, '@', "Waypoint" },
{ 5, 'A', "White Buoy" },
{ 5, 'B', "White Dot" },
{ 5, 'C', "Zoo" },
};

long SymbolInt(char Table, char Character)
{
	if (Table < '!') return ((int)Table<<8) | Character;	/* Non-APRS symbols */
	return (Table!='/'?0x100:0) | Character | ((Table!='/'&&Table!='\\')?Table<<16:0);
}

long APRSSymbolIndexToInt(int Index)
{	int Table = (Index / MAX_SYMBOL_TABLE)+'!';
	int Symbol = (Index % MAX_SYMBOL_TABLE)+'!';
	if (Index < 0 || Index >= MAX_SYMBOL_INDEX) return 0;	/* Bogus */
	return SymbolInt(Table, Symbol);
}

int APRSSymbolIndex(int Symbol)
{	int Org = Symbol;
	int Overlay = (Symbol>>16)&0xff;
	int Table = (Symbol>>8)&0xff;
	Symbol = Symbol & 0xff;

	if (!Table) Overlay = '/';	/* Primary table */
	else if (Table > 1)			/* Non-APRS symbol? */
		return MAX_SYMBOL_INDEX;	/* Don't index these */
	else if (Table && !Overlay) Overlay = '\\';	/* Secondary table */
	Overlay -= '!'; Symbol -= '!';
	if (Overlay < 0 || Overlay > MAX_SYMBOL_TABLE
	|| Symbol < 0 || Symbol > MAX_SYMBOL_TABLE)
		Symbol = MAX_SYMBOL_INDEX;	/* Yes, 9216 */
	else Symbol = Overlay*MAX_SYMBOL_TABLE+Symbol;	/* 96*96=0..9215 */
	return Symbol;
}

int GetSymbolByName(char *Name)
{	int s;
	for (s=0; s<ARRAYSIZE(SymbolNames); s++)
	{	if (!_stricmp(SymbolNames[s].Primary, Name))
		{	return SymbolInt('/', SymbolNames[s].symbol);
		} else if (!_stricmp(SymbolNames[s].Alternate, Name))
		{	return SymbolInt('\\', SymbolNames[s].symbol);
		} else if (SymbolNames[s].Overlays)
		{	OVERLAY_DEFINITION_S *o;
			for (o=SymbolNames[s].Overlays;
				o && o->overlay && o->Description; o++)
			{	if (!_stricmp(o->Description, Name))
				{	return SymbolInt(o->overlay, SymbolNames[s].symbol);
				}
			}
		}
	}
	for (s=0; s<ARRAYSIZE(GPXSymbols); s++)
	{	if (!_stricmp(GPXSymbols[s].Desc, Name))
			return SymbolInt(GPXSymbols[s].Table, GPXSymbols[s].Symbol);
	}
	return 0;
}

char *GetSymbolName(int Symbol)
{	int SymIndex = (Symbol & 0xff) - SymbolNames[0].symbol;
	int Page = (Symbol >> 8) & 0xff;
	int Overlay = (Symbol >> 16) & 0xff;

	if (Page > 1 && Page < '!')	/* 0 and 1 are APRS symbols */
	{	int s;
		for (s=0; s<ARRAYSIZE(GPXSymbols); s++)
			if (GPXSymbols[s].Table==Page && GPXSymbols[s].Symbol==(Symbol&0xff))
				return GPXSymbols[s].Desc;
		return "*Unknown*";
	}

	if (SymIndex < 0 || SymIndex >= ARRAYSIZE(SymbolNames))			/* Out of range */
		return "*Unknown*";
	else if (!Page) return SymbolNames[SymIndex].Primary;	/* Primary Table */
	else if (!Overlay) return SymbolNames[SymIndex].Alternate;	/* Alternate Table */
/*	Alternate with overlay, see if it is explicit */
	else
	{	OVERLAY_DEFINITION_S *o = SymbolNames[SymIndex].Overlays;
		for (o=SymbolNames[SymIndex].Overlays; o && o->overlay && o->Description; o++)
		{	if (o->overlay == Overlay)
				return o->Description;
		}
		return SymbolNames[SymIndex].Alternate;
	}
}

char *GetDisplayableSymbol(int Symbol)
{	int SymIndex = (Symbol & 0xff) - SymbolNames[0].symbol;
	int Page = (Symbol >> 8) & 0xff;
	int Overlay = (Symbol >> 16) & 0xff;

	if (Page > 1 && Page < '!')	/* 0 and 1 are APRS symbols */
	{	int s;
		for (s=0; s<ARRAYSIZE(GPXSymbols); s++)
			if (GPXSymbols[s].Table==Page && GPXSymbols[s].Symbol==(Symbol&0xff))
				return _strdup(GPXSymbols[s].Desc);
		return _strdup("*Unknown*");
	}

	if (SymIndex < 0 || SymIndex >= ARRAYSIZE(SymbolNames))			/* Out of range */
		return _strdup("*Unknown*");
	else if (!Page) return _strdup(SymbolNames[SymIndex].Primary);	/* Primary Table */
	else if (!Overlay) return _strdup(SymbolNames[SymIndex].Alternate);	/* Alternate Table */
/*	Alternate with overlay, see if it is explicit */
	else
	{	OVERLAY_DEFINITION_S *o = SymbolNames[SymIndex].Overlays;
		for (o=SymbolNames[SymIndex].Overlays; o && o->overlay && o->Description; o++)
			if (o->overlay == Overlay)
				return _strdup(o->Description);
		{	char *symName = SymbolNames[SymIndex].Alternate;
			char *symBuf = (char*)malloc(strlen(symName)+8);
			sprintf(symBuf,"%s %c",symName, isprint(Overlay&0xff)?Overlay:'?');
			return symBuf;
		}
	}
}

int IsValidAltNet(char *AltNet)
{	unsigned int i;
static char *Generics[] = { "AIR", "ALL", "AP", "BEACON", "CQ", "GPS", "DF",
							"DGPS", "DRILL", "DX", "ID", "JAVA", "MAIL", "MICE",
							"QST", "QTH", "RTCM", "SKY", "SPACE", "SPC", "SYM",
							"TEL", "TEST", "TLM", "WX", "ZIP" };

	if (!AltNet || !*AltNet) return FALSE;
	if (strlen(AltNet) > 6) return FALSE;
	for (i=0; i<strlen(AltNet); i++)
		if (!isalnum(AltNet[i]&0xff)) return FALSE;
		else AltNet[i] = toupper(AltNet[i]);
	if (AltNet[0] == 'A' && AltNet[1] == 'P') return FALSE;
	for (i=0; i<sizeof(Generics)/sizeof(Generics[0]); i++)
		if (!strncmp(Generics[i], AltNet, strlen(Generics[i])))
			return FALSE;
	return TRUE;
}

// Mic-E latitude lookup
const unsigned char miclatvalid[] = "0123456789ABCDEFGHIJKLPQRSTUVWXYZ";
const unsigned char miclat[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	                            ':', ';', '<', '=', '>', '?', '@', '0', '1', '2',
    	                        '3', '4', '5', '6', '7', '8', '9', ' ', ' ', 'M',
        	                    'N', 'O', '0', '1', '2', '3', '4', '5', '6', '7',
            	                '8', '9', ' '};
const unsigned char null_position[] = "000000000";

char *digicall = NULL;
void *MsgUserArg, *BullUserArg;
void (*MessageCallback)(void *userarg, char *from, char *message) = NULL;
void (*BulletinCallback)(void *userarg, char *from, char identifier, char *group, char *message) = NULL;

#ifdef UNDER_CE
int stricmp(char *a, char *b)
{	if (a && b)
	while (*a && *b)
	{	char ca = toupper(*a);
		char cb = toupper(*b);
		if (ca < cb) return -1;
		if (ca > cb) return 1;
		a++; b++;
	}
	if (*a) return -1;
	if (*b) return 1;
	return 0;
}
#endif

/* return true if source ends in end, possibly with extra space, or missing SINGLE space */
static char *checkend(char *source, char *end, APRS_PARSED_INFO_S *Info)
{	int ls=strlen(source), le=strlen(end);
	if (ls >= le)
	{	if (!strcmp(&source[ls-le], end))
			return &source[ls-le];	/* found it easily! */
		if (source[ls-1] == ' ' && ls > le)
		{	char *e, s, *p;
			for (e=&source[ls-1]; e>=source+le; e--)
				if (*e != ' ') break;
			s = *++e; *e = 0;
			p = checkend(source, end, Info);
			*e = s;
			if (p)
			{
#ifdef VERBOSE
				fprintf(stderr,"FOUND Trailing (%s) for (%s) - ", source, end);
				{	int h;
					for (h=0; h<Info->Path.hopCount; h++)
					if (h != 1)
						fprintf(stderr,"%s%.*s", h?",":"",sizeof(Info->Path.Hops[h]),Info->Path.Hops[h]);
				}
				fprintf(stderr,"\n");
#endif
				return p;
			}
		}
	}
	if (le == 2 && end[1] == ' ')	/* Check it without the space */
	{	char temp[2], *p;
		temp[0] = end[0]; temp[1] = 0;
		if ((p=checkend(source,temp,Info)) != NULL)
		{
#ifdef VERBOSE
			fprintf(stderr,"FOUND (%s) for Trailing (%s)(%s) - ", source, end, temp);
			{	int h;
				for (h=0; h<Info->Path.hopCount; h++)
				if (h != 1)
					fprintf(stderr,"%s%.*s", h?",":"",sizeof(Info->Path.Hops[h]),Info->Path.Hops[h]);
			}
			fprintf(stderr,"\n");
#endif
			return p;
		}
	}
	return NULL;
}

void set_message_handler(char *CallSign, void (*pMessageCallback)(void *userarg, char *from, char *message), void *userarg)
{	if (digicall) free(digicall);
	digicall = _strdup(CallSign);
	MsgUserArg = userarg;
	MessageCallback = pMessageCallback;
}

void set_bulletin_handler(void (*pBulletinCallback)(void *userarg, char *from, char identifier, char *group, char *bulletin), void *userarg)
{	BullUserArg = userarg;
	BulletinCallback = pBulletinCallback;
}

#ifdef OLD_WAY
// (2^31 / 180) / 380926 semicircles per Base 91 unit
static const unsigned long b91val[4] = {23601572L, 259358L, 2851L, 32L};

// Convert Base91 to 2^31/180 degree units (11930464.7111 per degree)
static BOOL base91decode(char *s, signed long *l)
{
	unsigned char c;

	for (c=0, *l=0; c<4; c++)
	if (s[c] < '!' || s[c] > '|') return FALSE;
	else
	{
		s[c] -= '!';
		while (s[c])
		{
			*l += b91val[c];
			s[c]--;
		}
	}
	// Correct for Base91 bias.  Longitude will be in half units.
	*l = 1073741824L - *l;
	return TRUE;
}

// Constants for converting lat/lon to semicircles
static const long valtable[]  = {1193046471L, 119304647L, 11930465L, 1988411L,
   	                     198841L, 19884L, 1988L, 199L, 20L, 2L};

static void semicircledecode(signed long sslat, signed long sslon, unsigned char *lat, unsigned char *lon,
                  unsigned char *ns, unsigned char *ew)
{
	*ns = 'N';
	*ew = 'E';

	if (sslat < 0)
	{
		sslat = 0 - sslat;
		*ns = 'S';
	}
	
	if (sslon < 0)
	{
		sslon = 0 - sslon;
		*ew = 'W';
	}
	
	// Convert to decimal
	for (lat[0] = '0'; sslat >= valtable[1]; sslat -= valtable[1]) lat[0]++;
	for (lat[1] = '0'; sslat >= valtable[2]; sslat -= valtable[2]) lat[1]++;
	for (lat[2] = '0'; sslat >= valtable[3]; sslat -= valtable[3]) lat[2]++;
	for (lat[3] = '0'; sslat >= valtable[4]; sslat -= valtable[4]) lat[3]++;
	lat[4] = '.';
	for (lat[5] = '0'; sslat >= valtable[5]; sslat -= valtable[5]) lat[5]++;
	for (lat[6] = '0'; sslat >= valtable[6]; sslat -= valtable[6]) lat[6]++;
	if (lat[6] > '9') lat[6] = '9';
	
	// Convert to decimal
	for (lon[0] = '0'; sslon >= valtable[0]; sslon -= valtable[0]) lon[0]++;
	for (lon[1] = '0'; sslon >= valtable[1]; sslon -= valtable[1]) lon[1]++;
	for (lon[2] = '0'; sslon >= valtable[2]; sslon -= valtable[2]) lon[2]++;
	for (lon[3] = '0'; sslon >= valtable[3]; sslon -= valtable[3]) lon[3]++;
	for (lon[4] = '0'; sslon >= valtable[4]; sslon -= valtable[4]) lon[4]++;
	lon[5] = '.';
	for (lon[6] = '0'; sslon >= valtable[5]; sslon -= valtable[5]) lon[6]++;
	for (lon[7] = '0'; sslon >= valtable[6]; sslon -= valtable[6]) lon[7]++;
	if (lon[7] > '9') lon[7] = '9';
}
#else
int newbase91decode(char *s, int len, signed long *l)
{
	unsigned char c;
//printf("newbase91decode:Converting(%.*s)\n", len, s);
	for (c=0, *l=0; c<len; c++)
	if (s[c] < '!' || s[c] > '|') return FALSE;
	else
	{	*l *= 91;
		*l += s[c] - 33;
	}
//printf("Conversion(%.*s) is %ld\n", len, s, (long) *l);
	return TRUE;
}
#endif

static double degrees(double coord)
{	int neg = (coord<0)?-1:1;
	if (coord < 0) coord = -coord;
	return neg*(floor(coord/100)+fmod(coord,100)/60.0);
}

// Parse APRS packet contents

//#define CALLCOPY(d,s) { strncpy(d,s,sizeof(d)); d[sizeof(d)-1] = 0; }	/* Copy from null terminated call to [] array and null terminate/truncate dest */
#define CALLCOPY(d,s) CallCopy(d,s,sizeof(d))

static VOID CallCopy(char *d, char *s, size_t l)
{	strncpy(d,s,l);
	d[--l] = 0;
	while (--l && (!d[l] || d[l]==' ')) d[l] = 0;
}

#define PARSERR(a) {Info->ParseError=#a; return FALSE; }

static int internal_parse_aprs(char *InBuf, APRS_PARSED_INFO_S *Info)
{
	int packet_data_len;
	char *srccall, *destcall, *route;
//	unsigned long m;	/* For base 10 conversions */
	unsigned char *packet_data;
	unsigned char c = 0, d = 0;
	unsigned char decoded_lat[9] = {0}, decoded_lon[9] = {0};
	unsigned char *rx_lat, *rx_lon, rx_ns, rx_ew, *comment, nolatlon = 0;
#ifdef FUTURE
	float range, bearing;
	const symbol_mapping *sym_search;
	unsigned int gps_symbol;
#endif
	unsigned int i;
	unsigned char *e, *p, *b91;
	
	memset(Info, 0, sizeof(*Info));

	for (p=InBuf+strlen(InBuf)-1; p>=InBuf; p--)
		if (*p == '\n' || *p == '\r') *p=0;
		else break;

	srccall = InBuf;
	destcall = strchr(InBuf,'>');
	if (!destcall) PARSERR(Missing toCall)
	route = strchr(destcall,',');
	packet_data = strchr(destcall,':');
	if (!packet_data) PARSERR(Missing Payload)	/* Missing payload delimiter */
	if (!route || route > packet_data) route = packet_data;

	*destcall++ = 0;	/* Null terminate srccall */
	*route++ = 0;		/* Null terminate destcall */
	*packet_data++ = 0;	/* Null terminate route (hops) */
	packet_data_len = strlen(packet_data);

	{	int l = packet_data_len;
		//while (l && packet_data[l-1] == ' ') --l;	/* Ignore trailing spaces for CRC dupe check */
		Info->CRC32 = CRC32(packet_data, l);	/* Do the CRC before mucking around inside the packet */
	}

	CALLCOPY(Info->srcCall, srccall);
	CALLCOPY(Info->dstCall, destcall);

	CALLCOPY(Info->Path.Hops[0], srccall);
	CALLCOPY(Info->Path.Hops[1], destcall);
	Info->Path.hopUnused = Info->Path.hopCount = 2;

	Info->latAmbiguity = Info->lonAmbiguity = 0;

	if (route < packet_data)	/* Otherwise it was direct */
	{
		while (*route && Info->Path.hopCount < MAX_HOPS)
		{	char *e = strchr(route,',');	/* Find the comma */
			if (e) *e++ = 0;	/* Null term element */
			else e = strchr(route,0);	/* Must be the last one! */
			CALLCOPY(Info->Path.Hops[Info->Path.hopCount], route);
			if (strchr(Info->Path.Hops[Info->Path.hopCount],'*'))	/* This was used, next must be first unused */
				Info->Path.hopUnused = Info->Path.hopCount+1;
			if (*Info->Path.Hops[Info->Path.hopCount] == 'T'
			&& (!strncmp(Info->Path.Hops[Info->Path.hopCount],"TCPIP",5)
			|| !strncmp(Info->Path.Hops[Info->Path.hopCount],"TCPXX",5)))
				Info->Path.TCPIP = TRUE;
			Info->Path.hopCount++;	/* keep this side effect OUTSIDE the macro! */
			route = e;
		}
		if (Info->Path.hopCount >= MAX_HOPS)
			TraceLogThread("TooManyHops", TRUE, "%s>%s has Too Many Hops %ld/%ld\n",
							Info->Path.Hops[0], Info->Path.Hops[1],
							(long) Info->Path.hopCount, (long) MAX_HOPS);
	}

	Info->Valid |= APRS_HOPS_VALID;

	for (i=Info->Path.hopCount-1; ((int)i)>=0; i--)
	{	if (*Info->Path.Hops[i] == 'q')	/* Found the q! */
		{	CALLCOPY(Info->qCode,Info->Path.Hops[i]);
#ifdef VERBOSE
			if (i==Info->Path.hopCount-1)
			{	fprintf(stderr,"Huh?  q(%s) is %ld/%ld?\n", Info->Path.Hops[i], (long) i, (long) Info->Path.hopCount);
			} else if (!strcmp(Info->Path.Hops[i], "qAI"))	/* Trace packet */
			{	fprintf(stderr,"Trace: ");
				for (c=i+1; c<Info->hopCount; c++)
					fprintf(stderr,"%s ",Info->Hops[c]);
				fprintf(stderr,"\n");
			} else
#endif
			if (!strcmp(Info->Path.Hops[i], "qAS"))	/* Server logon follows (call it the iGate) */
			{	CALLCOPY(Info->relayCall, Info->Path.Hops[i+1]);
				for (c=0; c<i; c++)
				{	if (strchr(Info->Path.Hops[c],'*'))
					{	CALLCOPY(Info->entryCall,Info->Path.Hops[c]);
						if (c && !strncmp(Info->Path.Hops[c],"WIDE",4))
						{	CALLCOPY(Info->entryCall,Info->Path.Hops[c-1]);
						}
						break;
					}
				}
#ifdef DONT_THINK_SO
				if (!strncmp(*entry,"WIDE",4))
				{	*entry = *relay;
					*relay = "";
				}
#endif
			} else if (!strcmp(Info->Path.Hops[i], "qAR")	/* Gated directly by the following server */
			|| !strcmp(Info->Path.Hops[i], "qAr")		/* Gated indirectly from an iGate */
			|| !strcmp(Info->Path.Hops[i], "qAo")		/* Received from client, previously gated? (logon matches) */
			|| !strcmp(Info->Path.Hops[i], "qAO"))		/* Received from client, previously gated? (logon mismatch) */
			{	CALLCOPY(Info->relayCall,Info->Path.Hops[i+1]);
				for (c=0; c<i; c++)
				{	if (strchr(Info->Path.Hops[c],'*'))
					{	CALLCOPY(Info->entryCall,Info->Path.Hops[c]);
						if (c && !strncmp(Info->Path.Hops[c],"WIDE",4))
						{	CALLCOPY(Info->entryCall,Info->Path.Hops[c-1]);
						}
						break;
					}
				}
			} else if (!strcmp(Info->Path.Hops[i], "qAC")	/* Received directly by following server (verified client) */
			|| !strcmp(Info->Path.Hops[i], "qAX")		/* Received directly by following server (unverified client) */
			|| !strcmp(Info->Path.Hops[i], "qAU"))		/* Received directly via UDP by following server */
			{	CALLCOPY(Info->relayCall,Info->Path.Hops[i+1]);
				for (c=0; c<i; c++)
				{	if (strchr(Info->Path.Hops[c],'*'))
					{	CALLCOPY(Info->entryCall,Info->Path.Hops[c]);
						break;
					}
				}
			}
#ifdef VERBOSE
			else fprintf(stderr,"Unrecognized q construct[%ld/%ld] %s\n", (long) i, (long) Info->Path.hopCount, Info->Path.Hops[i]);
#endif
			break;	/* Only process one q Construct (the last one) per message */
		}
	}
#ifdef VERBOSE
	if ((int)i < 0) fprintf(stderr,"Huh?  No q Construct in %ld Pieces?\n", (long) Info->hopCount);
#endif

//		PACKET_DATA_SIZE = (int) (&InBuf[sizeof(InBuf)] - packet_data);
	
	
	rx_lat = &decoded_lat[1];
	rx_lon = decoded_lon;
	comment = "";
	Info->symbol = '/';	/* / = Red Dot */
	Info->Valid |= APRS_SYMBOL_DEFAULTED;
	Info->ObjectKilled = 0;
	b91 = 0;
//	get_callsign(config->fromcall, digicall, 0);
	
	// TODO: Handle symbol overlays properly
	
	// Handle 3rd-party traffic header
	// eg }W6AHM>APRS,TCPIP,N6EX-3*:@230135z3350.28N/11818.85W_269/010...
	// or KJ4ERJ-2>APJI23,WX4MLB-3,WIDE2*:}N4RTD-9>R74Y5T,TCPIP,KJ4ERJ-2*:`n1$""R&j/]""4$}Monitoring 145.21 PL 77 repeater=
	if (packet_data[0] == '}')	/* 3rd party traffic */
	{
		srccall = packet_data+1;
		// Find end of source call
		destcall = strchr(srccall, '>');
		if (destcall == NULL) PARSERR(3rd Missing toCall)
		if (destcall-packet_data > 11) PARSERR(3rd Short)
		// Extract original source call
		route = strchr(destcall,',');
		if (!route) route = strchr(destcall,':');
		if (!route) PARSERR(3rd Missing Path)
		p = strchr(route,':') + 1;	/* End of header */
		if (p == (char*)1) PARSERR(3rd Missing Payload)

		*destcall++ = 0;	/* Null term and point to destination */
		*route++ = 0;	/* Null term destcall */

		CALLCOPY(Info->thirdCall, Info->srcCall);	/* Source is actually third-party handler */
		CALLCOPY(Info->srcCall, srccall);
		CALLCOPY(Info->dstCall, destcall);

		Info->ThirdPath = Info->Path;	/* This is busted because of the copy-down packet below */

		CALLCOPY(Info->Path.Hops[0], srccall);
		CALLCOPY(Info->Path.Hops[1], destcall);
		Info->Path.hopUnused = Info->Path.hopCount = 2;

		// Copy down parse buffer, overwriting 3rd-party header
		strcpy(packet_data, p);
		packet_data_len = strlen(packet_data);
		srccall = Info->srcCall;
		destcall = Info->dstCall;
	}
	
	// First character is data type identifier
	Info->datatype = packet_data[0];
	Info->Valid |= APRS_DATATYPE_VALID;
	switch (packet_data[0])
	{
/* USNA-1>APRFID,WB4APR-3*,WIDE2-1,qAR,N3UJJ:<0x03><0x02>2500ABDB6530 */
	case 0x03:	/* RFID Reader carryover */
		if (packet_data_len < 1 || packet_data[1] != 0x02) PARSERR(RFID Invalid)
		memmove(&packet_data[0], &packet_data[1], packet_data_len--);
		Info->datatype = packet_data[0];
/* USNA-1>APRFID,WB4APR-3*,WIDE2-1,qAR,N3UJJ:<0x02>2500ABDB6530 */
	case 0x02:	/* RFID Reader (0x03 modified falls into here */
	case 'x':	/* Temporary test for Lynn's RFID "reader" */
	{	int i;
		if (packet_data_len < 1) PARSERR(RFID Short)
		comment = &packet_data[1];
		for (i=0; i<sizeof(Info->Comment)-1 && *comment; comment++)
		{	if (isprint(*comment&0xff))
				Info->Comment[i++] = *comment;
		}
		Info->Valid |= APRS_SYMBOL_VALID;
		Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
		Info->symbol = 'A' | ALT_TABLE | ('R'<<16);	/* Symbol = RA */
		return TRUE;
	}
	case '=':	/* Message capable */
		Info->MessageCapable = TRUE;
	case '!':	/* Falls into here */
		if (packet_data_len < 1) PARSERR(Short Position)
		if (packet_data[1] == '!')	/* Some weather data */
		{	strncpy(Info->Comment, &packet_data[1], sizeof(Info->Comment));
			Info->symbol = '_';	/* _ = Weather Station */
			Info->Valid |= APRS_SYMBOL_VALID;
			Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
			ParsePEETWeather(packet_data, Info);
			return TRUE;
		}

		// Position
		//if (packet_data[1] == '/' || packet_data[1] == '\\')
		if (!isdigit(packet_data[1]&0xff))
		{	// Compressed format
			if (packet_data_len < 12) PARSERR(Short Compressed)
			if (!IsValidOverlay(packet_data[1])) PARSERR(Invalid Overlay)
			if (packet_data[1] >= 'a' && packet_data[1] <= 'j')
				packet_data[1] = packet_data[1] - 'a' + '0';	/* 0-9 in compressed */
			Info->symbol = packet_data[10] | (packet_data[1] == '/' ? STD_TABLE : ALT_TABLE);
			if (packet_data[1] != '/' && packet_data[1] != '\\') Info->symbol |= packet_data[1]<<16;
			/* if (packet_data[11] == ' ')
				comment = &packet_data[12];
			else */
			if (packet_data_len < 14) PARSERR(Missing csT)
			else comment = &packet_data[14];	/* Skip the csT bytes */
			b91 = &packet_data[2];
		} else
		{
			if (packet_data_len < 20) PARSERR(Short Position)
			if (!IsValidOverlay(packet_data[9])) PARSERR(Invalid Overlay)
			Info->symbol = packet_data[19] | (packet_data[9] == '/' ? STD_TABLE : ALT_TABLE);
			if (packet_data[9] != '/' && packet_data[9] != '\\') Info->symbol |= packet_data[9]<<16;
			rx_lat = &packet_data[1];
			rx_ns = packet_data[8];
			rx_lon = &packet_data[10];
			rx_ew = packet_data[18];
			comment = &packet_data[20];
		}
		Info->Valid |= APRS_SYMBOL_VALID;
		Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
		break;
		
/*
W4GCW>WIDE1-1,WX4MLB-3*,WIDE2-1,qAR,KJ4ERJ:
@160839h/@4Q{:*+hP z/A=000082 W4GCW@cfl.rr.com {UIV32}
0123456789012345678901234567890123456789012345678901234567890
          1         2         3         4         5         6
*/
/* W4GCW>APU25N,WX4MLB-3*,WIDE2-2,qAR,KJ4ERJ-2:
@180912h/@5?R:)vEP  z/A=000055 W4GCW@cfl.rr.com {UIV32}
0123456789012345678901234567890123456789012345678901234567890
          1         2         3         4         5         6
*/

	case '@':	/* Message capable */
		Info->MessageCapable = TRUE;
	case '/':	/* Falls into here */
		if (packet_data_len < 8) PARSERR(Short Position)
		// Position with timestamp
		//if (packet_data[8] == '/' || packet_data[8] == '\\')

		Info->Time.type = packet_data[7];
		if (packet_data[7] == 'h')
		{	if (cFromDec(&packet_data[1], 2, &Info->Time.hour)
			&& cFromDec(&packet_data[3], 2, &Info->Time.minute)
			&& cFromDec(&packet_data[5], 2, &Info->Time.second))
				Info->Valid |= APRS_TIME_VALID;
			else PARSERR(NonNumeric Date)
		} else if (packet_data[7] == 'z')
		{	if (cFromDec(&packet_data[1], 2, &Info->Time.day)
			&& cFromDec(&packet_data[3], 2, &Info->Time.hour)
			&& cFromDec(&packet_data[5], 2, &Info->Time.minute))
				Info->Valid |= APRS_DATE_VALID | APRS_TIME_VALID;
			else PARSERR(NonNumeric Date)
		} else if (packet_data[7] == '/')
		{	if (cFromDec(&packet_data[1], 2, &Info->Time.day)
			&& cFromDec(&packet_data[3], 2, &Info->Time.hour)
			&& cFromDec(&packet_data[5], 2, &Info->Time.minute))
				Info->Valid |= APRS_DATE_VALID | APRS_TIME_VALID;
			else PARSERR(NonNumeric Date)
			//TraceLogThread("Timestamps", TRUE, "Local Time (%c) from %s>%s\n",
			//		packet_data[7], Info->srcCall, Info->dstCall);
		} else
		{	TraceLogThread("Timestamps", TRUE, "Unrecognized Timestamp (%c) from %s>%s in %s\n",
					packet_data[7], Info->srcCall, Info->dstCall, packet_data);
			PARSERR(Invalid Date)
		}

		if (!isdigit(packet_data[8]&0xff))
		{	// Compressed format
/* Bogus Base91 Parse: */
/* "W4GCW>APT311,WIDE1-1,WX4MLB-3,WC4PEM-14,WIDE2*,qAR,K2KZ-3:/17W>079/000/A=000075"
                                                              0123456789012345678901234567890123456789012345678901234567890
                                                                        1         2         3         4         5         6
*/
			if (packet_data_len < 19) PARSERR(Short Compressed)
			if (!IsValidOverlay(packet_data[8])) PARSERR(Invalid Overlay)
			if (packet_data[8] >= 'a' && packet_data[8] <= 'j')
				packet_data[8] = packet_data[8] - 'a' + '0';	/* 0-9 in compressed */
			Info->symbol = packet_data[17] | (packet_data[8] == '/' ? STD_TABLE : ALT_TABLE);
			if (packet_data[8] != '/' && packet_data[8] != '\\') Info->symbol |= packet_data[8]<<16;
			/* if (packet_data[18] == ' ')
				comment = &packet_data[19];
			else */
			if (packet_data_len < 21) PARSERR(Missing csT)
			else comment = &packet_data[21];	/* Skip the csT bytes */
			b91 = &packet_data[9];
		} else
		{	if (packet_data_len < 27) PARSERR(Short Position)
			if (!IsValidOverlay(packet_data[16])) PARSERR(Invalid Overlay)
			Info->symbol = packet_data[26];
			if (packet_data[16] != '/') Info->symbol |= ALT_TABLE;
			if (packet_data[16] != '/' && packet_data[16] != '\\') Info->symbol |= packet_data[16]<<16;
			rx_lat = &packet_data[8];
			rx_ns = packet_data[15];
			rx_lon = &packet_data[17];
			rx_ew = packet_data[25];
			comment = &packet_data[27];
		}
		Info->Valid |= APRS_SYMBOL_VALID;
		Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
		break;
		
	case ';':
		if (packet_data_len < 18) PARSERR(Short Object)
		// Object report
		// Check to see if it's been killed
/*
;JaneGreen_001136h2806.40N/08055.65W;Jane Green Camp
*/
		if (packet_data[10] == '_') Info->ObjectKilled = 1;
		else if (packet_data[10] != '*') PARSERR(Invalid Object)
		// Mark end of name with null
		for (c=9; c>1; c--) if (packet_data[c] != ' ') break;
		packet_data[c+1] = 0;
		CALLCOPY(Info->objCall, &packet_data[1]);
		Info->Valid |= APRS_OBJECT_VALID;

		Info->Time.type = packet_data[17];
		if (packet_data[17] == 'h')
		{	if (cFromDec(&packet_data[11], 2, &Info->Time.hour)
			&& cFromDec(&packet_data[13], 2, &Info->Time.minute)
			&& cFromDec(&packet_data[15], 2, &Info->Time.second))
				Info->Valid |= APRS_TIME_VALID;
		} else if (packet_data[17] == 'z')
		{	if (cFromDec(&packet_data[11], 2, &Info->Time.day)
			&& cFromDec(&packet_data[13], 2, &Info->Time.hour)
			&& cFromDec(&packet_data[15], 2, &Info->Time.minute))
				Info->Valid |= APRS_DATE_VALID | APRS_TIME_VALID;
		}

		//if (packet_data[18] == '/' || packet_data[18] == '\\')
		if (!isdigit(packet_data[18]&0xff))
		{	// Compressed format
			if (packet_data_len < 29) PARSERR(Short Compressed)
			if (!IsValidOverlay(packet_data[18])) PARSERR(Invalid Overlay)
			if (packet_data[18] >= 'a' && packet_data[18] <= 'j')
				packet_data[18] = packet_data[18] - 'a' + '0';	/* 0-9 in compressed */
			Info->symbol = packet_data[27] | (packet_data[18] == '/' ? STD_TABLE : ALT_TABLE);
			if (packet_data[18] != '/' && packet_data[18] != '\\') Info->symbol |= packet_data[18]<<16;
			/* if (packet_data[28] == ' ')
				comment = &packet_data[29];
			else */
			if (packet_data_len < 31) PARSERR(Missing csT)
			else comment = &packet_data[31];	/* Skip the csT bytes */
			b91 = &packet_data[19];
		} else
		{	if (packet_data_len < 37) PARSERR(Short Object)
			if (!IsValidOverlay(packet_data[26])) PARSERR(Invalid Overlay)
			Info->symbol = packet_data[36];
			if (packet_data[26] != '/') Info->symbol |= ALT_TABLE;
			if (packet_data[26] != '/' && packet_data[26] != '\\') Info->symbol |= packet_data[26]<<16;
			rx_lat = &packet_data[18];
			rx_ns = packet_data[25];
			rx_lon = &packet_data[27];
			rx_ew = packet_data[35];
			comment = &packet_data[37];
		}
		Info->Valid |= APRS_SYMBOL_VALID;
		Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
		break;
		
	case ')':	// This code is duplicated under ITEM_IN_MESSAGE
		// Item report - note that items have variable-length names
		// Find and mark end of name
		for (c=1; c<min(packet_data_len,10); c++)
			if (packet_data[c] == '!' || packet_data[c] == '_')
				break;
		if (packet_data[c] != '!' && packet_data[c] != '_') PARSERR(Obj Too Long);
		// Check to see if it's been killed
		if (packet_data[c] == '_') Info->ObjectKilled = 1;
		packet_data[c] = 0;
		if (packet_data_len < c+1) PARSERR(Short Item)
		CALLCOPY(Info->objCall, &packet_data[1]);
		Info->Valid |= APRS_ITEM_VALID;
		//if (packet_data[c+1] == '/' || packet_data[c+1] == '\\')
		if (!isdigit(packet_data[c+1]&0xff))
		{	// Compressed format
			if (packet_data_len < c+12) PARSERR(Short Compressed)
			if (!IsValidOverlay(packet_data[c+1])) PARSERR(Invalid Overlay)
			if (packet_data[c+1] >= 'a' && packet_data[c+1] <= 'j')
				packet_data[c+1] = packet_data[c+1] - 'a' + '0';	/* 0-9 in compressed */
			Info->symbol = packet_data[c+10] | (packet_data[c+1] == '/' ? STD_TABLE : ALT_TABLE);
			if (packet_data[c+1] != '/' && packet_data[c+1] != '\\') Info->symbol |= packet_data[c+1]<<16;
			/* if (packet_data[c+11] == ' ')
				comment = &packet_data[c+12];
			else */
			if (packet_data_len < c+14) PARSERR(Missing csT)
			else comment = &packet_data[c+14];	/* Skip the csT bytes */
			b91 = &packet_data[c+2];
		} else
		{	if (packet_data_len < c+20) PARSERR(Short Item)
			if (!IsValidOverlay(packet_data[c+9])) PARSERR(Invalid Overlay)
			Info->symbol = packet_data[c+19];
			if (packet_data[c+9] != '/') Info->symbol |= ALT_TABLE;
			if (packet_data[c+9] != '/' && packet_data[c+9] != '\\') Info->symbol |= packet_data[c+9]<<16;
			rx_lat = &packet_data[c+1];
			rx_ns = packet_data[c+8];
			rx_lon = &packet_data[c+10];
			rx_ew = packet_data[c+18];
			comment = &packet_data[c+20];
		}
		Info->Valid |= APRS_SYMBOL_VALID;
		Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
		break;
		
	case '`':
	case 0x27:
	{	BOOL KnownPlatform=FALSE;
		if (packet_data_len < 9) PARSERR(Short Mic-E)
		for (c=0; c<6; c++)
			if (!destcall[c])	/* nulls not valid */
				PARSERR(Short Mic-E toCall)
			else if (!strchr(miclatvalid, destcall[c]))
				PARSERR(Invalid Mic-E)
		if (destcall[6] && destcall[6]!='-') PARSERR(Invalid Mic-E)	/* 6 and only 6 characters */
		if (packet_data[1] < 38 || packet_data[1] > 127) PARSERR(Invalid Mic-E)
		if (packet_data[2] < 38 || packet_data[2] > 97) PARSERR(Invalid Mic-E)
		if (packet_data[3] < 28 || packet_data[3] > 127) PARSERR(Invalid Mic-E)
		if (packet_data[4] < 28 || packet_data[4] > 127) PARSERR(Invalid Mic-E)
		if (packet_data[5] < 28 || packet_data[5] > 125) PARSERR(Invalid Mic-E)
		if (packet_data[6] < 28 || packet_data[6] > 127) PARSERR(Invalid Mic-E)
		
		c = 0;
		if (destcall[0]>='P' && destcall[0]<='Z') c |= 04;
		else if (destcall[0]>='A' && destcall[0]<='K') c |= 040;
		if (destcall[1]>='P' && destcall[1]<='Z') c |= 02;
		else if (destcall[1]>='A' && destcall[1]<='K') c |= 020;
		if (destcall[2]>='P' && destcall[2]<='Z') c |= 01;
		else if (destcall[2]>='A' && destcall[2]<='K') c |= 010;

		if (c&07 && c&070)
		{	Info->MicEMessage = "unknown";
			TraceLogThread("Invalids", TRUE, "Invalid Mic-E Status 0x%lX from %s via %s\n",
					(long) c, Info->srcCall, Info->Path.Hops[Info->Path.hopCount-1]);
			PARSERR(Invalid Mic-E Status)
		} else if (!c)
			Info->MicEMessage = "EMERGENCY!";
		else if (c&07)
		{static char *MicMessage[] = { "EMERGENCY!", "Priority", "Special", "Committed", "Returning", "In Service", "En Route", "Off Duty" };
			Info->MicEMessage = MicMessage[c];
		} else if (c&070)
		{static char *MicCustom[] = { "EMERGENCY!", "Custom-6", "Custom-5", "Custom-4", "Custom-3", "Custom-2", "Custom-1", "Custom-0" };
			Info->MicEMessage = MicCustom[c>>3];
		} else Info->MicEMessage = "unknown";
		Info->Valid |= APRS_MICE_MESSAGE_VALID;

		// Mic-E format
		// Decode latitude using lookup table
		decoded_lat[1] = miclat[destcall[0] - '0'];
		decoded_lat[2] = miclat[destcall[1] - '0'];
		decoded_lat[3] = miclat[destcall[2] - '0'];
		decoded_lat[4] = miclat[destcall[3] - '0'];
		decoded_lat[5] = '.';
		decoded_lat[6] = miclat[destcall[4] - '0'];
		decoded_lat[7] = miclat[destcall[5] - '0'];		
		// Decode hemispheres
		rx_ns = destcall[3] < 'P' ? 'S' : 'N';
		rx_ew = destcall[5] < 'P' ? 'E' : 'W';
		// Decode longitude degrees
		c = packet_data[1] - 28;
		if (destcall[4] >= 'P') c += 100;
		if (c >= 180 && c <= 189) c -= 80;
		else if (c >= 190 && c <= 199) c -= 190;
		// Convert to decimal
		for (decoded_lon[0] = '0'; c >= 100; c -= 100) decoded_lon[0]++;
		for (decoded_lon[1] = '0'; c >= 10 ; c -= 10 ) decoded_lon[1]++;
		for (decoded_lon[2] = '0'; c >= 1  ; c -= 1  ) decoded_lon[2]++;
		// Decode longitude minutes
		c = packet_data[2] - 28;
		if (c >= 60) c -= 60;
		// Convert to decimal
		for (decoded_lon[3] = '0'; c >= 10 ; c -= 10 ) decoded_lon[3]++;
		for (decoded_lon[4] = '0'; c >= 1  ; c -= 1  ) decoded_lon[4]++;
		// Decode longitude hundredths
		decoded_lon[5] = '.';
		c = packet_data[3] - 28;
		for (decoded_lon[6] = '0'; c >= 10 ; c -= 10 ) decoded_lon[6]++;
		for (decoded_lon[7] = '0'; c >= 1  ; c -= 1  ) decoded_lon[7]++;

#ifdef VERBOSE
#define ISBAD(d) (!isdigit(d&0xff)&&d!=' ')
		if (ISBAD(decoded_lat[1])
		|| ISBAD(decoded_lat[2])
		|| ISBAD(decoded_lat[3])
		|| ISBAD(decoded_lat[4])
		|| ISBAD(decoded_lat[6])
		|| ISBAD(decoded_lat[7])

		|| ISBAD(decoded_lon[0])
		|| ISBAD(decoded_lon[1])
		|| ISBAD(decoded_lon[2])
		|| ISBAD(decoded_lon[3])
		|| ISBAD(decoded_lon[4])
		|| ISBAD(decoded_lon[6])
		|| ISBAD(decoded_lon[7]))
		TraceLogThread("Invalids", TRUE, "NonNumeric Mic-E %s decoded_lat(%.7s) decoded_lon(%.8s) via %s Dst:%ld %ld %ld %ld %ld %ld  Pkt:%ld %ld %ld\n",
					Info->srcCall, &decoded_lat[1], decoded_lon, Info->Path.Hops[Info->Path.hopCount-1],
					destcall[0]&0xff, destcall[1]&0xff, destcall[2]&0xff, destcall[3]&0xff, destcall[4]&0xff, destcall[5]&0xff, 
					packet_data[1]&0xff, packet_data[2]&0xff, packet_data[3]&0xff);
#endif
		/* This needs to be after all references to destcall due to third party Mic-E */
		strncpy(Info->dstCall, "MICE", sizeof(Info->dstCall));

		// Get course and speed
		Info->speed = packet_data[4] - 28;
		if (Info->speed >= 80) Info->speed -= 80;
		Info->speed *= 10;
		{	char t = packet_data[5] - 28;
			Info->speed += t/10;
			Info->course = t%10;
			if (Info->course >= 4) Info->course -= 4;
			Info->course *= 100;
		}
		Info->course += packet_data[6] - 28;
		if (Info->course == 360) Info->course = 0;
		Info->Valid |= APRS_CRSSPD_VALID;

		// Get symbol
		if (!IsValidOverlay(packet_data[8])) PARSERR(Invalid Overlay)
		Info->symbol = packet_data[7] | (packet_data[8] == '/' ? STD_TABLE : ALT_TABLE);
		if (packet_data[8] != '/' && packet_data[8] != '\\') Info->symbol |= packet_data[8]<<16;
		Info->Valid |= APRS_SYMBOL_VALID;
		Info->Valid &= ~APRS_SYMBOL_DEFAULTED;

		comment = &packet_data[9];

#ifdef OBSOLETE_MICE_TELEMETRY
		// And get any telemetry that might be available
		if (packet_data[9] == '`'	/* Chan 1 & 3 hex telemetry */
		&& packet_data[10] != '_'	/* Not a comment-less Yaesu */
		&& packet_data[13] != '}')	/* But not a message Mic-E + Altitude */
		{	unsigned long t1, t3;
			if (FromHex(&packet_data[10], 2, &t1)
			&& FromHex(&packet_data[12], 2, &t3))
			{	Info->Telemetry.Analog[0] = (unsigned short) t1;
				Info->Telemetry.Analog[2] = (unsigned short) t3;
				Info->Valid |= APRS_TELEMETRY_VALID;
TraceLogThread("Telemetry", TRUE, "%s Mic-E1/3(%.5s)\n", Info->srcCall, &packet_data[9]);
				comment = &packet_data[14];
			}
else TraceLogThread("Telemetry", FALSE, "%s NOT Mic-E1/3(%s)\n", Info->srcCall, comment);
		} else if (packet_data[9] == '\''	/* 5 channel hex telemetry */
		&& packet_data[13] != '}')	/* But not a non-message Mic-E + Altitude */
		{	unsigned long t1, t2, t3, t4, t5;
			if (FromHex(&packet_data[10], 2, &t1)
			&& FromHex(&packet_data[12], 2, &t2)
			&& FromHex(&packet_data[14], 2, &t3)
			&& FromHex(&packet_data[16], 2, &t4)
			&& FromHex(&packet_data[18], 2, &t5))
			{	Info->Telemetry.Analog[0] = (unsigned short) t1;
				Info->Telemetry.Analog[1] = (unsigned short) t2;
				Info->Telemetry.Analog[2] = (unsigned short) t3;
				Info->Telemetry.Analog[3] = (unsigned short) t4;
				Info->Telemetry.Analog[4] = (unsigned short) t5;
				Info->Valid |= APRS_TELEMETRY_VALID;
TraceLogThread("Telemetry", TRUE, "%s Mic-EHex(%.11s)\n", Info->srcCall, &packet_data[9]);
				comment = &packet_data[20];
			}
else TraceLogThread("Telemetry", FALSE, "%s NOT Mic-EHex(%.11s)\n", Info->srcCall, &packet_data[9]);
		} else if (packet_data[9] == 0x1d)	/* 5 channel binary telemetry */
		{	Info->Telemetry.Analog[0] = packet_data[10];
			Info->Telemetry.Analog[1] = packet_data[11];
			Info->Telemetry.Analog[2] = packet_data[12];
			Info->Telemetry.Analog[3] = packet_data[13];
			Info->Telemetry.Analog[4] = packet_data[14];
			Info->Valid |= APRS_TELEMETRY_VALID;
TraceLogThread("Telemetry", TRUE, "%s Mic-E5(0x%lX 0x%lX 0x%lX 0x%lX 0x%lX 0x%lX)\n", Info->srcCall, (long) packet_data[9]&0xff, (long) packet_data[10]&0xff, (long) packet_data[11]&0xff, (long) packet_data[12]&0xff, (long) packet_data[13]&0xff, (long) packet_data[14]&0xff);
			comment = &packet_data[15];
		}
#endif

/* KJ4ERJ-8>2WUY7X,WIDE1-1,WIDE2-1,qAR,KJ4ERJ-2:`lCPm^bv/>"4'} */
/* KQ4KK-9>RW5Q1X,WC4PEM-10,WIDE1*,WIDE2-1     :`n0j"iV>/]"4(}= */
/* N4JZY-9>SU1R2Y,WIDE2-2,qAR,W4DEX            :`lN?l?\v/]"6A}= */
/* SV1UY-7>DI5P99,WIDE1-1,WIDE3-3,qAR,SV8BUR-10:`0Enl \b/`"3r}Demetre. Lefkas_ (Vx8r?) */
/* AB9HW-9>TS4X7R,WIDE1-1,WIDE2-2,qAS,KB9KTD-1 :`w)il"Vj/>"61} */
/* KC9OAG-7>SYTU1U,N9AMN-2*,WIDE2,qAR,WD9EKA   :`r,'l"cv/`"6L}Monitoring 146.67 _  */
/* W6GPS>TYRW16,WIDE1-1,WIDE2-1,qAR,DB0VOX:`'\Xl5\K\>= (D72) */

/* From http://aprs.org/aprs12/mic-e-types.txt through 31 May 15 (Updated 2015/06/03) */
		switch (*comment)
		{
		case ' ':	/* Original, non-message capable Mic-E */
			comment++;
			strncpy(Info->Platform, "Mic-E", sizeof(Info->Platform));
			Info->tPlatform = PLATFORM_MICE;
			Info->Valid |= APRS_PLATFORM_VALID;
			KnownPlatform = TRUE;
			break;
		case ']':	/* D700 */
			comment++;
			Info->MessageCapable = TRUE;
			strncpy(Info->Platform, "Kenwood D700", sizeof(Info->Platform));
			Info->tPlatform = PLATFORM_KENWOOD_D700;
			Info->Valid |= APRS_PLATFORM_VALID;
			KnownPlatform = TRUE;

			// if (strlen(comment)>=1 && comment[strlen(comment)-1] == '=')	/* D710 is an updated D700 (trailing =) */
			if ((p=checkend(comment, "=", Info)) != NULL)
			{	strncpy(Info->Platform, "Kenwood D710", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_KENWOOD_D710;
				*p = 0;	/* Null out trailing */
			}
			if ((p=checkend(comment, "v", Info)) != NULL)
			{	strncpy(Info->Platform, "future D7xx", sizeof(Info->Platform));
				*p = 0;	/* Null out trailing */
			}
			break;
		case '>':	/* D7 */
			comment++;
			Info->MessageCapable = TRUE;
			Info->Valid |= APRS_PLATFORM_VALID;
			strncpy(Info->Platform, "Kenwood D7", sizeof(Info->Platform));
			Info->tPlatform = PLATFORM_KENWOOD_D7;
			KnownPlatform = TRUE;
			if ((p=checkend(comment, "=", Info)) != NULL)
			{	strncpy(Info->Platform, "Kenwood D72", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_KENWOOD_D72;
				*p = 0;	/* Null out trailing */
			} else if ((p=checkend(comment, "^", Info)) != NULL)
			{	strncpy(Info->Platform, "Kenwood D74", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_KENWOOD_D74;
				*p = 0;	/* Null out trailing */
			} else if ((p=checkend(comment, "v", Info)) != NULL)
			{	strncpy(Info->Platform, "future TH-D7A", sizeof(Info->Platform));
				*p = 0;	/* Null out trailing */
			}
			break;
		case '`':	/* Message capable Other Mic-E */
			comment++;
			Info->MessageCapable = TRUE;
			Info->Valid |= APRS_PLATFORM_VALID;
			Info->tPlatform = PLATFORM_MICE;
			strncpy(Info->Platform, "Mic-E msg", sizeof(Info->Platform));
			if (strlen(comment) >= 2)
			{	strcat(Info->Platform, "(");
				strncat(Info->Platform, &comment[strlen(comment)-2], 2);	/* last 2 ends in null! */
				strcat(Info->Platform, ")");
			}
			//if (strlen(comment)>=2 && comment[strlen(comment)-2] == '_' && comment[strlen(comment)-1] == ' ')	/* Yaesu VX-8R */
			if ((p=checkend(comment, "_ ", Info)) != NULL)
			{	strncpy(Info->Platform, "Yaesu VX-8R", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_YAESU_VX8R;
				*p = 0;	/* Null out trailing _<space> */
				KnownPlatform = TRUE;
			}
			if ((p=checkend(comment, "_\"", Info)) != NULL)
			{	strncpy(Info->Platform, "Yaesu FTM-350", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_YAESU_FTM350;
				*p = 0;	/* Null out trailing _<space> */
				KnownPlatform = TRUE;
			}
			if ((p=checkend(comment, "_%", Info)) != NULL)
			{	strncpy(Info->Platform, "Yaesu FTM-400DR", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_YAESU_FTM400DR;
				*p = 0;	/* Null out trailing _<space> */
				KnownPlatform = TRUE;
			}
			if ((p=checkend(comment, "_#", Info)) != NULL)
			{	strncpy(Info->Platform, "Yaesu VX-8G", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_YAESU_VX8G;
				*p = 0;	/* Null out trailing _<space> */
				KnownPlatform = TRUE;
			}
			if ((p=checkend(comment, "_$", Info)) != NULL)
			{	strncpy(Info->Platform, "Yaesu FT1D", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_YAESU_FT1D;
				*p = 0;	/* Null out trailing _<space> */
				KnownPlatform = TRUE;
			}
			if ((p=checkend(comment, "_(", Info)) != NULL)
			{	strncpy(Info->Platform, "Yaesu FT2D", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_YAESU_FT2D;
				*p = 0;	/* Null out trailing _<space> */
				KnownPlatform = TRUE;
			}
			if ((p=checkend(comment, "_)", Info)) != NULL)
			{	strncpy(Info->Platform, "Yaesu FTM-100D", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_YAESU_FTM100D;
				*p = 0;	/* Null out trailing _<space> */
				KnownPlatform = TRUE;
			}
			//if (strlen(comment)>=1 && comment[strlen(comment)-1] == '_')	/* Yaesu VX-8R (Truncated!) */
			//{	strncpy(Info->Platform, "VX-8R (nosp)", sizeof(Info->Platform));
			//	comment[strlen(comment)-2] = 0;	/* Null out trailing _<space> */
			//}
			break;
		case '\'':	/* Non-Message capable Other Mic-E */
			comment++;
			Info->Valid |= APRS_PLATFORM_VALID;
			strncpy(Info->Platform, "Mic-E trk", sizeof(Info->Platform));
			Info->tPlatform = PLATFORM_MICE;
			if (strlen(comment) >= 2)
			{	strcat(Info->Platform, "(");
				strncat(Info->Platform, &comment[strlen(comment)-2], 2);	/* last 2 ends in null! */
				strcat(Info->Platform, ")");
			}
			//if (strlen(comment)>=2 && comment[strlen(comment)-2] == '|' && comment[strlen(comment)-1] == '3')	/* Byonics TinyTrack3 (TT3) */
			if ((p=checkend(comment,"|3", Info)) != NULL)
			{	strncpy(Info->Platform, "Byonics TT3", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_TINYTRAK3;
				*p = 0;	/* Null out trailing |3 */
				KnownPlatform = TRUE;
			}
			//if (strlen(comment)>=2 && comment[strlen(comment)-2] == '|' && comment[strlen(comment)-1] == '4')	/* Byonics TinyTrack4 (TT4) */
			if ((p=checkend(comment,"|4", Info)) != NULL)
			{	strncpy(Info->Platform, "Byonics TT4", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_TINYTRAK4;
				*p = 0;	/* Null out trailing |3 */
				KnownPlatform = TRUE;
			}
			break;
		case 'T':	/* Manufacturer in next-to-last byte, version in last */
			if (strlen(comment)>=2	/* Got enough for manufacturer/version? */
			&& strchr("\\/`'^:;.*~",comment[strlen(comment-2)]))	/* is it a valid one? */
			{	comment++;
				Info->Valid |= APRS_PLATFORM_VALID;
				strncpy(Info->Platform, "Mic-E T", sizeof(Info->Platform));
				Info->tPlatform = PLATFORM_MICE;
				if (strlen(comment) >= 2)	/* Default to putting in raw manufacturer code */
				{	strcat(Info->Platform, "(");
					strncat(Info->Platform, &comment[strlen(comment)-2], 2);	/* last 2 ends in null! */
					strcat(Info->Platform, ")");
				}
				if (strlen(comment)>=2 && comment[strlen(comment)-2] == '\\')	/* Hamhud (unconfirmed 2008-10-21) */
				{	strncpy(Info->Platform, "Hamhud(", sizeof(Info->Platform));
					strcat(Info->Platform, &comment[strlen(comment)-1]);	/* last one ends in null! */
					strcat(Info->Platform, ")");
					Info->tPlatform = PLATFORM_HAMHUD;
					comment[strlen(comment)-2] = 0;	/* Null out trailing \v */
				}
				if (strlen(comment)>=2 && comment[strlen(comment)-2] == '/')	/* Argent (unconfirmed 2008-10-21) */
				{	strncpy(Info->Platform, "Argent(", sizeof(Info->Platform));
					strcat(Info->Platform, &comment[strlen(comment)-1]);	/* last one ends in null! */
					strcat(Info->Platform, ")");
					Info->tPlatform = PLATFORM_OPEN_TRACK;
					comment[strlen(comment)-2] = 0;	/* Null out trailing /v */
				}
				if (strlen(comment)>=2 && comment[strlen(comment)-2] == '^')	/* HinzTec anyfrog? (Added 2011/01/27) */
				{	strncpy(Info->Platform, "HinzTec(", sizeof(Info->Platform));
					strcat(Info->Platform, &comment[strlen(comment)-1]);	/* last one ends in null! */
					strcat(Info->Platform, ")");
					Info->tPlatform = PLATFORM_HINZTEC;
					comment[strlen(comment)-2] = 0;	/* Null out trailing ^v */
				}
				if (strlen(comment)>=2 && comment[strlen(comment)-2] == '*')	/* APOZxx www.KissOZ.dk Tracker. OZ1EKD and OZ7HVO (Added 2012/01/11) */
				{	strncpy(Info->Platform, "KissOZ(", sizeof(Info->Platform));
					strcat(Info->Platform, &comment[strlen(comment)-1]);	/* last one ends in null! */
					strcat(Info->Platform, ")");
					Info->tPlatform = PLATFORM_KISSOZ;
					comment[strlen(comment)-2] = 0;	/* Null out trailing ^v */
				}
/* `'-:;. are still undefined */
				if (strlen(comment)>=2 && comment[strlen(comment)-2] == '~')	/* OTHER (used when all others are allocated) */
				{	strncpy(Info->Platform, "Other(", sizeof(Info->Platform));
					strcat(Info->Platform, &comment[strlen(comment)-1]);	/* last one ends in null! */
					strcat(Info->Platform, ")");
					comment[strlen(comment)-2] = 0;	/* Null out trailing ~v */
				}
			}
			break;
		default:
			Info->Valid |= APRS_PLATFORM_VALID;
			Info->tPlatform = PLATFORM_MICE;
			if (*comment && (strlen(comment)<4 || comment[3] != '}'))
			{	strncpy(Info->Platform, "Mic-E", sizeof(Info->Platform));
				if (*comment)
				{	strcat(Info->Platform, "(");
					strncat(Info->Platform, comment, 5);
					strcat(Info->Platform, ")");
				}
#ifdef VERBOSE
				fprintf(stderr,"MicE(%c) %10s (%s)\n", isprint(*comment&0xff)?*comment:'?', InBuf, comment);
#endif
			} else
			{	strncpy(Info->Platform, "Mic-E+alt", sizeof(Info->Platform));
				if (strlen(comment)>4)
				{	strcat(Info->Platform, "(");
					strncat(Info->Platform, comment+4, 5);
					strcat(Info->Platform, ")");
				}
			}
		}
		// Check for altitude
		// TODO: Check length first, avoid parsing old data
		if (strlen(comment) > 3 && comment[3] == '}')
		{
			Info->alt = (float) (comment[0] - 33) * 8281;
			Info->alt += (float) (comment[1] - 33) * 91;
			Info->alt += (comment[2] - 33);
			Info->alt -= 10000;
			Info->Valid |= APRS_ALTITUDE_VALID;
//TraceLogThread("Mic-E", TRUE, "%s Altitude(%.4s)\n", Info->srcCall, comment);
			comment += 4;
#ifdef _MSC
if (Info->alt < -100 || Info->alt > 10000)
TraceError(NULL,"%s:Mic-E Alt(%s): %ld %ld %ld = %.0lf\n", Info->srcCall, comment, (long) comment[0], (long) comment[1], (long) comment[2], (double) Info->alt);
#endif
#ifdef INFO
1. In Mic-E format, the altitude in meters relative to 10km below mean sea level.
#endif
		}
#ifdef VERBOSE
		if (!KnownPlatform /*&& (Info->Valid & APRS_PLATFORM_VALID)*/ && *comment)
			printf("MicE %10s %.*s (%s)\n", srccall, sizeof(Info->Platform), Info->Platform, comment);
#endif
		break;
	}
	case '$':
		if (packet_data_len < 8) PARSERR(Short NMEA)
		if (packet_data[1] != 'G' || packet_data[2] != 'P')	/* Not a $GPxxx sentence */
		{	if (!strncmp(&packet_data[0],"$ULTW",5))
			{	strncpy(Info->Comment, &packet_data[0], sizeof(Info->Comment));
				Info->symbol = '_';	/* _ = Weather Station */
				Info->Valid |= APRS_SYMBOL_VALID;
				Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
				ParseULTWeather(packet_data, Info);
			} else
			{	Info->symbol = '}';	/* } = Red Cross (No Symbol) */
				Info->Valid |= APRS_SYMBOL_DEFAULTED;
			}
			return TRUE;
		}
		// NMEA.  Could be one of several formats.
// Caller is null terminated		packet_data[PACKET_DATA_SIZE - 1] = 0;
		rx_lat = 0;
		if (packet_data[3] == 'G' && packet_data[4] == 'G' && packet_data[5] == 'A')
		{	// GPGGA format
#ifdef DOCUMENTATION
	/* $GPGGA,032500.067,2759.8030,N,08039.5431,W,2,05,2.5,-10.1,M,-30.7,M,1.7,0000*55 */

	Commas[0];	/* Timestamp HHMMSS */
	Commas[1];	/* Latitude ddmm.mm */
	Commas[2];	/* North/South */
	Commas[3];	/* Longitude dddmm.mm */
	Commas[4];	/* East/West */
	Commas[5];	/* GPS Quality (0=No, 1=GPS, 2=DGPS) */
	Commas[6];	/* Number of Satellites in Use */
	Commas[7];	/* HDOP */
	Commas[8];	/* Altitude (Meters above sea level) */
	Commas[9];	/* M=Meters (altitude units) */
	Commas[10];	/* Geoidal separation (diff between earth ellipsoid and sea lavel, -=geoid is below ellipsoid) */
	Commas[11];	/* M=Meters (geoidal separation units) */
	Commas[12];	/* Age in seconds since last diff station update */
	Commas[13];	/* Diff reference station ID */
#endif
			// $GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,<<ALTITUDE>>19.7,M,,,,0000*1F
			if (packet_data[6] == ','
			&& cFromDec(&packet_data[7], 2, &Info->Time.hour)
			&& cFromDec(&packet_data[9], 2, &Info->Time.minute)
			&& cFromDec(&packet_data[11], 2, &Info->Time.second))
				Info->Valid |= APRS_TIME_VALID;
			rx_lat = &packet_data[7];
			// Find start of lat/lon string
			while (*rx_lat && *rx_lat != ',') rx_lat++;
			if (!*rx_lat) PARSERR(Invalid GGA)
			rx_lat++;
			// Find and mark end of lat/lon
			for (c=0, d=0; rx_lat[c]; c++)
			{
				if (rx_lat[c] == ',') d++;
				if (d == 4) break;
			}
			if (!rx_lat[c]) PARSERR(Invalid GGA)
			rx_lat[c] = 0;	/* Null terminate comma after E/W */
			// Find altitude string, skipping 3 commas (Quality, Satellits, HDOP)
			for (p = &rx_lat[c+1], d=0; *p; p++)
			{
//				if (*p == ',' || (d == 3 && *p == '.')) d++;
				if (*p == ',') d++;
				if (d == 3) break;
			}
			if (*p == 0) PARSERR(Invalid GGA)
			// Parse altitude (in meters, ignoring fractions)
//			for (p--, m = 1; isdigit(*p&0xff); p--, m = m * 10)
//				Info->alt += (*p - '0') * m;
			Info->alt = strtod(p+1, &e);
			if (*e == ',' && toupper(e[1]&0xff)=='M')
				Info->Valid |= APRS_ALTITUDE_VALID;

			strncpy(Info->Platform, "NMEA(GGA)", sizeof(Info->Platform));
			Info->tPlatform = PLATFORM_NMEA;
			Info->Valid |= APRS_PLATFORM_VALID;
		}
		if (packet_data[3] == 'G' && packet_data[4] == 'L' && packet_data[5] == 'L')
		{	// GPGLL format
#ifdef DOCUMENTATION
	/* $GPGLL,4229.3770,N,07116.2909,W,175255,A*34 */
	Commas[0];	/* Latitude ddmm.mm */
	Commas[1];	/* North/South */
	Commas[2];	/* Longitude dddmm.mm */
	Commas[3];	/* East/West */
	Commas[4];	/* Timestamp HHMMSS *//* But we DON'T use this due to no date */
	Commas[5];	/* Validity, A=ok, V=invalid */
#endif
			rx_lat = &packet_data[7];
			for (c=0, d=0; rx_lat[c]; c++)
			{
				if (rx_lat[c] == ',') d++;
				if (d == 4) break;
			}
			if (!rx_lat[c]) PARSERR(Invalid GLL)
			rx_lat[c] = 0;	// Null comma after E/W

			p = &rx_lat[c+1];	/* Timestamp */
			if (cFromDec(p, 2, &Info->Time.hour)
			&& cFromDec(p+2, 2, &Info->Time.minute)
			&& cFromDec(p+4, 2, &Info->Time.second))
				Info->Valid |= APRS_TIME_VALID;

			strncpy(Info->Platform, "NMEA(GLL)", sizeof(Info->Platform));
			Info->tPlatform = PLATFORM_NMEA;
			Info->Valid |= APRS_PLATFORM_VALID;
		}
/*
          1         2         3         4         5         6         7
01234567890123456789012345678901234567890123456789012345678901234567890
$GPRMC,215954,V,5319.8320,S,09528.8203,E,0.000,0.0,021209,61.0
$GPRMC,050505,V,4907.7277,N,12157.9591,W,000,000,291209,,*17
$GPRMC,125822,V,5348.7215,N,12255.8241,W,000.0,000.0,240112,021.9,E*76
Time   F Latitude  H Longigude  H Speed Brg Date   ????
From: http://www.codepedia.com/1/The+GPRMC+Sentence
A value of "A" (for "active") indicates that a fix is currently obtained, whereas a value of "V" (for "inValid") indicates that a fix is not obtained.
*/
		if (packet_data[3] == 'R' && packet_data[4] == 'M' && packet_data[5] == 'C')
		{	// GPRMC format
#ifdef DOCUMENTATION
	/* $GPRMC,032500.067,A,2759.8030,N,08039.5431,W,0.00,66.87,081209,,*2A */
	/* $GPRMC,003149,A,4259.6701,N,07130.3863,W,0.0,326.0,040310,,*00 */
	/* $GPRMC  ,003149,A,4259.6701,N,07130.3863,W,0.0 ,326.0,040310,   ,*00 */
	/* $PKWDPOS,130430,A,2759.8045,N,08039.5386,W,0.29,20   ,290510,6.6 *15 */
	Commas[0];	/* Timestamp HHMMSS */
	Commas[1];	/* Validity, A=ok, V=invalid */
	Commas[2];	/* Latitude ddmm.mm */
	Commas[3];	/* North/South */
	Commas[4];	/* Longitude dddmm.mm */
	Commas[5];	/* East/West */
	Commas[6];	/* Speed in knots */
	Commas[7];	/* True course */
	Commas[8];	/* Date Stamp ddmmyy */
	Commas[9];	/* Magnetic Variation */
	Commas[10];	/* East/West */
#endif

			if (packet_data[6] == ','
			&& cFromDec(&packet_data[7], 2, &Info->Time.hour)
			&& cFromDec(&packet_data[9], 2, &Info->Time.minute)
			&& cFromDec(&packet_data[11], 2, &Info->Time.second))
				Info->Valid |= APRS_TIME_VALID;

			rx_lat = &packet_data[7];
			for (d=0; *rx_lat && d < 2; rx_lat++)
			{	if (*rx_lat == ',')
				{	if (!d++)	/* First comma has Fix after it */
					{	if (rx_lat[1] != 'A')	/* Needs to be Active */
						{
#ifdef _MSC
							if (rx_lat[1] != 'V') TraceError(NULL,"%s Fix(%c)!=A in %s\n", Info->srcCall, rx_lat[1], packet_data);
#endif
							PARSERR(Inactive RMC)
						}
					}
				}
			}
			if (!*rx_lat) PARSERR(Invalid RMC)
			for (c=0, d=0; rx_lat[c]; c++)
			{
				if (rx_lat[c] == ',') d++;
				if (d == 4) break;
			}
			if (!rx_lat[c]) PARSERR(Invalid RMC)
			rx_lat[c] = 0;	/* Null out the comma after the E/W */

			p = &rx_lat[c+1];	/* Speed in knots */
			Info->speed = (short) strtod(p, &e);
			if (*e == ',')
			{	Info->course = (short) strtod(e+1, &e);	/* Course */
				if (*e == ',')
				{	Info->Valid |= APRS_CRSSPD_VALID;
					if (cFromDec(e+1, 2, &Info->Time.day)
					&& cFromDec(e+3, 2, &Info->Time.month))
						Info->Valid |= APRS_DATE_VALID;
				}
			}
			strncpy(Info->Platform, "NMEA(RMC)", sizeof(Info->Platform));
			Info->tPlatform = PLATFORM_NMEA;
			Info->Valid |= APRS_PLATFORM_VALID;
		}
		if (packet_data[3] == 'W' && packet_data[4] == 'P' && packet_data[5] == 'L')
		{	// GPWPL format - position and waypoint name
			rx_lat = &packet_data[7];
			for (c=0, d=0; rx_lat[c]; c++)
			{
				if (rx_lat[c] == ',') d++;
				if (d == 4) break;
			}
			if (!rx_lat[c]) PARSERR(Invalid WPL)
			for (p = &rx_lat[c+1]; *p && *p != '*'; p++);
			*p = 0;
			strcpy(srccall, &rx_lat[c+1]);
			rx_lat[c+1] = 0;

			strncpy(Info->Platform, "NMEA(WPL)", sizeof(Info->Platform));
			Info->tPlatform = PLATFORM_NMEA;
			Info->Valid |= APRS_PLATFORM_VALID;
		}

		// Exit if not parsed, could be Ultimeter or something
		if (rx_lat == 0)
		{	Info->symbol = '}';	/* } = Red Cross (No Symbol) */
			Info->Valid |= APRS_SYMBOL_DEFAULTED;
			return TRUE;
		}

		// Find N/S
		p = rx_lat;
		while (*p != ',' && *p) p++;
		if (*p == 0) PARSERR(Invalid NMEA)
		rx_ns = *(++p);
		if (*(++p) != ',') PARSERR(Invalid NMEA)
		rx_lon = ++p;
		// Find E/W
		while (*p != ',' && *p) p++;
		if (*p == 0) PARSERR(Invalid NMEA)
		rx_ew = *(++p);
		comment = "";

#ifdef FUTURE
		if (destcall[0] == 'G' && destcall[1] == 'P' && destcall[2] == 'S')
		{
			for (sym_search = symbols;
			     sym_search->aprs_symbol && !(sym_search->gps_symbol1 == destcall[3] && sym_search->gps_symbol2 == destcall[4]);
			     sym_search++);
		}
		else sym_search = &default_symbol;
		symbol = sym_search->aprs_symbol;
#endif
		break;
		
	case ':':
		// Various message and bulletin formats
		if (packet_data_len < 11) PARSERR(Short Message)
		if (packet_data[10] == ':')
		{	char *src = _strdup(srccall);
			char *ack = NULL;
			if (strchr(src,'>')) *strchr(src,'>') = '\0';

			for (c=9; c>1; c--) if (packet_data[c] != ' ') break;
			packet_data[c+1] = 0;

			strncpy(Info->msgCall, &packet_data[1], sizeof(Info->msgCall));
			strncpy(Info->Comment, &packet_data[11], sizeof(Info->Comment));
			comment = &packet_data[11];
			ack = strrchr(comment,'{');
			if (ack && ack[1] && strlen(ack) <= 6)
				strncpy(Info->msgAck, ack, sizeof(Info->msgAck));

			if (!strncmp(&packet_data[1],"BLN",3))
			{	Info->Valid |= APRS_BULLETIN_VALID;
			} else
			{	Info->Valid |= APRS_MESSAGE_VALID;
				if (packet_data_len > 17		/* Long enough to maybe? */
				&& Info->Comment[4] == '.'		/* Smell like parameters? */
				&& (!strncmp(Info->Comment, "PARM.", 5)
					|| !strncmp(Info->Comment, "UNIT.", 5)
					|| !strncmp(Info->Comment, "EQNS.", 5)
					|| !strncmp(Info->Comment, "BITS.", 5)))	/* Yep! */
				{	Info->Valid |= APRS_TELEMETRYDEF_VALID;
				} else Info->MessageCapable = TRUE;	/* If they send a message, they must DO messaing! */
			}

			if (digicall && !_stricmp(&packet_data[1], digicall))
			{
				// Message was addressed to us
				if (MessageCallback) MessageCallback(MsgUserArg, src, &packet_data[11]);
#ifdef FUTURE
				p = &packet_data[11];
				if (!strncmp(p, "?APRS", 5))
				{
					// Directed station query
					RaiseEvent(EVENT_TX_POSITION | EVENT_TX_WX);
					return FALSE;
				}
				if (!strncmp(p, "ack", 3))
				{
					// Looks like an ACK, see if it's for our current message
					c = (p[3] - 'a') << 4;
					c += p[4] - 'a';
					if (c == msg.id)
					{
						// Only print acknowledgement once
						if (msg.retries) console_write("\r\nMessage delivered.\r\n", 1);
						msg.retries = 0;
					}
					return FALSE;
				}
				comment = p;

				// Check for ACK request after we've finished processing the message
				p = strchr(p, '{');
				if (p && *p == '{')
				{
					// Make sure we didn't already hear this message
					if (strncmp(ack_src, srccall, 10) || strncmp(ack_id, &p[1], 5))
					{
						// Got a message ID, need to ACK it
						strncpy(ack_src, srccall, 10);
						strncpy(ack_id, &p[1], 5);
						RaiseEvent(EVENT_ACK_PENDING);
						*p = 0;
					}
					else return FALSE;
				}
				// Pass null-terminated message string to handler
				message_handler(comment);
				return FALSE;
#endif
			} else if (!strncmp(&packet_data[1], "BLN", 3))
			{	if (BulletinCallback)				/* 012345678901234 */
				{	char Save = packet_data[10];	/* :BLNxggggg:Text */
					packet_data[10] = '\0';
					BulletinCallback(BullUserArg, src, packet_data[4], &packet_data[5], &packet_data[11]);
					packet_data[10] = Save;
				}
			}
			free(src);

#define ITEM_IN_MESSAGE
#ifdef ITEM_IN_MESSAGE
// )NAME!DDMM.mmN/DDDMM.mmW$text.....
// ) will appear at c==11 which is text body after target's :123456789:
		if (packet_data[11]==')' && packet_data_len >= 15)
		{	int o=11;	// Offset to item in packet
			int e=min(packet_data_len,10+o);	// Max end of item name
			// Item report - note that items have variable-length names
			// Find and mark end of name
			for (c=1+o; c<e; c++)
				if (packet_data[c] == '!' || packet_data[c] == '_')
					break;
			if (packet_data[c] == '!' || packet_data[c] == '_')
			{	// Check to see if it's been killed
				if (packet_data[c] == '_') Info->ObjectKilled = 1;
				if (packet_data_len >= c+1)
				{
					//if (packet_data[c+1] == '/' || packet_data[c+1] == '\\')
					if (!isdigit(packet_data[c+1]&0xff))
					{	// Compressed format
						if (packet_data_len >= c+12)
						{
							if (!IsValidOverlay(packet_data[c+1])) PARSERR(Invalid Overlay)
							if (packet_data[c+1] >= 'a' && packet_data[c+1] <= 'j')
								packet_data[c+1] = packet_data[c+1] - 'a' + '0';	/* 0-9 in compressed */
							Info->symbol = packet_data[c+10] | (packet_data[c+1] == '/' ? STD_TABLE : ALT_TABLE);
							if (packet_data[c+1] != '/' && packet_data[c+1] != '\\') Info->symbol |= packet_data[c+1]<<16;
							/* if (packet_data[c+11] == ' ')
								comment = &packet_data[c+12];
							else */
							if (packet_data_len < c+14) PARSERR(Missing csT)
							else comment = &packet_data[c+14];	/* Skip the csT bytes */
							b91 = &packet_data[c+2];

							packet_data[c] = 0;	// Null terminate object name
							CALLCOPY(Info->objCall, &packet_data[1+o]);
							Info->Valid |= APRS_ITEM_VALID;
							Info->Valid |= APRS_SYMBOL_VALID;
							Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
							if (ack && strlen(ack) <= 6)
								*ack = '\0';	/* Wipe out ack field from comment */
							break;
						}	// Packet not long enough
					} else
					{	if (packet_data_len >= c+20)
						{	if (!IsValidOverlay(packet_data[c+9])) PARSERR(Invalid Overlay)
							Info->symbol = packet_data[c+19];
							if (packet_data[c+9] != '/') Info->symbol |= ALT_TABLE;
							if (packet_data[c+9] != '/' && packet_data[c+9] != '\\') Info->symbol |= packet_data[c+9]<<16;
							rx_lat = &packet_data[c+1];
							rx_ns = packet_data[c+8];
							rx_lon = &packet_data[c+10];
							rx_ew = packet_data[c+18];
							comment = &packet_data[c+20];

							packet_data[c] = 0;	// Null terminate object name
							CALLCOPY(Info->objCall, &packet_data[1+o]);
							Info->Valid |= APRS_ITEM_VALID;
							Info->Valid |= APRS_SYMBOL_VALID;
							Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
							if (ack && strlen(ack) <= 6)
								*ack = '\0';	/* Wipe out ack field from comment */
							break;
						}	// Packet not long enough
					}
				}	// Packet not long enough
			}	// Missing end of object name
		}
		// If we get here, the item failed, so keep going as a message
#endif
		}
		Info->symbol = '}';	/* } = Red Cross (No Symbol) */
		Info->Valid |= APRS_SYMBOL_DEFAULTED;
		goto CheckNWS;
//		return TRUE;
		
	case '?':
		strncpy(Info->Comment, &packet_data[0], sizeof(Info->Comment));
		// Queries ?APRS ?IGATE ?WX or is it ?APRS? ?IGATE? and ?WX?
#ifdef OBSOLETE
		if (!strncmp(packet_data, "?APRS?", 6))
		{
			// General station query - we should send a position packet
//			RaiseEvent(EVENT_TX_POSITION);
			PARSERR(Unsupported ?APRS?)
		}
		if (!strncmp(packet_data, "?INFO", 5))
		{
//			RaiseEvent(EVENT_INFO_PENDING);
			PARSERR(Unsupported ?INFO?)
		}
#endif
		Info->symbol = '}';	/* } = Red Cross (No Symbol) */
		Info->Valid |= APRS_SYMBOL_DEFAULTED;
		return TRUE;

	case '#':	/* Peet Bros U-II Weather (KPH) */
	case '*':	/* Peet Bros U-II Weather (MPH) */
#ifdef FOR_INFORMATION_ONLY
* or # START OF RECORD (1 BYTE)
CURRENT WIND DIRECTION (1 HEX DIGIT) 0 IS NORTH, 4 IS EAST, ETC.
CURRENT WIND SPEED (2 HEX DIGITS) IN MPH IF *: IN KMPH IF #
CURRENT TEMPERATURE (2 HEX DIGITS) SUBTRACT 56 FOR DEGREES F
UPPER RAIN GAUGE (4 HEX DIGITS) DIVIDE BY 10 OR 100 FOR INCHES*
LOWER RAIN GAUGE (4 HEX DIGITS) DIVIDE BY 10 OR 100 FOR INCHES*
CR LF
* Divide by 10 if your rain gauge senses each 0.1 (0.5mm).
* Divide by 100 if your rain gauge senses each 0.01 (or 0.25mm).

2012-08-29 01:46:48 UTC: N7ZEF-6>APRS,TRACE3-3,qAR,N7ZEF:#5007E00000000 [Unsupported packet format]
2012-08-29 01:47:38 UTC: N7ZEF-6>APRS,WIDE2-2,qAR,N7ZEF:#5007F00000000 [Unsupported packet format]
2012-08-29 01:56:29 UTC: N7ZEF-6>APRS,TRACE3-3,qAR,N7ZEF:#8007D00000000 [Unsupported packet format]
2012-08-29 02:01:18 UTC: N7ZEF-6>APRS,TRACE3-3,qAR,N7ZEF:#8197D00000000 [Unsupported packet format]
2012-08-29 02:01:53 UTC: N7ZEF-6>APRS,WIDE2-2,qAR,N7ZEF:#91E7D00000000 [Unsupported packet format]
2012-08-29 02:06:09 UTC: N7ZEF-6>APRS,TRACE3-3,qAR,N7ZEF:#8217D00000000 [Unsupported packet format]
2012-08-29 02:06:38 UTC: N7ZEF-6>APRS,WIDE2-2,qAR,N7ZEF:#9237D00000000 [Unsupported packet format]
2012-08-29 02:10:58 UTC: N7ZEF-6>APRS,TRACE3-3,qAR,N7ZEF:#8007C00000000 [Unsupported packet format]
2012-08-29 02:20:38 UTC: N7ZEF-6>APRS,TRACE3-3,qAR,N7ZEF:#81D7C00000000 [Unsupported packet format]
2012-08-29 02:20:53 UTC: N7ZEF-6>APRS,WIDE2-2,qAR,N7ZEF:#8127C00000000 [Unsupported packet format]
2012-08-29 02:35:09 UTC: N7ZEF-6>APRS,WIDE2-2,qAR,N7ZEF:#8107D00000000 [Unsupported packet format]
2012-08-29 02:39:53 UTC: N7ZEF-6>APRS,WIDE2-2,qAR,N7ZEF:#8007C00000000 [Unsupported packet format]
#endif
	if (strlen(packet_data) == 14)
	{	unsigned long r;

		if (FromHex(&packet_data[1],1,&r))
		{	Info->Weather.direction = (int)(r*(90.0/4.0));	/* 4==East */
			Info->Weather.Valid |= VALID_WX_DIRECTION;
		}
		if (FromHex(&packet_data[2],2,&r))
		{	Info->Weather.windspeed = r;
			if (packet_data[0] == '#')
				Info->Weather.windspeed /= KmPerMile;	/* KPH->MPH */
			Info->Weather.Valid |= VALID_WX_WINDSPEED;
		}
		if (FromHex(&packet_data[4],2,&r))
		{	Info->Weather.temperature = ((int)r)-56;
			Info->Weather.Valid |= VALID_WX_TEMPERATURE;
		}
		if (FromHex(&packet_data[6],4,&r))	/* Upper Rain Gauge */
		{	Info->Weather.rainHour = r;
			Info->Weather.Valid |= VALID_WX_RAIN_HOUR;
		}
		if (FromHex(&packet_data[10],4,&r))	/* Lower Rain Gauge */
		{	Info->Weather.rain24Hour = r;
			Info->Weather.Valid |= VALID_WX_RAIN_24HOUR;
		}

//	int rainMidnight;
//#define VALID_WX_RAIN_MIDNIGHT		(1<<2)

		if (Info->Weather.Valid)
		{	strncpy(Info->Weather.Unit, "Peet Bros U-II", sizeof(Info->Weather.Unit));
			Info->Weather.Valid |= VALID_WX_UNIT;
			Info->Valid |= APRS_WEATHER_VALID;
			Info->symbol = '_';	/* _ = Weather station */
			Info->Valid |= APRS_SYMBOL_VALID;
			Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
			return TRUE;
		}
	}
	// #/* falls through here if it didn't parse anything
	case '.':	/* Reserved space weather */
		strncpy(Info->Comment, &packet_data[1], sizeof(Info->Comment));
		Info->symbol = '_';	/* _ = Weather station */
		Info->Valid |= APRS_SYMBOL_VALID;
		Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
//		Info->Valid |= APRS_WEATHER_VALID;	/* Modifies course/speed below */
		return TRUE;

	case '_':	/* Positionless weather report */
	{	char *p;

		strncpy(Info->Comment, &packet_data[1], sizeof(Info->Comment));
		Info->symbol = '_';	/* _ = Weather station */
		Info->Valid |= APRS_SYMBOL_VALID;
		Info->Valid &= ~APRS_SYMBOL_DEFAULTED;

		for (c=1; c<9; c++)	/* mmddhhmm */
		if (!isdigit(packet_data[c]&0xff))
			return TRUE;	/* Don't go for the weather data */

		Info->Time.type = 'w';
		if (cFromDec(&packet_data[1], 2, &Info->Time.month)
		&& cFromDec(&packet_data[3], 2, &Info->Time.day)
		&& cFromDec(&packet_data[5], 2, &Info->Time.hour)
		&& cFromDec(&packet_data[7], 2, &Info->Time.minute))
			Info->Valid |= APRS_DATE_VALID | APRS_TIME_VALID;

		p = ParseAPRSWeather(&packet_data[9], Info);
		if (p) strncpy(Info->Comment, p, sizeof(Info->Comment));

		return TRUE;
	}

	case '&':	/* Reserved map feature */
	case '+':	/* Reserved shelter data */
	case ',':	/* Invalid data or test data */
		Info->symbol = '}';	/* } = Red Cross (No Symbol) */
		Info->Valid |= APRS_SYMBOL_DEFAULTED;
		return TRUE;

	case '[':	/* Maidenhead grid locator beacon (obsolete) */
		nolatlon = 1;
		Info->symbol = 'G';	/* G = GridSquare - 3x3 (q = 2x2) */
		Info->Valid |= APRS_SYMBOL_VALID;
		Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
		if (packet_data_len >= 8	/* [XXnnXX] at least */
		&& packet_data[7] == ']')	/* Must have the closing ] */
		{	if (packet_data[1] >= 'A' && packet_data[1] <= 'R'
			&& packet_data[2] >= 'A' && packet_data[2] <= 'R'
			&& isdigit(packet_data[3]&0xff) && isdigit(packet_data[4]&0xff))	/* Maybe a Maidenhead locator */
			{	char a1 = toupper(packet_data[5]);
				char a2 = toupper(packet_data[6]);
				if (a1 >= 'A' && a1 <= 'X'
				&& a2 >= 'A' && a2 <= 'X')
				{	strncpy(Info->GridSquare, &packet_data[1], 6);
					Info->Valid |= APRS_GRIDSQUARE_VALID;
					comment = &packet_data[8];
				} else PARSERR(Invalid GridSquare2)
			} else PARSERR(Invalid GridSquare1)
		} else PARSERR(NOT GridSquare)
		break;

	case 'T':	/* Telemetry */
	{	char *p;
		int ValueCount=0;

		if (packet_data[1] == '#')	/* Anything else is Probably a plain text status starting with T */
		{	if (packet_data[2] == 'M' && packet_data[3] == 'I' && packet_data[4] == 'C')
			{	Info->Telemetry.Sequence = 0;
				p = &packet_data[5];	/* Commas start here */
			} else
			{	p = &packet_data[2];	/* Start with the number */
				Info->Telemetry.Sequence = (unsigned short) strtoul(p, &p, 10);
			}

			while (*p && ValueCount<ARRAYSIZE(Info->Telemetry.Analog))
			{	if (*p == ',') p++;		/* Move to first Analog value */
				Info->Telemetry.Analog[ValueCount++] = (unsigned short) strtoul(p, &p, 10);
				//if (Info->Telemetry.Analog[ValueCount-1] > 255) TraceLogThread("Telemetry>255", FALSE, "%s[%ld]=%ld (dst:%s)\n", Info->srcCall, (long) ValueCount, (long) Info->Telemetry.Analog[ValueCount-1], Info->dstCall);
			}
			if (*p == ',') p++;
			Info->Telemetry.Digital = (unsigned char) strtoul(p, &p, 2);
			strncpy(Info->Comment, p, sizeof(Info->Comment));
		} else ValueCount = 0;

		if (ValueCount < 5)
		{	comment = &packet_data[0];
			nolatlon = 1;
			break;
		}

		Info->Valid |= APRS_TELEMETRY_VALID;	/* Mark it valid */
		Info->symbol = '~';	/* } = Tilde for Telemetry */
		Info->Valid |= APRS_SYMBOL_DEFAULTED;
		return TRUE;
	}

	case '<':	/* Station Capabilties */
		nolatlon = 1;
		comment = &packet_data[1];
		while (isspace(*comment&0xff)) comment++;
		for (p=comment+strlen(comment)-1; p>=comment; p--)
			if (isspace(*p&0xff)) *p = 0;
			else break;

		if (*comment)
		{	strncpy(Info->Capabilities, comment, sizeof(Info->Capabilities));
			comment = "";	/* no more comment to parse */
		}
		break;

	case '>':	/* Status */
		Info->symbol = '}';	/* } = Red Cross (No Symbol) */
		Info->Valid |= APRS_SYMBOL_DEFAULTED;
		comment = &packet_data[1];
		nolatlon = 1;
		if (packet_data_len >= 7)	/* Room for some extra parsing */
		{
			if (packet_data[7] == 'z'
			&& cFromDec(&packet_data[1], 2, &Info->Time.day)
			&& cFromDec(&packet_data[3], 2, &Info->Time.hour)
			&& cFromDec(&packet_data[5], 2, &Info->Time.minute))
			{	Info->Time.type = packet_data[7];
				Info->Valid |= APRS_DATE_VALID | APRS_TIME_VALID;
				comment = &packet_data[8];
			} else if (packet_data[1] >= 'A' && packet_data[1] <= 'R'
			&& packet_data[2] >= 'A' && packet_data[2] <= 'R'
			&& isdigit(packet_data[3]&0xff) && isdigit(packet_data[4]&0xff))	/* Maybe a Maidenhead locator */
			{
#ifdef FUTURE
/* Home = EL97QX */
            ll.Long = (locator[0] - 'A') * 20 + (locator[2] - '0') * 2 + (locator[4] - 'A' + 0.5) / 12 - 180;
            ll.Lat = (locator[1] - 'A') * 10 + (locator[3] - '0') + (locator[5] - 'A' + 0.5) / 24 - 90;
			strcpy(rx_lat,"0000.00");
			strcpy(rx_lon,"00000.00");
			rx_ns = 'N'; rx_ew = 'W';
#endif
				if (packet_data[7] == ' ' || packet_data[7] == '\0')	/* Short version confirmed */
				{	if (!IsValidOverlay(packet_data[5])) PARSERR(Invalid Overlay)
					strncpy(Info->GridSquare, &packet_data[1], 4);
					Info->Valid |= APRS_GRIDSQUARE_VALID;
					Info->symbol = packet_data[6] | (packet_data[5] == '/' ? STD_TABLE : ALT_TABLE);
					if (packet_data[5] != '/' && packet_data[5] != '\\') Info->symbol |= packet_data[5]<<16;
					Info->Valid |= APRS_SYMBOL_VALID;
					Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
					comment = &packet_data[7];
				} else if (packet_data_len >= 9)
				{	char a1 = toupper(packet_data[5]);
					char a2 = toupper(packet_data[6]);
					if (a1 >= 'A' && a1 <= 'X'
					&& a2 >= 'A' && a2 <= 'X'
					&& (packet_data[9] == ' ' || packet_data[9] == '\0'))	/* Long form */
					{	if (!IsValidOverlay(packet_data[7])) PARSERR(Invalid Overlay)
						strncpy(Info->GridSquare, &packet_data[1], 6);
						Info->Valid |= APRS_GRIDSQUARE_VALID;
						Info->symbol = packet_data[8] | (packet_data[7] == '/' ? STD_TABLE : ALT_TABLE);
						if (packet_data[7] != '/' && packet_data[7] != '\\') Info->symbol |= packet_data[7]<<16;
						Info->Valid |= APRS_SYMBOL_VALID;
						Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
						comment = &packet_data[9];
					}
				}
			}
#ifdef FUTURE
			if (strlen(comment)>=3 && comment[strlen(comment)-3]=='^')	// Beam/ERP
			{	TraceLogThread("BeamERP", TRUE, "%s -> %s\n", Info->srcCall, comment);
			}
#endif
		}
		while (isspace(*comment&0xff)) comment++;
		for (p=comment+strlen(comment)-1; p>=comment; p--)
			if (isspace(*p&0xff)) *p = 0;
			else break;

		if (*comment)
		{	strncpy(Info->StatusReport, comment, sizeof(Info->StatusReport));
			comment = "";	/* no more comment to parse */
		}
		break;
	case '{':	/* User-defined experimental datatype */
		comment = &packet_data[0];	/* Whole packet is comment for now */
		nolatlon = 1;	/* and no coordinates */
		break;
	default:
#ifdef VERBOSE
fprintf(stderr,"Unrecognized Datatype(%c) in '%s' (Src:%.*s Dst:%.*s)\n",
		packet_data[0], packet_data,
		sizeof(Info->srcCall), Info->srcCall,
		sizeof(Info->dstCall), Info->dstCall);
#endif

		comment = &packet_data[0];	/* Whole packet is comment */
		nolatlon = 1;

		while (isspace(*comment&0xff)) comment++;
		for (p=comment+strlen(comment)-1; p>=comment; p--)
			if (isspace(*p&0xff)) *p = 0;
			else break;
		/* Per aprs101.pdf, treat unrecognized packets as StatusReports */
		if (*comment)
		{	strncpy(Info->StatusReport, comment, sizeof(Info->StatusReport));
			comment = "";	/* no more comment to parse */
		}
		break;
	}

	if (Info->Valid & APRS_SYMBOL_DEFAULTED)	/* No defined symbol yet, check for dstCall spec */
	{	if (isdigit(Info->dstCall[4]&0xff) && isdigit(Info->dstCall[5]&0xff)	/* GPSCnn=Primary GPSEnn=Alternate */
		&& Info->dstCall[0] == 'G'	/* Short circuit the calls */
		&& (!strncmp(Info->dstCall,"GPSC",4) || !strncmp(Info->dstCall,"GPSE",4)))
		{	int s = (Info->dstCall[4]-'0')*10+(Info->dstCall[5]-'0')-1;	/* Seem to start at 01 */
			if (s >= 0 && s < sizeof(SymbolNames)/sizeof(SymbolNames[0]))
			{	Info->symbol = SymbolNames[s].symbol | (packet_data[3] == 'C' ? STD_TABLE : ALT_TABLE);
				Info->Valid |= APRS_SYMBOL_VALID;
				Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
			}
		} else if ((Info->dstCall[0] == 'G' && !strncmp(Info->dstCall,"GPS",3))	/* GPS/SPC/SYMxyz */
		|| (Info->dstCall[0] == 'S' && (!strncmp(Info->dstCall,"SPC",3) || !strncmp(Info->dstCall,"SYM",3))))
		{	int s;
			for (s=0; s<sizeof(SymbolNames)/sizeof(SymbolNames[0]); s++)
			{	if (Info->dstCall[3] == SymbolNames[s].pxy[0]	/* Primary hit */
				&& Info->dstCall[4] == SymbolNames[s].pxy[1])
				{	Info->symbol = SymbolNames[s].symbol | STD_TABLE;
					if (Info->dstCall[5] && Info->dstCall[5] != ' ') Info->symbol |= packet_data[5]<<16;
					Info->Valid |= APRS_SYMBOL_VALID;
					Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
				} else if (Info->dstCall[3] == SymbolNames[s].axy[0]	/* Alternate hit */
				&& Info->dstCall[4] == SymbolNames[s].axy[1])
				{	Info->symbol = SymbolNames[s].symbol | ALT_TABLE;
					if (Info->dstCall[5] && Info->dstCall[5] != ' ') Info->symbol |= packet_data[5]<<16;
					Info->Valid |= APRS_SYMBOL_VALID;
					Info->Valid &= ~APRS_SYMBOL_DEFAULTED;
				}
			}
		}
	}

	if (Info->Valid & APRS_SYMBOL_DEFAULTED)	/* No defined symbol yet, use SSID inference */
	{	char *c = strchr(Info->srcCall,'-');
		if (c)
		{	char *e;
			int s = strtol(c+1,&e,10);
			if (s > 0 && s < sizeof(SSIDSymbols))
				Info->symbol = SSIDSymbols[s-1];
		}
	}

/*	These are most of the CWOP stations that probably DON'T do messaging */
	if (Info->MessageCapable	/* '@' Messaging */
	&& (Info->symbol&0xff) == '_'	/* Weather station */
	&& !strncmp(Info->dstCall, "APRS", sizeof(Info->dstCall)))	/* Generic APRS */
		Info->MessageCapable = FALSE;

	if (*comment)
	{	strncpy(Info->Comment, comment, sizeof(Info->Comment));

/* NOTE: Mic-E type identifiers with | must have already been removed! */
/* Get the new Base-91 telemetry out of there FIRST as it might contain nearly ANYTHING! */
/* Especially before the !DAO! is checked as the telemetry contain a !xxx! */

		{	char *s, *e = strrchr(comment,'|');		/* Start at the LAST | per recommendations */
			if (e && e > comment)
			{	*e = '\0';	/* Need the one BEFORE this one! */
				s = strrchr(comment,'|');
				*e = '|';	/* Put it back */
				if (s)
				{	int len = e-s-1;	/* length inside, must be even and > 4 */
					if (!(len&1) && len >= 4 && len <= 14)	/* |ss1122334455dd| */
					{	long v;
						if (newbase91decode(s+1,2,&v))
						{	int n;
							BOOL AllOk = TRUE;
							Info->Telemetry.Sequence = (unsigned short) v;
							for (n=0; n<len/2-1; n++)
							{	if (newbase91decode(s+3+n*2,2,&v))
								{	if (n == 5)	/* Digital bits */
										Info->Telemetry.Digital = (unsigned char) v;
									else Info->Telemetry.Analog[n] = (unsigned short) v;
								} else AllOk = FALSE;
							}
							if (AllOk)
							{	Info->Valid |= APRS_TELEMETRY_VALID;
								//TraceLogThread("Telemetry", FALSE, "%s Compressed%ld(%.*s)\n", Info->srcCall, (long) (len/2-1), (int) (len+2), s);
								strcpy(s,e+1);	/* Eliminate telemetry from the comment */
							} else TraceLogThread("NoParse", FALSE, "%s BAD Telemetry Compressed%ld(%.*s)\n", Info->srcCall, (long) (len/2-1), (int) (len+2), s);
						} else TraceLogThread("NoParse", FALSE, "%s BAD Telemetry Compressed%ld(%.*s)\n", Info->srcCall, (long) (len/2-1), (int) (len+2), s);
					}
				}
			}
		}

		p = strstr(comment, "PHG");	/* PHGphgd */
		if (p && strlen(p) >= 7 && isdigit(p[3]&0xff) && isdigit(p[4]&0xff) && isdigit(p[5]&0xff) && isdigit(p[6]&0xff))
		{	double g;
			Info->PHG.power = (p[3]-'0')*(p[3]-'0');	/* Power in watts */
			Info->PHG.height = (1<<(p[4]-'0'))*10;		/* Height in feet */
			Info->PHG.gain = p[5]-'0';					/* in dB */
			Info->PHG.dir = (p[6]-'0') * 45;			/* 45 degree increments */
			g = pow(10,((double)Info->PHG.gain)/10);
			if (Info->PHG.power > 0	/* sqrt(0) or negative is a bad idea */
			&& Info->PHG.height > 0
			&& g > 0)	/* ditto */
				Info->PHG.range = sqrt(2*Info->PHG.height*sqrt((Info->PHG.power/10.0)*(g/2)));
			Info->Valid |= APRS_PHG_VALID|APRS_PHG_RANGE_VALID;

			if (isalnum(p[7]&0xff) && p[8]=='/')	/* http://aprs.org/aprs12/probes.txt */
			{	Info->PHG.rate = toupper(p[7]);
				if (Info->PHG.rate >= '0' && Info->PHG.rate <= '9')
					Info->PHG.rate -= '0';
				else if (Info->PHG.rate >= 'A' && Info->PHG.rate <= 'Z')
					Info->PHG.rate -= 'A'-10;
				else Info->PHG.rate = 0;
				if (p[9]=='A' && p[10]=='=')
					strcpy(p,p+8);		/* Keep the /A= (G0MAS & others) */
				else strcpy(p,p+9);		/* Eat the full probe spec */
			} else
			{	Info->PHG.rate = 0;
				if (p[7] == '/'
				&& (p[8]!='A' || p[9]!='='))	/* Don't kill PHGnnnn/A= */
					strcpy(p,p+8);	/* Eat the separator */
				else strcpy(p,p+7);	/* Just eat PHGphgd */
			}
		}

		p = strstr(comment, "DFS");	/* DFSshgd */
		if (p && strlen(p) >= 7 && isdigit(p[3]&0xff) && isdigit(p[4]&0xff) && isdigit(p[5]&0xff) && isdigit(p[6]&0xff))
		{	double g;
			Info->PHG.sunits = p[3]-'0';				/* "S" Units */
			Info->PHG.height = (1<<(p[4]-'0'))*10;		/* Height in feet */
			Info->PHG.gain = p[5]-'0';					/* in dB */
			Info->PHG.dir = (p[6]-'0') * 45;			/* 45 degree increments */
			g = pow(10,((double)Info->PHG.gain)/10);
			if (Info->PHG.sunits >= 0
			&& Info->PHG.height > 0
			&& g > 0)
			{
/* Note: Range from http://www.aprs.net/vm/DOS/DF.HTM */
//PLOTTING DETAILS FOR OMNI-DF CIRCELS; I used the radio horizon forumla for 
//the radius of the circles, modified by the signal strength value. Here is
//the equation for the four DFSshgd or PHGphgd characters.
//
//    P = 10 / s For Power plots, P = p; For DFS, P is INVERSLY
//
//        proportional to signal strength s.
//
//
//    H = 10 * 2 ^ h Convert character to power in Watts
//    G = 10 ^ (g / 10) Convert from dB
//    D = 45 * VAL(d) Convert to degrees. If D is not zero, then the circle
//
//        is offset in the indicated direction by 1/3rd radius
//
//    R = SQR(2 * H * SQR((P / 10) * (G / 2))) range modified by adding
//        SQR(P/10 *G/2) to make it unity at 10 watts and 3 dB
//
//    R = R * .85 Present fudge factor

				Info->PHG.range = sqrt(2*Info->PHG.height*sqrt((10.0/(Info->PHG.sunits>0?Info->PHG.sunits:1))*(g/2)));
				Info->PHG.range *= 0.85;	/* Present fudge factor */
			}
			Info->Valid |= APRS_DFS_VALID;

			strcpy(p,p+7);	/* Eat PHGphgd */
		}

		p = strstr(comment, "RNG");	/* RNGrrrr (Miles) */
		if (p && strlen(p) >= 7 && isdigit(p[3]&0xff) && isdigit(p[4]&0xff) && isdigit(p[5]&0xff) && isdigit(p[6]&0xff))
		{	unsigned long range;
			if (FromDec(p+3,4,&range))
			{	Info->PHG.range = range;
				Info->Valid |= APRS_PHG_RANGE_VALID;
				strcpy(p,p+7);
			}
		}

		if (Info->symbol == SymbolInt('\\','l')
		&& isdigit(comment[0]&0xff) && isdigit(comment[1]&0xff)
		&& isdigit(comment[2]&0xff)
		&& (comment[3]=='/' || comment[3]=='1')
		&& isdigit(comment[4]&0xff) && isdigit(comment[5]&0xff)
		&& isdigit(comment[6]&0xff))
		{	unsigned long v;
			if (comment[3] == '/') comment[3] = '0';
			if (FromDec(&comment[0], 3, &v))
				Info->course = (short) v;
			if (FromDec(&comment[3], 4, &v))
				Info->speed = (short) v;
			if (comment[3] == '0') comment[3] = '/';	/* Put it back */
			Info->Valid |= APRS_AREA_OBJECT_VALID;
		} else //if ((p=strchr(comment,'/'))!=NULL)	/* Don't loop if no / */
		{	char *s = comment;

			//Uncomment break; below also!
			//for (s=comment; *s; s++)	/* CSE/SPD must be FIRST! */
			{	if (((isdigit(s[0]&0xff) && isdigit(s[1]&0xff) && isdigit(s[2]&0xff))
					|| (s[0]=='.' && s[1]=='.' && s[2]=='.')
					|| (s[0]==' ' && s[1]==' ' && s[2]==' '))
				&& s[3] == '/'
				&& ((isdigit(s[4]&0xff) && isdigit(s[5]&0xff) && isdigit(s[6]&0xff))
					|| (s[4]=='.' && s[5]=='.' && s[6]=='.')
					|| (s[4]==' ' && s[5]==' ' && s[6]==' ')))
				{	unsigned long v;

//if (s != comment) TraceLogThread("CSE/SPD", FALSE, "%s>%s CSE/SPD[%ld] NOT first in %s\n", Info->srcCall, Info->dstCall, (long)(s-comment), comment);

//#define DIGIT3(d) ((d)[0]-'0')*100 + ((d)[1]-'0')*10 + ((d)[2]-'0')
					if ((Info->Valid & APRS_WEATHER_VALID)
					|| (Info->symbol&0xff) == '_'	/* weather itself */
					|| (Info->symbol&0xff) == 'w'	/* Water has weather */
					|| (Info->symbol&0xff) == 'H')	/* Hazards also */
					{	if (FromDec(&s[0], 3, &v))
						{	Info->Weather.direction = v;
							Info->Weather.Valid |= VALID_WX_DIRECTION;
						}
						if (FromDec(&s[4], 3, &v))
						{	Info->Weather.windspeed = v/MilePerNM;	// mph->knots
							Info->Weather.Valid |= VALID_WX_WINDSPEED;
						}
						p = ParseAPRSWeather(s+7, Info);
						strcpy(s, p);
						p = comment;	/* Go back to start */
					} else
					{	if (FromDec(&s[0], 3, &v))
							Info->course = (short) v;
						if (FromDec(&s[4], 3, &v))
							Info->speed = (short) v;
						strcpy(s, s+7);	/* Only eat if valid */
/* Area Objects are alternate table l (lowercase L) symbols */
						if (Info->course || Info->speed
						|| (isdigit(s[0]&0xff) && isdigit(s[4]&0xff)))
							Info->Valid |= APRS_CRSSPD_VALID;

/* DF (/\) stations /BRG/NRQ that follows the CSE/SPD */
						if (Info->symbol == '\\'	/* DF is primary \ */
						&& *s == '/' && strlen(s) >= 8
						&& (isdigit(s[1]&0xff) && isdigit(s[2]&0xff) && isdigit(s[3]&0xff))
						&& s[4] == '/'
						&& (isdigit(s[5]&0xff) && isdigit(s[6]&0xff) && isdigit(s[7]&0xff)))
						{	if (FromDec(&s[1], 3, &v))
								Info->BRGNRQ.bearing = (short) v;
							Info->BRGNRQ.number = s[5]-'0';
							Info->BRGNRQ.range = 1<<(s[6]-'0');
							Info->BRGNRQ.quality = s[7]-'0';
							if (Info->BRGNRQ.quality)	/* 0 is 0 */
							{	if (Info->BRGNRQ.quality == 1)
									Info->BRGNRQ.quality = 240;
								else if (Info->BRGNRQ.quality == 2)
									Info->BRGNRQ.quality = 60;
								else Info->BRGNRQ.quality = 1<<(9-Info->BRGNRQ.quality);
							}
							Info->Valid |= APRS_BRGNRQ_VALID;
							strcpy(s, s+8);	/* Eat the stuff */
						}
						p = comment;	/* Go back to start */

						if ((Info->symbol&0xff) == '@'	/* Storm object */
						&& comment[0] == '/'		/* Valid storm type */
						&& comment[3] == '/')	/* And wind speed */
						{	Info->Valid |= APRS_STORM_VALID;
							Info->Storm.valid = VALID_STORM_TYPE;

							strncpy(Info->Storm.type, comment+1, sizeof(Info->Storm.type));
							if (FromDec(&comment[4], 3, &v))
							{	Info->Storm.valid |= VALID_STORM_WINDSPEED;
								Info->Storm.windspeed = v;	// already knots
							}
							if (comment[7] == '^'
							&& FromDec(&comment[8], 3, &v))
							{	Info->Storm.valid |= VALID_STORM_GUST;
								Info->Storm.gust = v;		// already knots
							}
							if (comment[11] == '/'
							&& FromDec(&comment[12], 4, &v))
							{	Info->Storm.valid |= VALID_STORM_PRESSURE;
								Info->Storm.pressure = v;
							}
							if (comment[16] == '>'
							&& FromDec(&comment[17], 3, &v))
							{	Info->Storm.valid |= VALID_STORM_RADIUS_HURR;
								Info->Storm.radius.hurricane = v;
							}
							if (comment[20] == '&'
							&& FromDec(&comment[21], 3, &v))
							{	Info->Storm.valid |= VALID_STORM_RADIUS_TS;
								Info->Storm.radius.tropical_storm = v;
							}
							if (comment[24] == '%'
							&& FromDec(&comment[25], 3, &v))
							{	Info->Storm.valid |= VALID_STORM_RADIUS_GALE;
								Info->Storm.radius.gale = v;
							}
						}
					}
					//break;
				}
			}

	// Find altitude.  Negatives aren't in spec, but are in common use
			//p = strstr(p, "/A=");	/* p isn't set anymore */
			p = strstr(comment, "/A=");
			if (Info->alt == 0 && p != NULL && strlen(p) >= 9
			&& (isdigit(p[3]&0xff) || p[3] == '-')
			&& isdigit(p[4]&0xff) && isdigit(p[5]&0xff) && isdigit(p[6]&0xff)
			&& isdigit(p[7]&0xff) && isdigit(p[8]&0xff))
			{	char *e, s = p[9];
				p[9] = '\0';	/* Null term the 6 digits */
				Info->alt = (float) strtol(&p[3],&e,10) / (float) 3.2808399;
				Info->Valid |= APRS_ALTITUDE_VALID;
				p[9] = s;
				strcpy(p,e);
			}
		}
		
		if ((p=strchr(comment,'!')) != NULL)
		{	char *e = strchr(p+1,'!');
			if (e)	/* Have ! and a second !, worth searching */
			{	int s;
			static	struct
			{	char *Text;
				char *MicEMessage;
			} MicEStrings[] = {	/* http://aprs.org/aprs12/EmergencyCode.txt */
				{ "!EMERGENCY!", "EMERGENCY!" },	/* RED + Center, Zoom, Alarm */
				{ "!PRIORITY!", "Priority" },	/* ORANGE + Center, Zoom */
				{ "!SPECIAL!", "Special" },		/* YELLOW + Center */
				{ "!COMMITTED!", "Committed" },
				{ "!RETURNING!", "Returning" },
				{ "!INSERVICE!", "In Service" },
				{ "!ENROUTE!", "En Route" },
				{ "!OFF-DUTY!", "Off Duty" },
				{ "!TESTALARM!", "Test Alarm" },	/* Extensions to Mic-E */
				{ "!ALARM!", "Alarm" },			/* Center, Zoom */
				{ "!ALERT!", "Alert" },			/* Center, Zoom */
				{ "!WARNING!", "Warning" },		/* Center, Zoom */
				{ "!WXALARM!", "WX Alarm" },	/* Center, Zoom */
				{ "!EM!", "Emergency" } };		/* Center, Zoom, No Alarm? */

				char *upper = _strupr(_strdup(p));
				for (s=0; s<ARRAYSIZE(MicEStrings); s++)
				{	char *w;
					if ((w=strstr(upper, MicEStrings[s].Text)) != NULL)
					{	Info->MicEMessage = MicEStrings[s].MicEMessage;
						Info->Valid |= APRS_MICE_MESSAGE_VALID;
						strcpy(&p[w-upper],&p[w-upper]+strlen(MicEStrings[s].Text));	/* Eat the string */
						break;
					}
				}
				free(upper);
#ifdef SUPPORT_SPOTTER_NETWORK
				p = strstr(comment, "!SN!");	/* SkyWarn? */
				if (p)
				{	Info->SkyWarn = TRUE;
					strcpy(p,p+4);	/* Eat the string */
				}
#endif
			}
		}
/*	Check toCall for xastir(db.c)/APRS+SA compatibiity */
		if (!(Info->Valid & APRS_MICE_MESSAGE_VALID))	/* Only check if we don't already have one */
		{	int s;
		static	struct
		{	char *Text;
			char *MicEMessage;
		} MicEStrings[] = {	/* http://aprs.org/aprs12/EmergencyCode.txt */
			{ "ALARM", "Alarm" },			/* Center, Zoom */
			{ "ALERT", "Alert" },			/* Center, Zoom */
			{ "WARNING", "Warning" },		/* Center, Zoom */
			{ "WXALARM", "WX Alarm" },	/* Center, Zoom */
			{ "EM", "Emergency" } };		/* Center, Zoom, No Alarm? */

			for (s=0; s<ARRAYSIZE(MicEStrings); s++)
			{	if (!_stricmp(MicEStrings[s].Text, Info->dstCall))
				{	Info->MicEMessage = MicEStrings[s].MicEMessage;
					Info->Valid |= APRS_MICE_MESSAGE_VALID;
					break;
				}
			}
		}

		if ((p=strchr(comment, '{'))!=NULL)	/* Don't look for the individuals if no { */
		{	char *t;

			t = strstr(p, "{UISS");	/* {UISSnn} */
			if (t)
			{	if (isdigit(t[5]&0xff) && isdigit(t[6]&0xff) && t[7] == '}')
				{	strncpy(Info->Platform, "UISS ",sizeof(Info->Platform));
					strncpy(&Info->Platform[5], &t[5], 2);
					Info->tPlatform = PLATFORM_UISS;
					Info->Valid |= APRS_PLATFORM_VALID;
					strcpy(t,t+8);	/* Eat the string */
				}
else TraceLogThread("Bracket", TRUE, "%s>%s %s\n", Info->srcCall, Info->dstCall, p);
			}
			t = strstr(p, "{UIV32N}");	/* UI-View */
			if (t)
			{	if (Info->dstCall[0] != 'A'
				|| Info->dstCall[1] != 'P'
				|| (Info->dstCall[2] != '1' && Info->dstCall[2] != '2' && Info->dstCall[2] != '3'))
				{	strncpy(Info->Platform, "UI-View 32N",sizeof(Info->Platform));
					Info->tPlatform = PLATFORM_UI_VIEW_32N;
					Info->Valid |= APRS_PLATFORM_VALID;
				}
else TraceLogThread("Bracket", TRUE, "%s>%s %s\n", Info->srcCall, Info->dstCall, p);
				strcpy(t,t+8);	/* Eat the string */
			}
			t = strstr(p, "{UIV32}");	/* UI-View */
			if (t)
			{	if (Info->dstCall[0] != 'A'
				|| Info->dstCall[1] != 'P'
				|| (Info->dstCall[2] != '1' && Info->dstCall[2] != '2' && Info->dstCall[2] != '3'))
				{	strncpy(Info->Platform, "UI-View 32",sizeof(Info->Platform));
					Info->tPlatform = PLATFORM_UI_VIEW_32;
					Info->Valid |= APRS_PLATFORM_VALID;
				}
else TraceLogThread("Bracket", TRUE, "%s>%s %s\n", Info->srcCall, Info->dstCall, p);
				strcpy(t,t+7);	/* Eat the string */
			}
			t = strstr(p, "{UIV23N}");	/* UI-View */
			if (t)
			{	if (Info->dstCall[0] != 'A'
				|| Info->dstCall[1] != 'P'
				|| (Info->dstCall[2] != '1' && Info->dstCall[2] != '2' && Info->dstCall[2] != '3'))
				{	strncpy(Info->Platform, "UI-View 23N",sizeof(Info->Platform));
					Info->tPlatform = PLATFORM_UI_VIEW_23N;
					Info->Valid |= APRS_PLATFORM_VALID;
				}
else TraceLogThread("Bracket", TRUE, "%s>%s %s\n", Info->srcCall, Info->dstCall, p);
				strcpy(t,t+8);	/* Eat the string */
			}
			t = strstr(p, "{UIV23}");	/* UI-View */
			if (t)
			{	if (Info->dstCall[0] != 'A'
				|| Info->dstCall[1] != 'P'
				|| (Info->dstCall[2] != '1' && Info->dstCall[2] != '2' && Info->dstCall[2] != '3'))
				{	strncpy(Info->Platform, "UI-View 23",sizeof(Info->Platform));
					Info->tPlatform = PLATFORM_UI_VIEW_23;
					Info->Valid |= APRS_PLATFORM_VALID;
				}
else TraceLogThread("Bracket", TRUE, "%s>%s %s\n", Info->srcCall, Info->dstCall, p);
				strcpy(t,t+7);	/* Eat the string */
			}
			t = strstr(p, "{UIV22}");	/* UI-View */
			if (t)
			{	if (Info->dstCall[0] != 'A'
				|| Info->dstCall[1] != 'P'
				|| (Info->dstCall[2] != '1' && Info->dstCall[2] != '2' && Info->dstCall[2] != '3'))
				{	strncpy(Info->Platform, "UI-View 22",sizeof(Info->Platform));
					Info->tPlatform = PLATFORM_UI_VIEW_22;
					Info->Valid |= APRS_PLATFORM_VALID;
				}
else TraceLogThread("Bracket", TRUE, "%s>%s %s\n", Info->srcCall, Info->dstCall, p);
				strcpy(t,t+7);	/* Eat the string */
			}
		}

		if (!(Info->Valid & APRS_WEATHER_VALID)
		&& ((Info->symbol&0xff) == '_'	/* weather itself */
			|| (Info->symbol&0xff) == 'w'	/* Water has weather */
			|| (Info->symbol&0xff) == 'H'))	/* and Hazards */
		{	p = ParseAPRSWeather(comment, Info);
			if (p != comment)
			{	if (Info->Weather.Valid && Info->Weather.Valid!=VALID_WX_UNIT)
				{
//					Thread("Non-Weather", FALSE, "%s 0x%lX Weather(%.*s)\n",
//									Info->srcCall, Info->Weather.Valid, (int)(p-comment),comment);
					strcpy(comment,p);
				}
//				else TraceLogThread("Non-Weather", FALSE, "%s NOT Weather(%.*s)\n",
//									Info->srcCall, (int)(p-comment),comment);
			}
		}
	}

	if (!nolatlon)
	{
		// Base91 decoding
		if (b91)
		{	signed long sslat, sslon;
//TraceLogThread("NoParse", FALSE, "%s Has Compressed(%.13s)%s\n", Info->srcCall, b91-1, (b91[11] & 0x18) == 0x18?" GGA":"");
#ifdef OLD_WAY
			if (!base91decode(b91, &sslat)
			|| !base91decode(b91+4, &sslon))
			{
TraceLogThread("NoParse", TRUE, "%s Has Compressed(%.13s) INVALID\n", Info->srcCall, b91-1);
				return FALSE;
			} else
			{
				sslon = (0 - sslon) << 1;
				semicircledecode(sslat, sslon, rx_lat, rx_lon, &rx_ns, &rx_ew);
#else
			if (!newbase91decode(b91, 4, &sslat)
			|| !newbase91decode(b91+4, 4, &sslon))
			{
TraceLogThread("NoParse", FALSE, "%s Has Compressed(%.13s) INVALID\n", Info->srcCall, b91-1);
				PARSERR(Invalid Base91)
			} else
			{
				Info->lat = 90.0 - sslat / 380926.0;
				Info->lon = sslon / 190463.0 - 180.0;
				Info->latlonExtended = 2;	/* b91 is accurate */
#endif
/*
/YYYYXXXX$csT where
	/ is the Symbol Table Identifier
	YYYY is the compressed latitude
	XXXX is the compressed longitude
	$ is the Symbol Code
	cs is the compressed course/speed or
		compressed pre-calculated radio range or
		compressed altitude
	T is the compression type indicator

The two cs bytes following the Symbol Code character can contain either
the compressed course and speed (c between ! and z) or the compressed pre-calculated radio
range (c={) or the station’s altitude (T bit 4/3 = 1/0). These two bytes are in base 91 format.
In the special case of c = V (space), there is no course, speed or range
data, in which case the csT bytes are ignored. (But must be present lwd-2011/01/07)
*/
				if (b91[9] != ' ')	/* Space ignores csT */
				{
/*
Altitude: If the T byte indicates that the raw data originates from a GGA sentence (i.e.
bits 4 and 3 of the T byte are 10), then the sentence contains an altitude
value, in feet. After compression, the compressed altitude data is placed in
the cs bytes, such that:

altitude = 1.002^cs feet

For example, if the received cs bytes are S], the computation is as follows:
Subtract 33 from the ASCII code for each character:
c = 83 - 33 = 50
s = 93 - 33 = 60
Multiply c by 91 and add s to obtain cs:
cs = 50 x 91 + 60
= 4610
Then altitude = 1.002^4610
= 10004 feet
*/
					if ((b91[11] & 0x18) == 0x10)	/* GGA Source has altitude */
					{	long cs;
						cs = (((unsigned char)b91[9])-33)*91+((unsigned char)b91[10])-33;
						Info->alt = pow(1.002,cs) / (float) 3.2808399;
						Info->Valid |= APRS_ALTITUDE_VALID;
//TraceLogThread("NoParse", TRUE, "%s Has Compressed(%.13s) ALTITUDE(%.2lfm)\n", Info->srcCall, b91-1, (double) Info->alt);
					}
/*
CRSSPD: If the ASCII code for c is in the range ! to z inclusive -
corresponding to numeric values in the range 0-89 decimal (i.e. after
subtracting 33 from the ASCII code) - then cs represents a compressed
course/speed value:

course = c x 4
speed = 1.08^s - 1
For example, if the cs characters are 7P, the corresponding values of c and
s (after subtracting 33 from the ASCII character code) are 22 and 47
respectively. Substituting these values in the above equations:
course = 22 x 4 = 88 degrees
speed = 1.08^47 - 1 = 36.2 knots
*/
					else if (b91[9] >= '!' && b91[9] <= 'z')
					{	long course;
						double speed;
						course = (((unsigned char)b91[9])-33) * 4;
						speed = pow(1.08,((unsigned char)b91[10])-33) - 1;

						if ((Info->Valid & APRS_WEATHER_VALID) || (Info->symbol&0xff) == '_')
						{	Info->Weather.direction = course;
							Info->Weather.Valid |= VALID_WX_DIRECTION;
							Info->Weather.windspeed = speed;
							Info->Weather.Valid |= VALID_WX_WINDSPEED;
						} else
						{	Info->course = (short) course;
							Info->speed = (short) (speed+0.5);	/* Rounded */
							Info->Valid |= APRS_CRSSPD_VALID;
						}
/*
Pre-Calculated Radio Range : If c = {, then cs represents a compressed
pre-calculated radio range value:

range = 2 x 1.08^s
For example, if the cs bytes are {?, the ASCII code for ? is 63, so the value
of s is 30 (i.e. 63-33). Thus:
range = 2 x 1.08^30
~ 20 miles

So APRS will draw a circle of radius 20 miles around the station plot on the
screen.
*/
					} else if (b91[9] == '{')
					{	Info->PHG.range = 2 * pow(1.08,((unsigned char)b91[10])-33);
						Info->Valid |= APRS_PHG_RANGE_VALID;
					}
				}
			}
		} else
		{	rx_lat[7] = 0;
			rx_lon[8] = 0;
			if (rx_lat[4] != '.' || rx_lon[5] != '.') PARSERR(Invalid Coords-Dot)
		
			// Check for valid N/S E/W bytes (missing from some null positions)
			rx_ns = toupper(rx_ns);
			rx_ew = toupper(rx_ew);
			if ((rx_ns != 'N' && rx_ns != 'S') || (rx_ew != 'E' && rx_ew != 'W')) PARSERR(Invalid NS/EW)


#ifdef FUTURE
		// Encode position in semicircles format if we haven't already
		// TODO: Handle added precision (!DAO! extension)
		if (!b91)
		{
			rx_lat[7] = 0;
			rx_lon[8] = 0;
			sslat = semicircles(rx_lat, rx_ns == 'S');
			sslon = semicircles(rx_lon, rx_ew == 'W');
		}
		geodistance(&range, &bearing, sslat, sslon, fixdata.latitude, fixdata.longitude);
#endif

#ifdef APRPARSE
printf("Converting rx_lat(%s) rx_lon(%s)\n", rx_lat, rx_lon);
#endif

			{	char *e1, *e2, *t;
				Info->lat = strtod(rx_lat,&e1);
				Info->lon = strtod(rx_lon,&e2);

				if (*e1 == ' ' || *e2 == ' ')
				{static double Ambiguity[] = { 0.05, 0.50, 5.00, 50.00 };	/* still ddmm.mmm, degrees() conversion below */

					if (*e1)
					for (t=e1+1; *t; t++)
					{	if (*t != ' ' && *t != '.')
							PARSERR(Invalid Ambig-Lat)
					}
					if (*e2)
					for (t=e2+1; *t; t++)
					{	if (*t != ' ' && *t != '.')
							PARSERR(Invalid Ambig-Lon)
					}
				
					Info->latAmbiguity = 7 - (int) (e1-rx_lat);
					Info->lonAmbiguity = 8 - (int) (e2-rx_lon);
					if (Info->latAmbiguity > 3)
					{	Info->latAmbiguity--;	/* Adjust for . */
						Info->lat *= Info->latAmbiguity==4?100.0:10.0;
					}
					if (Info->lonAmbiguity > 3)
					{	Info->lonAmbiguity--;	/* Adjust for . */
						Info->lon *= Info->lonAmbiguity==4?100.0:10.0;
					}

					if (Info->latAmbiguity > 0
					&& Info->latAmbiguity <= sizeof(Ambiguity)/sizeof(Ambiguity[0]))
					{	Info->lat += Ambiguity[Info->latAmbiguity-1];
					} else PARSERR(Lat Ambiguity)

					//if (Info->lonAmbiguity) PARSERR(Lon Ambiguity)
					if (Info->lonAmbiguity > 0
					&& Info->lonAmbiguity <= sizeof(Ambiguity)/sizeof(Ambiguity[0]))
					{	Info->lon += Ambiguity[Info->lonAmbiguity-1];
					}

//2009-10-21T15:05:08 ***** GB7DXC Lat(5201.8 )0x20( ) or Lon(00206.4 )0x20( ) Ambiguous (6 7)  (52.030 -2.107)
//2009-10-21T15:05:21 ***** BOLDER Lat(4619.  )0x20( ) or Lon(11206.  )0x20( ) Ambiguous (5 6)  (46.317 -112.100)
//2009-10-21T15:08:07 ***** VACA Lat(382 .  )0x20( ) or Lon(1220 .  )0x20( ) Ambiguous (3 4)  (4.367 -12.333)
//int latAmbiguity, lonAmbiguity;	/* In powers of 10, 0=all 1=.nx 2=.xx 3=x.xx */
#ifdef VERBOSE
					fprintf(stderr,"%s Lat(%s)0x%02lX(%c) or Lon(%s)0x%02lX(%c) Ambiguous (%ld %ld)  (%.3lf %.3lf)\n",
							Info->srcCall, rx_lat, (long) *e1, isprint(*e1&0xff)?*e1:'?',
							rx_lon, (long) *e2, isprint(*e2&0xff)?*e2:'?',
							(int) (e1-rx_lat), (int) (e2-rx_lon),
							(double) Info->lat, (double) Info->lon);
#endif
				}
else if (*e1 || *e2)
{
	TraceLogThread("Invalids", TRUE, "ERROR Converting %s rx_lat(%s) rx_lon(%s) via %s\n",
					Info->srcCall, rx_lat, rx_lon, Info->Path.Hops[Info->Path.hopCount-1]);
	PARSERR(Bad Lat/Lon)
}
				if (!Info->lat && !Info->lon)
					PARSERR(Zero Lat/Lon)

				Info->lat = degrees(Info->lat * (rx_ns=='S'?-1:1));
				if (toupper(*e1) != 'N' && toupper(*e1) != 'S' && *e1 != ' ' && *e1 != ',')
				{	if (*e1 || strlen(rx_lat) != 7)
					{
#ifdef VERBOSE
						fprintf(stderr,"%s Bad End 0x%lX(%c) For Lat(%s) len %ld\n", Info->srcCall, (long) *e1, isprint(*e1&0xff)?*e1:'?', rx_lat, (long) strlen(rx_lat));
#endif
						PARSERR(Invalid Lat)
					}
				}
				Info->lon = degrees(Info->lon * (rx_ew=='W'?-1:1));
				if (toupper(*e2) != 'E' && toupper(*e2) != 'W' && *e2 != ' ' && *e2 != ',')
				{	if (*e2 || strlen(rx_lon) != 8)
					{
#ifdef VERBOSE
						fprintf(stderr,"%s Bad End 0x%lX(%c) For Lon(%s) len %ld\n", Info->srcCall, (long) *e2, isprint(*e2&0xff)?*e2:'?', rx_lon, (long) strlen(rx_lon));
#endif
						PARSERR(Invalid Lon)
					}
				}
/*
	Now process the !DAO! extension
*/
				{	char *s = strrchr(comment,'!');		/* Start at the LAST ! per recommendations */
					if (s && s >= comment+4
					&& s[-4]=='!'	/* Smells like a !DAO! extension */
					&& (s[-3]=='W' || s[-3]=='w'))	/* Valid Datum? */
					{	s -= 4;		/* Need to process !DAO! here */

#define sign(v) (((v)<0)?-1.0:1.0)

						if (s[1] == 'W' && isdigit(s[2]&0xff) && isdigit(s[3]&0xff))
						{	Info->lat += sign(Info->lat)*(s[2]-'0')/1000.0/60.0;
							Info->lon += sign(Info->lon)*(s[3]-'0')/1000.0/60.0;
							Info->latlonExtended = 1;
							strcpy(s,s+5);	/* Remove the !DAO! from comment */
						} else if (s[1] == 'w' && s[2] >= 33 && s[3] >= 33 && s[2] <= (33+90) && s[3] <= (33+90))
						{	Info->lat += sign(Info->lat)*(s[2]-33)*1.1/10000.0/60.0;
							Info->lon += sign(Info->lon)*(s[3]-33)*1.1/10000.0/60.0;
							Info->latlonExtended = 2;
							strcpy(s,s+5);	/* Remove the !DAO! from comment */
						} else	/* Unsupported datum */
						{
						}
#undef sign
					}
				}
			}
		}

#ifdef DAO_DOCUMENTATION
!W23! means it is WGS84 and the upper case indicates it is the human
readable 3rd digit format.  "2" is the third decimal digit of latitude
minutes and is human readable.  "3" is the added digit of longitude 
minutes and is also human readable.

!wAb! means it is WGS84 but the lower case indicates this is the added
precision to the nearest foot.  "A" is the base 91 code for two more 
digits (65 minus 33 yields "32") and "b" is two more digits of longitude
(98 -33 or "...65").

!w:\! would also be WGS84 but with : and \ decoding to two 
additional digits of "27" to latitude and "59" to longitude to the 
nearest foot or so.

BASE-91 CONVERSION:  In the first !W23! example above, the actual digits
are simply added to the existing LAT/LONG for example to add precision
to DDMM.mmN/DDDMM.mmW to be come equivalent to DDMM.mm2N and DDDMM.mm3W.

But in the next two examples !wAb! and !w:\! the added two digits 
cannot simply be added to the ASCII position string, since they can only
go from 00 to 90.  THus they need to be scaled so that they go from
00 to 99.  Do this by multiplying the two digits by 1.10.  So for the
!wAb! example with added latitude digits of 32, you multiply that by
1.10 to arrive at an actual added digits of 35.2.  So the high precision
latitude becomes DDMM.mm352N.
#endif

		if (Info->lat < -90 || Info->lat > 90
		|| Info->lon < -180 || Info->lon > 180)
		{
#ifdef VERBOSE
				fprintf(stderr,"%s Bad Lat(%s)=%.5lf or Lon(%s)=%.5lf\n",
							Info->srcCall,
							rx_lat, (double) Info->lat,
							rx_lon, (double) Info->lon);
#endif
				Info->lat = Info->lon = 0.0;
				PARSERR(Lat/Lon Out-Of-Range)
		}
		Info->Valid |= APRS_LATLON_VALID;
	}

	// Not sure why many stations have a slash here! (Because PHG recommended the trailing /)
	if (*comment == '/') comment++;

	// Remove leading and trailing spaces on comments
	while (isspace(*comment&0xff)) comment++;
	for (p=comment+strlen(comment)-1; p>=comment; p--)
		if (isspace(*p&0xff)) *p = 0;
		else break;

/*
	Now that everything else is done, check for http://www.aprs.org/info/freqspec.txt
	But what is also http://www.aprs.org/freq/AFRSspec.txt
*/
	if (Info->Valid & (APRS_OBJECT_VALID | APRS_ITEM_VALID)
	&& Info->objCall[3]=='.'	/* Call this probable cause */
	&& ParseFreqSpec(Info->objCall, comment, Info))
		Info->Valid |= APRS_FREQUENCY_VALID;
	else if (strlen(comment) >= 6	/* Must fit FFF.FFfMHz */
	&& comment[3]=='.'	/* [0] might be alpha (microwaves) */
	&& isdigit(comment[1]&0xff)
	&& isdigit(comment[2]&0xff)
	&& isdigit(comment[4]&0xff)
	&& isdigit(comment[5]&0xff)	/* optional [6] checked in routine */
	&& ParseFreqSpec(comment, comment, Info))
		Info->Valid |= APRS_FREQUENCY_VALID;

	if (*comment)
	{	strncpy(Info->CleanComment, comment, sizeof(Info->CleanComment));
	}

CheckNWS:	/* Messages from above come down through here... */

/* Now detect for a valid NWS packet */
/* Per Roger Bille (SM5NRK) on 1/11/2011 */
/* a) Fromcall has 6 alphabetic characters */
/* b) Sequencenumber (after '{') has 5 alphanumeric characters */
	if (!Info->srcCall[6]	/* 6 characters long */
	&& isalpha(Info->srcCall[5]&0xff)
	&& isalpha(Info->srcCall[4]&0xff)
	&& isalpha(Info->srcCall[3]&0xff)
	&& isalpha(Info->srcCall[2]&0xff)
	&& isalpha(Info->srcCall[1]&0xff)
	&& isalpha(Info->srcCall[0]&0xff))
	{	char *c = strrchr(comment,'{');
		if (c && strlen(c) == 6
				&& isalnum(c[1]&0xff)
				&& isalnum(c[2]&0xff)
				&& isalnum(c[3]&0xff)
				&& isalnum(c[4]&0xff)
				&& isalnum(c[5]&0xff))
		{	Info->Valid |= APRS_NWS_VALID;
		}
	}

	return TRUE;
}

#ifdef FUTURE
static void parse_route(char *route, char **entry, char **relay, char **q)
{	int pcount = 1, i, c = 0;
	char *p, **pieces;

	for (p=route; *p; p++)
		if (*p==',') pcount++;

	*entry = *relay = *q = "";
#ifdef VERBOSE
	if (pcount < 2) fprintf(stderr,"Huh, Only %ld pieces in %s\n", (long) pcount, route);
#endif
	if (pcount < 2) return;

	pieces = malloc(sizeof(*pieces)*pcount);
	pieces[c++] = route;
	for (p=route; *p; p++)
	{	if (*p==',')
		{	*p++ = '\0';
			pieces[c++] = p;
		}
	}
#ifdef VERBOSE
	if (c != pcount) fprintf(stderr,"Huh?  %ld != %ld\n", (long) c, (long) pcount);
#endif

	for (i=pcount-1; i>=0; i--)
	{	if (*pieces[i] == 'q')	/* Found the q! */
		{	*q = pieces[i];
#ifdef VERBOSE
			if (i==pcount-1)
			{	fprintf(stderr,"Huh?  q(%s) is %ld/%ld?\n", pieces[i], (long) i, (long) pcount);
			} else if (!strcmp(pieces[i], "qAI"))	/* Trace packet */
			{	fprintf(stderr,"Trace: ");
				for (c=i+1; c<pcount; c++)
					fprintf(stderr,"%s ",pieces[c]);
				fprintf(stderr,"\n");
			} else
#endif
			if (!strcmp(pieces[i], "qAS"))	/* Server logon follows (call it the iGate) */
			{	*relay = pieces[i+1];
				for (c=0; c<i; c++)
				{	if (strchr(pieces[c],'*'))
					{	*entry = pieces[c];
						if (c && !strncmp(pieces[c],"WIDE",4))
						{	*entry = pieces[c-1];
						}
						break;
					}
				}
#ifdef DONT_THINK_SO
				if (!strncmp(*entry,"WIDE",4))
				{	*entry = *relay;
					*relay = "";
				}
#endif
			} else if (!strcmp(pieces[i], "qAR")	/* Gated directly by the following server */
			|| !strcmp(pieces[i], "qAr")		/* Gated indirectly from an iGate */
			|| !strcmp(pieces[i], "qAo")		/* Received from client, previously gated? (logon matches) */
			|| !strcmp(pieces[i], "qAO"))		/* Received from client, previously gated? (logon mismatch) */
			{	*relay = pieces[i+1];
				for (c=0; c<i; c++)
				{	if (strchr(pieces[c],'*'))
					{	*entry = pieces[c];
						if (c && !strncmp(pieces[c],"WIDE",4))
						{	*entry = pieces[c-1];
						}
						break;
					}
				}
			} else if (!strcmp(pieces[i], "qAC")	/* Received directly by following server (verified client) */
			|| !strcmp(pieces[i], "qAX")		/* Received directly by following server (unverified client) */
			|| !strcmp(pieces[i], "qAU"))		/* Received directly via UDP by following server */
			{	*relay = pieces[i+1];
				for (c=0; c<i; c++)
				{	if (strchr(pieces[c],'*'))
					{	*entry = pieces[c];
						break;
					}
				}
			}
#ifdef VERBOSE
			else fprintf(stderr,"Unrecognized q construct[%ld/%ld] %s\n", (long) i, (long) pcount, pieces[i]);
#endif
			break;	/* Only process one q Construct (the last one) per message */
		}
	}
#ifdef VERBOSE
	if (i < 0) fprintf(stderr,"Huh?  No q Construct in %ld Pieces?\n", (long) pcount);
#endif
	free(pieces);
}
#endif

void parse_route(int pcount, char **pieces, char **entry, char **relay, char **q)
{	int i, c = 0;

	*entry = *relay = *q = "";
	for (i=pcount-1; i>=0; i--)
	{	if (*pieces[i] == 'q')	/* Found the q! */
		{	*q = pieces[i];
#ifdef VERBOSE
			if (i==pcount-1)
			{	fprintf(stderr,"Huh?  q(%s) is %ld/%ld?\n", pieces[i], (long) i, (long) pcount);
			} else if (!strcmp(pieces[i], "qAI"))	/* Trace packet */
			{	fprintf(stderr,"Trace: ");
				for (c=i+1; c<pcount; c++)
					fprintf(stderr,"%s ",pieces[c]);
				fprintf(stderr,"\n");
			} else
#endif
			if (!strcmp(pieces[i], "qAS"))	/* Server logon follows (call it the iGate) */
			{	*relay = pieces[i+1];
				for (c=0; c<i; c++)
				{	if (strchr(pieces[c],'*'))
					{	*entry = pieces[c];
						if (c && !strncmp(pieces[c],"WIDE",4))
						{	*entry = pieces[c-1];
						}
						break;
					}
				}
#ifdef DONT_THINK_SO
				if (!strncmp(*entry,"WIDE",4))
				{	*entry = *relay;
					*relay = "";
				}
#endif
			} else if (!strcmp(pieces[i], "qAR")	/* Gated directly by the following server */
			|| !strcmp(pieces[i], "qAr")		/* Gated indirectly from an iGate */
			|| !strcmp(pieces[i], "qAo")		/* Received from client, previously gated? (logon matches) */
			|| !strcmp(pieces[i], "qAO"))		/* Received from client, previously gated? (logon mismatch) */
			{	*relay = pieces[i+1];
				for (c=0; c<i; c++)
				{	if (strchr(pieces[c],'*'))
					{	*entry = pieces[c];
						if (c && !strncmp(pieces[c],"WIDE",4))
						{	*entry = pieces[c-1];
						}
						break;
					}
				}
			} else if (!strcmp(pieces[i], "qAC")	/* Received directly by following server (verified client) */
			|| !strcmp(pieces[i], "qAX")		/* Received directly by following server (unverified client) */
			|| !strcmp(pieces[i], "qAU"))		/* Received directly via UDP by following server */
			{	*relay = pieces[i+1];
				for (c=0; c<i; c++)
				{	if (strchr(pieces[c],'*'))
					{	*entry = pieces[c];
						break;
					}
				}
			}
#ifdef VERBOSE
			else fprintf(stderr,"Unrecognized q construct[%ld/%ld] %s\n", (long) i, (long) pcount, pieces[i]);
#endif
			break;	/* Only process one q Construct (the last one) per message */
		}
	}
#ifdef VERBOSE
	if (i < 0) fprintf(stderr,"Huh?  No q Construct in %ld Pieces?\n", (long) pcount);
#endif
}

static int internal_parse_full_aprs(char *InBuf, APRS_PARSED_INFO_S *Info, int *hopCount, char ***Hops)
{
	if (!internal_parse_aprs(InBuf, Info)) return FALSE;

	if (hopCount && Hops)
	{	int h;
		*hopCount = Info->Path.hopCount;
		*Hops = malloc(sizeof(*Hops)*(Info->Path.hopCount+1));
		for (h=0; h<Info->Path.hopCount; h++)
		{	(*Hops)[h] = Info->Path.Hops[h];
		}
		(*Hops)[h] = NULL;	/* just for grins */
	}
//		PACKET_DATA_SIZE = (int) (&InBuf[sizeof(InBuf)] - packet_data);
	return TRUE;
}

int parse_full_aprs(char *InBuf, APRS_PARSED_INFO_S *Info)
{	int Result = internal_parse_aprs(InBuf, Info);
	char Variable[10] = {0};

	if (!Result) Info->Valid = 0;	/* If it failed, NOTHING is valid! */
	else if (!(Info->Valid & APRS_PLATFORM_VALID)) /* Don't have a platform yet, infer from ToCall (if APxxxx) */
	if (Info->dstCall[0] == 'A' && Info->dstCall[1] == 'P')
	{	int p;
static struct
{	char ToCall[10];
	char Platform[16];
	APRS_PLATFORM_V tPlatform;
	char *VariablePrefix;
} ToCalls[] = {/* From http://www.aprs.org/aprs11/tocalls.txt as of 18 Nov 15 (updated 2015/11/21) */
{ "AP1MAJ", "M1MAJ inReach" },
{ "AP1WWX", "TAPR T-238+" },
{ "AP4Rnn", "APRS4R", PLATFORM_APRS4R },
{ "APnnnD", "uSmartDigi DGate" },
{ "APnnnU", "uSmartDigi Digi" },
{ "APAFxx", "AFilter" },
{ "APAGxx", "AGATE" },
{ "APAGW",  "AGWtracker", PLATFORM_AGW_TRACKER },
{ "APAGWx", "AGWtracker", PLATFORM_AGW_TRACKER },
{ "APAMxx", "Altus Metrum" },
{ "APAXxx", "AFilterX" },
{ "APAHxx", "AHub" },
{ "APAWxx", "AGWPE" },
{ "APAND1", "APRSdroid (pre)", PLATFORM_APRSDROID },
{ "APAVT5", "SainSonic AP510" },
{ "APBxxx", "TCPIP micros?" },
{ "APB2MF", "DL2MF MF2APRS" },
{ "APBBxx", "APRSBB", PLATFORM_APRSBB },
{ "APBLxx", "BeeLine", PLATFORM_BIGREDBEE },
{ "APBLO", "Model Rocketry" },
{ "APBPQx", "G8BPQ Digi/IGate" },
{ "APCxxx", "Cellular apps" },
{ "APCBBx", "VE7UDP BB Apps" },
{ "APCLEY", "EYTraker" },
{ "APCLEZ", "Telit EZ10" },
{ "APCLWX", "EYWeather" },
{ "APCWP8", "WinphoneAPRS" },
{ "APCYxx", "Cybiko apps" },
{ "APDxxx", "APRSd, etc", PLATFORM_APRSD },
{ "APD4xx", "UP4DAR platform", },
{ "APDDxx", "DV-RPTR" },
{ "APDIxx", "DIXPRS de HA5DI", PLATFORM_DIXPRS },	/* https://sites.google.com/site/dixprs/home */
{ "APDFxx", "Auto DF units" },
{ "APDGxx", "ircDDB de G4KLX" },
{ "APDHxx", "WinDV DUTCH*Star" },
{ "APDKxx", "KI4LKF g2_ircddb" },
{ "APDOxx", "ON8JL DStar" },
{ "APDPRS", "D-Star posits" },
{ "APDRxx", "APRSdroid", PLATFORM_APRSDROID },
{ "APDSxx", "dsDIGI/dsTRACKER" },
{ "APDTxx", "APRStouch Tone" },
{ "APDUxx", "U2APRS", PLATFORM_U2APRS },
{ "APDWxx", "WB2OSZ DireWolf" },
{ "APEx",  "Telemetry" },
{ "APExx",  "Telemetry" },
{ "APExxx", "Telemetry" },
{ "APECAN", "Pecan Pico" },
{ "APELNK", "EchoLink" },
{ "APERXQ", "PE1RXQ tracker" },
{ "APFxxx", "Firenet" },
{ "APFIxx", "aprs.fi OH7LZB" },
{ "APFGxx", "Flood Gage" },
{ "APGxxx", "Gates, etc" },
{ "APGOxx", "AA3NJ PDA apps" },
{ "APHxxx", "HamHud, etc", PLATFORM_HAMHUD },
{ "APHAXn", "SM2APRS PY2UEP" },
{ "APHH2", "HamHud 2", PLATFORM_HAMHUD },
{ "APHTxx", "IU0AAC HMTracker" },
{ "APIxxx", "Icom, etc" },
{ "APICxx", "HA9MCQ Pic IGate" },
{ "APICQx", "for ICQ" },
{ "APJAxx", "JavAPRS" },
{ "APJExx", "JeAPRS" },
{ "APJIxx", "jAPRSIgate", PLATFORM_JAPRS_IGATE },
{ "APJSxx", "javAPRSSrvr", PLATFORM_JAVAPRSSRVR },
{ "APJYxx", "YAAC de KA2DDO", PLATFORM_YAAC },
{ "APK0xx", "Kenwood THD7s", PLATFORM_KENWOOD_D7 },
{ "APK003", "Kenwood TH-D72", PLATFORM_KENWOOD_D72 },
{ "APK004", "Kenwood TH-D74", PLATFORM_KENWOOD_D74 },
{ "APK1xx", "Kenwood D700s", PLATFORM_KENWOOD_D700 },
{ "APK102", "Kenwood D710", PLATFORM_KENWOOD_D710 },
{ "APKRAM", "KRAMstuff.com" },
{ "APLxxx", "Liunx apps" },
{ "APLMxx", "WA0TQG XcvrCtl" },
{ "APLOBJ", "OBJECT Server" },
{ "APLQRU", "QRU Server" },
{ "APMxxx", "MacAPRS, etc", PLATFORM_MAC_APRS },
{ "APMIxx", "PLX Digi/Tracker" },
{ "APMGxx", "AB0TJ MiniGate" },
{ "APMTxx", "LZ1PPL tracker" },
{ "APNxxx", "Network devices" },
{ "APN3xx", "Kantronics KPC-3", PLATFORM_KPC_3 },
{ "APN9xx", "KPC-9612 Roms", PLATFORM_KPC_9612 },
{ "APNAxx", "WB6ZSU APRServe" },
{ "APNDxx", "DIGI_NED", PLATFORM_DIGI_NED },
{ "APNMxx", "MJF TNC roms" },
{ "APNPxx", "Paccom TNC roms" },
{ "APNTxx", "SV2AGW's TNT tnc" },
{ "APNUxx", "UIdigi", PLATFORM_UI_DIGI },
{ "APNWxx", "SQ3FYK WX/Digi", PLATFORM_SQ3FYK },
{ "APNXx", "TNC-X de K6DBG", PLATFORM_TNC_X },
{ "APNXxx", "TNC-X de K6DBG", PLATFORM_TNC_X },
{ "APNK01", "Kenwood D700", PLATFORM_KENWOOD_D700 },
{ "APNK80", "KAM v8.0", PLATFORM_KAM },
{ "APNKMP", "KAM+", PLATFORM_KAM },
{ "APOxxx", "APRSpoint" },
{ "APOAxx", "OpenAPRS", PLATFORM_OPEN_APRS },
{ "APOTxx", "Open Tracker", PLATFORM_OPEN_TRACK },
{ "APOZxx", "www.KissOZ.dk", PLATFORM_KISSOZ },
{ "APOLUx", "OSCAR/AMSAT-LU" },
{ "APOD1w", "Open Track+1 WX", PLATFORM_OPEN_TRACK },
{ "APOU2k", "Open Track Ulti", PLATFORM_OPEN_TRACK },
{ "APPxxx", "pocketAPRS, etc" },
{ "APPTxx", "KetaiTracker" },
{ "APQK", "Earthquake data" },
{ "APQxxx", "Earthquake data" },
{ "APR2MF", "DL2MF MF2APRS" },
{ "APR8xx", "APRSdos v800+", PLATFORM_APRS_DOS, "8" },
{ "APRDxx", "APRSdata, APRSdr" },
{ "APRFID", "RFID Reader" },
{ "APRGxx", "aprsg de OH2GVE" },
{ "APRHH2", "HamHud 2" },
{ "APRKxx", "APRStk" },
{ "APRNOW", "W5GGW ipad" },
{ "APRS",   "Generic APRS", PLATFORM_APRS },
{ "APRX0x", "aprx de OH2MQK", PLATFORM_APRX, "0" },
{ "APRX1x", "aprx de OH2MQK", PLATFORM_APRX, "1" },
{ "APRX2x", "aprx de OH2MQK", PLATFORM_APRX, "2" },
{ "APRX3x", "aprx de OH2MQK", PLATFORM_APRX, "3" },
{ "APRX4x", "APRSmax", PLATFORM_OTHER, "4" },
{ "APRX5x", "APRSmax", PLATFORM_OTHER, "5" },
{ "APRX6x", "APRSmax", PLATFORM_OTHER, "6" },
{ "APRX7x", "APRSmax", PLATFORM_OTHER, "7" },
{ "APRX8x", "APRSmax", PLATFORM_OTHER, "8" },
{ "APRX9x", "APRSmax", PLATFORM_OTHER, "9" },
//{ "APRX20", "APRX 20", PLATFORM_APRX },
{ "APRTLM", "MIM/Mic-lites" },
{ "APRTFC", "APRStraffic" },
{ "APRRTx", "RPC electronics" },
{ "APRSTx", "APRStt - DTMF", PLATFORM_APRSTT },
{ "APSxxx", "APRS+SA, etc", PLATFORM_APRS_SA },
{ "APSARx", "ZL4FOX SARTrack", PLATFORM_SARTRACK },
{ "APSAR", "ZL4FOX SARTrack", PLATFORM_SARTRACK },
{ "APSMSx", "SMS Gateway" },
{ "APSCxx", "aprsc" },
{ "APSKxx", "APRS Msgr", PLATFORM_APRS_MESSENGER },
{ "APSK25", "APRS Msgr GMSK", PLATFORM_APRS_MESSENGER },
{ "APSK63", "APRS Msgr PSK", PLATFORM_APRS_MESSENGER },
{ "APSMSx", "SMS gateway" },
{ "APSTMx", "W7QO's Balloons" },
{ "APSTPO", "N0AGI Sat Ops" },
//{ "APTHWX", "Unknown WX" },
{ "APTIGR", "TigerTrak" },
{ "APTTxx", "Tiny Trak", PLATFORM_TINYTRAK },
{ "APTT4", "TinyTrak 4", PLATFORM_TINYTRAK4 },
{ "APT2xx", "TinyTrak II", PLATFORM_TINYTRAK2 },
{ "APT3xx", "TinyTrak III", PLATFORM_TINYTRAK3 },
{ "APT3A1", "Micro-Trak AIO", PLATFORM_ALLINONE },
{ "APT4xx", "TinyTrak 4", PLATFORM_TINYTRAK4 },
{ "APTAxx", "K4ATM tiny track" },
{ "APTWxx", "Byons WXTrac", PLATFORM_WXTRAC },
{ "APTVxx", "ATV/APRN/SSTV" },
{ "APU1xx", "UIview 16 bit", PLATFORM_UI_VIEW_1xx, "1" },
{ "APU2xx", "UIview 32 bit", PLATFORM_UI_VIEW_2xx, "2" },
{ "APU3xx", "UIview terminal", PLATFORM_UI_VIEW_3xx, "3" },
{ "APUDRx", "UDR (APRS/Dstar)" },
{ "APVxxx", "VOIP Apps" },
{ "APVRxx", "for IRLP" },
{ "APVRTn", "AVRT Family", PLATFORM_AVRT },
{ "APVLxx", "for I-LINK" },
{ "APVExx", "for ECHO link" },
{ "APWxxx", "WinAPRS, etc", PLATFORM_WIN_APRS },
{ "APWAxx", "APRSISDR", PLATFORM_APRSISDR },
{ "APWSxx", "DF4IAN's WS2300" },
{ "APWMxx", "APRSISCE", PLATFORM_APRSISCE },
{ "APWWxx", "APRSIS32", PLATFORM_APRSIS32 },
{ "APXnnn", "Xastir", PLATFORM_XASTIR },
{ "APXRnn", "Xrouter" },
{ "APYxxx", "Yaesu" },
{ "APY008", "Yaesu VX-8r", PLATFORM_YAESU_VX8R },
{ "APY350", "Yaesu FTM-350", PLATFORM_YAESU_FTM350 },
{ "APY400", "Yaesu FTM-400", PLATFORM_YAESU_FTM400DR },
{ "APYTxx", "YagTracker", PLATFORM_YAGTRACKER },
{ "APZx", "Experimental", PLATFORM_EXPERIMENTAL },
{ "APZxx", "Experimental", PLATFORM_EXPERIMENTAL },
{ "APZxxx", "Experimental", PLATFORM_EXPERIMENTAL },
{ "APZ0xx", "Xastir-old vers", PLATFORM_XASTIR, "0" },
{ "APZ053", "PaulCo MiLog 5.3", PLATFORM_EXPERIMENTAL },
{ "APZ060", "PaulCo MiARM 6", PLATFORM_EXPERIMENTAL },
{ "APZ061", "PaulCo MiARM 6.1", PLATFORM_EXPERIMENTAL },
{ "APZ247", "NR0Q's UPRS", PLATFORM_EXPERIMENTAL },
{ "APZMAJ", "M1MAJ inReach", PLATFORM_EXPERIMENTAL },
{ "APZMDR", "HaMDR trackers", PLATFORM_EXPERIMENTAL },
{ "APZWKR", "GM1WKR NetSked", PLATFORM_EXPERIMENTAL },
{ "APZPnn", "PaulCo Stuff", PLATFORM_EXPERIMENTAL },
{ "APZPAD", "Smart Palm", PLATFORM_EXPERIMENTAL },
{ "APZTKP", "N0LP TrackPoint", PLATFORM_EXPERIMENTAL },
{ "APZWIT", "MAP27 by EI7IG", PLATFORM_EXPERIMENTAL }
};
		char *dstDash = strchr(Info->dstCall,'-');
		int dstLen = dstDash?(dstDash-Info->dstCall):strlen(Info->dstCall);
		for (p=sizeof(ToCalls)/sizeof(ToCalls[0])-1; p>=0; p--)	/* do it backwards for collision purposes */
		{	if (*Variable) memset(Variable,0,sizeof(Variable));	/* Clear previous accumulations */
			if (Info->dstCall[2] == ToCalls[p].ToCall[2]	/* Cheap short-circuit check */
			|| islower(ToCalls[p].ToCall[2]))			/* Handle wildcards also */
			{	int len = strlen(ToCalls[p].ToCall);
				if (dstLen == len)	/* Lengths have to match also */
				{	int i;
					for (i=0; i<len; i++)
					{	if (ToCalls[p].ToCall[i] == 'x')
						{	if (!isalnum(Info->dstCall[i]&0xff))
							{
#ifdef VERBOSE
//TraceError(NULL,"%s Unmatched Platform %s Not %s[%ld]x (%c!=%c) (Datatype:%c)\n", Info->srcCall, Info->dstCall, ToCalls[p].ToCall, (long) i, ToCalls[p].ToCall[i], Info->dstCall[i], Info->datatype);
#endif
								break;	/* Not a match */
							} else Variable[strlen(Variable)] = Info->dstCall[i];
						} else if (ToCalls[p].ToCall[i] == 'n')
						{	if (!isdigit(Info->dstCall[i]&0xff))
							{
#ifdef VERBOSE
//TraceError(NULL,"%s Unmatched Platform %s Not %s[%ld]n (%c!=%c) (Datatype:%c)\n", Info->srcCall, Info->dstCall, ToCalls[p].ToCall, (long) i, ToCalls[p].ToCall[i], Info->dstCall[i], Info->datatype);
#endif
								break;	/* Not a match */
							} else Variable[strlen(Variable)] = Info->dstCall[i];
						} else if (ToCalls[p].ToCall[i] != Info->dstCall[i])
						{
#ifdef VERBOSE
//TraceError(NULL,"%s Unmatched Platform %s Not %s[%ld]%c!=%c (Datatype:%c)\n", Info->srcCall, Info->dstCall, ToCalls[p].ToCall, (long) i, ToCalls[p].ToCall[i], Info->dstCall[i], Info->datatype);
#endif
							break;	/* Not a match */
						}
					}
					if (i >= len)	/* They all matched! */
					{	memset(Info->Platform,0,sizeof(Info->Platform));
						strncpy(Info->Platform, ToCalls[p].Platform, min(sizeof(Info->Platform),sizeof(ToCalls[p].Platform)));
						if (*Variable || ToCalls[p].VariablePrefix)
						{	strcat(Info->Platform," (");
							if (ToCalls[p].VariablePrefix)
								strcat(Info->Platform,ToCalls[p].VariablePrefix);
							strcat(Info->Platform,Variable);
							strcat(Info->Platform,")");
						}
						Info->tPlatform = ToCalls[p].tPlatform;
						if (!Info->tPlatform) Info->tPlatform = PLATFORM_OTHER;
						Info->Valid |= APRS_PLATFORM_VALID;
						break;
					}
#ifdef VERBOSE
//else TraceError(NULL,"%s Unmatched Platform %s Not %s (Datatype:%c)\n", Info->srcCall, Info->dstCall, ToCalls[p].ToCall, Info->datatype);
#endif
				}
			}
#ifdef VERBOSE
//else TraceError(NULL,"%s Unmatched Platform %s Not %s[2] (Datatype:%c)\n", Info->srcCall, Info->dstCall, ToCalls[p].ToCall, Info->datatype);
#endif
		}
#ifdef VERBOSE
//		if (p<0) TraceError(NULL,"%s Unmatched Platform %s (Datatype:%c)\n", Info->srcCall, Info->dstCall, Info->datatype);
#endif
	}
#ifdef OLD_WAY
	else if (*Info->dstCall == 'B'
	&& !strcmp(Info->dstCall, "BEACON"))
	{	strncpy(Info->Platform, "BEACON", sizeof(Info->Platform));
		Info->tPlatform = PLATFORM_BEACON;
		Info->Valid |= APRS_PLATFORM_VALID;
	} else if (*Info->dstCall == 'I'
	&& !strcmp(Info->dstCall, "ID"))
	{	strncpy(Info->Platform, "ID", sizeof(Info->Platform));
		Info->tPlatform = PLATFORM_ID;
		Info->Valid |= APRS_PLATFORM_VALID;
	}
#ifdef VERBOSE
//else TraceError(NULL,"%s Non-application Platform %s (Datatype:%c)\n", Info->srcCall, Info->dstCall, Info->datatype);
#endif
#else
	else
	{	int p;
static struct
{	char ToCall[10];
	APRS_PLATFORM_V tPlatform;
} GenericCalls[] = { { "AIR" },
{ "ALL" },
{ "AP" },
{ "BEACON", PLATFORM_BEACON },
{ "CQ" },
{ "GPS" },
{ "DF" },
{ "DGPS" },
{ "DRILL" },
{ "DX" },
{ "ID", PLATFORM_ID },
{ "JAVA" },
{ "MAIL" },
{ "MICE" },
{ "QST" },
{ "QTH" },
{ "RTCM" },
{ "SKY" },
{ "SPACE" },
{ "SPC" },
{ "SYM" },
{ "TEL" },
{ "TEST" },
{ "TLM" },
{ "WX" },
{ "ZIP" } };

		char *dstDash = strchr(Info->dstCall,'-');
		int dstLen = dstDash?(dstDash-Info->dstCall):strlen(Info->dstCall);
		for (p=sizeof(GenericCalls)/sizeof(GenericCalls[0])-1; p>=0; p--)	/* do it backwards for collision purposes */
		{	if (*Info->dstCall == *GenericCalls[p].ToCall)	/* Cheap short-circuit check */
			{	int len = strlen(GenericCalls[p].ToCall);
				if (dstLen >= len	/* only checking leading characters */
				&& !strncmp(Info->dstCall, GenericCalls[p].ToCall, len))
				{	memset(Info->Platform,0,sizeof(Info->Platform));
					strncpy(Info->Platform, GenericCalls[p].ToCall, min(sizeof(Info->Platform),sizeof(GenericCalls[p].ToCall)));
					if (dstLen > len)
					{	strcat(Info->Platform," (");
						strncat(Info->Platform,&Info->dstCall[len],dstLen-len);
						strcat(Info->Platform,")");
					}
					Info->tPlatform = GenericCalls[p].tPlatform;
					if (!Info->tPlatform) Info->tPlatform = PLATFORM_GENERIC;
					Info->Valid |= APRS_PLATFORM_VALID;
					break;
				}
			}
		}
	}
#endif
	return Result;
}

char * parse_aprs(char *InBuf, char **src, char **dst, int *hopCount, char ***Hops, double *rlat, double *rlon, double *alt, int *symbol, char *datatype)
{static APRS_PARSED_INFO_S Info;	/* Must be static as we return pointers inside the structure */

	if (internal_parse_full_aprs(InBuf, &Info, hopCount, Hops))
	{	if (Info.Valid)
		{	*src = Info.srcCall;
			*dst = Info.dstCall;
		} else *src = *dst = "";
		if (Info.Valid & APRS_LATLON_VALID)
		{	*rlat = Info.lat;
			*rlon = Info.lon;
		} else *rlat = *rlon = 0.0;
		if (Info.Valid & APRS_ALTITUDE_VALID)
			*alt = Info.alt;
		else *alt = 0.0;
		if (Info.Valid & APRS_SYMBOL_VALID)
			*symbol = Info.symbol;
		else if (Info.Valid & APRS_SYMBOL_DEFAULTED)
			*symbol = Info.symbol;
		else *symbol = 0;
		if (Info.Valid & APRS_DATATYPE_VALID)
			*datatype = Info.datatype;
		else *datatype = 0;

		if (Info.Valid & (APRS_ITEM_VALID|APRS_OBJECT_VALID))
			return Info.objCall;
		return Info.srcCall;
	}
	return NULL;
}

static char Generic[] = "Generic";	// For global comparison
int IsPlatformGeneric(APRS_PLATFORM_V tPlatform)
{	char *Group, *Platform=GetPlatformString(tPlatform,&Group);
	return Platform && Group == Generic;
}
char *GetPlatformString(APRS_PLATFORM_V tPlatform, char **pGroup)
{
static char Kenwood[] = "Kenwood";
static char Yaesu[] = "Yaesu";
static char Byonics[] = "Byonics";
static char Android[] = "Android";
static char ArgentData[] = "ArgentData";
static char BigRedBee[] = "BigRedBee";
static char Kantronics[] = "Kantronics";
static char APRSISCE32[] = "APRSISCE/32";
static char UIView[] = "UI-View";

static struct
{	APRS_PLATFORM_V tPlatform;
	char *String;
	char *Group;
} tPlatformStrings[] = {
/* Note: this routine is MUCH more efficient if the table is kept in order */
{PLATFORM_UNKNOWN,"Unrecognized"},
{PLATFORM_EXPERIMENTAL,"Experimental"},

{PLATFORM_APRS_DOS,"APRSdos"},
{PLATFORM_MAC_APRS,"MacAPRS"},
{PLATFORM_POCKET_APRS,"pocketAPRS"},
{PLATFORM_APRS_SA,"APRS+SA"},
{PLATFORM_WIN_APRS,"WinAPRS"},
{PLATFORM_X_APRS,"X-APRS"},

{PLATFORM_AGW_TRACKER,"AGWTracker"},
{PLATFORM_APRS4R,"APRS4R"},
{PLATFORM_APRSBB,"APRSBB"},
{PLATFORM_APRSD,"aprsd"},
{PLATFORM_APRSDROID,"APRSdroid",Android},
{PLATFORM_APRSISCE,"APRSISCE",APRSISCE32},
{PLATFORM_APRSISDR,"APRSISDR",APRSISCE32},
{PLATFORM_APRSIS32,"APRSIS32",APRSISCE32},
{PLATFORM_APRS_MESSENGER,"APRS Msngr"},
{PLATFORM_APRSTT,"APRStt"},
{PLATFORM_APRX,"aprx de OH2MQK"},
{PLATFORM_AVRT,"AVRT Family"},
{PLATFORM_DIGI_NED,"DIGI_NED"},
{PLATFORM_DIXPRS,"DIXPRS"},
{PLATFORM_JAPRS_IGATE,"JAPRS IGATE"},
{PLATFORM_JAVAPRSSRVR,"javAPRSsrvr"},
{PLATFORM_OPEN_APRS,"OpenAPRS"},
{PLATFORM_SARTRACK,"SARTrack"},
{PLATFORM_SQ3FYK,"SQ3FYK WX/Digi"},
{PLATFORM_U2APRS,"U2APRS",Android},
{PLATFORM_UI_VIEW_32N,"UI-View 32N",UIView},
{PLATFORM_UI_VIEW_32,"UI-View 32",UIView},
{PLATFORM_UI_VIEW_23N,"UI-View 23N",UIView},
{PLATFORM_UI_VIEW_23,"UI-View 23",UIView},
{PLATFORM_UI_VIEW_22,"UI-View 22",UIView},
{PLATFORM_UI_VIEW_1xx,"UI-View 16bit",UIView},
{PLATFORM_UI_VIEW_2xx,"UI-View 32bit",UIView},
{PLATFORM_UI_VIEW_3xx,"UI-View Term",UIView},
{PLATFORM_UI_VIEW_OTHER,"UI-View Other",UIView},
{PLATFORM_UISS,"UISS"},
{PLATFORM_XASTIR,"xastir"},
{PLATFORM_YAAC,"YAAC"},

{PLATFORM_KAM,"Kantronics KAM",Kantronics},
{PLATFORM_KISSOZ,"www.kissoz.dk"},
{PLATFORM_KPC_3,"Kantronics KPC-3",Kantronics},
{PLATFORM_KPC_9612,"Kantronics KPC-9612",Kantronics},
{PLATFORM_ALLINONE,"Byonics Micro-Trak AIO",Byonics},
{PLATFORM_TINYTRAK,"Tiny Trak (APTTxx)",Byonics},
{PLATFORM_TINYTRAK2,"Byonics TinyTrak2",Byonics},
{PLATFORM_TINYTRAK3,"Byonics TinyTrak3",Byonics},
{PLATFORM_TINYTRAK4,"Byonics TinyTrak4",Byonics},
{PLATFORM_WXTRAC,"Byonics WXTrak",Byonics},
{PLATFORM_OPEN_TRACK,"ArgentData Tracker2",ArgentData},
{PLATFORM_HAMHUD,"HAMHUD"},
{PLATFORM_HINZTEC,"HinzTec"},
{PLATFORM_BIGREDBEE,"BigRedBee",BigRedBee},
{PLATFORM_UI_DIGI,"UIDIGI"},
{PLATFORM_TNC_X,"TNC-X"},
{PLATFORM_KENWOOD_D7,"Kenwood D7",Kenwood},
{PLATFORM_KENWOOD_D72,"Kenwood D72",Kenwood},
{PLATFORM_KENWOOD_D74,"Kenwood D74",Kenwood},
{PLATFORM_KENWOOD_D700,"Kenwood D700",Kenwood},
{PLATFORM_KENWOOD_D710,"Kenwood D710",Kenwood},
{PLATFORM_YAESU_FT1D,"Yaesu FT1D",Yaesu},
{PLATFORM_YAESU_FT2D,"Yaesu FT2D",Yaesu},
{PLATFORM_YAESU_VX8R,"Yaesu VX8R",Yaesu},
{PLATFORM_YAESU_VX8G,"Yaesu VX8G",Yaesu},
{PLATFORM_YAESU_FTM100D,"Yaesu FTM100D",Yaesu},
{PLATFORM_YAESU_FTM350,"Yaesu FTM350",Yaesu},
{PLATFORM_YAESU_FTM400DR,"Yaesu FTM400DR",Yaesu},
{PLATFORM_YAGTRACKER,"YagTracker"},

{PLATFORM_APRS,"Generic APRS",Generic},
{PLATFORM_BEACON,"Generic BEACON",Generic},
{PLATFORM_GENERIC,"Generic Beacon-Style",Generic},
{PLATFORM_ID,"Generic ID",Generic},
{PLATFORM_MICE,"Generic Mic-E",Generic},
{PLATFORM_NMEA,"Generic Raw NMEA",Generic},
{PLATFORM_OBSOLETE,"Generic Obsolete",Generic},

{PLATFORM_OTHER,"Other"}};
/* Note: this routine is MUCH more efficient if the table is kept in order */

	if (tPlatform >= 0
	&& tPlatform < sizeof(tPlatformStrings)/sizeof(tPlatformStrings[0])
	&& tPlatformStrings[tPlatform].tPlatform == tPlatform)
	{	if (pGroup) *pGroup = tPlatformStrings[tPlatform].Group;
		return tPlatformStrings[tPlatform].String;
	} else
	{	int i;
		for (i=0; i<sizeof(tPlatformStrings)/sizeof(tPlatformStrings[0]); i++)
			if (tPlatformStrings[i].tPlatform == tPlatform)
			{	if (pGroup) *pGroup = tPlatformStrings[i].Group;
TraceLogThread("PlatformBust",TRUE,"Out Of Order Platform(%s)\n", tPlatformStrings[i].String);
return tPlatformStrings[i].String;
			}
	}
TraceLogThread("PlatformBust",TRUE,"Undefined Or OutOfRange Platform(%ld)\n", (long) tPlatform);
return NULL;
}
