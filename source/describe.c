#include "describe.exo"

#ifdef INCLUDE_ENUM_TEMPLATE
ENUM_TRANSLATION_S TDATATYPE_V[] = {
	ENUM_TRANS(DATATYPE_LONG, "DATATYPE_LONG"),
	ENUM_TRANS(DATATYPE_SHORT, "DATATYPE_SHORT"),
	ENUM_TRANS(DATATYPE_INT, "DATATYPE_INT"),
	ENUM_TRANS(DATATYPE_DOUBLE, "DATATYPE_DOUBLE"),
	ENUM_TRANS(DATATYPE_FLOAT, "DATATYPE_FLOAT"),
	ENUM_TRANS(DATATYPE_CHAR, "DATATYPE_CHAR"),
	ENUM_TRANS(DATATYPE_STRING, "DATATYPE_STRING"),
	ENUM_TRANS(DATATYPE_ID, "DATATYPE_ID"),
	ENUM_TRANS(DATATYPE_POINTER, "DATATYPE_POINTER"),
	ENUM_TRANS(DATATYPE_TIMESTAMP, "DATATYPE_TIMESTAMP"),
	ENUM_TRANS(DATATYPE_SELF, "DATATYPE_SELF"),
	ENUM_TRANS(DATATYPE_ENUM, "DATATYPE_ENUM"),
	ENUM_TRANS(DATATYPE_STRUCT, "DATATYPE_STRUCT"),
	ENUM_TRANS(DATATYPE_ARRAY, "DATATYPE_ARRAY"),
	ENUM_TRANS(DATATYPE_CUSTOM, "DATATYPE_CUSTOM"),
	ENUM_TRANS(DATATYPE_LAST, "DATATYPE_LAST"),
	ENUM_TRANS(DATATYPE_CONST, "DATATYPE_CONST"),
	ENUM_TRANS(DATATYPE_BOOLEAN, "DATATYPE_BOOLEAN"),
	ENUM_TRANS(DATATYPE_GUID, "DATATYPE_GUID"),
	ENUM_TRANS(DATATYPE_NSTRING, "DATATYPE_NSTRING"),
	ENUM_TRANS(DATATYPE_SIZET, "DATATYPE_SIZET"),
	NULL_ENUM };
#else
extern ENUM_TRANSLATION_S TDATATYPE_V[];
#endif

#ifdef INCLUDE_ENUM_TEMPLATE
ENUM_TRANSLATION_S TCURSOR_TYPE_V[] = {
	ENUM_TRANS(ARRAY_CURSOR, "ARRAY_CURSOR"),
	ENUM_TRANS(LINKED_CURSOR, "LINKED_CURSOR"),
	ENUM_TRANS(CUSTOM_CURSOR, "CUSTOM_CURSOR"),
	ENUM_TRANS(LAST_CURSOR, "LAST_CURSOR"),
	NULL_ENUM };
#else
extern ENUM_TRANSLATION_S TCURSOR_TYPE_V[];
#endif

#ifdef INCLUDE_ENUM_TEMPLATE
ENUM_TRANSLATION_S TDG_COUNTER_TYPE_V[] = {
	ENUM_TRANS(COUNT_COUNTER, "COUNT_COUNTER"),
	ENUM_TRANS(DOUBLE_COUNTER, "DOUBLE_COUNTER"),
	ENUM_TRANS(TIMESTAMP_COUNTER, "TIMESTAMP_COUNTER"),
	ENUM_TRANS(STRUCTURE_COUNTER, "STRUCTURE_COUNTER"),
	NULL_ENUM };
#else
extern ENUM_TRANSLATION_S TDG_COUNTER_TYPE_V[];
#endif

#ifdef INCLUDE_ENUM_TEMPLATE
ENUM_TRANSLATION_S TDB_OPERATOR_V[] = {
	ENUM_TRANS(DB_AND, "DB_AND"),
	ENUM_TRANS(DB_OR, "DB_OR"),
	ENUM_TRANS(DB_COMMA, "DB_COMMA"),
	ENUM_TRANS(DB_PLUS_EQUAL, "DB_PLUS_EQUAL"),
	ENUM_TRANS(DB_MINUS_EQUAL, "DB_MINUS_EQUAL"),
	ENUM_TRANS(DB_TIMES_EQUAL, "DB_TIMES_EQUAL"),
	ENUM_TRANS(DB_DIVIDE_EQUAL, "DB_DIVIDE_EQUAL"),
	ENUM_TRANS(DB_INSENSITIVE, "DB_INSENSITIVE"),
	ENUM_TRANS(DB_EQUAL, "DB_EQUAL"),
	ENUM_TRANS(DB_EQ, "DB_EQ"),
	ENUM_TRANS(DB_NE, "DB_NE"),
	ENUM_TRANS(DB_LE, "DB_LE"),
	ENUM_TRANS(DB_GE, "DB_GE"),
	ENUM_TRANS(DB_LT, "DB_LT"),
	ENUM_TRANS(DB_GT, "DB_GT"),
	NULL_ENUM };
#else
extern ENUM_TRANSLATION_S TDB_OPERATOR_V[];
#endif

#ifdef INCLUDE_ENUM_TEMPLATE
ENUM_TRANSLATION_S TDB_ACTION_V[] = {
	ENUM_TRANS(DB_INSERTALLOWED, "DB_INSERTALLOWED"),
	ENUM_TRANS(DB_INSERT, "DB_INSERT"),
	ENUM_TRANS(DB_UPDATE, "DB_UPDATE"),
	NULL_ENUM };
#else
extern ENUM_TRANSLATION_S TDB_ACTION_V[];
#endif

#define D(f,t) DESCRIBE(TH_DELAY_INFO_S,f,t)
#define DA(f,t,a) DESCRIBEA(TH_DELAY_INFO_S,f,t,a)
#define DC(f,t,c) DESCRIBEC(TH_DELAY_INFO_S,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(TH_DELAY_INFO_S,f,t,c,a)
DESCRIPTOR_S DTH_DELAY_INFO_S[] = {
	SELF_DESCRIPTOR(TH_DELAY_INFO_S),
	D(Name,STRING),
	D(Group,STRING),
	DA(GroupLock,POINTER,"MUTEX_SEMAPHORE_S"),
	D(PassesCompleted,LONG),
	D(LastStart,TIMESTAMP),
	D(LastStop,TIMESTAMP),
	D(LastElapsed,LONG),
	D(Elapsed,LONG),
	D(Active,BOOLEAN),
	D(ETA,TIMESTAMP),
	D(TotalCount,LONG),
	D(CurrentCount,LONG),
	D(PercentComplete,LONG),
	D(msSleep,LONG),
	D(msStart,DOUBLE),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRSISCE_USER0_S,f,t)
#define DA(f,t,a) DESCRIBEA(APRSISCE_USER0_S,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRSISCE_USER0_S,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRSISCE_USER0_S,f,t,c,a)
DESCRIPTOR_S DAPRSISCE_USER0_S[] = {
	SELF_DESCRIPTOR(APRSISCE_USER0_S),
	D(StationID,CHAR),
	D(Registered,TIMESTAMP),
	D(FirstHeard,TIMESTAMP),
	DC(First,STRUCT,2),
	D(First.Latitude,DOUBLE),
	D(First.Longitude,DOUBLE),
	D(LastHeard,TIMESTAMP),
	DC(Last,STRUCT,2),
	D(Last.Latitude,DOUBLE),
	D(Last.Longitude,DOUBLE),
	D(PacketsReceived,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRSISCE_USER_S,f,t)
#define DA(f,t,a) DESCRIBEA(APRSISCE_USER_S,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRSISCE_USER_S,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRSISCE_USER_S,f,t,c,a)
DESCRIPTOR_S DAPRSISCE_USER_S[] = {
	SELF_DESCRIPTOR(APRSISCE_USER_S),
	D(StationID,CHAR),
	D(Registered,TIMESTAMP),
	D(FirstHeard,TIMESTAMP),
	DC(First,STRUCT,2),
	D(First.Latitude,DOUBLE),
	D(First.Longitude,DOUBLE),
	D(LastHeard,TIMESTAMP),
	DC(Last,STRUCT,2),
	D(Last.Latitude,DOUBLE),
	D(Last.Longitude,DOUBLE),
	D(Version,CHAR),
	D(PacketsReceived,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(USER_PACKET_S,f,t)
#define DA(f,t,a) DESCRIBEA(USER_PACKET_S,f,t,a)
#define DC(f,t,c) DESCRIBEC(USER_PACKET_S,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(USER_PACKET_S,f,t,c,a)
DESCRIPTOR_S DUSER_PACKET_S[] = {
	SELF_DESCRIPTOR(USER_PACKET_S),
	D(StationID,CHAR),
	D(When,TIMESTAMP),
	DC(Where,STRUCT,2),
	D(Where.Latitude,DOUBLE),
	D(Where.Longitude,DOUBLE),
	D(IPAddress,CHAR),
	D(ReverseDNS,CHAR),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(STATION_IDS_S,f,t)
#define DA(f,t,a) DESCRIBEA(STATION_IDS_S,f,t,a)
#define DC(f,t,c) DESCRIBEC(STATION_IDS_S,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(STATION_IDS_S,f,t,c,a)
DESCRIPTOR_S DSTATION_IDS_S[] = {
	SELF_DESCRIPTOR(STATION_IDS_S),
	D(OwnerID,CHAR),
	D(StationID,CHAR),
	D(StationIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(POSITION_S,f,t)
#define DA(f,t,a) DESCRIBEA(POSITION_S,f,t,a)
#define DC(f,t,c) DESCRIBEC(POSITION_S,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(POSITION_S,f,t,c,a)
DESCRIPTOR_S DPOSITION_S[] = {
	SELF_DESCRIPTOR(POSITION_S),
	D(PositionIndex,LONG),
	DC(Where,STRUCT,2),
	D(Where.Latitude,DOUBLE),
	D(Where.Longitude,DOUBLE),
	D(Touched,TIMESTAMP),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(HOP_S,f,t)
#define DA(f,t,a) DESCRIBEA(HOP_S,f,t,a)
#define DC(f,t,c) DESCRIBEC(HOP_S,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(HOP_S,f,t,c,a)
DESCRIPTOR_S DHOP_S[] = {
	SELF_DESCRIPTOR(HOP_S),
	D(PacketIndex,LONG),
	D(Sequence,LONG),
	D(StationIndex,LONG),
	D(PositionIndex,LONG),
	D(Used,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(STATION1_S,f,t)
#define DA(f,t,a) DESCRIBEA(STATION1_S,f,t,a)
#define DC(f,t,c) DESCRIBEC(STATION1_S,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(STATION1_S,f,t,c,a)
DESCRIPTOR_S DSTATION1_S[] = {
	SELF_DESCRIPTOR(STATION1_S),
	D(OwnerID,CHAR),
	D(StationID,CHAR),
	D(StationIndex,LONG),
	DC(Last,STRUCT,2),
	D(Last.Latitude,DOUBLE),
	D(Last.Longitude,DOUBLE),
	D(Altitude,DOUBLE),
	D(Symbol,INT),
	D(FirstHeard,TIMESTAMP),
	D(LastHeard,TIMESTAMP),
	D(LastPosition,TIMESTAMP),
	D(LastMotion,TIMESTAMP),
	D(LastBearing,DOUBLE),
	D(LastSpeed,DOUBLE),
	D(Odometer,DOUBLE),
	D(PacketsReceived,LONG),
	D(PacketsHeard,LONG),
	D(PositionIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(STATION_S,f,t)
#define DA(f,t,a) DESCRIBEA(STATION_S,f,t,a)
#define DC(f,t,c) DESCRIBEC(STATION_S,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(STATION_S,f,t,c,a)
DESCRIPTOR_S DSTATION_S[] = {
	SELF_DESCRIPTOR(STATION_S),
	D(OwnerID,CHAR),
	D(StationID,CHAR),
	D(StationIndex,LONG),
	DC(Last,STRUCT,2),
	D(Last.Latitude,DOUBLE),
	D(Last.Longitude,DOUBLE),
	D(Altitude,DOUBLE),
	D(Symbol,INT),
	D(FirstHeard,TIMESTAMP),
	D(LastHeard,TIMESTAMP),
	D(LastPosition,TIMESTAMP),
	D(LastMotion,TIMESTAMP),
	D(LastBearing,DOUBLE),
	D(LastSpeed,DOUBLE),
	D(Odometer,DOUBLE),
	D(PacketsReceived,LONG),
	D(PacketsHeard,LONG),
	D(PacketsIGated,LONG),
	D(LastIGate,TIMESTAMP),
	D(RFPacketsIGated,LONG),
	D(LastRFIGate,TIMESTAMP),
	D(PositionIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(PACKET_S,f,t)
#define DA(f,t,a) DESCRIBEA(PACKET_S,f,t,a)
#define DC(f,t,c) DESCRIBEC(PACKET_S,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(PACKET_S,f,t,c,a)
DESCRIPTOR_S DPACKET_S[] = {
	SELF_DESCRIPTOR(PACKET_S),
	D(PacketIndex,LONG),
	D(OriginIndex,LONG),
	D(DestIndex,LONG),
	D(HeardIndex,LONG),
	D(HeardOnRF,BOOLEAN),
	D(DataType,CHAR),
	D(When,TIMESTAMP),
	D(PositionIndex,LONG),
	D(Altitude,DOUBLE),
	D(Distance,DOUBLE),
	D(Seconds,LONG),
	D(Speed,DOUBLE),
	D(Bearing,DOUBLE),
	D(LogOffset,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_LOG_PACKET_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_LOG_PACKET_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_LOG_PACKET_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_LOG_PACKET_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_LOG_PACKET_SRQ[] = {
	SELF_DESCRIPTOR(APRS_LOG_PACKET_SRQ),
	D(When,TIMESTAMP),
	D(Packet,STRING),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_LOG_PACKET_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_LOG_PACKET_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_LOG_PACKET_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_LOG_PACKET_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_LOG_PACKET_SRP[] = {
	SELF_DESCRIPTOR(APRS_LOG_PACKET_SRP),
	D(Dummy,CHAR),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_LOG_RECEIVED_PACKET_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_LOG_RECEIVED_PACKET_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_LOG_RECEIVED_PACKET_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_LOG_RECEIVED_PACKET_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_LOG_RECEIVED_PACKET_SRQ[] = {
	SELF_DESCRIPTOR(APRS_LOG_RECEIVED_PACKET_SRQ),
	D(When,TIMESTAMP),
	D(ReceivedBy,STRING),
	D(Packet,STRING),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_LOG_RECEIVED_PACKET_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_LOG_RECEIVED_PACKET_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_LOG_RECEIVED_PACKET_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_LOG_RECEIVED_PACKET_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_LOG_RECEIVED_PACKET_SRP[] = {
	SELF_DESCRIPTOR(APRS_LOG_RECEIVED_PACKET_SRP),
	D(Dummy,CHAR),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_LOG_DIGIPEATED_PACKET_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_LOG_DIGIPEATED_PACKET_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_LOG_DIGIPEATED_PACKET_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_LOG_DIGIPEATED_PACKET_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_LOG_DIGIPEATED_PACKET_SRQ[] = {
	SELF_DESCRIPTOR(APRS_LOG_DIGIPEATED_PACKET_SRQ),
	D(When,TIMESTAMP),
	D(ReceivedBy,STRING),
	D(Packet,STRING),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_LOG_DIGIPEATED_PACKET_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_LOG_DIGIPEATED_PACKET_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_LOG_DIGIPEATED_PACKET_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_LOG_DIGIPEATED_PACKET_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_LOG_DIGIPEATED_PACKET_SRP[] = {
	SELF_DESCRIPTOR(APRS_LOG_DIGIPEATED_PACKET_SRP),
	D(Dummy,CHAR),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_REPARSE_LOG_FILE_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_REPARSE_LOG_FILE_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_REPARSE_LOG_FILE_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_REPARSE_LOG_FILE_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_REPARSE_LOG_FILE_SRQ[] = {
	SELF_DESCRIPTOR(APRS_REPARSE_LOG_FILE_SRQ),
	D(Path,CHAR),
	DC(From,STRUCT,2),
	D(From.Latitude,DOUBLE),
	D(From.Longitude,DOUBLE),
	DC(To,STRUCT,2),
	D(To.Latitude,DOUBLE),
	D(To.Longitude,DOUBLE),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_REPARSE_LOG_FILE_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_REPARSE_LOG_FILE_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_REPARSE_LOG_FILE_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_REPARSE_LOG_FILE_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_REPARSE_LOG_FILE_SRP[] = {
	SELF_DESCRIPTOR(APRS_REPARSE_LOG_FILE_SRP),
	D(Processed,LONG),
	D(NullIsland,LONG),
	D(OutOfBounds,LONG),
	D(ParseFail,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_PURGE_PACKETS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_PURGE_PACKETS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_PURGE_PACKETS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_PURGE_PACKETS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_PURGE_PACKETS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_PURGE_PACKETS_SRQ),
	D(OriginID,CHAR),
	D(DestID,CHAR),
	D(RelayID,CHAR),
	D(IGateID,CHAR),
	D(DataType,CHAR),
	D(FasterThan,DOUBLE),
	D(SlowerThan,DOUBLE),
	D(StartTime,TIMESTAMP),
	D(EndTime,TIMESTAMP),
	D(ReallyAll,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_PURGE_PACKETS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_PURGE_PACKETS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_PURGE_PACKETS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_PURGE_PACKETS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_PURGE_PACKETS_SRP[] = {
	SELF_DESCRIPTOR(APRS_PURGE_PACKETS_SRP),
	D(Count,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_HOPS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_HOPS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_HOPS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_HOPS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_HOPS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_HOPS_SRQ),
	D(PacketIndex,LONG),
	D(Sequence,LONG),
	D(StationIndex,LONG),
	D(ReallyAll,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_HOPS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_HOPS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_HOPS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_HOPS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_HOPS_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_HOPS_SRP),
	DC(Min,STRUCT,2),
	D(Min.Latitude,DOUBLE),
	D(Min.Longitude,DOUBLE),
	DC(Max,STRUCT,2),
	D(Max.Latitude,DOUBLE),
	D(Max.Longitude,DOUBLE),
	D(Count,LONG),
	DC(Hops,ARRAY,1),
	DC(Hops[0],STRUCT,9),
	D(Hops[0].PacketIndex,LONG),
	D(Hops[0].Sequence,LONG),
	D(Hops[0].StationIndex,LONG),
	D(Hops[0].PositionIndex,LONG),
	D(Hops[0].Used,BOOLEAN),
	D(Hops[0].StationID,CHAR),
	DC(Hops[0].Where,STRUCT,2),
	D(Hops[0].Where.Latitude,DOUBLE),
	D(Hops[0].Where.Longitude,DOUBLE),
	D(Hops[0].Distance,DOUBLE),
	D(Hops[0].Bearing,DOUBLE),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_PACKETS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_PACKETS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_PACKETS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_PACKETS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_PACKETS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_PACKETS_SRQ),
	D(PacketIndex,LONG),
	D(OriginID,CHAR),
	D(DestID,CHAR),
	D(RelayID,CHAR),
	D(IGateID,CHAR),
	D(OriginIndex,LONG),
	D(DestIndex,LONG),
	D(RelayIndex,LONG),
	D(HeardIndex,LONG),
	D(HeardOnRF,BOOLEAN),
	D(DataType,CHAR),
	D(FasterThan,DOUBLE),
	D(SlowerThan,DOUBLE),
	D(StartTime,TIMESTAMP),
	D(EndTime,TIMESTAMP),
	D(ReallyAll,BOOLEAN),
	D(IncludeHopCount,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_PACKETS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_PACKETS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_PACKETS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_PACKETS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_PACKETS_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_PACKETS_SRP),
	DC(Min,STRUCT,2),
	D(Min.Latitude,DOUBLE),
	D(Min.Longitude,DOUBLE),
	DC(Max,STRUCT,2),
	D(Max.Latitude,DOUBLE),
	D(Max.Longitude,DOUBLE),
	D(Count,LONG),
	DC(Packets,ARRAY,1),
	DC(Packets[0],STRUCT,20),
	D(Packets[0].When,TIMESTAMP),
	D(Packets[0].OriginID,CHAR),
	D(Packets[0].DataType,CHAR),
	D(Packets[0].DestID,CHAR),
	DC(Packets[0].Where,STRUCT,2),
	D(Packets[0].Where.Latitude,DOUBLE),
	D(Packets[0].Where.Longitude,DOUBLE),
	D(Packets[0].Altitude,DOUBLE),
	D(Packets[0].Distance,DOUBLE),
	D(Packets[0].Seconds,LONG),
	D(Packets[0].Speed,DOUBLE),
	D(Packets[0].Bearing,DOUBLE),
	D(Packets[0].IGateID,CHAR),
	DC(Packets[0].IGateLoc,STRUCT,2),
	D(Packets[0].IGateLoc.Latitude,DOUBLE),
	D(Packets[0].IGateLoc.Longitude,DOUBLE),
	D(Packets[0].HopCount,LONG),
	D(Packets[0].LogOffset,LONG),
	D(Packets[0].PacketIndex,LONG),
	D(Packets[0].OriginIndex,LONG),
	D(Packets[0].DestIndex,LONG),
	D(Packets[0].HeardIndex,LONG),
	D(Packets[0].HeardOnRF,BOOLEAN),
	D(Packets[0].PositionIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_DELETE_PACKETS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_DELETE_PACKETS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_DELETE_PACKETS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_DELETE_PACKETS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_DELETE_PACKETS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_DELETE_PACKETS_SRQ),
	D(PacketIndex,LONG),
	D(OriginID,CHAR),
	D(DestID,CHAR),
	D(RelayID,CHAR),
	D(IGateID,CHAR),
	D(OriginIndex,LONG),
	D(DestIndex,LONG),
	D(HeardIndex,LONG),
	D(HeardOnRF,BOOLEAN),
	D(DataType,CHAR),
	D(StartTime,TIMESTAMP),
	D(EndTime,TIMESTAMP),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_DELETE_PACKETS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_DELETE_PACKETS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_DELETE_PACKETS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_DELETE_PACKETS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_DELETE_PACKETS_SRP[] = {
	SELF_DESCRIPTOR(APRS_DELETE_PACKETS_SRP),
	D(Count,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_LOOKUP_STATION_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_LOOKUP_STATION_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_LOOKUP_STATION_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_LOOKUP_STATION_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_LOOKUP_STATION_SRQ[] = {
	SELF_DESCRIPTOR(APRS_LOOKUP_STATION_SRQ),
	D(StationIndex,LONG),
	D(OwnerID,CHAR),
	D(StationID,CHAR),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_LOOKUP_STATION_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_LOOKUP_STATION_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_LOOKUP_STATION_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_LOOKUP_STATION_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_LOOKUP_STATION_SRP[] = {
	SELF_DESCRIPTOR(APRS_LOOKUP_STATION_SRP),
	D(StationIndex,LONG),
	D(OwnerID,CHAR),
	D(StationID,CHAR),
	D(Symbol,INT),
	D(SymbolName,STRING),
	D(FirstHeard,TIMESTAMP),
	D(LastHeard,TIMESTAMP),
	D(LastPosition,TIMESTAMP),
	DC(Last,STRUCT,2),
	D(Last.Latitude,DOUBLE),
	D(Last.Longitude,DOUBLE),
	D(Altitude,DOUBLE),
	D(LastMotion,TIMESTAMP),
	D(LastBearing,DOUBLE),
	D(LastSpeed,DOUBLE),
	D(Odometer,DOUBLE),
	D(PacketsReceived,LONG),
	D(PacketsHeard,LONG),
	D(PacketsIGated,LONG),
	D(LastIGate,TIMESTAMP),
	D(RFPacketsIGated,LONG),
	D(LastRFIGate,TIMESTAMP),
	D(PositionIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_STATION_INDICES_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_STATION_INDICES_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_STATION_INDICES_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_STATION_INDICES_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_STATION_INDICES_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_STATION_INDICES_SRQ),
	D(OwnerID,CHAR),
	D(StationID,CHAR),
	D(ReallyAll,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_STATION_INDICES_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_STATION_INDICES_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_STATION_INDICES_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_STATION_INDICES_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_STATION_INDICES_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_STATION_INDICES_SRP),
	D(Count,LONG),
	DC(Stations,ARRAY,1),
	DC(Stations[0],STRUCT,21),
	D(Stations[0].StationIndex,LONG),
	D(Stations[0].OwnerID,CHAR),
	D(Stations[0].StationID,CHAR),
	D(Stations[0].Symbol,INT),
	D(Stations[0].SymbolName,STRING),
	D(Stations[0].FirstHeard,TIMESTAMP),
	D(Stations[0].LastHeard,TIMESTAMP),
	D(Stations[0].LastPosition,TIMESTAMP),
	DC(Stations[0].Last,STRUCT,2),
	D(Stations[0].Last.Latitude,DOUBLE),
	D(Stations[0].Last.Longitude,DOUBLE),
	D(Stations[0].Altitude,DOUBLE),
	D(Stations[0].LastMotion,TIMESTAMP),
	D(Stations[0].LastBearing,DOUBLE),
	D(Stations[0].LastSpeed,DOUBLE),
	D(Stations[0].Odometer,DOUBLE),
	D(Stations[0].PacketsReceived,LONG),
	D(Stations[0].PacketsHeard,LONG),
	D(Stations[0].PacketsIGated,LONG),
	D(Stations[0].LastIGate,TIMESTAMP),
	D(Stations[0].RFPacketsIGated,LONG),
	D(Stations[0].LastRFIGate,TIMESTAMP),
	D(Stations[0].PositionIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_STATIONS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_STATIONS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_STATIONS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_STATIONS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_STATIONS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_STATIONS_SRQ),
	D(NewerThan,TIMESTAMP),
	D(HeardSince,TIMESTAMP),
	D(InactiveSince,TIMESTAMP),
	D(MovedSince,TIMESTAMP),
	D(FasterThan,DOUBLE),
	D(FurtherThan,DOUBLE),
	D(IGateSince,TIMESTAMP),
	D(RFIGateSince,TIMESTAMP),
	D(ReallyAll,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_STATIONS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_STATIONS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_STATIONS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_STATIONS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_STATIONS_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_STATIONS_SRP),
	D(Count,LONG),
	DC(Stations,ARRAY,1),
	DC(Stations[0],STRUCT,18),
	D(Stations[0].OwnerID,CHAR),
	D(Stations[0].StationID,CHAR),
	D(Stations[0].StationIndex,LONG),
	D(Stations[0].Symbol,INT),
	D(Stations[0].FirstHeard,TIMESTAMP),
	DC(Stations[0].First,STRUCT,2),
	D(Stations[0].First.Latitude,DOUBLE),
	D(Stations[0].First.Longitude,DOUBLE),
	D(Stations[0].LastHeard,TIMESTAMP),
	DC(Stations[0].Last,STRUCT,2),
	D(Stations[0].Last.Latitude,DOUBLE),
	D(Stations[0].Last.Longitude,DOUBLE),
	D(Stations[0].LastMotion,TIMESTAMP),
	D(Stations[0].LastBearing,DOUBLE),
	D(Stations[0].LastSpeed,DOUBLE),
	D(Stations[0].Odometer,DOUBLE),
	D(Stations[0].PacketsReceived,LONG),
	D(Stations[0].PacketsIGated,LONG),
	D(Stations[0].LastIGate,TIMESTAMP),
	D(Stations[0].RFPacketsIGated,LONG),
	D(Stations[0].LastRFIGate,TIMESTAMP),
	D(Stations[0].PositionIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_IGATES_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_IGATES_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_IGATES_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_IGATES_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_IGATES_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_IGATES_SRQ),
	D(StartTime,TIMESTAMP),
	D(EndTime,TIMESTAMP),
	D(OnlyKnown,BOOLEAN),
	D(MaxMotion,DOUBLE),
	D(ReallyAll,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_IGATES_SRA,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_IGATES_SRA,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_IGATES_SRA,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_IGATES_SRA,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_IGATES_SRA[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_IGATES_SRA),
	D(OwnerID,CHAR),
	D(StationID,CHAR),
	D(Symbol,INT),
	D(PacketsGated,LONG),
	D(FirstHeard,TIMESTAMP),
	DC(First,STRUCT,2),
	D(First.Latitude,DOUBLE),
	D(First.Longitude,DOUBLE),
	D(LastHeard,TIMESTAMP),
	DC(Last,STRUCT,2),
	D(Last.Latitude,DOUBLE),
	D(Last.Longitude,DOUBLE),
	D(LastMotion,TIMESTAMP),
	D(LastBearing,DOUBLE),
	D(LastSpeed,DOUBLE),
	D(Odometer,DOUBLE),
	D(PacketsReceived,LONG),
	D(PacketsIGated,LONG),
	D(LastIGate,TIMESTAMP),
	D(RFPacketsIGated,LONG),
	D(LastRFIGate,TIMESTAMP),
	D(PositionIndex,LONG),
	D(StationIndex,LONG),
	D(MaxDistance,DOUBLE),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_IGATES_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_IGATES_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_IGATES_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_IGATES_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_IGATES_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_IGATES_SRP),
	D(Count,LONG),
	DC(Stations,ARRAY,1),
	DC(Stations[0],STRUCT,20),
	D(Stations[0].OwnerID,CHAR),
	D(Stations[0].StationID,CHAR),
	D(Stations[0].Symbol,INT),
	D(Stations[0].PacketsGated,LONG),
	D(Stations[0].FirstHeard,TIMESTAMP),
	DC(Stations[0].First,STRUCT,2),
	D(Stations[0].First.Latitude,DOUBLE),
	D(Stations[0].First.Longitude,DOUBLE),
	D(Stations[0].LastHeard,TIMESTAMP),
	DC(Stations[0].Last,STRUCT,2),
	D(Stations[0].Last.Latitude,DOUBLE),
	D(Stations[0].Last.Longitude,DOUBLE),
	D(Stations[0].LastMotion,TIMESTAMP),
	D(Stations[0].LastBearing,DOUBLE),
	D(Stations[0].LastSpeed,DOUBLE),
	D(Stations[0].Odometer,DOUBLE),
	D(Stations[0].PacketsReceived,LONG),
	D(Stations[0].PacketsIGated,LONG),
	D(Stations[0].LastIGate,TIMESTAMP),
	D(Stations[0].RFPacketsIGated,LONG),
	D(Stations[0].LastRFIGate,TIMESTAMP),
	D(Stations[0].PositionIndex,LONG),
	D(Stations[0].StationIndex,LONG),
	D(Stations[0].MaxDistance,DOUBLE),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_DESTINATIONS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_DESTINATIONS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_DESTINATIONS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_DESTINATIONS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_DESTINATIONS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_DESTINATIONS_SRQ),
	D(StartTime,TIMESTAMP),
	D(EndTime,TIMESTAMP),
	D(StartsWith,CHAR),
	D(ReallyAll,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_DESTINATIONS_SRA,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_DESTINATIONS_SRA,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_DESTINATIONS_SRA,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_DESTINATIONS_SRA,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_DESTINATIONS_SRA[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_DESTINATIONS_SRA),
	D(DestID,CHAR),
	D(DataType,CHAR),
	D(PacketsReceived,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_DESTINATIONS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_DESTINATIONS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_DESTINATIONS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_DESTINATIONS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_DESTINATIONS_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_DESTINATIONS_SRP),
	D(Count,LONG),
	DC(Stations,ARRAY,1),
	DC(Stations[0],STRUCT,3),
	D(Stations[0].DestID,CHAR),
	D(Stations[0].DataType,CHAR),
	D(Stations[0].PacketsReceived,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_PATHS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_PATHS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_PATHS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_PATHS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_PATHS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_PATHS_SRQ),
	D(OriginID,CHAR),
	D(RelayID,CHAR),
	D(IGateID,CHAR),
	D(OriginIndex,LONG),
	D(RelayIndex,LONG),
	D(HeardIndex,LONG),
	D(HeardOnRF,BOOLEAN),
	D(StartTime,TIMESTAMP),
	D(EndTime,TIMESTAMP),
	D(ReallyAll,BOOLEAN),
	D(DirectOnly,BOOLEAN),
	D(IncludeInvalidStations,BOOLEAN),
	D(MaxHopDistance,DOUBLE),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_PATHS_SRA,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_PATHS_SRA,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_PATHS_SRA,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_PATHS_SRA,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_PATHS_SRA[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_PATHS_SRA),
	D(FromStationID,CHAR),
	D(ToStationID,CHAR),
	D(Relay,BOOLEAN),
	D(IGate,BOOLEAN),
	D(UseCount,LONG),
	D(LastHeard,TIMESTAMP),
	D(FirstHeard,TIMESTAMP),
	D(Distance,DOUBLE),
	D(Bearing,DOUBLE),
	DC(From,STRUCT,2),
	D(From.Latitude,DOUBLE),
	D(From.Longitude,DOUBLE),
	DC(To,STRUCT,2),
	D(To.Latitude,DOUBLE),
	D(To.Longitude,DOUBLE),
	D(FromIndex,LONG),
	D(FromPosIndex,LONG),
	D(ToIndex,LONG),
	D(ToPosIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_PATHS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_PATHS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_PATHS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_PATHS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_PATHS_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_PATHS_SRP),
	DC(Min,STRUCT,2),
	D(Min.Latitude,DOUBLE),
	D(Min.Longitude,DOUBLE),
	DC(Max,STRUCT,2),
	D(Max.Latitude,DOUBLE),
	D(Max.Longitude,DOUBLE),
	DC(Actual,STRUCT,2),
	DC(Actual.Min,STRUCT,2),
	D(Actual.Min.Latitude,DOUBLE),
	D(Actual.Min.Longitude,DOUBLE),
	DC(Actual.Max,STRUCT,2),
	D(Actual.Max.Latitude,DOUBLE),
	D(Actual.Max.Longitude,DOUBLE),
	D(Count,LONG),
	DC(Paths,ARRAY,1),
	DC(Paths[0],STRUCT,15),
	D(Paths[0].FromStationID,CHAR),
	D(Paths[0].ToStationID,CHAR),
	D(Paths[0].Relay,BOOLEAN),
	D(Paths[0].IGate,BOOLEAN),
	D(Paths[0].UseCount,LONG),
	D(Paths[0].LastHeard,TIMESTAMP),
	D(Paths[0].FirstHeard,TIMESTAMP),
	D(Paths[0].Distance,DOUBLE),
	D(Paths[0].Bearing,DOUBLE),
	DC(Paths[0].From,STRUCT,2),
	D(Paths[0].From.Latitude,DOUBLE),
	D(Paths[0].From.Longitude,DOUBLE),
	DC(Paths[0].To,STRUCT,2),
	D(Paths[0].To.Latitude,DOUBLE),
	D(Paths[0].To.Longitude,DOUBLE),
	D(Paths[0].FromIndex,LONG),
	D(Paths[0].FromPosIndex,LONG),
	D(Paths[0].ToIndex,LONG),
	D(Paths[0].ToPosIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_ALT_PATHS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_ALT_PATHS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_ALT_PATHS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_ALT_PATHS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_ALT_PATHS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_ALT_PATHS_SRQ),
	D(OriginID,CHAR),
	D(RelayID,CHAR),
	D(IGateID,CHAR),
	D(OriginIndex,LONG),
	D(RelayIndex,LONG),
	D(HeardIndex,LONG),
	D(HeardOnRF,BOOLEAN),
	D(StartTime,TIMESTAMP),
	D(EndTime,TIMESTAMP),
	D(ReallyAll,BOOLEAN),
	D(DirectOnly,BOOLEAN),
	D(IncludeInvalidStations,BOOLEAN),
	D(MaxHopDistance,DOUBLE),
	D(MinAltitude,DOUBLE),
	D(MaxAltitude,DOUBLE),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_ALT_PATHS_SRA,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_ALT_PATHS_SRA,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_ALT_PATHS_SRA,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_ALT_PATHS_SRA,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_ALT_PATHS_SRA[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_ALT_PATHS_SRA),
	D(FromStationID,CHAR),
	D(ToStationID,CHAR),
	D(Relay,BOOLEAN),
	D(IGate,BOOLEAN),
	D(UseCount,LONG),
	D(LastHeard,TIMESTAMP),
	D(FirstHeard,TIMESTAMP),
	D(Distance,DOUBLE),
	D(Bearing,DOUBLE),
	DC(From,STRUCT,2),
	D(From.Latitude,DOUBLE),
	D(From.Longitude,DOUBLE),
	DC(To,STRUCT,2),
	D(To.Latitude,DOUBLE),
	D(To.Longitude,DOUBLE),
	D(FromIndex,LONG),
	D(FromPosIndex,LONG),
	D(ToIndex,LONG),
	D(ToPosIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_UNIQUE_ALT_PATHS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_UNIQUE_ALT_PATHS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_UNIQUE_ALT_PATHS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_UNIQUE_ALT_PATHS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_UNIQUE_ALT_PATHS_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_UNIQUE_ALT_PATHS_SRP),
	DC(Min,STRUCT,2),
	D(Min.Latitude,DOUBLE),
	D(Min.Longitude,DOUBLE),
	DC(Max,STRUCT,2),
	D(Max.Latitude,DOUBLE),
	D(Max.Longitude,DOUBLE),
	DC(Actual,STRUCT,2),
	DC(Actual.Min,STRUCT,2),
	D(Actual.Min.Latitude,DOUBLE),
	D(Actual.Min.Longitude,DOUBLE),
	DC(Actual.Max,STRUCT,2),
	D(Actual.Max.Latitude,DOUBLE),
	D(Actual.Max.Longitude,DOUBLE),
	D(Count,LONG),
	DC(Paths,ARRAY,1),
	DC(Paths[0],STRUCT,15),
	D(Paths[0].FromStationID,CHAR),
	D(Paths[0].ToStationID,CHAR),
	D(Paths[0].Relay,BOOLEAN),
	D(Paths[0].IGate,BOOLEAN),
	D(Paths[0].UseCount,LONG),
	D(Paths[0].LastHeard,TIMESTAMP),
	D(Paths[0].FirstHeard,TIMESTAMP),
	D(Paths[0].Distance,DOUBLE),
	D(Paths[0].Bearing,DOUBLE),
	DC(Paths[0].From,STRUCT,2),
	D(Paths[0].From.Latitude,DOUBLE),
	D(Paths[0].From.Longitude,DOUBLE),
	DC(Paths[0].To,STRUCT,2),
	D(Paths[0].To.Latitude,DOUBLE),
	D(Paths[0].To.Longitude,DOUBLE),
	D(Paths[0].FromIndex,LONG),
	D(Paths[0].FromPosIndex,LONG),
	D(Paths[0].ToIndex,LONG),
	D(Paths[0].ToPosIndex,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_USERS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_USERS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_USERS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_USERS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_USERS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_USERS_SRQ),
	D(StationID,CHAR),
	D(NewerThan,TIMESTAMP),
	D(InactiveSince,TIMESTAMP),
	D(ActiveSince,TIMESTAMP),
	D(RegisteredSince,TIMESTAMP),
	D(ReallyAll,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_USERS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_USERS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_USERS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_USERS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_USERS_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_USERS_SRP),
	D(Count,LONG),
	DC(Users,ARRAY,1),
	DC(Users[0],STRUCT,8),
	D(Users[0].StationID,CHAR),
	D(Users[0].FirstHeard,TIMESTAMP),
	DC(Users[0].First,STRUCT,2),
	D(Users[0].First.Latitude,DOUBLE),
	D(Users[0].First.Longitude,DOUBLE),
	D(Users[0].LastHeard,TIMESTAMP),
	DC(Users[0].Last,STRUCT,2),
	D(Users[0].Last.Latitude,DOUBLE),
	D(Users[0].Last.Longitude,DOUBLE),
	D(Users[0].Version,CHAR),
	D(Users[0].Registered,TIMESTAMP),
	D(Users[0].PacketsReceived,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_REGISTER_USER_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_REGISTER_USER_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_REGISTER_USER_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_REGISTER_USER_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_REGISTER_USER_SRQ[] = {
	SELF_DESCRIPTOR(APRS_REGISTER_USER_SRQ),
	D(StationID,CHAR),
	D(When,TIMESTAMP),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_REGISTER_USER_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_REGISTER_USER_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_REGISTER_USER_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_REGISTER_USER_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_REGISTER_USER_SRP[] = {
	SELF_DESCRIPTOR(APRS_REGISTER_USER_SRP),
	D(Password,STRING),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_USER_PACKETS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_USER_PACKETS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_USER_PACKETS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_USER_PACKETS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_USER_PACKETS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_USER_PACKETS_SRQ),
	D(StationID,CHAR),
	D(StartTime,TIMESTAMP),
	D(EndTime,TIMESTAMP),
	D(ReallyAll,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_USER_PACKETS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_USER_PACKETS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_USER_PACKETS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_USER_PACKETS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_USER_PACKETS_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_USER_PACKETS_SRP),
	D(Count,LONG),
	DC(Packets,ARRAY,1),
	DC(Packets[0],STRUCT,5),
	D(Packets[0].StationID,CHAR),
	D(Packets[0].When,TIMESTAMP),
	DC(Packets[0].Where,STRUCT,2),
	D(Packets[0].Where.Latitude,DOUBLE),
	D(Packets[0].Where.Longitude,DOUBLE),
	D(Packets[0].IPAddress,CHAR),
	D(Packets[0].ReverseDNS,CHAR),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_PURGE_USER_PACKETS_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_PURGE_USER_PACKETS_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_PURGE_USER_PACKETS_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_PURGE_USER_PACKETS_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_PURGE_USER_PACKETS_SRQ[] = {
	SELF_DESCRIPTOR(APRS_PURGE_USER_PACKETS_SRQ),
	D(StationID,CHAR),
	D(StartTime,TIMESTAMP),
	D(EndTime,TIMESTAMP),
	D(ReallyAll,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_PURGE_USER_PACKETS_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_PURGE_USER_PACKETS_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_PURGE_USER_PACKETS_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_PURGE_USER_PACKETS_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_PURGE_USER_PACKETS_SRP[] = {
	SELF_DESCRIPTOR(APRS_PURGE_USER_PACKETS_SRP),
	D(Count,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_TRAFFIC_RATES_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_TRAFFIC_RATES_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_TRAFFIC_RATES_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_TRAFFIC_RATES_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_TRAFFIC_RATES_SRQ[] = {
	SELF_DESCRIPTOR(APRS_QUERY_TRAFFIC_RATES_SRQ),
	D(OriginID,CHAR),
	D(RelayID,CHAR),
	D(IGateID,CHAR),
	D(OriginIndex,LONG),
	D(RelayIndex,LONG),
	D(HeardIndex,LONG),
	D(HeardOnRF,BOOLEAN),
	D(StartTime,TIMESTAMP),
	D(EndTime,TIMESTAMP),
	D(BucketSize,LONG),
	D(IncludeEmpties,BOOLEAN),
	D(ReallyAll,BOOLEAN),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_TRAFFIC_RATES_SRA,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_TRAFFIC_RATES_SRA,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_TRAFFIC_RATES_SRA,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_TRAFFIC_RATES_SRA,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_TRAFFIC_RATES_SRA[] = {
	SELF_DESCRIPTOR(APRS_QUERY_TRAFFIC_RATES_SRA),
	D(StartTime,TIMESTAMP),
	D(PacketCount,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_QUERY_TRAFFIC_RATES_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_QUERY_TRAFFIC_RATES_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_QUERY_TRAFFIC_RATES_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_QUERY_TRAFFIC_RATES_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_QUERY_TRAFFIC_RATES_SRP[] = {
	SELF_DESCRIPTOR(APRS_QUERY_TRAFFIC_RATES_SRP),
	D(Total,LONG),
	D(Count,LONG),
	DC(Rates,ARRAY,1),
	DC(Rates[0],STRUCT,2),
	D(Rates[0].StartTime,TIMESTAMP),
	D(Rates[0].PacketCount,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_CALC_OSM_TILES_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_CALC_OSM_TILES_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_CALC_OSM_TILES_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_CALC_OSM_TILES_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_CALC_OSM_TILES_SRQ[] = {
	SELF_DESCRIPTOR(APRS_CALC_OSM_TILES_SRQ),
	DC(Min,STRUCT,2),
	D(Min.Latitude,DOUBLE),
	D(Min.Longitude,DOUBLE),
	DC(Max,STRUCT,2),
	D(Max.Latitude,DOUBLE),
	D(Max.Longitude,DOUBLE),
	D(Zoom,LONG),
	D(MaxTiles,LONG),
	D(TopMargin,LONG),
	D(LeftMargin,LONG),
	D(BottomMargin,LONG),
	D(RightMargin,LONG),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_CALC_OSM_TILES_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_CALC_OSM_TILES_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_CALC_OSM_TILES_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_CALC_OSM_TILES_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_CALC_OSM_TILES_SRP[] = {
	SELF_DESCRIPTOR(APRS_CALC_OSM_TILES_SRP),
	DC(Min,STRUCT,2),
	D(Min.Latitude,DOUBLE),
	D(Min.Longitude,DOUBLE),
	DC(Max,STRUCT,2),
	D(Max.Latitude,DOUBLE),
	D(Max.Longitude,DOUBLE),
	D(Zoom,LONG),
	D(TileCount,LONG),
	D(StartX,LONG),
	D(EndX,LONG),
	D(StartY,LONG),
	D(EndY,LONG),
	D(DeltaLongitude,DOUBLE),
	D(Count,LONG),
	DC(Tiles,ARRAY,1),
	DC(Tiles[0],STRUCT,4),
	D(Tiles[0].Y,LONG),
	D(Tiles[0].StartLatitude,DOUBLE),
	D(Tiles[0].EndLatitude,DOUBLE),
	D(Tiles[0].DeltaLatitude,DOUBLE),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_INJECT_STATION_POSITION_SRQ,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_INJECT_STATION_POSITION_SRQ,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_INJECT_STATION_POSITION_SRQ,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_INJECT_STATION_POSITION_SRQ,f,t,c,a)
DESCRIPTOR_S DAPRS_INJECT_STATION_POSITION_SRQ[] = {
	SELF_DESCRIPTOR(APRS_INJECT_STATION_POSITION_SRQ),
	D(OwnerID,STRING),
	D(StationID,STRING),
	D(ToCall,STRING),
	D(MessagingCapable,BOOLEAN),
	D(Symbol,INT),
	D(Timestamp,TIMESTAMP),
	DC(Pos,STRUCT,2),
	D(Pos.Latitude,DOUBLE),
	D(Pos.Longitude,DOUBLE),
	D(Speed,DOUBLE),
	D(Bearing,DOUBLE),
	D(Altitude,DOUBLE),
	D(Comment,STRING),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

#define D(f,t) DESCRIBE(APRS_INJECT_STATION_POSITION_SRP,f,t)
#define DA(f,t,a) DESCRIBEA(APRS_INJECT_STATION_POSITION_SRP,f,t,a)
#define DC(f,t,c) DESCRIBEC(APRS_INJECT_STATION_POSITION_SRP,f,t,c)
#define DCA(f,t,c,a) DESCRIBECA(APRS_INJECT_STATION_POSITION_SRP,f,t,c,a)
DESCRIPTOR_S DAPRS_INJECT_STATION_POSITION_SRP[] = {
	SELF_DESCRIPTOR(APRS_INJECT_STATION_POSITION_SRP),
	D(Packet,STRING),
	NULL_DESCRIPTOR };
#undef DCA
#undef DC
#undef DA
#undef D

