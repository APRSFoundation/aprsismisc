/*
<pre>
	When	Who		What
	080731	L.Deffenbaugh	Adopt from TMS
	080928	L.Deffenbaugh	Complete normalization renovation
	091006	L.Deffenbaugh	Add Version to APRSISCE_USER_S obsoleting 0
</pre>
*/

#ifndef GOT_APRS_DATABASE
#define GOT_APRS_DATABASE

#include <df/base.h>

#include <df/global.h>

#include <db/include/dbdef.h>

#include <aprs/include/aprsdef.h>

typedef struct APRSISCE_USER0_S
{	STATION_ID_F	StationID;
	TIMESTAMP_F	Registered;
	TIMESTAMP_F	FirstHeard;
	COORDINATE_S	First;
	TIMESTAMP_F	LastHeard;
	COORDINATE_S	Last;
	COUNT_F		PacketsReceived;
} APRSISCE_USER0_S;
extern DESCRIPTOR_S DAPRSISCE_USER0_S[];

typedef struct APRSISCE_USER_S
{	STATION_ID_F	StationID;
	TIMESTAMP_F	Registered;
	TIMESTAMP_F	FirstHeard;
	COORDINATE_S	First;
	TIMESTAMP_F	LastHeard;
	COORDINATE_S	Last;
	VERSION_F	Version;
	COUNT_F		PacketsReceived;
} APRSISCE_USER_S;
extern DESCRIPTOR_S DAPRSISCE_USER_S[];
GLOBAL_STORAGE DB_TABLE_S *UserTable INIT(=NULL);

typedef struct USER_PACKET_S
{	STATION_ID_F	StationID;
	TIMESTAMP_F	When;
	COORDINATE_S	Where;
	IP_ADDRESS_F	IPAddress;
	DNS_NAME_F	ReverseDNS;
} USER_PACKET_S;
extern DESCRIPTOR_S DUSER_PACKET_S[];
GLOBAL_STORAGE DB_TABLE_S *UserPktTable INIT(=NULL);

typedef struct STATION_IDS_S
{	STATION_ID_F	OwnerID;
	STATION_ID_F	StationID;
	INTEGER_ID_F	StationIndex;
} STATION_IDS_S;
extern DESCRIPTOR_S DSTATION_IDS_S[];
GLOBAL_STORAGE DB_TABLE_S *StatIDTable INIT(=NULL);

typedef struct POSITION_S
{	INTEGER_ID_F	PositionIndex;
	COORDINATE_S	Where;
	TIMESTAMP_F	Touched;
} POSITION_S;
extern DESCRIPTOR_S DPOSITION_S[];
GLOBAL_STORAGE DB_TABLE_S *PosTable INIT(=NULL);

typedef struct HOP_S
{	INTEGER_ID_F	PacketIndex;
	INDEX_F		Sequence;
	INTEGER_ID_F	StationIndex;
	INTEGER_ID_F	PositionIndex;
	BOOLEAN_F	Used;
} HOP_S;
extern DESCRIPTOR_S DHOP_S[];
GLOBAL_STORAGE DB_TABLE_S *HopTable INIT(=NULL);

typedef struct STATION1_S
{	STATION_ID_F	OwnerID;
	STATION_ID_F	StationID;
	INTEGER_ID_F	StationIndex;
	COORDINATE_S	Last;
	ALTITUDE_F	Altitude;
	APRS_SYMBOL_F	Symbol;
	TIMESTAMP_F	FirstHeard;
	TIMESTAMP_F	LastHeard;
	TIMESTAMP_F	LastPosition;
	TIMESTAMP_F	LastMotion;
	BEARING_F	LastBearing;
	SPEED_F		LastSpeed;
	DISTANCE_F	Odometer;
	COUNT_F		PacketsReceived;
	COUNT_F		PacketsHeard;
	INTEGER_ID_F	PositionIndex;
} STATION1_S;
extern DESCRIPTOR_S DSTATION1_S[];

typedef struct STATION_S
{	STATION_ID_F	OwnerID;
	STATION_ID_F	StationID;
	INTEGER_ID_F	StationIndex;
	COORDINATE_S	Last;
	ALTITUDE_F	Altitude;
	APRS_SYMBOL_F	Symbol;
	TIMESTAMP_F	FirstHeard;
	TIMESTAMP_F	LastHeard;
	TIMESTAMP_F	LastPosition;
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
} STATION_S;
extern DESCRIPTOR_S DSTATION_S[];
GLOBAL_STORAGE DB_TABLE_S *StationTable INIT(=NULL);

typedef struct PACKET_S
{	INTEGER_ID_F	PacketIndex;
	INTEGER_ID_F	OriginIndex;
	INTEGER_ID_F	DestIndex;	/* 0 for MIC-E packets */
	INTEGER_ID_F	HeardIndex;	/* Either Gate or Local Callsign */
	BOOLEAN_F	HeardOnRF;	/* TRUE if RF packet, FALSE if APRS-IS packet */
	APRS_DATA_TYPE_F	DataType;
	TIMESTAMP_F	When;
	INTEGER_ID_F	PositionIndex;	/* 0 if none */
	ALTITUDE_F	Altitude;
	DISTANCE_F	Distance;
	COUNT_F		Seconds;
	SPEED_F		Speed;
	BEARING_F	Bearing;
	COUNT_F		LogOffset;	/* Byte offset within When-based log file */
} PACKET_S;
extern DESCRIPTOR_S DPACKET_S[];
GLOBAL_STORAGE DB_TABLE_S *PacketTable INIT(=NULL);

#endif /* GOT_APRS_DATABASE */

