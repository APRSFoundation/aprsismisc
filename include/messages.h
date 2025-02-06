/*
<pre>
	When	Who		What
	080731	L.Deffenbaugh	Original Implementation
	090520	L.Deffenbaugh	Support Actual.Min/Max on QueryUniquePaths
	091006	L.Deffenbaugh	Add Version to User query
	120223	L.Deffenbaugh	Support LogDigipeatedPacket for Queueing purposes
	140317	L.Deffenbaugh	Support QueryUniqueAltPaths for altitude constraints
</pre>
*/

#ifndef GOT_APRS_MESSAGES
#define GOT_APRS_MESSAGES

#include <aprs/include/aprsdef.h>

typedef struct APRS_LOG_PACKET_SRQ
{	TIMESTAMP_F	When;	/* 0 defaults to NOW */
	STRING_F	Packet;
} APRS_LOG_PACKET_SRQ;

typedef EMPTY_S APRS_LOG_PACKET_SRP;

extern DESCRIPTOR_S DAPRS_LOG_PACKET_SRQ[];
extern DESCRIPTOR_S DAPRS_LOG_PACKET_SRP[];

typedef struct APRS_LOG_RECEIVED_PACKET_SRQ
{	TIMESTAMP_F	When;	/* 0 defaults to NOW */
	STRING_F	ReceivedBy;	/* Callsign (treated as IGate?) */
	STRING_F	Packet;
} APRS_LOG_RECEIVED_PACKET_SRQ;

typedef EMPTY_S APRS_LOG_RECEIVED_PACKET_SRP;

extern DESCRIPTOR_S DAPRS_LOG_RECEIVED_PACKET_SRQ[];
extern DESCRIPTOR_S DAPRS_LOG_RECEIVED_PACKET_SRP[];

typedef APRS_LOG_RECEIVED_PACKET_SRQ APRS_LOG_DIGIPEATED_PACKET_SRQ;

typedef APRS_LOG_RECEIVED_PACKET_SRP APRS_LOG_DIGIPEATED_PACKET_SRP;

extern DESCRIPTOR_S DAPRS_LOG_DIGIPEATED_PACKET_SRQ[];
extern DESCRIPTOR_S DAPRS_LOG_DIGIPEATED_PACKET_SRP[];

typedef struct APRS_REPARSE_LOG_FILE_SRQ
{	PATHNAME_F		Path;
	COORDINATE_S	From, To;
} APRS_REPARSE_LOG_FILE_SRQ;

typedef struct APRS_REPARSE_LOG_FILE_SRP
{	COUNT_F		Processed;
	COUNT_F		NullIsland;
	COUNT_F		OutOfBounds;
	COUNT_F		ParseFail;
} APRS_REPARSE_LOG_FILE_SRP;

extern DESCRIPTOR_S DAPRS_REPARSE_LOG_FILE_SRQ[];
extern DESCRIPTOR_S DAPRS_REPARSE_LOG_FILE_SRP[];

typedef struct APRS_PURGE_PACKETS_SRQ
{	STATION_ID_F	OriginID;	/* NULL for all */
	STATION_ID_F	DestID;		/* NULL for all */
	STATION_ID_F	RelayID;	/* NULL for all */
	STATION_ID_F	IGateID;	/* NULL for all */
	APRS_DATA_TYPE_F	DataType;	/* NULL (0) for all */
	SPEED_F		FasterThan;	/* 0 for all */
	SPEED_F		SlowerThan;	/* 0 for all */
	TIMESTAMP_F	StartTime;	/* 0 for all */
	TIMESTAMP_F	EndTime;	/* 0 for all */

	BOOLEAN_F	ReallyAll;
} APRS_PURGE_PACKETS_SRQ;

typedef struct APRS_PURGE_PACKETS_SRP
{	COUNT_F		Count;
} APRS_PURGE_PACKETS_SRP;

extern DESCRIPTOR_S DAPRS_PURGE_PACKETS_SRQ[];
extern DESCRIPTOR_S DAPRS_PURGE_PACKETS_SRP[];

typedef struct APRS_QUERY_PACKETS_SRQ
{	INTEGER_ID_F	PacketIndex;	/* 0 for all */
	STATION_ID_F	OriginID;	/* NULL for all */
	STATION_ID_F	DestID;		/* NULL for all */
	STATION_ID_F	RelayID;	/* NULL for all */
	STATION_ID_F	IGateID;	/* NULL for all */

	INTEGER_ID_F	OriginIndex;	/* Use OriginID if 0 */
	INTEGER_ID_F	DestIndex;	/* Use Origin/DestID if 0 */
	INTEGER_ID_F	RelayIndex;	/* Use RelayID if 0 */
	INTEGER_ID_F	HeardIndex;	/* Use IGateID if 0 */
	BOOLEAN_F	HeardOnRF;	/* TRUE for RF, FALSE for APRS-IS */

	APRS_DATA_TYPE_F	DataType;	/* NULL (0) for all */
	SPEED_F		FasterThan;	/* 0 for all */
	SPEED_F		SlowerThan;	/* 0 for all */
	TIMESTAMP_F	StartTime;	/* 0 for all */
	TIMESTAMP_F	EndTime;	/* 0 for all */
	BOOLEAN_F	ReallyAll;
	BOOLEAN_F	IncludeHopCount;	/* TRUE to populate HopCount */
} APRS_QUERY_PACKETS_SRQ;

typedef struct APRS_QUERY_PACKETS_SRP
{	COORDINATE_S	Min, Max;	/* Bounding Box Coordinates */
	COUNT_F	Count;
	struct
	{
		TIMESTAMP_F	When;
		STATION_ID_F	OriginID;
		APRS_DATA_TYPE_F	DataType;
		STATION_ID_F	DestID;		/* Bogus for MIC-E packets */
		COORDINATE_S	Where;
		ALTITUDE_F	Altitude;
		DISTANCE_F	Distance;
		COUNT_F		Seconds;
		SPEED_F		Speed;
		BEARING_F	Bearing;
		STATION_ID_F	IGateID;
		COORDINATE_S	IGateLoc;
		COUNT_F		HopCount;	/* If IncludeHopCount */
		COUNT_F		LogOffset;	/* Byte offset within When-based log file */
		INTEGER_ID_F	PacketIndex;
		INTEGER_ID_F	OriginIndex;
		INTEGER_ID_F	DestIndex;
		INTEGER_ID_F	HeardIndex;
		BOOLEAN_F	HeardOnRF;	/* TRUE if RF packet, FALSE if APRS-IS packet */
		INTEGER_ID_F	PositionIndex;
	} Packets[1];
} APRS_QUERY_PACKETS_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_PACKETS_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_PACKETS_SRP[];

typedef struct APRS_DELETE_PACKETS_SRQ
{	INTEGER_ID_F	PacketIndex;	/* 0 for all */
	STATION_ID_F	OriginID;	/* NULL for all */
	STATION_ID_F	DestID;		/* NULL for all */
	STATION_ID_F	RelayID;	/* NULL for all */
	STATION_ID_F	IGateID;	/* NULL for all */

	INTEGER_ID_F	OriginIndex;	/* Use OriginID if 0 */
	INTEGER_ID_F	DestIndex;	/* Use Origin/DestID if 0 */
	INTEGER_ID_F	HeardIndex;	/* Use IGateID if 0 */
	BOOLEAN_F	HeardOnRF;	/* TRUE for RF, FALSE for APRS-IS */

	APRS_DATA_TYPE_F	DataType;	/* NULL (0) for all */
	TIMESTAMP_F	StartTime;	/* 0 for all */
	TIMESTAMP_F	EndTime;	/* 0 for all */
} APRS_DELETE_PACKETS_SRQ;

typedef struct APRS_DELETE_PACKETS_SRP
{	COUNT_F Count;
} APRS_DELETE_PACKETS_SRP;

extern DESCRIPTOR_S DAPRS_DELETE_PACKETS_SRQ[];
extern DESCRIPTOR_S DAPRS_DELETE_PACKETS_SRP[];

typedef struct APRS_QUERY_HOPS_SRQ
{	INTEGER_ID_F	PacketIndex;	/* 0 for any */
	INDEX_F		Sequence;	/* 0 for all */
	INTEGER_ID_F	StationIndex;	/* 0 for all */
	BOOLEAN_F	ReallyAll;
} APRS_QUERY_HOPS_SRQ;

typedef struct APRS_QUERY_HOPS_SRP
{	COORDINATE_S	Min, Max;	/* Bounding Box Coordinates */
	COUNT_F		Count;
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

typedef struct APRS_LOOKUP_STATION_SRQ
{	INTEGER_ID_F	StationIndex;	/* Use Owner/Station if 0 */
	STATION_ID_F	OwnerID;	/* Use Station if null */
	STATION_ID_F	StationID;
} APRS_LOOKUP_STATION_SRQ;

typedef struct APRS_LOOKUP_STATION_SRP
{	INTEGER_ID_F	StationIndex;
	STATION_ID_F	OwnerID;
	STATION_ID_F	StationID;
	APRS_SYMBOL_F	Symbol;
	STRING_F	SymbolName;
	TIMESTAMP_F	FirstHeard;
	TIMESTAMP_F	LastHeard;
	TIMESTAMP_F	LastPosition;
	COORDINATE_S	Last;
	ALTITUDE_F	Altitude;
	TIMESTAMP_F	LastMotion;
	BEARING_F	LastBearing;
	SPEED_F		LastSpeed;
	DISTANCE_F	Odometer;
	COUNT_F		PacketsReceived;
	COUNT_F		PacketsHeard;
	COUNT_F		PacketsIGated;
	TIMESTAMP_F	LastIGate;
	COUNT_F		RFPacketsIGated;
	TIMESTAMP_F	LastRFIGate;
	INTEGER_ID_F	PositionIndex;
} APRS_LOOKUP_STATION_SRP;

extern DESCRIPTOR_S DAPRS_LOOKUP_STATION_SRQ[];
extern DESCRIPTOR_S DAPRS_LOOKUP_STATION_SRP[];

typedef struct APRS_QUERY_STATIONS_SRQ
{	TIMESTAMP_F	NewerThan;	/* 0 for all, FirstHeard after this time */
	TIMESTAMP_F	HeardSince;	/* 0 for all, LastHeard after this time */
	TIMESTAMP_F	InactiveSince;	/* 0 for all, LastHeard before this time */
	TIMESTAMP_F	MovedSince;	/* -1 for non-movers, 0 for all, others LastMotion after this time */
	SPEED_F		FasterThan;	/* 0 for all */
	DISTANCE_F	FurtherThan;	/* 0 for all */
	TIMESTAMP_F	IGateSince;	/* 0 for all, LastIGate after this time */
	TIMESTAMP_F	RFIGateSince;	/* 0 for all, LastRFIGate after this time */
	BOOLEAN_F	ReallyAll;
} APRS_QUERY_STATIONS_SRQ;

typedef struct APRS_QUERY_STATIONS_SRP
{	COUNT_F	Count;
	struct
	{	STATION_ID_F	OwnerID;
		STATION_ID_F	StationID;
		INTEGER_ID_F	StationIndex;
		APRS_SYMBOL_F	Symbol;
		TIMESTAMP_F	FirstHeard;
		COORDINATE_S	First;
		TIMESTAMP_F	LastHeard;
		COORDINATE_S	Last;
		TIMESTAMP_F	LastMotion;
		BEARING_F	LastBearing;
		SPEED_F		LastSpeed;
		DISTANCE_F	Odometer;
		COUNT_F		PacketsReceived;
		COUNT_F		PacketsIGated;
		TIMESTAMP_F	LastIGate;
		COUNT_F		RFPacketsIGated;
		TIMESTAMP_F	LastRFIGate;
		INTEGER_ID_F	PositionIndex;
	} Stations[1];
} APRS_QUERY_STATIONS_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_STATIONS_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_STATIONS_SRP[];

typedef struct APRS_QUERY_STATION_INDICES_SRQ
{	STATION_ID_F	OwnerID;	/* May be NULL */
	STATION_ID_F	StationID;	/* May be NULL */
	BOOLEAN_F	ReallyAll;
} APRS_QUERY_STATION_INDICES_SRQ;

typedef struct APRS_QUERY_STATION_INDICES_SRP
{	COUNT_F Count;
	APRS_LOOKUP_STATION_SRP Stations[1];
} APRS_QUERY_STATION_INDICES_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_STATION_INDICES_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_STATION_INDICES_SRP[];

typedef struct APRS_QUERY_UNIQUE_IGATES_SRQ
{	TIMESTAMP_F	StartTime;	/* 0 for all */
	TIMESTAMP_F	EndTime;	/* 0 for all */
	BOOLEAN_F	OnlyKnown;	/* TRUE filters unknown stations */
	DISTANCE_F	MaxMotion;	/* Max motion in any given packet */
	BOOLEAN_F	ReallyAll;
} APRS_QUERY_UNIQUE_IGATES_SRQ;

typedef struct APRS_QUERY_UNIQUE_IGATES_SRA
{	STATION_ID_F	OwnerID;
	STATION_ID_F	StationID;
	APRS_SYMBOL_F	Symbol;
	COUNT_F		PacketsGated;
	TIMESTAMP_F	FirstHeard;
	COORDINATE_S	First;
	TIMESTAMP_F	LastHeard;
	COORDINATE_S	Last;
	TIMESTAMP_F	LastMotion;
	BEARING_F	LastBearing;
	SPEED_F		LastSpeed;
	DISTANCE_F	Odometer;
	COUNT_F		PacketsReceived;
	COUNT_F		PacketsIGated;
	TIMESTAMP_F	LastIGate;
	COUNT_F		RFPacketsIGated;
	TIMESTAMP_F	LastRFIGate;
	INTEGER_ID_F	PositionIndex;
	INTEGER_ID_F	StationIndex;
	DISTANCE_F	MaxDistance;	/* Between Packets */
} APRS_QUERY_UNIQUE_IGATES_SRA;

typedef struct APRS_QUERY_UNIQUE_IGATES_SRP
{	COUNT_F	Count;
	APRS_QUERY_UNIQUE_IGATES_SRA Stations[1];
} APRS_QUERY_UNIQUE_IGATES_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_IGATES_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_IGATES_SRA[];
extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_IGATES_SRP[];

typedef struct APRS_QUERY_UNIQUE_PATHS_SRQ
{	STATION_ID_F	OriginID;	/* NULL for all */
	STATION_ID_F	RelayID;	/* NULL for all */
	STATION_ID_F	IGateID;	/* NULL for all */

	INTEGER_ID_F	OriginIndex;	/* Use OriginID if 0 */
	INTEGER_ID_F	RelayIndex;	/* Use RelayID if 0 */
	INTEGER_ID_F	HeardIndex;	/* Use IGateID if 0 */
	BOOLEAN_F	HeardOnRF;	/* TRUE for RF, FALSE for APRS-IS */

	TIMESTAMP_F	StartTime;	/* 0 for all */
	TIMESTAMP_F	EndTime;	/* 0 for all */
	BOOLEAN_F	ReallyAll;
	BOOLEAN_F	DirectOnly;	/* Origin is From, IGate is To, Relay is Either */
	BOOLEAN_F	IncludeInvalidStations;	/* TRUE to include ALL stations (but not too far out) */
	DISTANCE_F	MaxHopDistance;	/* Maximum distance of single hop (default=400 miles) */
} APRS_QUERY_UNIQUE_PATHS_SRQ;

typedef struct APRS_QUERY_UNIQUE_PATHS_SRA
{	STATION_ID_F	FromStationID;
	STATION_ID_F	ToStationID;
	BOOLEAN_F	Relay;	/* From is Relay */
	BOOLEAN_F	IGate;	/* To is IGate */
	COUNT_F		UseCount;
	TIMESTAMP_F	LastHeard;
	TIMESTAMP_F	FirstHeard;
	DISTANCE_F	Distance;
	BEARING_F	Bearing;
	COORDINATE_S	From;
	COORDINATE_S	To;
	INTEGER_ID_F	FromIndex;
	INTEGER_ID_F	FromPosIndex;
	INTEGER_ID_F	ToIndex;
	INTEGER_ID_F	ToPosIndex;
} APRS_QUERY_UNIQUE_PATHS_SRA;

typedef struct APRS_QUERY_UNIQUE_PATHS_SRP
{	COORDINATE_S	Min, Max;	/* Bounding Box Coordinates */
	struct
	{	COORDINATE_S	Min, Max;	/* Bounding Box Coordinates */
	} Actual;
	COUNT_F	Count;
	APRS_QUERY_UNIQUE_PATHS_SRA Paths[1];
} APRS_QUERY_UNIQUE_PATHS_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_PATHS_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_PATHS_SRA[];
extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_PATHS_SRP[];

typedef struct APRS_QUERY_UNIQUE_ALT_PATHS_SRQ
{	STATION_ID_F	OriginID;	/* NULL for all */
	STATION_ID_F	RelayID;	/* NULL for all */
	STATION_ID_F	IGateID;	/* NULL for all */

	INTEGER_ID_F	OriginIndex;	/* Use OriginID if 0 */
	INTEGER_ID_F	RelayIndex;	/* Use RelayID if 0 */
	INTEGER_ID_F	HeardIndex;	/* Use IGateID if 0 */
	BOOLEAN_F	HeardOnRF;	/* TRUE for RF, FALSE for APRS-IS */

	TIMESTAMP_F	StartTime;	/* 0 for all */
	TIMESTAMP_F	EndTime;	/* 0 for all */
	BOOLEAN_F	ReallyAll;
	BOOLEAN_F	DirectOnly;	/* Origin is From, IGate is To, Relay is Either */
	BOOLEAN_F	IncludeInvalidStations;	/* TRUE to include ALL stations (but not too far out) */
	DISTANCE_F	MaxHopDistance;	/* Maximum distance of single hop (default=400 miles) */
	
	ALTITUDE_F	MinAltitude, MaxAltitude;	/* Min/Max Altitude (0,0 = no check) */
	
} APRS_QUERY_UNIQUE_ALT_PATHS_SRQ;

typedef APRS_QUERY_UNIQUE_PATHS_SRA APRS_QUERY_UNIQUE_ALT_PATHS_SRA;
typedef APRS_QUERY_UNIQUE_PATHS_SRP APRS_QUERY_UNIQUE_ALT_PATHS_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_ALT_PATHS_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_ALT_PATHS_SRA[];
extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_ALT_PATHS_SRP[];

typedef struct APRS_QUERY_UNIQUE_DESTINATIONS_SRQ
{	TIMESTAMP_F	StartTime;	/* 0 for all */
	TIMESTAMP_F	EndTime;	/* 0 for all */
	STATION_ID_F	StartsWith;	/* NULL for all */
	BOOLEAN_F	ReallyAll;
} APRS_QUERY_UNIQUE_DESTINATIONS_SRQ;

typedef struct APRS_QUERY_UNIQUE_DESTINATIONS_SRA
{	STATION_ID_F	DestID;		/* Bogus for MIC-E packets */
	APRS_DATA_TYPE_F	DataType;
	COUNT_F		PacketsReceived;
} APRS_QUERY_UNIQUE_DESTINATIONS_SRA;

typedef struct APRS_QUERY_UNIQUE_DESTINATIONS_SRP
{	COUNT_F	Count;
	APRS_QUERY_UNIQUE_DESTINATIONS_SRA Stations[1];
} APRS_QUERY_UNIQUE_DESTINATIONS_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_DESTINATIONS_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_DESTINATIONS_SRA[];
extern DESCRIPTOR_S DAPRS_QUERY_UNIQUE_DESTINATIONS_SRP[];

typedef struct APRS_QUERY_USERS_SRQ
{	STATION_ID_F	StationID;	/* For one specific station */
	TIMESTAMP_F	NewerThan;	/* 0 for all, FirstHeard after this time */
	TIMESTAMP_F	InactiveSince;	/* 0 for all, LastHeard before this time */
	TIMESTAMP_F	ActiveSince;	/* 0 for all, LastHeard after this time */
	TIMESTAMP_F	RegisteredSince;	/* 0 for all, -1 for never, 1 for IS, Other is since */
	BOOLEAN_F	ReallyAll;
} APRS_QUERY_USERS_SRQ;

typedef struct APRS_QUERY_USERS_SRP
{	COUNT_F	Count;
	struct
	{	STATION_ID_F	StationID;
		TIMESTAMP_F	FirstHeard;
		COORDINATE_S	First;
		TIMESTAMP_F	LastHeard;
		COORDINATE_S	Last;
		VERSION_F	Version;
		TIMESTAMP_F	Registered;
		COUNT_F		PacketsReceived;
	} Users[1];
} APRS_QUERY_USERS_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_USERS_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_USERS_SRP[];

typedef struct APRS_REGISTER_USER_SRQ
{	STATION_ID_F	StationID;	/* For one specific station */
	TIMESTAMP_F	When;		/* 0 for Now */
} APRS_REGISTER_USER_SRQ;

typedef struct APRS_REGISTER_USER_SRP
{	STRING_F	Password;
} APRS_REGISTER_USER_SRP;

extern DESCRIPTOR_S DAPRS_REGISTER_USER_SRQ[];
extern DESCRIPTOR_S DAPRS_REGISTER_USER_SRP[];

typedef struct APRS_QUERY_USER_PACKETS_SRQ
{	STATION_ID_F	StationID;	/* NULL for all */
	TIMESTAMP_F	StartTime;	/* 0 for all */
	TIMESTAMP_F	EndTime;	/* 0 for all */

	BOOLEAN_F	ReallyAll;
} APRS_QUERY_USER_PACKETS_SRQ;

typedef struct APRS_QUERY_USER_PACKETS_SRP
{	COUNT_F	Count;
	struct
	{	STATION_ID_F	StationID;
		TIMESTAMP_F	When;
		COORDINATE_S	Where;
		IP_ADDRESS_F	IPAddress;
		DNS_NAME_F	ReverseDNS;
	} Packets[1];
} APRS_QUERY_USER_PACKETS_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_USER_PACKETS_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_USER_PACKETS_SRP[];

typedef struct APRS_PURGE_USER_PACKETS_SRQ
{	STATION_ID_F	StationID;	/* NULL for all */
	TIMESTAMP_F	StartTime;	/* 0 for all */
	TIMESTAMP_F	EndTime;	/* 0 for all */

	BOOLEAN_F	ReallyAll;
} APRS_PURGE_USER_PACKETS_SRQ;

typedef struct APRS_PURGE_USER_PACKETS_SRP
{	COUNT_F	Count;
} APRS_PURGE_USER_PACKETS_SRP;

extern DESCRIPTOR_S DAPRS_PURGE_USER_PACKETS_SRQ[];
extern DESCRIPTOR_S DAPRS_PURGE_USER_PACKETS_SRP[];

typedef struct APRS_QUERY_TRAFFIC_RATES_SRQ
{	STATION_ID_F	OriginID;	/* NULL for all */
	STATION_ID_F	RelayID;	/* NULL for all */
	STATION_ID_F	IGateID;	/* NULL for all */
	INTEGER_ID_F	OriginIndex;	/* Use OriginID if 0 */
	INTEGER_ID_F	RelayIndex;	/* Use RelayID if 0 */
	INTEGER_ID_F	HeardIndex;	/* Use IGateID if 0 */
	BOOLEAN_F	HeardOnRF;	/* TRUE for RF, FALSE for APRS-IS */
	TIMESTAMP_F	StartTime;	/* 0 for all */
	TIMESTAMP_F	EndTime;	/* 0 for all */
	COUNT_F		BucketSize;	/* Must NOT be zero - in Minutes */
	BOOLEAN_F	IncludeEmpties;	/* TRUE to flesh out buckets from Start (or earliest) to End (or latest) */
	BOOLEAN_F	ReallyAll;
} APRS_QUERY_TRAFFIC_RATES_SRQ;

typedef struct APRS_QUERY_TRAFFIC_RATES_SRA
{	TIMESTAMP_F	StartTime;
	COUNT_F		PacketCount;
} APRS_QUERY_TRAFFIC_RATES_SRA;

typedef struct APRS_QUERY_TRAFFIC_RATES_SRP
{	COUNT_F Total;
	COUNT_F	Count;
	APRS_QUERY_TRAFFIC_RATES_SRA Rates[1];
} APRS_QUERY_TRAFFIC_RATES_SRP;

extern DESCRIPTOR_S DAPRS_QUERY_TRAFFIC_RATES_SRQ[];
extern DESCRIPTOR_S DAPRS_QUERY_TRAFFIC_RATES_SRA[];
extern DESCRIPTOR_S DAPRS_QUERY_TRAFFIC_RATES_SRP[];

typedef struct APRS_CALC_OSM_TILES_SRQ
{	COORDINATE_S	Min, Max;	/* Bounding Box Coordinates */
	COUNT_F		Zoom;		/* Forced zoom for calculations, 0 for MaxTiles */
	COUNT_F		MaxTiles;	/* Maximum tiles (sets zoom level) */
	COUNT_F		TopMargin;	/* Margins are in Pixels */
	COUNT_F		LeftMargin;
	COUNT_F		BottomMargin;
	COUNT_F		RightMargin;
} APRS_CALC_OSM_TILES_SRQ;

typedef struct APRS_CALC_OSM_TILES_SRP
{	COORDINATE_S	Min, Max;	/* Actual (rounded out) Bounding Box Coordinates */
	COUNT_F		Zoom;		/* Resulting zoom level */
	COUNT_F		TileCount;	/* Total Tile Count */
	COUNT_F		StartX, EndX;	/* Range for first part of URL */
	COUNT_F		StartY, EndY;	/* Range for second part of URL */
	COORDINATE_F	DeltaLongitude;	/* Longitude coverage per X */
	COUNT_F		Count;
	struct
	{	COUNT_F		Y;	/* Y of Tile */
		COORDINATE_F	StartLatitude;	/* Bottom Latitude */
		COORDINATE_F	EndLatitude;	/* Top Latitude */
		COORDINATE_F	DeltaLatitude;	/* Delta across tile */
	} Tiles[1];
} APRS_CALC_OSM_TILES_SRP;

extern DESCRIPTOR_S DAPRS_CALC_OSM_TILES_SRQ[];
extern DESCRIPTOR_S DAPRS_CALC_OSM_TILES_SRP[];

typedef struct APRS_INJECT_STATION_POSITION_SRQ
{	STRING_F	OwnerID;	/* Use Station if null */
	STRING_F	StationID;
	STRING_F	ToCall;
	BOOLEAN_F	MessagingCapable;
	APRS_SYMBOL_F	Symbol;
	TIMESTAMP_F	Timestamp;
	COORDINATE_S	Pos;
	SPEED_F		Speed;
	BEARING_F	Bearing;
	ALTITUDE_F	Altitude;
	STRING_F	Comment;
} APRS_INJECT_STATION_POSITION_SRQ;

typedef struct APRS_INJECT_STATION_POSITION_SRP
{	STRING_F	Packet;
} APRS_INJECT_STATION_POSITION_SRP;

extern DESCRIPTOR_S DAPRS_INJECT_STATION_POSITION_SRQ[];
extern DESCRIPTOR_S DAPRS_INJECT_STATION_POSITION_SRP[];

#endif /* GOT_APRS_MESSAGES */
