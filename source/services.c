#ifdef FOR_INFO_ONLY
INTEGER_ID_F AprsGetPositionIndex(COORDINATE_S *Coord);
INTEGER_ID_F AprsGetStationIndex(STATION_ID_F OwnerID, STATION_ID_F StationID);
INTEGER_ID_F AprsGetNextPacketIndex(void);
#endif
/*
<pre>
	When	Who		What
	080731	L.Deffenbaugh	Original Implementation
	100916	L.Deffenbaugh	Use new APRS parser to suppress RF-received third-party packets
	110721	L.Deffenbaugh	Support British 2E0 callsigns (NXNX* calls).
	120223	L.Deffenbaugh	Support LogDigipeatedPacket for Queueing purposes
	120808	L.Deffenbaugh	Only store posit packets in database
</pre>
*/

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <df/base.h>

#include <uf/include/base.h>
#include <uf/source/dgprint.h>
#include <uf/source/dsrtns.h>
#include <uf/source/hprtns.h>
#include <uf/source/lgrtns.h>
#include <uf/source/rtrtns.h>
#include <uf/source/smrtns.h>
#include <uf/source/syrtns.h>
#include <uf/source/thrtns.h>

#include <ci/include/cidef.h>
#include <ci/source/ciarray.h>
#include <ci/source/ciconv.h>

#include <db/include/dbdef.h>
#include <db/source/database.h>
#include <db/source/dbaccess.h>
#include <db/source/message.h>

#include <aprs/include/aprsdef.h>
#include <aprs/include/database.h>
#include <aprs/include/messages.h>

#include <aprs/source/aprs.h>
#include <aprs/source/database.h>
#include <aprs/source/parsedef.h>
#include <aprs/source/parse.h>
#include <aprs/source/UDPListn.h>	/* For AprsLookupStationLocation */

#include <aprs/source/services.h>

#ifdef OBSOLETE
struct
{	char symbol;
	char *Primary;
	char *Alternate;
} SymbolNames[] = {
{ '!', "Police Stn", "Emergency"},
{ '"', "No Symbol", "No Symbol"},
{ '#', "Digi", "No. Digi"},
{ '$', "Phone", "Bank"},
{ '%', "DX Cluster", "No Symbol"},
{ '&', "HF Gateway", "No. Diam'd"},
{ '\'', "Plane sm", "Crash site"},
{ '(', "Mob Sat Stn", "Cloudy"},
{ ')', "WheelChair", "MEO"},
{ '*', "Snowmobile", "Snow"},
{ '+', "Red Cross", "Church"},
{ ',', "Boy Scout", "Girl Scout"},
{ '-', "Home", "Home (HF)"},
{ '.', "X", "UnknownPos"},
{ '/', "Red Dot", "Destination"},
{ '0', "Circle (0)", "No. Circle"},
{ '1', "Circle (1)", "No Symbol"},
{ '2', "Circle (2)", "No Symbol"},
{ '3', "Circle (3)", "No Symbol"},
{ '4', "Circle (4)", "No Symbol"},
{ '5', "Circle (5)", "No Symbol"},
{ '6', "Circle (6)", "No Symbol"},
{ '7', "Circle (7)", "No Symbol"},
{ '8', "Circle (8)", "No Symbol"},
{ '9', "Circle (9)", "Petrol Stn"},
{ ':', "Fire", "Hail"},
{ ';', "Campground", "Park"},
{ '<', "Motorcycle", "Gale Fl"},
{ '=', "Rail Eng.", "No Symbol"},
{ '>', "Car", "No. Car"},
{ '?', "File svr", "Info Kiosk"},
{ '@', "HC Future", "Hurricane"},
{ 'A', "Aid Stn", "No. Box"},
{ 'B', "BBS", "Snow blwng"},
{ 'C', "Canoe", "Coast G'rd"},
{ 'D', "No Symbol", "Drizzle"},
{ 'E', "Eyeball", "Smoke"},
{ 'F', "Tractor", "Fr'ze Rain"},
{ 'G', "Grid Squ.", "Snow Shwr"},
{ 'H', "Hotel", "Haze"},
{ 'I', "Tcp/ip", "Rain Shwr"},
{ 'J', "No Symbol", "Lightning"},
{ 'K', "School", "Kenwood"},
{ 'L', "Usr Log-ON", "Lighthouse"},
{ 'M', "MacAPRS", "No Symbol"},
{ 'N', "NTS Stn", "Nav Buoy"},
{ 'O', "Balloon", "Rocket"},
{ 'P', "Police", "Parking  "},
{ 'Q', "TBD", "Quake"},
{ 'R', "Rec Veh'le", "Restaurant"},
{ 'S', "Shuttle", "Sat/Pacsat"},
{ 'T', "SSTV", "T'storm"},
{ 'U', "Bus", "Sunny"},
{ 'V', "ATV", "VORTAC"},
{ 'W', "WX Service", "No. WXS"},
{ 'X', "Helo", "Pharmacy"},
{ 'Y', "Yacht", "No Symbol"},
{ 'Z', "WinAPRS", "No Symbol"},
{ '[', "Jogger", "Wall Cloud"},
{ '\\', "Triangle", "No Symbol"},
{ ']', "PBBS", "No Symbol"},
{ '^', "Plane lrge", "No. Plane"},
{ '_', "WX Station", "No. WX Stn"},
{ '`', "Dish Ant.", "Rain"},
{ 'a', "Ambulance", "No. Diamond"},
{ 'b', "Bike", "Dust blwng"},
{ 'c', "ICP", "No. CivDef"},
{ 'd', "Fire Station", "DX Spot"},
{ 'e', "Horse", "Sleet"},
{ 'f', "Fire Truck", "Funnel Cld"},
{ 'g', "Glider", "Gale"},
{ 'h', "Hospital", "HAM store"},
{ 'i', "IOTA", "No. Blk Box"},
{ 'j', "Jeep", "WorkZone"},
{ 'k', "Truck", "SUV"},
{ 'l', "Laptop", "Area Locns"},
{ 'm', "Mic-E Rptr", "Milepost"},
{ 'n', "Node", "No. Triang"},
{ 'o', "EOC", "Circle sm"},
{ 'p', "Rover", "Part Cloud"},
{ 'q', "Grid squ.", "No Symbol"},
{ 'r', "Antenna", "Restrooms"},
{ 's', "Power Boat", "No. Boat"},
{ 't', "Truck Stop", "Tornado"},
{ 'u', "Truck 18wh", "No. Truck"},
{ 'v', "Van", "No. Van"},
{ 'w', "Water Stn", "Flooding"},
{ 'x', "XAPRS", "No Symbol"},
{ 'y', "Yagi", "Sky Warn"},
{ 'z', "Shelter", "No. Shelter"},
{ '{', "No Symbol", "Fog"},
{ '|', "TNC Stream Sw", "TNC Stream SW"},
{ '}', "No Symbol", "No Symbol"},
{ '~', "TNC Stream Sw", "TNC Stream SW"} };

char *GetSymbolName(int Symbol)
{	int Page = Symbol >> 8;
	Symbol = (Symbol & 0xff) - SymbolNames[0].symbol;
	if (Symbol < 0 || Symbol >= ACOUNT(SymbolNames))
		return "*Unknown*";
	return Page?SymbolNames[Symbol].Alternate:SymbolNames[Symbol].Primary;
}
#endif

VFUNCTION AprsMakeStationID(STRING_F ID, STATION_ID_F *rID)
{	STRING_F p, q;
	memset(rID,0,sizeof(*rID));
	if (ID)
	for (p=ID, q=*rID; *p && q<&((*rID)[sizeof(*rID)]); p++)
		if (*p != '*')
			*q++ = *p;
}

static char AprsWhatChar(char t)
{	if (isdigit(t)) return 'N';
	if (t == '-') return '-';
	if (isalpha(t)) return 'X';
	if (t=='*') return '*';
	return '\?';
}

static BOOLEAN_F AprsIsLegalStation(STATION_ID_F *ID)
{	BOOLEAN_F Result;
	INDEX_F i, p=0;
	char Pattern[sizeof(*ID)+1] = {0};
	COUNT_F Len = RtStrnlen(STRING(*ID));
static	STRING_F KnownDigis[] = { "HARIN", "CNABAR", "EMIGRA", "SLTRVR", "BLKMTN", "GENEVA",	/* W7BOZ */
								  "GRACID", "SNKRVR", "REDSPR", "LNCBL", "LB2424", "LB2414-2",	/* LOTOJA 2013 */
								  "REX", "ERARC", "IDAFLS", "HARIN", "PEQUOP", "VIC",
								  "LEWIS", "CNABAR", "SHAFER", "TOULON", "RAZOR", "GERLCH",
								  "VIRGPK", "SNOW", "LIME", "SNOBNK", "HOWARD", "COLDSP",
								  "ROCKY", "BUTLER", "CHILL", "BOLDER", "MACPAS", "GOLDCR",
								  "UNVMTN", "POINT6", "PNKMTN", "LOOKOT", "CREST", "BAKER",
								  "HALL", "SPARWD", "CNP", "LYTTLE", "FARS-2", "ELKWTR",
								  "RCHFLD", "HOLDEN", "STRLNG", "HORNMT", "CEDRMT", "SKYLIN",
								  "STARPT", "ABAJO", "BALD", "BAXTER", "BLUEMT", "CEDAR",
								  "ANVIL", "GRAND", "UBUTTE", "RBERRY", "HAYDN", "KENDAL",
								  "PARRTT", "DURNGO", "HARRIS", "WOLFCK", "HILL71", "GREEN",
								  "TENDER", "CHAIR", "OLYS", "INDYPS", "CLIMAX", "VAILMT",
								  "LKOUT", "SUNLGT", "WERNER", "METHOD", "THOR", "ALMOSA",

								  "CREBC-1", "CREBC-2",	/* N0GPS-3 */

								  "GLASS", "DARKCN", "BENRDG", "ELPASO", "CABALL", "JACKPK",	/* El Paso */
								  "FRISCO", "ALPAZ", "DVPORT", "MMTN", "GALLUP", "LAMOSC",
								  "GRNSAZ", "PORTER", "HOLBRK", "HELIO", "GPARZ", "GALPK",
								  "COWMTN", "ROSWEL", "CAPTAN", "CAPILA", "RAVEN", "CEDRO",
								  "TAPIA", "SANDIA", "ELKMTN", "TESQUE", "EUREKA", "ANGFIR",
								  "TUCARI", "MELROZ", "PORTLS", "BLKWTR", "GRULKY", "AMAS",
								  "DUMAS", "WAYSID", "PAMPA", "CANADN", "WALSH", "SPEAR",
								  "ARBUKL", "NORMAN", "SOKC", "EARC", "PONCA", "BRTLVL",
								  "ETULSA", "KTCHM", "DECATU", "CLRWTR", "BAYLOR", "BOAT" };

	if (!Len) return FALSE;
	if ((*ID)[0] == 'q') return FALSE;

	Pattern[p] = AprsWhatChar((*ID)[0]);
	for (i=1; i<Len; i++)
	{	char t = AprsWhatChar((*ID)[i]);
		if (t != Pattern[p] && t != '*')	/* Ignore used marking */
			Pattern[++p] = t;
	}
	Result = !strcmp(Pattern,"XNX")		/* Straight callsign */
		|| !strcmp(Pattern,"XNX-N")	/* Callsign and numeric SSID */
		|| !strcmp(Pattern,"XNX-X")	/* Callsign and alpha SSID */
		|| !strcmp(Pattern,"XNX-NX") || !strcmp(Pattern,"XNX-XN");	/* Callsign and mixed SSID */
	if (!Result)
	Result = !strcmp(Pattern,"NXNX")		/* Straight callsign */
		|| !strcmp(Pattern,"NXNX-N")	/* Callsign and numeric SSID */
		|| !strcmp(Pattern,"NXNX-X")	/* Callsign and alpha SSID */
		|| !strcmp(Pattern,"NXNX-NX") || !strcmp(Pattern,"NXNX-XN");	/* Callsign and mixed SSID */

	if (!Result)
	{	INDEX_F i;
		for (i=0; i<ACOUNT(KnownDigis); i++)
		{	if (**ID == *KnownDigis[i]
			&& !strnicmp(*ID, KnownDigis[i], sizeof(STATION_ID_F)))
			{	Result = TRUE;
//				DgPrintf("Approving KnownDigi(%s)\n", KnownDigis[i]);
				break;
			}
		}
	}

	if (!Result)
	{static	HASH_S *Hash = NULL;
		if (!Hash) Hash = DsCreateHash("RouteCalls", sizeof(*ID), DSTRING_F, HERE);
		if (!DsLookupHashKey(Hash,ID))
		{	STRING_F Value = strcpy(STRING_CALLOC(Len,sizeof(**ID)),*ID);
			DgPrintf("%.*s -> %s %s\n", STRING(*ID), Pattern, Result?"GOOD":"NOT");
			if (!DsInsertIfNewHashKey(Hash,ID,Value,HERE))
				STRING_FREE(Value);
		}
	}

	return Result;
}

VFUNCTION AprsBoundingBox(COORDINATE_S *Where, COORDINATE_S *Min, COORDINATE_S *Max)
{	if (Where->Latitude || Where->Longitude)
	{	if (!Min->Latitude && !Min->Longitude)
			*Min = *Where;
		else
		{	if (Where->Latitude < Min->Latitude) Min->Latitude = Where->Latitude;
			if (Where->Longitude < Min->Longitude) Min->Longitude = Where->Longitude;
		}
		if (!Max->Latitude && !Max->Longitude)
			*Max = *Where;
		else
		{	if (Where->Latitude > Max->Latitude) Max->Latitude = Where->Latitude;
			if (Where->Longitude > Max->Longitude) Max->Longitude = Where->Longitude;
		}
	}
}


/*	From: http://www.movable-type.co.uk/scripts/latlong.html */
VFUNCTION AprsHaversine(COORDINATE_S *From, COORDINATE_S *To, double *Dist, double *Bearing)
{static	double EarthRadius = 3959.0;	/* miles */
static	double DegreesPerRadian = 57.2957795;

	double lat1 = From->Latitude / DegreesPerRadian;
	double lon1 = From->Longitude / DegreesPerRadian;
	double lat2 = To->Latitude / DegreesPerRadian;
	double lon2 = To->Longitude / DegreesPerRadian;

/*	var R = 6371; // km
	var dLat = (lat2-lat1).toRad();
	var dLon = (lon2-lon1).toRad(); 
	var a = Math.sin(dLat/2) * Math.sin(dLat/2) +
	        Math.cos(lat1.toRad()) * Math.cos(lat2.toRad()) * 
	        Math.sin(dLon/2) * Math.sin(dLon/2); 
	var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a)); 
	var d = R * c;
*/
	double dLat = lat2 - lat1;
	double dLon = lon2 - lon1;
	double sindLat2 = sin(dLat/2);
	double sindLon2 = sin(dLon/2);
	double a = sindLat2 * sindLat2 + cos(lat1)*cos(lat2) * sindLon2*sindLon2;
	double c = 2 * atan2(sqrt(a), sqrt(1.0-a));
/*
	var y = Math.sin(dLon) * Math.cos(lat2);
	var x = Math.cos(lat1)*Math.sin(lat2) -
	        Math.sin(lat1)*Math.cos(lat2)*Math.cos(dLon);
	var brng = Math.atan2(y, x).toBrng();
*/
	double y = sin(dLon) * cos(lat2);
	double x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(dLon);

	*Dist = EarthRadius * c;
	*Bearing = fmod(atan2(y,x) * DegreesPerRadian + 360.0,360.0);
}

static BOOLEAN_F AprsUpdateStationIGate(STRING_F ID, INTEGER_ID_F StationIndex, TIMESTAMP_F When, BOOLEAN_F OnRF)
{	STATION_S Station = {0};
	STRING_F Mapping;
	POINTER_F *Plan = NULL;
	
	if (!ID || !*ID) return FALSE;

	DCOPY(Station.StationID, ID);
	Station.StationIndex = StationIndex;
	if (OnRF)
	{static	POINTER_F RFPlan=NULL;
		Station.RFPacketsIGated = 1;	/* Added in DB */
		Station.LastRFIGate = When;
		Mapping = "RFPacketsIGated+=RFPacketsIGated, LastRFIGate=LastRFIGate";
		Plan = &RFPlan;
	} else
	{static	POINTER_F NonRFPlan=NULL;
		Station.PacketsIGated = 1;	/* Added in DB */
		Station.LastIGate = When;
		Mapping = "PacketsIGated+=PacketsIGated, LastIGate=LastIGate";
		Plan = &NonRFPlan;
	}

	if (!DbPutValues(StationTable, DSTATION_S, "StationIndex=StationIndex", sizeof(Station), &Station,
		DSTATION_S, Mapping, sizeof(Station), &Station,
		DB_UPDATE, Plan))
	{	STRING_F Symbol = DsMakeSubName("NOGATE", ID);
		if (!SySymbolTrue(Symbol))
		{	SySetSymbolValue(Symbol,"1");
			DgPrintf("Failed To Update Station %ld IGate %.*s via %s, Suppressing Messages\n", (long) Station.StationIndex, STRING(Station.StationID), Mapping);
		}
		THREAD_FREE(Symbol);
	}
	return TRUE;
}

static BOOLEAN_F AprsUpdateStationPosition
(	STRING_F OwnerID,
	STRING_F ID,
	APRS_SYMBOL_F Symbol,
	TIMESTAMP_F When,
	COORDINATE_S *Where,
	ALTITUDE_F Altitude,
	DISTANCE_F *pDistance,
	BEARING_F *pBearing,
	COUNT_F *pSeconds,
	SPEED_F *pSpeed,
	BOOLEAN_F Heard
)
{	STATION_S Station = {0}, oStation;
static	POINTER_F GetPlan=NULL, UpdatePlan=NULL, InsertPlan=NULL;

	AprsMakeStationID(OwnerID,&Station.OwnerID);
	AprsMakeStationID(ID,&Station.StationID);
	Station.Symbol = Symbol;
	Station.LastPosition = Station.LastHeard = When;
	Station.Last = *Where;
	Station.Altitude = Altitude;
	Station.PositionIndex = AprsGetPositionIndex(Where);
	Station.Odometer = 0;	/* For now! */
	if (Heard) Station.PacketsHeard = 1;	/* Added in DB */
	else Station.PacketsReceived = 1;	/* Added in DB */
	Station.StationIndex = AprsGetStationIndex(Station.OwnerID, Station.StationID, TRUE);

	if (DbGetValues(StationTable, DSTATION_S, "StationIndex=StationIndex", sizeof(Station), &Station,
			DSTATION_S, NULL, sizeof(oStation), &oStation, &GetPlan))
	{	if (!oStation.LastPosition	/* Don't have a position yet */
		|| (Station.Last.Latitude == oStation.Last.Latitude
		&& Station.Last.Longitude == oStation.Last.Longitude))
		{	Station.LastMotion = oStation.LastMotion;
			Station.LastBearing = oStation.LastBearing;
			Station.LastSpeed = *pSpeed = 0;
			*pDistance = 0;
			*pBearing = oStation.LastBearing;
			*pSeconds = When - oStation.LastPosition;
		} else
		{	double dist, bearing;

			AprsHaversine(&oStation.Last, &Station.Last, &dist, &bearing);

			Station.Odometer = *pDistance = dist;
			Station.LastMotion = When;
			Station.LastBearing = *pBearing = bearing;
			*pSeconds = When - oStation.LastPosition;
			if (*pSeconds)
				Station.LastSpeed = *pSpeed = dist / (*pSeconds / 3600.0);
			else Station.LastSpeed = *pSpeed = 0;

#ifdef VERBOSE
			DgPrintf("Station %.*s Moved from %.5lf %.5lf to %.5lf %.5lf, or %.4lf mi in %ld seconds or %.2lf mph at %.1lf bearing\n",
				STRING(Station.StationID),
				(double) oStation.Last.Latitude, (double) oStation.Last.Longitude, 
				(double) Station.Last.Latitude, (double) Station.Last.Longitude,
				(double) Station.Odometer, (long) *pSeconds, (double) Station.LastSpeed,
				(double) Station.LastBearing);
#endif
			DgDirectPrintf("Movers", "Station %.*s Moved from %.5lf %.5lf to %.5lf %.5lf, or %.4lf mi in %ld seconds or %.2lf mph at %.1lf bearing\n",
				STRING(Station.StationID),
				(double) oStation.Last.Latitude, (double) oStation.Last.Longitude, 
				(double) Station.Last.Latitude, (double) Station.Last.Longitude,
				(double) Station.Odometer, (long) *pSeconds, (double) Station.LastSpeed,
				(double) Station.LastBearing);
		}
		if (Station.Symbol != oStation.Symbol)
			DgPrintf("Station %.*s Symbol Changed From %s (0x%lX) to %s (0x%lX)\n",
				STRING(Station.StationID),
				GetSymbolName(oStation.Symbol),
				(long) oStation.Symbol & 0xffff,
				GetSymbolName(Station.Symbol),
				(long) Station.Symbol & 0xffff);
		if (!DbPutValues(StationTable, DSTATION_S, "StationIndex=StationIndex", sizeof(Station), &Station,
			DSTATION_S, "Symbol=Symbol, LastMotion=LastMotion, LastHeard=LastHeard, LastPosition=LastPosition, LastBearing=LastBearing, LastSpeed=LastSpeed, Last.Latitude=Last.Latitude, Last.Longitude=Last.Longitude, Altitude=Altitude, PositionIndex=PositionIndex, Odometer+=Odometer, PacketsReceived+=PacketsReceived, PacketsHeard+=PacketsHeard", sizeof(Station), &Station,
			DB_UPDATE, &UpdatePlan))
			DgPrintf("Failed To Update Station %.*s\n", STRING(Station.StationID));
	} else
	{	DgPrintf("New Station %.*s\n", STRING(Station.StationID));
		Station.FirstHeard = When;
		if (!DbPutValues(StationTable, DSTATION_S,  "StationIndex=StationIndex", sizeof(Station), &Station,
				DSTATION_S, NULL, sizeof(Station), &Station,
				DB_INSERT, &InsertPlan))
			DgPrintf("Failed To Insert New Station %.*s\n", STRING(Station.StationID));
		*pDistance = 0;
		*pBearing = 0;
		*pSeconds = 0;
		*pSpeed = 0;
	}
	return TRUE;
}

static BOOLEAN_F AprsTouchStation
(	STRING_F OwnerID,
	STRING_F ID,
	TIMESTAMP_F When,
	BOOLEAN_F Heard
)
{	STATION_S Station = {0}, oStation;
static	POINTER_F GetPlan=NULL, UpdatePlan=NULL, InsertPlan=NULL;

	AprsMakeStationID(OwnerID,&Station.OwnerID);
	AprsMakeStationID(ID,&Station.StationID);
	Station.LastHeard = When;
	if (Heard) Station.PacketsHeard = 1;	/* Added in DB */
	else Station.PacketsReceived = 1;	/* Added in DB */
	Station.StationIndex = AprsGetStationIndex(Station.OwnerID, Station.StationID, TRUE);

	if (DbGetValues(StationTable, DSTATION_S, "StationIndex=StationIndex", sizeof(Station), &Station,
			DSTATION_S, NULL, sizeof(oStation), &oStation, &GetPlan))
	{	if (!DbPutValues(StationTable, DSTATION_S, "StationIndex=StationIndex", sizeof(Station), &Station,
			DSTATION_S, "LastHeard=LastHeard, PacketsReceived+=PacketsReceived, PacketsHeard+=PacketsHeard", sizeof(Station), &Station,
			DB_UPDATE, &UpdatePlan))
			DgPrintf("Failed To Update Station %.*s\n", STRING(Station.StationID));
	} else
	{	DgPrintf("New Station %.*s\n", STRING(Station.StationID));
		Station.FirstHeard = When;
		if (!DbPutValues(StationTable, DSTATION_S,  "StationIndex=StationIndex", sizeof(Station), &Station,
				DSTATION_S, NULL, sizeof(Station), &Station,
				DB_INSERT, &InsertPlan))
			DgPrintf("Failed To Insert New Station %.*s\n", STRING(Station.StationID));
	}
	return TRUE;
}

BOOLEAN_F AprsGetStationLocation
(	STATION_ID_F OwnerID,
	STATION_ID_F StationID,
	INTEGER_ID_F *pStationIndex,
	COORDINATE_S *pLoc,
	INTEGER_ID_F *pIndex
)
{	STATION_S Station;
static	POINTER_F GetPlan=NULL;

	AprsMakeStationID(OwnerID,&Station.OwnerID);
	AprsMakeStationID(StationID,&Station.StationID);
	Station.StationIndex = AprsGetStationIndex(Station.OwnerID, Station.StationID, TRUE);
	if (pStationIndex) *pStationIndex = Station.StationIndex;
	if (!DbGetValues(StationTable, DSTATION_S,  "StationIndex=StationIndex", sizeof(Station), &Station,
			DSTATION_S, NULL, sizeof(Station), &Station, &GetPlan))
	{	STRING_F Symbol = DsMakeSubName("UNKNOWN", StationID);
		if (!SySymbolTrue(Symbol))
		{	SySetSymbolValue(Symbol,"1");
			DgPrintf("No Location For Station %.*s (%ld), Suppressing Messages\n", sizeof(STATION_ID_F), StationID, (long) Station.StationIndex);
		}
		if (pLoc) memset(pLoc,0,sizeof(*pLoc));
		if (pIndex) *pIndex = 0;
		THREAD_FREE(Symbol);
		return FALSE;
	}
	if (pLoc) *pLoc = Station.Last;
	if (pIndex) *pIndex = Station.PositionIndex;
	return TRUE;
}


BOOLEAN_F FUNCTION AprsRelayToAnyTrak(STRING_F obj, double lat, double lon)
{	BOOLEAN_F Result = TRUE;
	STRING_F NewValue = SyGetSymbolValueOrDefault("ANYTRAC","");
static	STRING_F LastValue = NULL;

	if (!LastValue || strcmp(NewValue,LastValue))
	{	if (LastValue)
		{	COUNT_F Count, i;
			STRING_F *Targets = RtSplitCommas(LastValue,&Count);
			for (i=0; i<Count; i++)
			{	STRING_F Target, UserPass;
				if (RtIsRoutineCall(Targets[i],&Target,&UserPass))
				{	DgPrintf("Clearing Old anyTrac target %s\n", Target);
					SySetSymbolValue(Target, "");	/* Clear the symbol value */
					THREAD_FREE(Target); THREAD_FREE(UserPass);
				} else DgPrintf("%s isn't CALLSIGN(user:pass)\n", Targets[i]);
			}
			RtFreeCommas(Targets);
			FREE(LastValue);
		}
		LastValue = STRDUP(NewValue);
		{	COUNT_F Count, i;
			STRING_F *Targets = RtSplitCommas(LastValue,&Count);
			DgPrintf("Defining %ld anyTrac Targets for %s\n", (long) Count, LastValue);
			for (i=0; i<Count; i++)
			{	STRING_F Target, UserPass;
				if (RtIsRoutineCall(Targets[i],&Target,&UserPass))
				{	if (strchr(UserPass,':'))
					{	DgPrintf("Setting anyTrac target %s to %s\n", Target, UserPass);
						SySetSymbolValue(Target, UserPass);
					} else DgPrintf("%s isn't CALLSIGN(user:pass)\n", Targets[i]);
					THREAD_FREE(Target); THREAD_FREE(UserPass);
				} else DgPrintf("%s isn't CALLSIGN(user:pass)\n", Targets[i]);
			}
			RtFreeCommas(Targets);
		}
	}
	NewValue = SyGetSymbolValueOrDefault(obj,"");
	if (*NewValue && (lat || lon))
	{	COUNT_F Count;
		STRING_F *Parts = RtSplitStrings(NewValue,":", &Count);
		if (Count == 2)
		{	MESSAGE_S *Msg;
			STRING_F URL = THREAD_MALLOC(1024);
			sprintf(URL,"/api/update.ashx?lat=%.5lf&lon=%.5lf&u=%s&p=%s", (double) lat, (double) lon, Parts[0], Parts[1]);
			DgPrintf("Relaying %s at %.5lf %.5lf to anyTrac's %s\n", obj, (double) lat, (double) lon, Parts[0]);
			DgDirectPrintf("anyTrac", "Relaying %s at %.5lf %.5lf to anyTrac's %s\n", obj, (double) lat, (double) lon, Parts[0]);
			Msg = CiGetFromTimeout("anytrac.bluearray.net",URL,15000,NULL,NULL,HERE);
			if (!Msg->Success)
			{	DgPrintf("AnyTrac(%s) for %s Failed With %s\n", Parts[0], obj, CiGetResponseStatusText(Msg));
				DgPrintf("anyTrac", "AnyTrac(%s) for %s Failed With %s\n", Parts[0], obj, CiGetResponseStatusText(Msg));
				Result = FALSE;
			}
			THREAD_FREE(URL);
			CiDestroyMessage(Msg);
		} else DgPrintf("%s's %s isn't user:pass)\n", obj, NewValue);
		RtFreeSplitStrings(Parts);
	}
	return Result;
}

static void AprsInsertHops(INTEGER_ID_F PacketIndex, TIMESTAMP_F When, int hopCount, char **Hops)
{	INDEX_F i;
	HOP_S Hop = {0};
#ifdef CHECK_FOR_BADPATHS
	BOOLEAN_F PassedUsed = FALSE, qAiPacket = FALSE;
#endif
static	POINTER_F PutPlan = NULL;

#ifdef DEBUG
	{	int h;
		for (h=0; h<hopCount; h++)
			DgPrintf("Hop[%ld] = %s\n", (long) h, Hops[h]);
	}
#endif
	Hop.PacketIndex = PacketIndex;
	for (i=hopCount-1; i>=0; i--)
	if (i != 1)	/* Don't store destination */
	{	Hop.Sequence++;	/* IGate becomes sequence 1 */
		Hop.Used = strchr(Hops[i],'*') != NULL;
		AprsGetStationLocation(Hops[i], Hops[i], &Hop.StationIndex, NULL, &Hop.PositionIndex);
		if (!DbPutValues(HopTable, DHOP_S, NULL, sizeof(Hop), &Hop,
				DHOP_S, NULL, sizeof(Hop), &Hop, DB_INSERT, &PutPlan))
			DgPrintf("Failed To Insert Packet %ld Hop Sequence %ld\n", (long) Hop.PacketIndex, (long) Hop.Sequence);
	}

#ifdef CHECK_FOR_BADPATHS
	for (i=1; i<hopCount-1; i++)	/* Don't do IGate at the end */
	if (i != 1)	/* Don't check destination */
	{	if (!stricmp(Hops[i],"qAI"))	/* Trace packets have lots of stuff after the IGate */
			qAiPacket = TRUE;
		else if (PassedUsed && !qAiPacket
		&& *Hops[i] != 'q'
		&& strncmp(Hops[i],"WIDE",4)
		&& strncmp(Hops[i],"TRACE",5))
		{	DgPrintf("%s:Legal Station %s after *Used* In Packet %ld Index %ld",
				Hops[0], Hops[i], (long) PacketIndex, (long) i);
			LgSprintfEvent("PACKETS", ".LOG", -1,"BADPATH", When, FALSE, "%s:Legal Station %s after *Used* In Packet %ld Index %ld",
				Hops[0], Hops[i], (long) PacketIndex, (long) i);
		}
		if (strchr(Hops[i],'*') != NULL) PassedUsed = TRUE;
	}
#endif
}

MUTEX_SEMAPHORE_S *ParseLock = NULL;

static BOOLEAN_F FUNCTION AprsLogPacket(TIMESTAMP_F When, STRING_F Packet)
{	COUNT_F Len = strlen(Packet);
	char *obj, *src, *dst, *relay, *igate, *q, datatype;
	double lat, lon, alt;
	int hopCount = 0;
	char **Hops = NULL;
	int symbol;
	STRING_F Safe = THREAD_STRDUP(Packet);
static POINTER_F InsertPlan=NULL;

	if (!Len) return FALSE;
	if (!When) When = RtNow(NULL);

	LgLogEvent("PACKETS", ".LOG", -1,"GOOD", When, strlen(Packet), Packet, FALSE);

	SmLockMutex(&ParseLock,HERE);
	if ((obj = parse_aprs(Packet, &src, &dst, &hopCount, &Hops, &lat, &lon, &alt, &symbol, &datatype)) != NULL)
	{	PACKET_S Packet = {0};
		STATION_ID_F srcID, objID, gateID;

		parse_route(hopCount, Hops, &relay, &igate, &q);
		if (lat != 0.0 || lon != 0.0)
		{	COORDINATE_S Where;
			Where.Latitude = lat;
			Where.Longitude = lon;
			Packet.PositionIndex = AprsGetPositionIndex(&Where);
			Packet.Altitude = alt;
			if (!AprsUpdateStationPosition(src, obj, symbol, When, &Where, alt, &Packet.Distance, &Packet.Bearing, &Packet.Seconds, &Packet.Speed, FALSE))
				DgPrintf("Failed To Update Station %s at %.5lf %.5lf %.5lf\n", obj, (double) lat, (double) lon, (double) alt);
		}
#ifdef TEST_ONLY
		else if (datatype == 0x02)	/* RFID? */
		{	APRS_PARSED_INFO_S Info = {0};
			STRING_F LocalPacket = THREAD_STRDUP(Safe);
			if (parse_full_aprs(LocalPacket, &Info))
			{	LgLogEvent("PACKETS", ".LOG", -1, "RFID", When, strlen(Safe), Safe, TRUE);
				DgDirectPrintf("RFID", "%.*s (%.*s) Read (%.*s)\n",
						STRING(Info.srcCall), STRING(Info.dstCall), STRING(Info.Comment));
				{static	STRING_F Monitors[] = { "KJ4ERJ-12", "KJ4ERJ-AP", "WB4APR-3" };
					RtStrnuprTrim(STRING(Info.Comment));
					{	INDEX_F m;
						char Buffer[256];
						STRING_F symRFID = DsMakeSubName("RFID",Info.Comment);
						STRING_F Default = "*UNKNOWN*";
						STRING_F Owner = SyGetSymbolValueOrDefault(symRFID,Default);
						for (m=0; m<ACOUNT(Monitors); m++)
						{	sprintf(Buffer,"KJ4ERJ>APZAPM::%-9s:%.*s (%.*s) Read (%.*s) Owner %s (Sent to %s)",
									Monitors[m],
									STRING(Info.srcCall), STRING(Info.dstCall),
									STRING(Info.Comment), Owner,
									Monitors[m]);
							AprsQueueXmitPacket(Buffer);
						}
						THREAD_FREE(symRFID);
						if (Owner != Default)
						{	COORDINATE_S Pos;
							TIMESTAMP_F Now = RtNow(NULL);
							struct tm *tm = gmtime(&Now);
							if (AprsLookupStationLocation(Info.srcCall, Info.srcCall, NULL, &Pos, NULL))
							{	char *LatLon = APRSLatLon(Pos.Latitude, Pos.Longitude, 'R', 'A');
								sprintf(Buffer,"%s>APRFID,WIDE2-2:/%02ld%02ld%02ldh%s%.*s via %.*s",
									Owner, (long) tm->tm_hour, (long) tm->tm_min, (long) tm->tm_sec,
									LatLon, STRING(Info.Comment), STRING(Info.srcCall));
								AprsQueueXmitPacket(Buffer);
								THREAD_FREE(LatLon);
							}
						}
					}
				}
			} else
			{	LgLogEvent("PACKETS", ".LOG", -1, "BAD", When, strlen(Safe), Safe, TRUE);
				DgDirectPrintf("BadPackets","%s",Safe);
			}
			THREAD_FREE(LocalPacket);
		}
#endif
		else if (datatype == ':')	/* Message? */
		{	APRS_PARSED_INFO_S Info = {0};
			STRING_F LocalPacket = THREAD_STRDUP(Safe);
			if (parse_full_aprs(LocalPacket, &Info) && (Info.Valid & APRS_MESSAGE_VALID))
			{	LgLogEvent("PACKETS", ".LOG", -1, "MESSAGE", When, strlen(Safe), Safe, TRUE);
				DgDirectPrintf("Message", "%.*s (%.*s) Read (%.*s)\n",
						STRING(Info.srcCall), STRING(Info.dstCall), STRING(Info.Comment));
#ifdef TESTING_ONLY
				if (!strncmp(Info.msgCall, "RFID", sizeof(Info.msgCall)))
				{	char *ack = strrchr(Info.Comment, '{');	/* Do we need to ack it? */
					if (ack && strlen(ack) <= 6)
					{	char Buffer[80];
						sprintf(Buffer,"RFID>APZAPM::%-9.*s:ack%s", STRING(Info.srcCall), ack+1);
						AprsQueueXmitPacket(Buffer);
						*ack = '\0';	/* Null terminate message early */
					}
					RtStrnuprTrim(STRING(Info.Comment));
					if (Info.Comment[0] == '?')
					{	STRING_F symRFID = DsMakeSubName("RFID",&Info.Comment[1]);
						STRING_F symCALL = DsMakeSubName("RFIDCALL",&Info.Comment[1]);
						COORDINATE_S Pos;
						char Buffer[256];
						if (SyGetSymbolValue(symRFID))
						{	sprintf(Buffer,"RFID>APZAPM::%-9.*s:%.*s Is %s",
								STRING(Info.srcCall), sizeof(Info.Comment)-1,&Info.Comment[1], SyGetSymbolValue(symRFID));
							AprsQueueXmitPacket(Buffer);
						} else if (SyGetSymbolValue(symCALL))
						{	sprintf(Buffer,"RFID>APZAPM::%-9.*s:%.*s Is %s",
								STRING(Info.srcCall), sizeof(Info.Comment)-1,&Info.Comment[1], SyGetSymbolValue(symCALL));
							AprsQueueXmitPacket(Buffer);
						} else if (AprsLookupStationLocation(&Info.Comment[1], &Info.Comment[1], NULL, &Pos, NULL))
						{	char *LatLon = APRSLatLon(Pos.Latitude, Pos.Longitude, ' ', ' ');
							sprintf(Buffer,"RFID>APZAPM::%-9.*s:%.*s At %s",
								STRING(Info.srcCall), sizeof(Info.Comment)-1,&Info.Comment[1],
								LatLon);
							AprsQueueXmitPacket(Buffer);
							THREAD_FREE(LatLon);
						} else
						{	sprintf(Buffer,"RFID>APZAPM::%-9.*s:%.*s Is *UNKNOWN*",
								STRING(Info.srcCall), sizeof(Info.Comment)-1,&Info.Comment[1]);
							AprsQueueXmitPacket(Buffer);
						}

						THREAD_FREE(symCALL); THREAD_FREE(symRFID);
					} else
					{	STRING_F symRFID = DsMakeSubName("RFID",Info.Comment);
						STRING_F symCALL = DsMakeSubName("RFIDCALL",Info.srcCall);
						char Buffer[256];
						SySetSymbolValue(symRFID,Info.srcCall);
						SySetSymbolValue(symCALL,Info.Comment);
						sprintf(Buffer,"RFID>APZAPM::%-9.*s:%.*s Is %.*s",
								STRING(Info.srcCall), STRING(Info.Comment), STRING(Info.srcCall));
						AprsQueueXmitPacket(Buffer);
						THREAD_FREE(symCALL); THREAD_FREE(symRFID);
					}
				}
#endif
			} else
			{	LgLogEvent("PACKETS", ".LOG", -1, "BAD", When, strlen(Safe), Safe, TRUE);
				DgDirectPrintf("BadPackets","%s",Safe);
			}
			THREAD_FREE(LocalPacket);
		} else
		{	LgLogEvent("PACKETS", ".LOG", -1, "NOPOS", When, strlen(Safe), Safe, TRUE);
			DgDirectPrintf("PositionLess","%s",Safe);
			if (!AprsTouchStation(src, obj, When, FALSE))
				DgPrintf("Failed To Touch Station %s\n", obj);
		}

		if (lat != 0.0 || lon != 0.0)
		{
			AprsMakeStationID(src,&srcID);
			AprsMakeStationID(obj,&objID);
			AprsMakeStationID(igate,&gateID);
			Packet.OriginIndex = AprsGetStationIndex(srcID,objID,TRUE);
			if (datatype != '`' && datatype != 0x27)	/* MIC-E packets don't have dests */
			{	STATION_ID_F dstID;
				AprsMakeStationID(dst,&dstID);
				Packet.DestIndex = AprsGetStationIndex(srcID, dstID, TRUE);
			} else Packet.DestIndex = 0;
			Packet.HeardIndex = AprsGetStationIndex(gateID,gateID,TRUE);
			Packet.HeardOnRF = FALSE;

			Packet.PacketIndex = AprsGetNextPacketIndex();
			Packet.DataType = datatype;
			Packet.When = When;
			Packet.LogOffset = 0;	/* For now, should get from LgLogEvent eventually */

			if (Packet.OriginIndex != Packet.HeardIndex)	/* Not my own packet! */
				AprsUpdateStationIGate(gateID, Packet.HeardIndex, Packet.When, Packet.HeardOnRF);

			if (!DbPutValues(PacketTable, DPACKET_S, "PacketIndex=PacketIndex", sizeof(Packet), &Packet,
					DPACKET_S, NULL, sizeof(Packet), &Packet,
					DB_INSERT, &InsertPlan))
				DgPrintf("Failed To Insert Packet From %.*s at %.24s\n", STRING(srcID), ctime(&Packet.When));
			else AprsInsertHops(Packet.PacketIndex, Packet.When, hopCount, Hops);
			if (lat || lon) AprsRelayToAnyTrak(obj, lat, lon);
		}
	} else
	{	LgLogEvent("PACKETS", ".LOG", -1, "BAD", When, strlen(Safe), Safe, TRUE);
		DgDirectPrintf("BadPackets","%s",Safe);
	}
	SmUnlockMutex(&ParseLock,HERE);
	if (Hops) free(Hops);
	THREAD_FREE(Safe);
	return TRUE;
}

static BOOLEAN_F FUNCTION AprsLogReceivedPacket(TIMESTAMP_F When, STRING_F ReceivedBy, STRING_F Packet)
{	COUNT_F Len = strlen(Packet);
	APRS_PARSED_INFO_S APRS;
	char *obj, *src, *dst, datatype;
	double lat, lon, alt;
	int symbol;
	STRING_F Safe = THREAD_STRDUP(Packet);
static POINTER_F InsertPlan=NULL;

	if (!Len) return FALSE;
	if (!When) When = RtNow(NULL);

	DgDirectPrintf("RF", "%s", Packet);
	LgLogEvent("PACKETS", ".LOG", -1,ReceivedBy, When, Len, Packet, FALSE);

	SmLockMutex(&ParseLock,HERE);

	if (parse_full_aprs(Packet, &APRS))
	{	PACKET_S Packet = {0};
		STATION_ID_F srcID, objID, gateID;

		obj = (APRS.Valid & (APRS_ITEM_VALID|APRS_OBJECT_VALID))?APRS.objCall:APRS.srcCall;
		if (APRS.Valid)
		{	src = APRS.srcCall;
			dst = APRS.dstCall;
		} else src = dst = "";
		if (APRS.Valid & APRS_LATLON_VALID)
		{	lat = APRS.lat;
			lon = APRS.lon;
		} else lat = lon = 0.0;
		if (APRS.Valid & APRS_ALTITUDE_VALID)
		{	alt = APRS.alt;
		} else alt = 0.0;
		if (APRS.Valid & (APRS_SYMBOL_VALID | APRS_SYMBOL_DEFAULTED))
		{	symbol = APRS.symbol;
		} else symbol = 0;
		if (APRS.Valid & APRS_DATATYPE_VALID)
		{	datatype = APRS.datatype;
		} else datatype = 0;

		if (lat != 0.0 || lon != 0.0)
		{	COORDINATE_S Where;
			Where.Latitude = lat;
			Where.Longitude = lon;
			Packet.PositionIndex = AprsGetPositionIndex(&Where);
			Packet.Altitude = alt;
			if (!AprsUpdateStationPosition(src, obj, symbol, When, &Where, alt, &Packet.Distance, &Packet.Bearing, &Packet.Seconds, &Packet.Speed, TRUE))
				DgPrintf("Failed To Update Station %s at %.5lf %.5lf %.5lf\n", obj, (double) lat, (double) lon, (double) alt);
		} else
		{	LgLogEvent("PACKETS", ".LOG", -1, "NOPOS", When, strlen(Safe), Safe, TRUE);
			DgDirectPrintf("PositionLess","%s",Safe);
			if (!AprsTouchStation(src, obj, When, TRUE))
				DgPrintf("Failed To Touch Station %s\n", obj);
		}

		AprsMakeStationID(src,&srcID);
		AprsMakeStationID(obj,&objID);
		AprsMakeStationID(ReceivedBy,&gateID);
		Packet.OriginIndex = AprsGetStationIndex(srcID,objID,TRUE);
		if (datatype != '`' && datatype != 0x27)	/* MIC-E packets don't have dests */
		{	STATION_ID_F dstID;
			AprsMakeStationID(dst,&dstID);
			Packet.DestIndex = AprsGetStationIndex(srcID, dstID, TRUE);
		} else Packet.DestIndex = 0;
		Packet.HeardIndex = AprsGetStationIndex(ReceivedBy,ReceivedBy,TRUE);
		Packet.HeardOnRF = *APRS.thirdCall?FALSE:TRUE;

		if (*APRS.thirdCall)
		{	char SubDir[80];
			sprintf(SubDir,"3rdParty-%s",ReceivedBy);
			LgLogEvent("PACKETS", ".LOG", -1, SubDir, When, strlen(Safe), Safe, TRUE);
		}

		Packet.PacketIndex = AprsGetNextPacketIndex();
		Packet.DataType = datatype;
		Packet.When = When;
		Packet.LogOffset = 0;	/* For now, should get from LgLogEvent eventually */

		if (Packet.OriginIndex != Packet.HeardIndex)
			AprsUpdateStationIGate(ReceivedBy, Packet.HeardIndex, Packet.When, Packet.HeardOnRF);

		if (!DbPutValues(PacketTable, DPACKET_S, "PacketIndex=PacketIndex", sizeof(Packet), &Packet,
				DPACKET_S, NULL, sizeof(Packet), &Packet,
				DB_INSERT, &InsertPlan))
			DgPrintf("Failed To Insert Packet From %.*s at %.24s\n", STRING(srcID), ctime(&Packet.When));
		else
		{	INDEX_F i;
			COUNT_F newCount = APRS.Path.hopCount+1;
			char **newHops = THREAD_MALLOC(sizeof(*newHops)*newCount);
			for (i=0; i<APRS.Path.hopCount; i++)
				newHops[i] = APRS.Path.Hops[i];
			newHops[APRS.Path.hopCount] = ReceivedBy;
			AprsInsertHops(Packet.PacketIndex, Packet.When, newCount, newHops);
			THREAD_FREE(newHops);
		}
		if (lat || lon) AprsRelayToAnyTrak(obj, lat, lon);
	} else
	{	LgLogEvent("PACKETS", ".LOG", -1, "NoParse", When, strlen(Safe), Safe, TRUE);
		DgDirectPrintf("BadPackets","%s",Safe);
	}
	SmUnlockMutex(&ParseLock,HERE);
	/* if (Hops) free(Hops); */
	THREAD_FREE(Safe);
	return TRUE;
}

static BOOLEAN_F FUNCTION AprsLogDigipeatedPacket(TIMESTAMP_F When, STRING_F ReceivedBy, STRING_F Packet)
{	COUNT_F Len = strlen(Packet);

	if (!Len) return FALSE;
	if (!When) When = RtNow(NULL);
	
	{	STRING_F Digi = DsMakeSubName(ReceivedBy,"Digi");
		LgLogEvent("PACKETS", ".LOG", -1,Digi, When, Len, Packet, FALSE);
		THREAD_FREE(Digi);
	}
	
	return TRUE;
}

COUNT_F AprsGetLogLength(STRING_F Packet)
{	COUNT_F Len = strlen(Packet);

	if (Len > 60)
	{	STRING_F Colon = strchr(Packet,':');
		if (Colon)
		{	if (Colon[1] == '{')	/* 3rd party? */
			{	STRING_F Comma = strchr(Colon,',');
				if (!Comma) Comma = strchr(Colon+1,':');
				if (Comma)
				{	Len = Comma - Packet;	/* Drop comma or colon */
				} else Len = Colon - Packet + 2 + 9;	/* Datatype + 3rd call */
			} else Len = Colon - Packet + 2;	/* Through datatype */
		} else Len = 60;
		if (Len < 60) Len = 60;
	}
	return Len;
}


BOOLEAN_F FUNCTION AprsSvcLogPacket(MESSAGE_S *Req, STRING_F URL)
{	APRS_LOG_PACKET_SRQ *Svc = (POINTER_F) Req->Body;
	TIMESTAMP_F Now = RtNow(NULL);

	if (!Svc->When) Svc->When = RtNow(NULL);

	if (RtStrnWhite(-1, Svc->Packet))
		CiSendBadResponse2(Req, 204, "APRS::PACKETREQ", "Packet Required", 0);
	else
	{	//STRING_F Temp = THREAD_STRDUP(Svc->Packet);
static TIMESTAMP_F Next = 0;
		if (Now >= Next)
		if ((Svc->When-Now) > 1 || (Now-Svc->When) > 60)
		{	COUNT_F Len = strlen(Svc->Packet);
			COUNT_F LogLen = AprsGetLogLength(Svc->Packet);
/* Wed Mar 07 04:11:39 2012 */
			DgPrintf("%ld Old (%.15s) %ld bytes %.*s%s\n",
				(long) (Now-Svc->When), ctime(&Svc->When)+4,
				(long) Len, LogLen, Svc->Packet, LogLen<Len?"...":"");
			Next = Now + 2;
		}
		if (AprsLogPacket(Svc->When, Svc->Packet))
			CiSendGoodResponse(Req, NULL, 0);
		else	CiSendBadResponse2(Req, 204, "APRS::FAILOGPKT", "LogPacket Failed", 0);
		//THREAD_FREE(Temp);
	}
	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcLogReceivedPacket(MESSAGE_S *Req, STRING_F URL)
{	APRS_LOG_RECEIVED_PACKET_SRQ *Svc = (POINTER_F) Req->Body;
	TIMESTAMP_F Now = RtNow(NULL);

	if (!Svc->When) Svc->When = RtNow(NULL);

	if (RtStrnWhite(-1, Svc->Packet))
		CiSendBadResponse2(Req, 204, "APRS::PACKETREQ", "Packet Required", 0);
	else if (RtStrnWhite(-1, Svc->ReceivedBy))
		CiSendBadResponse2(Req, 204, "APRS::RCVDBYREQ", "ReceivedBy Required", 0);
	else
	{
static TIMESTAMP_F Next = 0;
		if (Now >= Next)
		if ((Svc->When-Now) > 1 || (Now-Svc->When) > 60)
		{	COUNT_F Len = strlen(Svc->Packet);
			COUNT_F LogLen = AprsGetLogLength(Svc->Packet);
			DgPrintf("%ld Old (%.15s) %s %ld bytes %.*s%s\n",
				(long) (Now-Svc->When), ctime(&Svc->When)+4,
				Svc->ReceivedBy,
				(long) Len, LogLen, Svc->Packet, LogLen<Len?"...":"");
			Next = Now + 2;
		}
		if (AprsLogReceivedPacket(Svc->When, Svc->ReceivedBy, Svc->Packet))
			CiSendGoodResponse(Req, NULL, 0);
		else	CiSendBadResponse2(Req, 204, "APRS::FAILOGPKT", "LogPacket Failed", 0);
	}
	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcLogDigipeatedPacket(MESSAGE_S *Req, STRING_F URL)
{	APRS_LOG_DIGIPEATED_PACKET_SRQ *Svc = (POINTER_F) Req->Body;
	TIMESTAMP_F Now = RtNow(NULL);

	if (!Svc->When) Svc->When = RtNow(NULL);

	if (RtStrnWhite(-1, Svc->Packet))
		CiSendBadResponse2(Req, 204, "APRS::PACKETREQ", "Packet Required", 0);
	else if (RtStrnWhite(-1, Svc->ReceivedBy))
		CiSendBadResponse2(Req, 204, "APRS::RCVDBYREQ", "ReceivedBy Required", 0);
	else
	{
static TIMESTAMP_F Next = 0;
		if (Now >= Next)
		if ((Svc->When-Now) > 1 || (Now-Svc->When) > 60)
		{	COUNT_F Len = strlen(Svc->Packet);
			COUNT_F LogLen = AprsGetLogLength(Svc->Packet);
			DgPrintf("%ld Old (%.15s) %s %ld bytes %.*s%s\n",
				(long) (Now-Svc->When), ctime(&Svc->When)+4,
				Svc->ReceivedBy,
				(long) Len, LogLen, Svc->Packet, LogLen<Len?"...":"");
			Next = Now + 2;
		}
		if (AprsLogDigipeatedPacket(Svc->When, Svc->ReceivedBy, Svc->Packet))
			CiSendGoodResponse(Req, NULL, 0);
		else	CiSendBadResponse2(Req, 204, "APRS::FAILOGPKT", "LogPacket Failed", 0);
	}
	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcReParseLogFile(MESSAGE_S *Req, STRING_F URL)
{	APRS_REPARSE_LOG_FILE_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_REPARSE_LOG_FILE_SRP Rsp = {0};
	TIMESTAMP_F Now = RtNow(NULL);

	if (Svc->From.Latitude >= Svc->To.Latitude
	|| Svc->From.Longitude >= Svc->To.Longitude)
		CiSendBadResponse2(Req, 204, "APRS::FRMLESSTO", "From Must Be Less Than To", 0);
	else if (RtStrnWhite(-1, Svc->Path))
		CiSendBadResponse2(Req, 204, "APRS::PATH_REQ", "Path Required", 0);
	else
	{
		FILE *In = fopen(Svc->Path,"rb");
		if (In)
		{
			char InBuf[4096];
			
			long fgetsCount = 0, fgetcCount=0;
			
			while (fgets(InBuf,sizeof(InBuf), In))
				fgetsCount += strlen(InBuf);
			fseek(In, 0, SEEK_SET);

			while (fgetc(In) != EOF)
				fgetcCount++;
			fseek(In, 0, SEEK_SET);
			
			
			while (fgets(InBuf,sizeof(InBuf), In))
			{
				STRING_F Packet = &InBuf[20];
				TIMESTAMP_F When;
				
				if (DsFromString(18, InBuf, DATATYPE_TIMESTAMP, sizeof(When), &When, NULL))
				{
					char *obj, *src, *dst, *relay, *igate, *q, datatype;
					double lat, lon, alt;
					int hopCount = 0;
					char **Hops = NULL;
					int symbol;
					STRING_F Safe = THREAD_STRDUP(Packet);

					SmLockMutex(&ParseLock,HERE);
					if ((obj = parse_aprs(Packet, &src, &dst, &hopCount, &Hops, &lat, &lon, &alt, &symbol, &datatype)) != NULL)
					{
						if (lat!=0 || lon!=0)
						{	if (lat >= Svc->From.Latitude && lat <= Svc->To.Latitude
							&& lon >= Svc->From.Longitude && lon <= Svc->To.Longitude)
							{	if (AprsLogPacket(When, Safe))
									Rsp.Processed++;
								else DgPrintf("LogPacket(%s) Failed\n", Safe);
							} else Rsp.OutOfBounds++;
						} else Rsp.NullIsland++;
					} else
					{	Rsp.ParseFail++;
						DgPrintf("Parse(%s) Failed\n", Safe);
					}
					SmUnlockMutex(&ParseLock,HERE);
					THREAD_FREE(Safe);
				} else
				{	Rsp.ParseFail++;
					DgPrintf("Conversion of Timestamp(%.18s) Failed\n", InBuf);
				}
			}
			if (!feof(In))
				DgPrintf("NOT AT EOF!\n");
			DgPrintf("fgets:%ld fgetc:%ld\n", fgetsCount, fgetcCount);
			fclose(In);
			CiSendGoodResponse(Req, &Rsp, sizeof(Rsp));
		} else CiSendBadResponse2(Req, 204, "APRS::PTHNOTFND", "Path(%1) Not Found", 1, STRING(Svc->Path));
	}
	return TRUE;
}

typedef struct HOP_FILTER_INFO_S
{	INTEGER_ID_F	StationIndex;
	COUNT_F		BucketSize;
	BOOLEAN_F	FoundUsed;
	BOOLEAN_F	FoundInPacket;
	BOOLEAN_F	IncludeInvalidStations;
	ALTITUDE_F	MinAltitude, MaxAltitude;	/* Min/Max Altitude (0,0 = no check) */
	POINTER_F UserArg;	/* For application use */
} HOP_FILTER_INFO_S;

static BOOLEAN_F AprsFilterPacketHops(POINTER_F DbRecord, POINTER_F UserArg)
{	HOP_S *p = DbRecord;
	HOP_FILTER_INFO_S *HopFilter = UserArg;

	if (!HopFilter->StationIndex
	|| p->StationIndex == HopFilter->StationIndex)
		HopFilter->FoundInPacket = TRUE;
	if (p->Used) HopFilter->FoundUsed = TRUE;
	return TRUE;
}

static BOOLEAN_F AprsCheckHopFilter(PACKET_S *pPacket, HOP_FILTER_INFO_S *HopFilter)
{static	POINTER_F CountPlan=NULL;

	if (!HopFilter->StationIndex) return TRUE;

	HopFilter->FoundInPacket = FALSE;
	DbQueryCount(HopTable, DPACKET_S, "PacketIndex=PacketIndex", sizeof(*pPacket), pPacket, AprsFilterPacketHops, HopFilter, &CountPlan);
	return HopFilter->FoundInPacket;
}

#ifdef OLD_WAY

static BOOLEAN_F AprsFilterUniqueHopPackets(POINTER_F DbRecord, POINTER_F UserArg)
{	HOP_S *pHop = DbRecord;
	HASH_S *Hash = UserArg;
	return DsInsertIfNewHashKey(Hash, &pHop->PacketIndex, (POINTER_F) pHop->PacketIndex, HERE);
}

static HASH_S *AprsCreateHopHash(INTEGER_ID_F RelayIndex, CALLER)
{	HASH_S *Hash;
	COUNT_F Count;
	HOP_S Hop;
static	POINTER_F CountPlan = NULL;

	if (!RelayIndex) return NULL;

	Hash = DsCreateLocalHash("HopPackets", sizeof(INTEGER_ID_F), NULL, MY_CALLER);

	Hop.StationIndex = RelayIndex;
	Count = DbQueryCount(HopTable, DHOP_S, "StationIndex=StationIndex", sizeof(Hop), &Hop,
				AprsFilterUniqueHopPackets, Hash, &CountPlan);
	DgPrintf("Filtering over %ld Packets (%ld In Hash)\n", (long) Count, (long) DsGetHashCount(Hash));
	return Hash;
}

static BOOLEAN_F AprsCheckHopHash(HASH_S *Hash, INTEGER_ID_F PacketIndex)
{	return !Hash || ((INTEGER_ID_F)DsLookupHashKey(Hash, &PacketIndex)) != 0;
}

static VFUNCTION AprsDestroyHopHash(HASH_S *Hash, CALLER)
{	if (Hash)
	{	DsEmptyHash(Hash, NULL, NULL, MY_CALLER);
		DsDestroyHash(Hash, MY_CALLER);
	}
}
#endif


BOOLEAN_F FUNCTION AprsSvcPurgePackets(MESSAGE_S *Req, STRING_F URL)
{	APRS_PURGE_PACKETS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_PURGE_PACKETS_SRP Rsp;
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F AccessPlan = NULL;

	RtStrnuprTrim(STRING(Svc->OriginID));
	RtStrnuprTrim(STRING(Svc->DestID));
	RtStrnuprTrim(STRING(Svc->RelayID));
	RtStrnuprTrim(STRING(Svc->IGateID));

	Condition = DbAddStringCondition(Condition,STRING(Svc->OriginID),"OriginID=OriginID");
	Condition = DbAddStringCondition(Condition,STRING(Svc->DestID),"DestID=DestID");
	Condition = DbAddStringCondition(Condition,STRING(Svc->RelayID),"RelayID=RelayID");
	Condition = DbAddStringCondition(Condition,STRING(Svc->IGateID),"IGateID=IGateID");
	if (Svc->DataType) Condition = DbAddCondition(Condition,"DataType=DataType");

	if (Svc->FasterThan) Condition = DbAddCondition(Condition,"Speed>=FasterThan");
	if (Svc->SlowerThan) Condition = DbAddCondition(Condition,"Speed<=SlowerThan");
	if (Svc->StartTime) Condition = DbAddCondition(Condition,"When>=StartTime");
	if (Svc->EndTime) Condition = DbAddCondition(Condition,"When<EndTime");

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	Rsp.Count = DbDeleteRecords(PacketTable, DAPRS_PURGE_PACKETS_SRQ, Condition,
						sizeof(*Svc), Svc, NULL, NULL, &AccessPlan);
		CiSendGoodResponse(Req, &Rsp, sizeof(Rsp));
	}
	THREAD_FREE(Condition);

	return TRUE;
}

static BOOLEAN_F AprsFilterQueryPackets(POINTER_F DbRecord, POINTER_F UserArg)
{	return AprsCheckHopFilter(DbRecord, UserArg);
}

BOOLEAN_F FUNCTION AprsSvcQueryPackets(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_PACKETS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_PACKETS_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F AccessPlan = NULL;

	if (Svc->PacketIndex)	/* Specific packet ignores all else */
		Condition = DbAddLongCondition(Condition,Svc->PacketIndex,"PacketIndex=PacketIndex");
	else
	{	RtStrnuprTrim(STRING(Svc->OriginID));
		RtStrnuprTrim(STRING(Svc->DestID));
		RtStrnuprTrim(STRING(Svc->RelayID));
		RtStrnuprTrim(STRING(Svc->IGateID));

		Condition = DbAddCondition(Condition, "HeardOnRF=HeardOnRF");

		if (!Svc->OriginIndex && !RtStrnWhite(STRING(Svc->OriginID)))
			Svc->OriginIndex = AprsGetStationIndex(Svc->OriginID, Svc->OriginID, FALSE);
		if (!Svc->DestIndex && !RtStrnWhite(STRING(Svc->DestID)))
		{	if (!RtStrnWhite(STRING(Svc->OriginID)))
				AprsGetStationID(Svc->OriginIndex, NULL, &Svc->OriginID);
			Svc->DestIndex = AprsGetStationIndex(Svc->OriginID, Svc->DestID, FALSE);
		}
		if (!Svc->RelayIndex && !RtStrnWhite(STRING(Svc->RelayID)))
			Svc->RelayIndex = AprsGetStationIndex(Svc->RelayID, Svc->RelayID, FALSE);
		if (!Svc->HeardIndex && !RtStrnWhite(STRING(Svc->IGateID)))
			Svc->HeardIndex = AprsGetStationIndex(Svc->IGateID, Svc->IGateID, FALSE);

		Condition = DbAddLongCondition(Condition,Svc->OriginIndex,"OriginIndex=OriginIndex");
		Condition = DbAddLongCondition(Condition,Svc->DestIndex,"DestIndex=DestIndex");
		Condition = DbAddLongCondition(Condition,Svc->HeardIndex,"HeardIndex=HeardIndex");

		if (Svc->DataType) Condition = DbAddCondition(Condition,"DataType=DataType");

		if (Svc->FasterThan) Condition = DbAddCondition(Condition,"Speed>=FasterThan");
		if (Svc->SlowerThan) Condition = DbAddCondition(Condition,"Speed<=SlowerThan");
		if (Svc->StartTime) Condition = DbAddCondition(Condition,"When>=StartTime");
		if (Svc->EndTime) Condition = DbAddCondition(Condition,"When<EndTime");
	}

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	HOP_FILTER_INFO_S Info = {0};

		Info.StationIndex = Svc->RelayIndex;
		Rsp = DbQueryIntoGrowingResponse(PacketTable, DAPRS_QUERY_PACKETS_SRQ, Condition,
						sizeof(*Svc), Svc, AprsFilterQueryPackets, &Info,
						DAPRS_QUERY_PACKETS_SRP, "Packets", NULL,
						sizeof(*Rsp), &AccessPlan, HERE);
		if (Rsp)
		{	INDEX_F i;
		static	POINTER_F GetHopPlan=NULL;
			for (i=0; i<Rsp->Count; i++)
			{	HOP_S Hop;
				AprsGetStationID(Rsp->Packets[i].OriginIndex, NULL, &Rsp->Packets[i].OriginID);
				AprsGetStationID(Rsp->Packets[i].DestIndex, NULL, &Rsp->Packets[i].DestID);
				AprsGetStationID(Rsp->Packets[i].HeardIndex, NULL, &Rsp->Packets[i].IGateID);
				if (AprsGetPosition(Rsp->Packets[i].PositionIndex, &Rsp->Packets[i].Where))
					AprsBoundingBox(&Rsp->Packets[i].Where, &Rsp->Min, &Rsp->Max);
#ifdef FILTER
				if (strncmp(Rsp->Packets[i].RelayID,"WIDE",4))
					AprsBoundingBox(&Rsp->Packets[i].RelayLoc, &Rsp->Min, &Rsp->Max);
#endif
				Hop.PacketIndex = Rsp->Packets[i].PacketIndex;
				Hop.Sequence = 1;	/* IGate or Heard station */
				if (DbGetValues(HopTable, DHOP_S, "PacketIndex=PacketIndex AND Sequence=Sequence", sizeof(Hop), &Hop,
						DHOP_S, NULL, sizeof(Hop), &Hop, &GetHopPlan))
				{	if (AprsGetPosition(Hop.PositionIndex, &Rsp->Packets[i].IGateLoc))
						AprsBoundingBox(&Rsp->Packets[i].IGateLoc, &Rsp->Min, &Rsp->Max);
				}
				if (Svc->IncludeHopCount)
				{static	POINTER_F QueryPlan=NULL;
					Hop.PacketIndex = Rsp->Packets[i].PacketIndex;
					Rsp->Packets[i].HopCount = DbQueryCount(HopTable, DHOP_S, "PacketIndex=PacketIndex", sizeof(Hop), &Hop, NULL, NULL, &QueryPlan);
				}
			}
			CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
		} else	CiSendBadResponse2(Req, 204, "APRS::FAIQRYPKT", "Failed To Query Packets", 0);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcDeletePackets(MESSAGE_S *Req, STRING_F URL)
{	APRS_DELETE_PACKETS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_DELETE_PACKETS_SRP Rsp;
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F AccessPlan = NULL;

	if (Svc->PacketIndex)	/* Specific packet ignores all else */
		Condition = DbAddLongCondition(Condition,Svc->PacketIndex,"PacketIndex=PacketIndex");
	else
	{	RtStrnuprTrim(STRING(Svc->OriginID));
		RtStrnuprTrim(STRING(Svc->DestID));
		RtStrnuprTrim(STRING(Svc->RelayID));
		RtStrnuprTrim(STRING(Svc->IGateID));

		Condition = DbAddCondition(Condition, "HeardOnRF=HeardOnRF");

		if (!Svc->OriginIndex && !RtStrnWhite(STRING(Svc->OriginID)))
			Svc->OriginIndex = AprsGetStationIndex(Svc->OriginID, Svc->OriginID, FALSE);
		if (!Svc->DestIndex && !RtStrnWhite(STRING(Svc->DestID)))
		{	if (!RtStrnWhite(STRING(Svc->OriginID)))
				AprsGetStationID(Svc->OriginIndex, NULL, &Svc->OriginID);
			Svc->DestIndex = AprsGetStationIndex(Svc->OriginID, Svc->DestID, FALSE);
		}
		if (!Svc->HeardIndex && !RtStrnWhite(STRING(Svc->IGateID)))
			Svc->HeardIndex = AprsGetStationIndex(Svc->IGateID, Svc->IGateID, FALSE);

		Condition = DbAddLongCondition(Condition,Svc->OriginIndex,"OriginIndex=OriginIndex");
		Condition = DbAddLongCondition(Condition,Svc->DestIndex,"DestIndex=DestIndex");
		Condition = DbAddLongCondition(Condition,Svc->HeardIndex,"HeardIndex=HeardIndex");

		if (Svc->DataType) Condition = DbAddCondition(Condition,"DataType=DataType");

		if (Svc->StartTime) Condition = DbAddCondition(Condition,"When>=StartTime");
		if (Svc->EndTime) Condition = DbAddCondition(Condition,"When<EndTime");
	}

	if (!*Condition)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	Rsp.Count = DbDeleteRecords(PacketTable, DAPRS_DELETE_PACKETS_SRQ, Condition,
						sizeof(*Svc), Svc, NULL, NULL, &AccessPlan);
		CiSendGoodResponse(Req, &Rsp, sizeof(Rsp));
	}
	THREAD_FREE(Condition);

	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcQueryHops(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_HOPS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_HOPS_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F AccessPlan = NULL;

	Condition = DbAddLongCondition(Condition,Svc->PacketIndex,"PacketIndex=PacketIndex");
	Condition = DbAddLongCondition(Condition,Svc->Sequence,"Sequence=Sequence");
	Condition = DbAddLongCondition(Condition,Svc->StationIndex,"StationIndex=StationIndex");

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	Rsp = DbQueryIntoGrowingResponse(HopTable, DAPRS_QUERY_HOPS_SRQ, Condition,
						sizeof(*Svc), Svc, NULL, NULL,
						DAPRS_QUERY_HOPS_SRP, "Hops", NULL,
						sizeof(*Rsp), &AccessPlan, HERE);
		if (Rsp)
		{	INDEX_F i;
			for (i=0; i<Rsp->Count; i++)
			{	AprsGetStationID(Rsp->Hops[i].StationIndex, NULL, &Rsp->Hops[i].StationID);
				if (AprsGetPosition(Rsp->Hops[i].PositionIndex, &Rsp->Hops[i].Where))
					AprsBoundingBox(&Rsp->Hops[i].Where, &Rsp->Min, &Rsp->Max);
			}
			if (Svc->PacketIndex)
			{	DgPrintf("Need to sort and start Distance/Bearings\n");
#ifdef FUTURE
				for (i=0; i<Rsp->Count; i++)
				{static	POINTER_F QueryPlan=NULL;
					POINTER_F Results;
					Hop.PacketIndex = Rsp->Packets[i].PacketIndex;
					Results = DbQuery(HopTable, DHOP_S, "PacketIndex=PacketIndex", sizeof(Hop), &Hop, NULL, NULL,
								DHOP_S, NULL, sizeof(*pHop), NULL, NULL, &QueryPlan);

typedef struct APRS_QUERY_HOPS_SRP
{	COUNT_F		HopCount;	/* -1 if more than 5 */
	struct
	{	INTEGER_ID_F	PacketIndex;
		INDEX_F		Sequence;
		INTEGER_ID_F	StationIndex;
		INTEGER_ID_F	PositionIndex;
		BOOLEAN_F	Used;
		STATION_ID_F	StationID;
		COORDINATE_S	Where;
		DISTANCE_F	Distance;	/* Only for PacketIndex */
		BEARING_F	Bearing;	/* Only for PacketIndex */
	} Hops[1];
} APRS_QUERY_HOPS_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_HOPS_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_HOPS_SRP[];


					if (Results)
					{	CURSOR_S *Cursor = DsSetupSortedCursor(DbSetupResultsCursor, Results, "{!Sequence}", TRUE, HERE);
						pHop = Cursor->Next(Cursor);	/* pHop = originating station */
						if (pHop)
						{	INTEGER_ID_F FromIndex = pHop->StationIndex;
							INTEGER_ID_F FromPos = pHop->PositionIndex;
							COORDINATE_S FromCoord;
							AprsGetPosition(FromPos, &FromCoord);
							while ((pHop = Cursor->Next(Cursor)) != NULL)
							{	INDEX_F h = Rsp->Packets[i].HopCount++;
								Rsp->Packets[i].Hops[h].StationIndex = pHop->StationIndex;
								Rsp->Packets[i].Hops[h].PositionIndex = pHop->PositionIndex;
								Rsp->Packets[i].Hops[h].Used = pHop->Used;
								AprsGetStationID(pHop->StationIndex, NULL, &Rsp->Packets[i].Hops[h].StationID);
								if (AprsGetPosition(pHop->PositionIndex, &Rsp->Packets[i].Hops[h].Where))
								{	AprsHaversine(&FromCoord, &Rsp->Packets[i].Hops[h].Where, &Rsp->Packets[i].Hops[h].Distance, &Rsp->Packets[i].Hops[h].Bearing);
									if (Rsp->Packets[i].Hops[h].Distance < 400)
										AprsBoundingBox(&Rsp->Packets[i].Hops[h].Where, &Rsp->Min, &Rsp->Max);
									else DgPrintf("Unrealistic Hop %.2lf MILES!\n", (double) Rsp->Packets[i].Hops[h].Distance);
									FromIndex = pHop->StationIndex;
									FromPos = pHop->PositionIndex;
									FromCoord = Rsp->Packets[i].Hops[h].Where;
								}
							}
						}
						Cursor->Destroy(Cursor);
						DbFreeResults(Results);
					}
				}
#endif
			}
			CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
		} else	CiSendBadResponse2(Req, 204, "APRS::FAIQRYHOP", "Failed To Query Hops", 0);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcQueryStations(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_STATIONS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_STATIONS_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F AccessPlan = NULL;

	if (Svc->NewerThan) Condition = DbAddCondition(Condition,"FirstHeard>=NewerThan");
	if (Svc->HeardSince) Condition = DbAddCondition(Condition,"LastHeard>=HeardSince");
	if (Svc->InactiveSince) Condition = DbAddCondition(Condition,"LastHeard<InactiveSince");
	if (Svc->MovedSince == (TIMESTAMP_F) -1) Condition = DbAddCondition(Condition,"LastMotion=0");
	else if (Svc->MovedSince) Condition = DbAddCondition(Condition,"LastMotion>=MovedSince");
	if (Svc->FasterThan) Condition = DbAddCondition(Condition,"LastSpeed>=FasterThan");
	if (Svc->FurtherThan) Condition = DbAddCondition(Condition,"Odometer>=FurtherThan");
	if (Svc->IGateSince) Condition = DbAddCondition(Condition,"LastIGate>=IGateSince");
	if (Svc->RFIGateSince) Condition = DbAddCondition(Condition,"LastRFIGate>=RFIGateSince");

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	Rsp = DbQueryIntoGrowingResponse(StationTable, DAPRS_QUERY_STATIONS_SRQ, Condition,
						sizeof(*Svc), Svc, NULL, NULL,
						DAPRS_QUERY_STATIONS_SRP, "Stations", NULL,
						sizeof(*Rsp), &AccessPlan, HERE);
		if (Rsp)
		{	CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
		} else	CiSendBadResponse2(Req, 204, "APRS::FAIQRYSTN", "Failed To Query Stations", 0);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcLookupStation(MESSAGE_S *Req, STRING_F URL)
{	APRS_LOOKUP_STATION_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_LOOKUP_STATION_SRP Rsp;
static	POINTER_F AccessPlan = NULL;

	if (!Svc->StationIndex)
	{	if (RtStrnWhite(STRING(Svc->OwnerID))) DCOPY(Svc->OwnerID, Svc->StationID);
		Svc->StationIndex = AprsGetStationIndex(Svc->OwnerID, Svc->StationID, FALSE);
		if (Svc->StationIndex == -1)
		{	RtStrnuprTrim(STRING(Svc->OwnerID));
			RtStrnuprTrim(STRING(Svc->StationID));
			Svc->StationIndex = AprsGetStationIndex(Svc->OwnerID, Svc->StationID, FALSE);
		}
	}

	if (DbGetValues(StationTable, DAPRS_LOOKUP_STATION_SRQ, NULL, sizeof(*Svc), Svc,
			DAPRS_LOOKUP_STATION_SRP, NULL, sizeof(Rsp), &Rsp, &AccessPlan))
	{	Rsp.SymbolName = GetSymbolName(Rsp.Symbol);
		CiSendGoodResponse(Req, &Rsp, sizeof(Rsp));
	} else	CiSendBadResponse2(Req, 204, "APRS::STNNOTFND", "Station (%1) Not Found",
					1, STRING(Svc->StationID));

	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcQueryStationIndices(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_STATION_INDICES_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_STATION_INDICES_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F QueryPlan = NULL;

	RtStrnuprTrim(STRING(Svc->OwnerID));
	RtStrnuprTrim(STRING(Svc->StationID));

	Condition = DbAddStringCondition(Condition,STRING(Svc->OwnerID),"OwnerID=OwnerID");
	Condition = DbAddStringCondition(Condition,STRING(Svc->StationID),"StationID=StationID");

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	
		Rsp = DbQueryIntoGrowingResponse(StatIDTable, DAPRS_QUERY_STATION_INDICES_SRQ, Condition,
						sizeof(*Svc), Svc, NULL, NULL,
						DAPRS_QUERY_STATION_INDICES_SRP, "Stations", NULL,
						sizeof(*Rsp), &QueryPlan, HERE);
		if (Rsp)
		{	INDEX_F i;
		static	POINTER_F GetPlan=NULL;
			for (i=0; i<Rsp->Count; i++)
			{	DbGetValues(StationTable, DAPRS_LOOKUP_STATION_SRP, "StationIndex=StationIndex", sizeof(Rsp->Stations[i]), &Rsp->Stations[i],
						DAPRS_LOOKUP_STATION_SRP, NULL, sizeof(Rsp->Stations[i]), &Rsp->Stations[i], &GetPlan);
				Rsp->Stations[i].SymbolName = GetSymbolName(Rsp->Stations[i].Symbol);
			}
			CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
		} else	CiSendBadResponse2(Req, 204, "APRS::FAIQRYSTA", "Failed To Query Stations", 0);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

static BOOLEAN_F AprsFilterUniqueIGates(POINTER_F DbRecord, POINTER_F UserArg)
{	PACKET_S *pPacket = DbRecord;
	HASH_S *Hash = UserArg;
	COUNT_F Value;

	if ((Value=(COUNT_F)DsLookupHashKey(Hash,&pPacket->HeardIndex)) != 0)
	{	DsUpdateHashValue(Hash, &pPacket->HeardIndex, (POINTER_F)(Value+1), HERE);
		return FALSE;
	} else return DsInsertIfNewHashKey(Hash, &pPacket->HeardIndex, (POINTER_F) 1, HERE);
}

static BOOLEAN_F AprsFilterMaxDistance(POINTER_F DbRecord, POINTER_F UserArg)
{	PACKET_S *pPacket = DbRecord;
	DISTANCE_F *MaxDistance = UserArg;

	if (pPacket->Distance > *MaxDistance) DgPrintf("Station %ld Packet %ld %lf->%lf\n", (long) pPacket->HeardIndex, (long) pPacket->PacketIndex, (double) *MaxDistance, (double) pPacket->Distance);
	if (pPacket->Distance > *MaxDistance) *MaxDistance = pPacket->Distance;
	return FALSE;
}

BOOLEAN_F FUNCTION AprsSvcQueryUniqueIGates(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_UNIQUE_IGATES_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_UNIQUE_IGATES_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F QueryPlan=NULL, GetPlan=NULL, DistancePlan=NULL;

	if (Svc->StartTime) Condition = DbAddCondition(Condition,"When>=StartTime");
	if (Svc->EndTime) Condition = DbAddCondition(Condition,"When<EndTime");

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	HASH_S *Hash = DsCreateLocalHash("IGates", sizeof(INTEGER_ID_F), NULL, HERE);
		APRS_QUERY_PACKETS_SRQ QueryReq;

		{	POINTER_F Result;
			Result = DbQuery(PacketTable, DAPRS_QUERY_UNIQUE_IGATES_SRQ,
					Condition, sizeof(*Svc), Svc, AprsFilterUniqueIGates, Hash,
					DPACKET_S, "HeardIndex=HeardIndex", sizeof(PACKET_S), NULL, NULL, &QueryPlan);
			if (Result) DbFreeResults(Result);
		}

		QueryReq.StartTime = Svc->StartTime;
		QueryReq.EndTime = Svc->EndTime;
		Condition = DbAddCondition(Condition,"OriginIndex=OriginIndex");	/* Need the Origin included on top of time range */

		{	CURSOR_S *Cursor = DsSetupHashKeysCursor(Hash,NULL,HERE);
			INTEGER_ID_F *pThis;

			Rsp = CiCreateGrowingResponse(DAPRS_QUERY_UNIQUE_IGATES_SRP, "Stations", DsGetHashCount(Hash), HERE);
			Rsp->Count = 0;
			while ((pThis=Cursor->Next(Cursor)) != NULL)
			{	DISTANCE_F MaxDistance = 0;
				QueryReq.OriginIndex = *pThis;
				DbQueryCount(PacketTable, DAPRS_QUERY_PACKETS_SRQ, Condition, sizeof(QueryReq), &QueryReq, AprsFilterMaxDistance, &MaxDistance, &DistancePlan);
				if (!Svc->MaxMotion || MaxDistance < Svc->MaxMotion)
				{	Rsp->Stations[Rsp->Count].StationIndex = *pThis;
					Rsp->Stations[Rsp->Count].MaxDistance = MaxDistance;	/* So the client can decide */
					AprsGetStationID(Rsp->Stations[Rsp->Count].StationIndex, &Rsp->Stations[Rsp->Count].OwnerID, &Rsp->Stations[Rsp->Count].StationID);
					Rsp->Stations[Rsp->Count].PacketsGated = (COUNT_F) DsLookupHashKey(Hash,pThis);
					if (!DbGetValues(StationTable, DAPRS_QUERY_UNIQUE_IGATES_SRA, "StationIndex=StationIndex",
							sizeof(Rsp->Stations[Rsp->Count]), &Rsp->Stations[Rsp->Count],
							DAPRS_QUERY_UNIQUE_IGATES_SRA, NULL,
							sizeof(Rsp->Stations[Rsp->Count]), &Rsp->Stations[Rsp->Count], &GetPlan))
					{	if (!Svc->OnlyKnown) Rsp->Count++;	/* Return it anyway! */
					} else Rsp->Count++;
				} else DgPrintf("IGate %ld Moved %lf (>%lf) TOO FAR\n", (long) *pThis, (double) MaxDistance, (double) Svc->MaxMotion);
			}
			Cursor->Destroy(Cursor);
			CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
		}
		DsEmptyHash(Hash, NULL, NULL, HERE);
		DsDestroyHash(Hash, HERE);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

static BOOLEAN_F AprsFilterUniqueDestinations(POINTER_F DbRecord, POINTER_F UserArg)
{	PACKET_S *pPacket = DbRecord;
	HASH_S *Hash = UserArg;
	COUNT_F Value;

	if (pPacket->DataType == 0x1c	/* Ignore Mic-E packets */
	|| pPacket->DataType == 0x1d
	|| pPacket->DataType == 0x27
	|| pPacket->DataType == '`') return FALSE;

	if ((Value=(COUNT_F)DsLookupHashKey(Hash,&pPacket->DestIndex)) != 0)
	{	DsUpdateHashValue(Hash, &pPacket->DestIndex, (POINTER_F) (Value+1), HERE);
		return FALSE;
	} else return DsInsertIfNewHashKey(Hash, &pPacket->DestIndex, (POINTER_F) 1, HERE);
}

BOOLEAN_F FUNCTION AprsSvcQueryUniqueDestinations(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_UNIQUE_DESTINATIONS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_UNIQUE_DESTINATIONS_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F QueryPlan=NULL;

	if (Svc->StartTime) Condition = DbAddCondition(Condition,"When>=StartTime");
	if (Svc->EndTime) Condition = DbAddCondition(Condition,"When<EndTime");

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	HASH_S *Hash = DsCreateLocalHash("DestIndices", sizeof(INTEGER_ID_F), NULL, HERE);

		{	POINTER_F Result;
			Result = DbQuery(PacketTable, DAPRS_QUERY_UNIQUE_DESTINATIONS_SRQ,
					Condition, sizeof(*Svc), Svc, AprsFilterUniqueDestinations, Hash,
					DPACKET_S, "DestIndex=DestIndex", sizeof(PACKET_S), NULL, NULL, &QueryPlan);
			if (Result) DbFreeResults(Result);
		}

		{	HASH_S *Hash2 = DsCreateLocalHash("Destinations", sizeof(STATION_ID_F), NULL, HERE);
			CURSOR_S *Cursor = DsSetupHashKeysCursor(Hash,NULL,HERE);
			INTEGER_ID_F *pThis;
			STATION_ID_F *pStation;
			COUNT_F StartLen = min(RtStrnlen(STRING(Svc->StartsWith)),sizeof(*pStation));

			while ((pThis=Cursor->Next(Cursor)) != NULL)
			{	STATION_ID_F DestID;
				COUNT_F Value2, Value = (COUNT_F) DsLookupHashKey(Hash,pThis);
				AprsGetStationID(*pThis, NULL, &DestID);
				if ((Value2=(COUNT_F)DsLookupHashKey(Hash2,DestID)) != 0)
				{	DsUpdateHashValue(Hash2, DestID, (POINTER_F) (Value2+Value), HERE);
				} else DsInsertIfNewHashKey(Hash2, DestID, (POINTER_F) Value, HERE);
			}
			Cursor->Destroy(Cursor);

			Cursor = DsSetupHashKeysCursor(Hash2,NULL,HERE);
			Rsp = CiCreateGrowingResponse(DAPRS_QUERY_UNIQUE_DESTINATIONS_SRP, "Stations", DsGetHashCount(Hash2), HERE);
			Rsp->Count = 0;
			while ((pStation=Cursor->Next(Cursor)) != NULL)
			if (!StartLen || !strncmp(*pStation,Svc->StartsWith,StartLen))
			{	DCOPY(Rsp->Stations[Rsp->Count].DestID, *pStation);
				Rsp->Stations[Rsp->Count].PacketsReceived = (COUNT_F) DsLookupHashKey(Hash2,pStation);
				Rsp->Count++;
			}
			Cursor->Destroy(Cursor);

			CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
			DsEmptyHash(Hash2, NULL, NULL, HERE);
			DsDestroyHash(Hash2, HERE);
		}
		DsEmptyHash(Hash, NULL, NULL, HERE);
		DsDestroyHash(Hash, HERE);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

typedef struct UNIQUE_PATH_INFO_KEY_S
{	INTEGER_ID_F	FromIndex;
	INTEGER_ID_F	FromPos;
	INTEGER_ID_F	ToIndex;
	INTEGER_ID_F	ToPos;
	BOOLEAN_F	Relay;	/* TRUE if from a relay */
	BOOLEAN_F	IGate;	/* TRUE if To IGate as opposed to Router */
} UNIQUE_PATH_INFO_KEY_S;	/* FALSE/FALSE = Original to first Relay */
				/* FALSE/TRUE = Direct */
				/* TRUE/FALSE = Relay to relay */
				/* TRUE/TRUE = Final relay to Gate */

typedef struct UNIQUE_PATH_INFO_S
{	UNIQUE_PATH_INFO_KEY_S Key;
	struct
	{	TIMESTAMP_F FirstHeard;
		TIMESTAMP_F LastHeard;
		COUNT_F UseCount;
	} Data;
} UNIQUE_PATH_INFO_S;

static BOOLEAN_F AprsFilterChkUniqueIPath(INTEGER_ID_F FromIndex, INTEGER_ID_F FromPos, BOOLEAN_F Relay,
										INTEGER_ID_F ToIndex, INTEGER_ID_F ToPos, BOOLEAN_F IGate,
										TIMESTAMP_F When, HASH_S *Hash)
{	UNIQUE_PATH_INFO_KEY_S Key = {0};
	UNIQUE_PATH_INFO_S *Value;

	Key.FromIndex = FromIndex;
	Key.FromPos = FromPos;
	Key.Relay = Relay;
	Key.ToIndex = ToIndex;
	Key.ToPos = ToPos;
	Key.IGate = IGate;

	if ((Value=DsLookupHashKey(Hash,&Key)) != NULL)
	{	Value->Data.UseCount++;
		if (Value->Data.LastHeard < When) Value->Data.LastHeard = When;
		if (Value->Data.FirstHeard > When) Value->Data.FirstHeard = When;
		return FALSE;
	} else
	{	Value = THREAD_CALLOC(1,sizeof(*Value));
		Value->Key = Key;
		Value->Data.UseCount = 1;
		Value->Data.FirstHeard = Value->Data.LastHeard = When;
		return DsInsertIfNewHashKey(Hash, &Key, Value, HERE);
	}
}

static BOOLEAN_F AprsFilterUniquePaths(POINTER_F DbRecord, POINTER_F UserArg)
{	PACKET_S *p = DbRecord;
	HOP_FILTER_INFO_S *HopFilter = UserArg;
	HASH_S *Hash = HopFilter->UserArg;
	HOP_S *pHop;
	POINTER_F Results;
	BOOLEAN_F Result = FALSE;
static	POINTER_F QueryPlan = NULL;

	if (HopFilter->MinAltitude || HopFilter->MaxAltitude)
	{	if (p->Altitude < HopFilter->MinAltitude || p->Altitude > HopFilter->MaxAltitude)
			return FALSE;
	}

//DgPrintf("AprsFilterUniquePaths(%ld)\n", (long) p->PacketIndex);
	HopFilter->FoundUsed = FALSE;
	HopFilter->FoundInPacket = FALSE;
	Results = DbQuery(HopTable, DPACKET_S, "PacketIndex=PacketIndex", sizeof(*p), p, AprsFilterPacketHops, HopFilter,
				DHOP_S, NULL, sizeof(*pHop), NULL, NULL, &QueryPlan);
	if (Results)
	{	if (HopFilter->FoundInPacket)
		{	CURSOR_S *Cursor = DsSetupSortedCursor(DbSetupResultsCursor, Results, "{!Sequence}", TRUE, HERE);
//DgPrintf("AprsFilterUniquePaths(%ld) has %ld Hops\n", (long) p->PacketIndex, (long) Cursor->GetCount(Cursor));
			pHop = Cursor->Next(Cursor);	/* pHop = originating station */
			if (pHop)
			{	INTEGER_ID_F FromIndex = pHop->StationIndex;
				INTEGER_ID_F FromPos = pHop->PositionIndex;
				BOOLEAN_F PassedUsed = FALSE;
				STATION_ID_F SourceID = {0};
				AprsGetStationID(FromIndex, NULL, &SourceID);
				if (!HopFilter->FoundUsed) PassedUsed = TRUE;	/* None used, we passed it! */
				while ((pHop = Cursor->Next(Cursor)) != NULL)
				{	INTEGER_ID_F ToIndex = pHop->StationIndex;
					INTEGER_ID_F ToPos = pHop->PositionIndex;
					STATION_ID_F ToID;
//DgPrintf("AprsFilterUniquePaths(%ld:%ld) ToIndex:%ld\n", (long) p->PacketIndex, (long) pHop->Sequence, (long) ToIndex);
					if (AprsGetStationID(ToIndex, NULL, &ToID)
					&& *ToID != 'q'
					&& strncmp(ToID,"WIDE",4)
					&& strncmp(ToID,"TRACE",5))
					{	if (HopFilter->IncludeInvalidStations
						|| AprsIsLegalStation(&ToID))
						{	if (PassedUsed && !pHop->Used && pHop->Sequence != 1)
							{
								/* DgPrintf("Legal (NotUsed) Station %ld:%.*s after Used In Packet %ld Sequence %ld From %.*s at %.20s\n",
									(long) ToIndex, STRING(ToID), (long) p->PacketIndex, (long) pHop->Sequence, STRING(SourceID), ctime(&p->When)+4); */
							} else
							{
//DgPrintf("AprsFilterUniquePaths(%ld:%ld) Checking ->%.*s\n", (long) p->PacketIndex, (long) pHop->Sequence, STRING(ToID));
								if (AprsFilterChkUniqueIPath(FromIndex, FromPos,
											FromIndex!=p->OriginIndex,
											ToIndex, ToPos,
											ToIndex==p->HeardIndex,
											p->When, Hash))
									Result = TRUE;
								FromIndex = ToIndex;
								FromPos = ToPos;
							}
						}
					}
//else DgPrintf("AprsFilterUniquePaths(%ld:%ld) ToIndex:%ld(%.*s%s) Skipped!\n", (long) p->PacketIndex, pHop->Sequence, (long) ToIndex, STRING(ToID), pHop->Used?"*":"");
					if (pHop->Used) PassedUsed = TRUE;
				}
			}
			Cursor->Destroy(Cursor);
		}
		DbFreeResults(Results);
	}
	return Result;	/* Don't copy anything */
}

static VFUNCTION AprsFreeHeapPacket(HASH_S *Hash, POINTER_F Value, POINTER_F UserArg)
{	HEAP_FREE(UserArg,Value);
}

BOOLEAN_F FUNCTION AprsSvcQueryUniquePaths(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_UNIQUE_PATHS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_UNIQUE_PATHS_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("HeardOnRF=HeardOnRF");
static	POINTER_F QueryPlan=NULL;

	RtStrnuprTrim(STRING(Svc->OriginID));
	RtStrnuprTrim(STRING(Svc->RelayID));
	RtStrnuprTrim(STRING(Svc->IGateID));
	if (!Svc->MaxHopDistance) Svc->MaxHopDistance = 400.0;

	if (!Svc->OriginIndex && !RtStrnWhite(STRING(Svc->OriginID)))
		Svc->OriginIndex = AprsGetStationIndex(Svc->OriginID, Svc->OriginID, FALSE);
	if (!Svc->RelayIndex && !RtStrnWhite(STRING(Svc->RelayID)))
		Svc->RelayIndex = AprsGetStationIndex(Svc->RelayID, Svc->RelayID, FALSE);
	if (!Svc->HeardIndex && !RtStrnWhite(STRING(Svc->IGateID)))
		Svc->HeardIndex = AprsGetStationIndex(Svc->IGateID, Svc->IGateID, FALSE);

	Condition = DbAddLongCondition(Condition,Svc->OriginIndex,"OriginIndex=OriginIndex");
	Condition = DbAddLongCondition(Condition,Svc->HeardIndex,"HeardIndex=HeardIndex");

	if (Svc->StartTime) Condition = DbAddCondition(Condition,"When>=StartTime");
	if (Svc->EndTime) Condition = DbAddCondition(Condition,"When<EndTime");

	if (!Svc->OriginIndex && !RtStrnWhite(STRING(Svc->OriginID)))
		CiSendBadResponse(Req, 204, "Failed To Lookup OriginID", 0, NULL);
	else if (!Svc->RelayIndex && !RtStrnWhite(STRING(Svc->RelayID)))
		CiSendBadResponse(Req, 204, "Failed To Lookup RelayID", 0, NULL);
	else if (!Svc->HeardIndex && !RtStrnWhite(STRING(Svc->IGateID)))
		CiSendBadResponse(Req, 204, "Failed To Lookup IGateID", 0, NULL);
	else if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	HOP_FILTER_INFO_S HopFilter= {0};
		HopFilter.StationIndex = Svc->RelayIndex;
		HopFilter.IncludeInvalidStations = Svc->IncludeInvalidStations;
		HopFilter.UserArg = DsCreateLocalHash("Paths", sizeof(UNIQUE_PATH_INFO_KEY_S), NULL, HERE);

		DbQueryCount(PacketTable, DAPRS_QUERY_UNIQUE_PATHS_SRQ,
				Condition, sizeof(*Svc), Svc, AprsFilterUniquePaths, &HopFilter, &QueryPlan);
		{	CURSOR_S *Cursor = DsSetupHashContentsCursor(HopFilter.UserArg,HERE);
			UNIQUE_PATH_INFO_S *Info;
			BOOLEAN_F HaveOrigin = Svc->OriginIndex != 0;
			BOOLEAN_F HaveRelay = Svc->RelayIndex != 0;
			BOOLEAN_F HaveIGate = Svc->HeardIndex != 0;

			Rsp = CiCreateGrowingResponse(DAPRS_QUERY_UNIQUE_PATHS_SRP, "Paths", DsGetHashCount(HopFilter.UserArg), HERE);
			Rsp->Count = 0;
			while ((Info=Cursor->Next(Cursor)) != NULL)
			{	if (!Svc->DirectOnly
				|| (HaveOrigin && Svc->OriginIndex == Info->Key.FromIndex)
				|| (HaveRelay && (Svc->RelayIndex == Info->Key.FromIndex || Svc->RelayIndex == Info->Key.ToIndex))
				|| (HaveIGate && Svc->HeardIndex == Info->Key.ToIndex))
				{	APRS_QUERY_UNIQUE_PATHS_SRA *a = &Rsp->Paths[Rsp->Count];
					a->FromIndex = Info->Key.FromIndex;
					a->ToIndex = Info->Key.ToIndex;
					a->FromPosIndex = Info->Key.FromPos;
					a->ToPosIndex = Info->Key.ToPos;
					a->Relay = Info->Key.Relay;
					a->IGate = Info->Key.IGate;
					a->LastHeard = Info->Data.LastHeard;
					a->FirstHeard = Info->Data.FirstHeard;
					a->UseCount = Info->Data.UseCount;
					AprsGetStationID(a->FromIndex, NULL, &a->FromStationID);
					AprsGetStationID(a->ToIndex, NULL, &a->ToStationID);
					if (AprsGetPosition(a->FromPosIndex, &a->From)
					&& AprsGetPosition(a->ToPosIndex, &a->To))
					{	AprsHaversine(&a->From, &a->To,	&a->Distance, &a->Bearing);
						if (a->Distance < Svc->MaxHopDistance)
						{	AprsBoundingBox(&a->From, &Rsp->Min, &Rsp->Max);
							AprsBoundingBox(&a->To, &Rsp->Min, &Rsp->Max);
							if (HaveOrigin && !strncmp(a->FromStationID, Svc->OriginID, sizeof(a->FromStationID)))
								AprsBoundingBox(&a->From, &Rsp->Actual.Min, &Rsp->Actual.Max);
							if (HaveIGate && !strncmp(a->ToStationID, Svc->IGateID, sizeof(a->ToStationID)))
								AprsBoundingBox(&a->To, &Rsp->Actual.Min, &Rsp->Actual.Max);
						} else DgPrintf("Unthinkable Path %.2lf MILES %.*s to %.*s!\n",
								(double) a->Distance, STRING(a->FromStationID), STRING(a->ToStationID));
					}
					Rsp->Count++;
				}
			}
			Cursor->Destroy(Cursor);
#ifdef UNNECESSARY
			if (Rsp->Count == 1)
			{	AprsBoundingBox(&Rsp->Paths[0].From, &Rsp->Min, &Rsp->Max);
				AprsBoundingBox(&Rsp->Paths[0].To, &Rsp->Min, &Rsp->Max);
			}
#endif
			CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
		}
		DsEmptyHash(HopFilter.UserArg, AprsFreeHeapPacket, THREAD_HEAP, HERE);
		DsDestroyHash(HopFilter.UserArg, HERE);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcQueryUniqueAltPaths(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_UNIQUE_ALT_PATHS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_UNIQUE_ALT_PATHS_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("HeardOnRF=HeardOnRF");
static	POINTER_F QueryPlan=NULL;

	RtStrnuprTrim(STRING(Svc->OriginID));
	RtStrnuprTrim(STRING(Svc->RelayID));
	RtStrnuprTrim(STRING(Svc->IGateID));
	if (!Svc->MaxHopDistance) Svc->MaxHopDistance = 400.0;

	if (!Svc->OriginIndex && !RtStrnWhite(STRING(Svc->OriginID)))
		Svc->OriginIndex = AprsGetStationIndex(Svc->OriginID, Svc->OriginID, FALSE);
	if (!Svc->RelayIndex && !RtStrnWhite(STRING(Svc->RelayID)))
		Svc->RelayIndex = AprsGetStationIndex(Svc->RelayID, Svc->RelayID, FALSE);
	if (!Svc->HeardIndex && !RtStrnWhite(STRING(Svc->IGateID)))
		Svc->HeardIndex = AprsGetStationIndex(Svc->IGateID, Svc->IGateID, FALSE);

	Condition = DbAddLongCondition(Condition,Svc->OriginIndex,"OriginIndex=OriginIndex");
	Condition = DbAddLongCondition(Condition,Svc->HeardIndex,"HeardIndex=HeardIndex");

	if (Svc->StartTime) Condition = DbAddCondition(Condition,"When>=StartTime");
	if (Svc->EndTime) Condition = DbAddCondition(Condition,"When<EndTime");

	if (!Svc->OriginIndex && !RtStrnWhite(STRING(Svc->OriginID)))
		CiSendBadResponse(Req, 204, "Failed To Lookup OriginID", 0, NULL);
	else if (!Svc->RelayIndex && !RtStrnWhite(STRING(Svc->RelayID)))
		CiSendBadResponse(Req, 204, "Failed To Lookup RelayID", 0, NULL);
	else if (!Svc->HeardIndex && !RtStrnWhite(STRING(Svc->IGateID)))
		CiSendBadResponse(Req, 204, "Failed To Lookup IGateID", 0, NULL);
	else if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	HOP_FILTER_INFO_S HopFilter= {0};
		HopFilter.StationIndex = Svc->RelayIndex;
		HopFilter.IncludeInvalidStations = Svc->IncludeInvalidStations;
		HopFilter.MinAltitude = Svc->MinAltitude;
		HopFilter.MaxAltitude = Svc->MaxAltitude;
		HopFilter.UserArg = DsCreateLocalHash("Paths", sizeof(UNIQUE_PATH_INFO_KEY_S), NULL, HERE);

		DbQueryCount(PacketTable, DAPRS_QUERY_UNIQUE_ALT_PATHS_SRQ,
				Condition, sizeof(*Svc), Svc, AprsFilterUniquePaths, &HopFilter, &QueryPlan);
		{	CURSOR_S *Cursor = DsSetupHashContentsCursor(HopFilter.UserArg,HERE);
			UNIQUE_PATH_INFO_S *Info;
			BOOLEAN_F HaveOrigin = Svc->OriginIndex != 0;
			BOOLEAN_F HaveRelay = Svc->RelayIndex != 0;
			BOOLEAN_F HaveIGate = Svc->HeardIndex != 0;

			Rsp = CiCreateGrowingResponse(DAPRS_QUERY_UNIQUE_ALT_PATHS_SRP, "Paths", DsGetHashCount(HopFilter.UserArg), HERE);
			Rsp->Count = 0;
			while ((Info=Cursor->Next(Cursor)) != NULL)
			{	if (!Svc->DirectOnly
				|| (HaveOrigin && Svc->OriginIndex == Info->Key.FromIndex)
				|| (HaveRelay && (Svc->RelayIndex == Info->Key.FromIndex || Svc->RelayIndex == Info->Key.ToIndex))
				|| (HaveIGate && Svc->HeardIndex == Info->Key.ToIndex))
				{	APRS_QUERY_UNIQUE_ALT_PATHS_SRA *a = &Rsp->Paths[Rsp->Count];
					a->FromIndex = Info->Key.FromIndex;
					a->ToIndex = Info->Key.ToIndex;
					a->FromPosIndex = Info->Key.FromPos;
					a->ToPosIndex = Info->Key.ToPos;
					a->Relay = Info->Key.Relay;
					a->IGate = Info->Key.IGate;
					a->LastHeard = Info->Data.LastHeard;
					a->FirstHeard = Info->Data.FirstHeard;
					a->UseCount = Info->Data.UseCount;
					AprsGetStationID(a->FromIndex, NULL, &a->FromStationID);
					AprsGetStationID(a->ToIndex, NULL, &a->ToStationID);
					if (AprsGetPosition(a->FromPosIndex, &a->From)
					&& AprsGetPosition(a->ToPosIndex, &a->To))
					{	AprsHaversine(&a->From, &a->To,	&a->Distance, &a->Bearing);
						if (a->Distance < Svc->MaxHopDistance)
						{	AprsBoundingBox(&a->From, &Rsp->Min, &Rsp->Max);
							AprsBoundingBox(&a->To, &Rsp->Min, &Rsp->Max);
							if (HaveOrigin && !strncmp(a->FromStationID, Svc->OriginID, sizeof(a->FromStationID)))
								AprsBoundingBox(&a->From, &Rsp->Actual.Min, &Rsp->Actual.Max);
							if (HaveIGate && !strncmp(a->ToStationID, Svc->IGateID, sizeof(a->ToStationID)))
								AprsBoundingBox(&a->To, &Rsp->Actual.Min, &Rsp->Actual.Max);
						} else DgPrintf("Unthinkable Path %.2lf MILES %.*s to %.*s!\n",
								(double) a->Distance, STRING(a->FromStationID), STRING(a->ToStationID));
					}
					Rsp->Count++;
				}
			}
			Cursor->Destroy(Cursor);
#ifdef UNNECESSARY
			if (Rsp->Count == 1)
			{	AprsBoundingBox(&Rsp->Paths[0].From, &Rsp->Min, &Rsp->Max);
				AprsBoundingBox(&Rsp->Paths[0].To, &Rsp->Min, &Rsp->Max);
			}
#endif
			CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
		}
		DsEmptyHash(HopFilter.UserArg, AprsFreeHeapPacket, THREAD_HEAP, HERE);
		DsDestroyHash(HopFilter.UserArg, HERE);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

static BOOLEAN_F AprsFilterTrafficRates(POINTER_F DbRecord, POINTER_F UserArg)
{	PACKET_S *pPacket = DbRecord;
	HOP_FILTER_INFO_S *HopFilter = UserArg;
	HASH_S *Hash = HopFilter->UserArg;
	TIMESTAMP_F When = (pPacket->When/HopFilter->BucketSize)*HopFilter->BucketSize;
	COUNT_F Value;

	if (!AprsCheckHopFilter(DbRecord, UserArg))
		return FALSE;

	if ((Value=(COUNT_F)DsLookupHashKey(Hash,&When)) != 0)
	{	DsUpdateHashValue(Hash, &When, (POINTER_F) (Value+1), HERE);
		return FALSE;
	}
	return DsInsertIfNewHashKey(Hash, &When, (POINTER_F) 1, HERE);
}

BOOLEAN_F FUNCTION AprsSvcQueryTrafficRates(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_TRAFFIC_RATES_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_TRAFFIC_RATES_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("HeardOnRF=HeardOnRF");
static	POINTER_F QueryPlan=NULL;

	RtStrnuprTrim(STRING(Svc->OriginID));
	RtStrnuprTrim(STRING(Svc->RelayID));
	RtStrnuprTrim(STRING(Svc->IGateID));

	if (!Svc->OriginIndex && !RtStrnWhite(STRING(Svc->OriginID)))
		Svc->OriginIndex = AprsGetStationIndex(Svc->OriginID, Svc->OriginID, FALSE);
	if (!Svc->RelayIndex && !RtStrnWhite(STRING(Svc->RelayID)))
		Svc->RelayIndex = AprsGetStationIndex(Svc->RelayID, Svc->RelayID, FALSE);
	if (!Svc->HeardIndex && !RtStrnWhite(STRING(Svc->IGateID)))
		Svc->HeardIndex = AprsGetStationIndex(Svc->IGateID, Svc->IGateID, FALSE);

	Condition = DbAddLongCondition(Condition,Svc->OriginIndex,"OriginIndex=OriginIndex");
	Condition = DbAddLongCondition(Condition,Svc->HeardIndex,"HeardIndex=HeardIndex");

	if (Svc->StartTime) Condition = DbAddCondition(Condition,"When>=StartTime");
	if (Svc->EndTime) Condition = DbAddCondition(Condition,"When<EndTime");

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else if (!Svc->BucketSize)
	{	CiSendBadResponse(Req, 204, "BucketSize Must Be Non-Zero", 0, NULL);
	} else
	{	HOP_FILTER_INFO_S Info = {0};

		Info.StationIndex = Svc->RelayIndex;
		Info.BucketSize = Svc->BucketSize*60;
		Info.UserArg = DsCreateLocalHash("Rates", sizeof(TIMESTAMP_F), NULL, HERE);

		{	POINTER_F Result;
			Result = DbQuery(PacketTable, DAPRS_QUERY_TRAFFIC_RATES_SRQ,
					Condition, sizeof(*Svc), Svc, AprsFilterTrafficRates, &Info,
					DPACKET_S, "When=When", sizeof(PACKET_S), NULL, NULL, &QueryPlan);
			if (Result) DbFreeResults(Result);
		}

		{	CURSOR_S *Cursor = DsSetupHashKeysCursor(Info.UserArg,NULL,HERE);
			TIMESTAMP_F *pThis, StartTime = (Svc->StartTime>1)?Svc->StartTime:0, EndTime = (Svc->EndTime>1)?Svc->EndTime:0;

			DgPrintf("Starting from %ld %.24s\n", (long) StartTime, ctime(&StartTime));
			DgPrintf("Starting to %ld %.24s\n", (long) EndTime, ctime(&EndTime));

			Rsp = CiCreateGrowingResponse(DAPRS_QUERY_TRAFFIC_RATES_SRP, "Rates", DsGetHashCount(Info.UserArg), HERE);
			Rsp->Count = 0;
			while ((pThis=Cursor->Next(Cursor)) != NULL)
			{	Rsp->Rates[Rsp->Count].StartTime = *pThis;
				Rsp->Rates[Rsp->Count].PacketCount = (COUNT_F) DsLookupHashKey(Info.UserArg,pThis);
				Rsp->Total += Rsp->Rates[Rsp->Count].PacketCount;
				Rsp->Count++;
				if (!StartTime || StartTime > *pThis) StartTime = *pThis;
				if (!EndTime || EndTime < *pThis) EndTime = *pThis;
			}
			Cursor->Destroy(Cursor);

			if (StartTime && EndTime && Svc->IncludeEmpties)
			{	TIMESTAMP_F Time;
				COUNT_F MyBucket = Svc->BucketSize*60;
				DgPrintf("Fleshing from %ld %.24s\n", (long) StartTime, ctime(&StartTime));
				DgPrintf("Fleshing to %ld %.24s\n", (long) EndTime, ctime(&EndTime));
				StartTime = (StartTime/MyBucket)*MyBucket;
				EndTime = ((EndTime+MyBucket-1)/MyBucket)*MyBucket;
				DgPrintf("Fleshing from %ld %.24s\n", (long) StartTime, ctime(&StartTime));
				DgPrintf("Fleshing to %ld %.24s\n", (long) EndTime, ctime(&EndTime));
				DgPrintf("Adding up to %ld Buckets\n", (EndTime-StartTime)/MyBucket);

				Rsp = CiExpandGrowingResponse(Rsp, (EndTime-StartTime)/MyBucket, HERE);

				for (Time=StartTime; Time<EndTime; Time += MyBucket)
				{	if (!DsLookupHashKey(Info.UserArg,&Time))
					{	INDEX_F NewIndex = Rsp->Count++;
						/* DgPrintf("Adding Missing %.24s At %ld\n", ctime(&Time), (long) NewIndex); */
						/* Rsp = CiExpandGrowingResponse(Rsp, NewIndex, HERE); */
						Rsp->Rates[NewIndex].StartTime = Time;
						Rsp->Rates[NewIndex].PacketCount = 0;
					}
				}
			}

			CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
		}
		DsEmptyHash(Info.UserArg, NULL, NULL, HERE);
		DsDestroyHash(Info.UserArg, HERE);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int long2tile(double lon, int z) 
{ 
	return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z))); 
}

static int lat2tile(double lat, int z)
{ 
	return (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z))); 
}

static double tile2long(int x, int z) 
{
	return x / pow(2.0, z) * 360.0 - 180;
}

static double tile2lat(int y, int z) 
{
	double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

/*
http://tile.openstreetmap.org/18/72337/109822.png
http://tile.openstreetmap.org/17/36168/54911.png
http://tah.openstreetmap.org/Tiles/tile/17/36168/54911.png

From: http://wiki.openstreetmap.org/index.php/Slippy_map_tilenames
Name 	 URL 	 zoomlevel
OSM Mapnik 	http://tile.openstreetmap.org/12/2047/1362.png 	0-18
OSM Osmarender/Tiles@Home:	http://tah.openstreetmap.org/Tiles/tile/12/2047/1362.png 	0-17
OSM Cycle Map 	http://andy.sandbox.cloudmade.com/tiles/cycle/12/2047/1362.png 	0-18
OSM CloudMade Web style 	http://tile.cloudmade.com/8bafab36916b5ce6b4395ede3cb9ddea/1/256/12/2047/1362.png 	0-18
OSM CloudMade Mobile style 	http://tile.cloudmade.com/8bafab36916b5ce6b4395ede3cb9ddea/2/256/12/2047/1362.png 	0-18
OSM CloudMade NoNames style 	http://tile.cloudmade.com/8bafab36916b5ce6b4395ede3cb9ddea/3/256/12/2047/1362.png 	0-18
OSM Maplint 	http://tah.openstreetmap.org/Tiles/maplint/12/2047/1362.png 	12-16
Further tilesets are available from various '3rd party' sources.
*/
BOOLEAN_F FUNCTION AprsSvcCalcOSMTiles(MESSAGE_S *Req, STRING_F URL)
{	APRS_CALC_OSM_TILES_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_CALC_OSM_TILES_SRP *Rsp = CiCreateGrowingResponse(DAPRS_CALC_OSM_TILES_SRP, "Tiles", 0, HERE);
	INDEX_F y, Zoom, zStart = 18, zEnd = 1;

	Rsp->Count = 0;

	if (Svc->Zoom > 0)
	{	zStart = Svc->Zoom;
		zEnd = Svc->Zoom-1;
	}
	for (Zoom=zStart; Zoom>zEnd; Zoom--)
	{	COORDINATE_F dMargin;

		Rsp->StartX = long2tile(Svc->Min.Longitude, Zoom);
		Rsp->EndX = long2tile(Svc->Max.Longitude, Zoom);
		if (Rsp->EndX < Rsp->StartX)
		{	COUNT_F Temp = Rsp->StartX;
			Rsp->StartX = Rsp->EndX;
			Rsp->EndX = Temp;
		}
		Rsp->StartY = lat2tile(Svc->Min.Latitude, Zoom);
		Rsp->EndY = lat2tile(Svc->Max.Latitude, Zoom);
		if (Rsp->EndY < Rsp->StartY)
		{	COUNT_F Temp = Rsp->StartY;
			Rsp->StartY = Rsp->EndY;
			Rsp->EndY = Temp;
		}

		Rsp->Max.Latitude = tile2lat(Rsp->StartY,Zoom);
		dMargin = Rsp->Max.Latitude-tile2lat(Rsp->StartY+1,Zoom);	/* Degrees in top tile */
		dMargin = (dMargin*(double)Svc->TopMargin/256.0);	/* Offset for top margin pixels */
		if (Rsp->Max.Latitude-Svc->Max.Latitude < dMargin) if (Rsp->StartY > 0) Rsp->StartY--;	/* Move up one more tile */
#ifdef VERBOSE
		DgPrintf("Top Needs %ld Pixels or %.4lf Degrees, Have %.4lf Degrees\n",
			(long) Svc->TopMargin, (double) dMargin, (double) Rsp->Max.Latitude-Svc->Max.Latitude);
#endif

		Rsp->Min.Latitude = tile2lat(Rsp->EndY+1,Zoom);
		dMargin = tile2lat(Rsp->EndY,Zoom)-Rsp->Min.Latitude;	/* Degrees in bottom tile */
		dMargin = (dMargin*(double)Svc->BottomMargin/256.0);	/* Offset for bottom margin pixels */
		if (Svc->Min.Latitude-Rsp->Min.Latitude < dMargin) Rsp->EndY++;	/* Move down one more tile */
#ifdef VERBOSE
		DgPrintf("Bottom Needs %ld Pixels or %.4lf Degrees, Have %.4lf Degrees\n",
			(long) Svc->BottomMargin, (double) dMargin, (double) Svc->Min.Latitude-Rsp->Min.Latitude);
#endif

		Rsp->Min.Longitude = tile2long(Rsp->StartX,Zoom);
		dMargin = tile2long(Rsp->StartX+1,Zoom)-Rsp->Min.Longitude;	/* Degrees in left-most tile */
		dMargin = (dMargin*(double)Svc->LeftMargin/256.0);	/* Offset for left margin pixels */
		if (Svc->Min.Longitude-Rsp->Min.Longitude < dMargin) if (Rsp->StartX > 0) Rsp->StartX--;	/* Move left one more tile */
#ifdef VERBOSE
		DgPrintf("Left Needs %ld Pixels or %.4lf Degrees, Have %.4lf Degrees\n",
			(long) Svc->LeftMargin, (double) dMargin, (double) Svc->Min.Longitude-Rsp->Min.Longitude);
#endif

		Rsp->Max.Longitude = tile2long(Rsp->EndX+1,Zoom);
		dMargin = Rsp->Max.Longitude-tile2long(Rsp->EndX,Zoom);	/* Degrees in right-most tile */
		dMargin = (dMargin*(double)Svc->RightMargin/256.0);	/* Offset for right margin pixels */
		if (Rsp->Max.Longitude-Svc->Max.Longitude < dMargin) Rsp->EndX++;	/* Move right one more tile */
#ifdef VERBOSE
		DgPrintf("Right Needs %ld Pixels or %.4lf Degrees, Have %.4lf Degrees\n",
			(long) Svc->RightMargin, (double) dMargin, (double) Rsp->Max.Longitude-Svc->Max.Longitude);
#endif

		if ((Rsp->EndX-Rsp->StartX+1.0) * (Rsp->EndY-Rsp->StartY+1.0) <= (double) Svc->MaxTiles)
		{	Zoom--;	/* Fix for post-increment */
			break;
		}
	}
	Rsp->Zoom = ++Zoom;
	Rsp->TileCount = (Rsp->EndX-Rsp->StartX+1.0) * (Rsp->EndY-Rsp->StartY+1.0);
	Rsp->Min.Longitude = tile2long(Rsp->StartX, Zoom);
	Rsp->Max.Longitude = tile2long(Rsp->EndX+1, Zoom);
	if (Rsp->Max.Longitude < Rsp->Min.Longitude)
	{	COORDINATE_F Temp = Rsp->Min.Longitude;
		Rsp->Min.Longitude = Rsp->Max.Longitude;
		Rsp->Max.Longitude = Temp;
	}
	Rsp->Min.Latitude = tile2lat(Rsp->StartY, Zoom);
	Rsp->Max.Latitude = tile2lat(Rsp->EndY+1, Zoom);
	if (Rsp->Max.Latitude < Rsp->Min.Latitude)
	{	COORDINATE_F Temp = Rsp->Min.Latitude;
		Rsp->Min.Latitude = Rsp->Max.Latitude;
		Rsp->Max.Latitude = Temp;
	}

	Rsp->Count = Rsp->EndY-Rsp->StartY+1;
	Rsp = CiExpandGrowingResponse(Rsp, Rsp->Count, HERE);
	Rsp->DeltaLongitude = tile2long(Rsp->StartX, Zoom) - tile2long(Rsp->StartX+1, Zoom);
	for (y=0; y<Rsp->Count; y++)
	{	Rsp->Tiles[y].Y = Rsp->StartY+y;
		Rsp->Tiles[y].StartLatitude = tile2lat(Rsp->StartY+y+1,Zoom);
		Rsp->Tiles[y].EndLatitude = tile2lat(Rsp->StartY+y,Zoom);
		Rsp->Tiles[y].DeltaLatitude = Rsp->Tiles[y].EndLatitude - Rsp->Tiles[y].StartLatitude;
	}
	CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));

#ifdef DEBUG
	{	FILE *Out = fopen("Coords.csv", "w");
		if (Out)
		{	INDEX_F x, y;
			fprintf(Out,"X,Y,Lat1,Long1,Lat2,Long2\n");
			for (x=Rsp->StartX; x<=Rsp->EndX; x++)
			{	for (y=Rsp->StartY; y<=Rsp->EndY; y++)
				{	COORDINATE_F Lat1 = tile2lat(y,Zoom);
					COORDINATE_F Lat2 = tile2lat(y+1,Zoom);
					COORDINATE_F Long1 = tile2long(x, Zoom);
					COORDINATE_F Long2 = tile2long(x+1, Zoom);
					fprintf(Out,"%ld,%ld,%.4lf,%.4lf,%.4lf,%.4lf\n",
						(long) x, (long) y,
						(double) Lat1, (double) Long1,
						(double) Lat2, (double) Long2);
				}
			}
			fclose(Out);
		}
	}
#endif
	CiDestroyGrowingResponse(Rsp);

	return TRUE;
}

char *APRSLatLon(double Lat, double Lon, char Table, char Code)
{	char *OutBuf = THREAD_MALLOC(30);
	char *p;
	char NS = (Lat<0)?'S':'N';
	char EW = (Lon<0)?'W':'E';
	if (Lat < 0) Lat = -Lat;
	if (Lon < 0) Lon = -Lon;

	if (Code != ' ' && (Code < '!' || Code > '~')) Code = '.';
	if (Table != ' ' && (Table < '!' || Table > '~')) Table = '/';

	sprintf(OutBuf, "%02ld%5.2lf%c%c%03ld%5.2lf%c%c",
					(long) floor(Lat), (double) (Lat - floor(Lat))*60.0, NS, Table,
					(long) floor(Lon), (double) (Lon - floor(Lon))*60.0, EW, Code);
	for (p=OutBuf; *p; p++)
		if (*p == ' ' && (p-OutBuf)!=8 && (p-OutBuf)!=18)
			*p = '0';

	return OutBuf;
}

char *APRSAltitude(int Valid, double Alt)
{	char *OutBuf = THREAD_MALLOC(30);
	if (!Valid || Alt <= 0.0) return strcpy(OutBuf,"");

	sprintf(OutBuf, "/A=%06ld", (long) (Alt * 3.2808399));

	return OutBuf;
}

char *APRSHeadSpeed(int Valid, double Hdg, double Spd)
{	char *OutBuf = THREAD_MALLOC(30);
	if (!Valid) return strcpy(OutBuf,"");
	if (Hdg < 0.5) Hdg = 360.0;	/* 0 is special for fixed position objects, 360 = North */
	sprintf(OutBuf, "%03ld/%03ld", (long) (Hdg+0.5), (long) (Spd+0.5));
	return OutBuf;
}

#define POS_TIMESTAMPED_WITH_MESSAGING '@'
#define POS_TIMESTAMPED_NO_MESSAGING '/'

BOOLEAN_F FUNCTION AprsSvcInjectStationPosition(MESSAGE_S *Req, STRING_F URL)
{	APRS_INJECT_STATION_POSITION_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_INJECT_STATION_POSITION_SRP Rsp;
	STRING_F LatLon, HeadSpeed, Altitude, Duplicate;
	struct tm *tm;
	INDEX_F Page;

	RtStrnuprTrim(-1,Svc->OwnerID);
	RtStrnuprTrim(-1,Svc->StationID);
	RtStrnuprTrim(-1,Svc->ToCall);
	if (RtStrnWhite(-1,Svc->OwnerID)) Svc->OwnerID = Svc->StationID;
	if (RtStrnWhite(-1,Svc->ToCall)) Svc->ToCall = "APZTST";
	if (!Svc->Timestamp) Svc->Timestamp = RtNow(NULL);
	tm = gmtime(&Svc->Timestamp);
	if (!Svc->Comment) Svc->Comment = "";
	if (!Svc->Symbol) Svc->Symbol = '.';

	DgPrintf("%s Symbol %ld\n", Svc->StationID, (long) Svc->Symbol);
	Page = Svc->Symbol >> 8;
	Svc->Symbol = Svc->Symbol & 0xff;
	if (Svc->Symbol < '!' || Svc->Symbol > '~')
	{	Svc->Symbol = '.';
		Page = 0;
	}
	DgPrintf("%s Page %ld Symbol %ld or %s\n", Svc->StationID, (long) Page, (long) Svc->Symbol, GetSymbolName((Page<<8)+Svc->Symbol));

	LatLon = APRSLatLon(Svc->Pos.Latitude, Svc->Pos.Longitude, Page?'\\':'/',Svc->Symbol);
	HeadSpeed = APRSHeadSpeed(FALSE, Svc->Bearing, Svc->Speed);
	Altitude = APRSAltitude(FALSE, Svc->Altitude);
	Rsp.Packet = THREAD_MALLOC(1024+strlen(Svc->Comment));
	sprintf(Rsp.Packet,"%s>%s,TCPIP*:%c%02ld%02ld%02ldh%s%s%s%s",
		Svc->StationID, Svc->ToCall,
		Svc->MessagingCapable?POS_TIMESTAMPED_WITH_MESSAGING:POS_TIMESTAMPED_NO_MESSAGING,
		(long) tm->tm_hour, (long) tm->tm_min, (long) tm->tm_sec,
		LatLon, HeadSpeed, Altitude, Svc->Comment);
	DgPrintf("Formatted %ld bytes of (%s)\n", (long) strlen(Rsp.Packet), Rsp.Packet);
	Duplicate = THREAD_STRDUP(Rsp.Packet);
	if (AprsLogReceivedPacket(Svc->Timestamp, "HTTP", Duplicate))
	{	AprsQueueXmitPacket(Rsp.Packet);
		CiSendGoodResponse(Req, &Rsp, sizeof(Rsp));
	} else	CiSendBadResponse2(Req, 204, "APRS::BADPACKET", "Formatted Unparsable Packet", 0);
	THREAD_FREE(Duplicate); THREAD_FREE(Rsp.Packet);
	THREAD_FREE(LatLon); THREAD_FREE(HeadSpeed); THREAD_FREE(Altitude);

	return TRUE;
}



