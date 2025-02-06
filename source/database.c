#define NEW_WAY
/*
<pre>
	When	Who		What
	080731	L.Deffenbaugh	Initial Implementation
	091006	L.Deffenbaugh	Add Version to APRSISCE_USER_S obsoleting 0
</pre>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <df/base.h>

#include <uf/include/base.h>

#include <uf/source/cfrtns.h>
#include <uf/source/dgprint.h>
#include <uf/source/dsrtns.h>
#include <uf/source/flrtns.h>
#include <uf/source/hprtns.h>
#include <uf/source/rtrtns.h>
#include <uf/source/smrtns.h>
#include <uf/source/syrtns.h>
#include <uf/source/thrtns.h>

#include <ci/include/cidef.h>
#include <ci/source/ciconv.h>

#include <db/include/dbdef.h>
#include <db/source/database.h>
#include <db/source/dbaccess.h>

#include <aprs/include/database.h>

#include <aprs/source/database.h>

static BOOLEAN_F AprsBuildPositionHash(POINTER_F DbRecord,  POINTER_F UserArg)
{	POSITION_S *p = DbRecord;
	HASH_S *Hash = UserArg;

	if (!DsInsertHashKey(Hash, &p->PositionIndex, (POINTER_F) p->PositionIndex, HERE))
		DgPrintf("Failed To Hash Position %ld\n", (long) p->PositionIndex);

	return FALSE;
}

static BOOLEAN_F AprsCheckHopPositionUsage(POINTER_F DbRecord,  POINTER_F UserArg)
{	HOP_S *p = DbRecord;
	HASH_S *Hash = UserArg;

	DsDeleteHashKey(Hash, &p->PositionIndex, HERE);

	return FALSE;
}

static BOOLEAN_F AprsCheckStationPositionUsage(POINTER_F DbRecord,  POINTER_F UserArg)
{	STATION_S *p = DbRecord;
	HASH_S *Hash = UserArg;

	DsDeleteHashKey(Hash, &p->PositionIndex, HERE);

	return FALSE;
}

static BOOLEAN_F AprsCheckPacketPositionUsage(POINTER_F DbRecord,  POINTER_F UserArg)
{	PACKET_S *p = DbRecord;
	HASH_S *Hash = UserArg;

	DsDeleteHashKey(Hash, &p->PositionIndex, HERE);

	return FALSE;
}

COUNT_F AprsPurgeOrphanPositions(TIMESTAMP_F Youngest)
{	POSITION_S Pos = {0};
	INTEGER_ID_F *pID;
	COUNT_F Count, CurrentSize, InUse, Extents;
	HASH_S *Hash = DsCreateLocalHash("PositionIndices", sizeof(Pos.PositionIndex), NULL, HERE);
static	POINTER_F BuildPlan=NULL, HopPlan=NULL, StationPlan=NULL, PacketPlan=NULL, DeletePlan=NULL;

	DgPrintf("Purging Orphan Positions Prior to %.24s\n", ctime(&Youngest));

	DbLockTable(PosTable, HERE);
	DbGetTableSizing(PosTable, &CurrentSize, &InUse, &Extents);
	DsSetExpectedHashSize(Hash, CurrentSize, HERE);

	Pos.Touched = Youngest;
	Count = DbQueryCount(PosTable, DPOSITION_S, "Touched<Touched", sizeof(Pos), &Pos,
				AprsBuildPositionHash, Hash, &BuildPlan);
	DgPrintf("Hashed %ld Positions Hash has %ld\n", (long) Count, (long) DsGetHashCount(Hash));

	if (DsGetHashCount(Hash))
	{	Count = DbQueryCount(StationTable, NULL, NULL, 0, NULL, AprsCheckStationPositionUsage, Hash, &StationPlan);
		DgPrintf("Checked %ld Stations %ld Positions Left\n", (long) Count, (long) DsGetHashCount(Hash));
	}

	if (DsGetHashCount(Hash))
	{	Count = DbQueryCount(PacketTable, NULL, NULL, 0, NULL, AprsCheckPacketPositionUsage, Hash, &PacketPlan);
		DgPrintf("Checked %ld Packets %ld Positions Left\n", (long) Count, (long) DsGetHashCount(Hash));
	}

	if (DsGetHashCount(Hash))
	{	Count = DbQueryCount(HopTable, NULL, NULL, 0, NULL, AprsCheckHopPositionUsage, Hash, &HopPlan);
		DgPrintf("Checked %ld Hops %ld Positions Left\n", (long) Count, (long) DsGetHashCount(Hash));
	}

	Count = 0;
	if (DsGetHashCount(Hash))
	{	CURSOR_S *Cursor;
		DgPrintf("Deleting %ld Positions\n", (long) DsGetHashCount(Hash));
		Cursor = DsSetupHashKeysCursor(Hash, NULL, HERE);
		while ((pID=Cursor->Next(Cursor)) != NULL)
		{	Pos.PositionIndex = *pID;
			Count += DbDeleteRecord(PosTable, DPOSITION_S, sizeof(Pos), &Pos, &DeletePlan);
		}
		Cursor->Destroy(Cursor);
		DsEmptyHash(Hash,NULL,NULL,HERE);
	} else DgPrintf("No orphan Positions to Purge\n");

	DbUnlockTable(PosTable, HERE);

	DsDestroyHash(Hash,HERE);

	DgPrintf("Deleted %ld Positions Prior to %.24s\n", (long) Count, ctime(&Youngest));

	return Count;
}

static BOOLEAN_F AprsMaxPositionIndex(POINTER_F DbRecord,  POINTER_F UserArg)
{	POSITION_S *pPos = DbRecord;
	INTEGER_ID_F *pIndex = UserArg;
	if (pPos->PositionIndex > *pIndex) *pIndex = pPos->PositionIndex;
	return TRUE;
}

static INTEGER_ID_F AprsGetNextPositionIndex(void)
{	INTEGER_ID_F MyId;
static	INTEGER_ID_F NextID = 0;	/* Protected by PosTable DbLock */
static	POINTER_F QueryPlan=NULL;

	if (!NextID)
	{	POSITION_S Pos;
		Pos.PositionIndex = 0;
		DbQueryCount(PosTable, DPOSITION_S, NULL, sizeof(Pos), &Pos, AprsMaxPositionIndex, &Pos.PositionIndex, &QueryPlan);
		NextID = Pos.PositionIndex+1;
	}
	MyId = NextID++;
	if (!MyId) MyId = NextID++;
	return MyId;
}

INTEGER_ID_F AprsGetPositionIndex(COORDINATE_S *Coord)
{	POSITION_S Pos;
static	POINTER_F QueryPlan=NULL, InsertPlan=NULL, UpdatePlan=NULL;

	Pos.Where = *Coord;
	Pos.PositionIndex = 0;
	Pos.Touched = RtNow(NULL);
	DbLockTable(PosTable,HERE);
	if (DbQueryCount(PosTable, DPOSITION_S, "Where.Latitude=Where.Latitude AND Where.Longitude=Where.Longitude",
			sizeof(Pos), &Pos, AprsMaxPositionIndex, &Pos.PositionIndex, &QueryPlan) == 0)
	{	int r;
		for (r=0; r<100; r++)
		{	Pos.PositionIndex = AprsGetNextPositionIndex();
			if (DbPutValues(PosTable, DPOSITION_S, NULL, sizeof(Pos), &Pos,
					DPOSITION_S, NULL, sizeof(Pos), &Pos, DB_INSERT, &InsertPlan))
			{	break;
			}
			DgPrintf("Retrying Position Index From %ld\n", (long) Pos.PositionIndex);
		}
	} else if (!DbPutValues(PosTable, DPOSITION_S, NULL, sizeof(Pos), &Pos,
				DPOSITION_S, "Touched=Touched", sizeof(Pos), &Pos, DB_UPDATE, &UpdatePlan))
		DgPrintf("Failed To Touch Position %ld\n", (long) Pos.PositionIndex);
	DbUnlockTable(PosTable, HERE);

	return Pos.PositionIndex;
}

BOOLEAN_F AprsGetPosition(INTEGER_ID_F PositionIndex, COORDINATE_S *Coord)
{	BOOLEAN_F Result = FALSE;
	POSITION_S Pos;
static	POINTER_F GetPlan=NULL;

	Pos.PositionIndex = PositionIndex;
	if (DbGetValues(PosTable, DPOSITION_S, "PositionIndex=PositionIndex", sizeof(Pos), &Pos,
			DPOSITION_S, NULL, sizeof(Pos), &Pos, &GetPlan))
	{	*Coord = Pos.Where;
		Result = TRUE;
	} else memset(Coord,0,sizeof(*Coord));
	return Result;
}

static BOOLEAN_F AprsMaxStationIndex(POINTER_F DbRecord,  POINTER_F UserArg)
{	STATION_IDS_S *pID = DbRecord;
	INTEGER_ID_F *pIndex = UserArg;
	if (pID->StationIndex > *pIndex) *pIndex = pID->StationIndex;
	return TRUE;
}

static INTEGER_ID_F AprsGetNextStationIndex(void)
{	INTEGER_ID_F MyId;
static	MUTEX_SEMAPHORE_S *Lock = NULL;
static	INTEGER_ID_F NextID = 0;
static	POINTER_F QueryPlan=NULL;

	SmLockMutex(&Lock,HERE);
	if (!NextID)
	{	STATION_IDS_S StatID;
		StatID.StationIndex = 0;
		DbQueryCount(StatIDTable, DSTATION_IDS_S, NULL, sizeof(StatID), &StatID, AprsMaxStationIndex, &StatID.StationIndex, &QueryPlan);
		NextID = StatID.StationIndex+1;
	}
	MyId = NextID++;
	if (MyId == -1) MyId = NextID++;
	if (!MyId) MyId = NextID++;
	SmUnlockMutex(&Lock,HERE);
	return MyId;
}

INTEGER_ID_F AprsGetStationIndex(STATION_ID_F OwnerID, STATION_ID_F StationID, BOOLEAN_F AllowCreate)
{	STATION_IDS_S StatID;
static	POINTER_F GetPlan=NULL, InsertPlan=NULL;

	if (!StationID || !*StationID) return -1;
	
	DCOPY(StatID.OwnerID, OwnerID);
	DCOPY(StatID.StationID, StationID);
	if (!DbGetValues(StatIDTable, DSTATION_IDS_S, "OwnerID=OwnerID AND StationID=StationID", sizeof(StatID), &StatID,
			DSTATION_IDS_S, NULL, sizeof(StatID), &StatID, &GetPlan))
	{	if (AllowCreate)
		{	int r;
			for (r=0; r<100; r++)
			{	StatID.StationIndex = AprsGetNextStationIndex();
DgPrintf("Inserting Station %.*s %.*s Index %ld\n", STRING(StatID.OwnerID), STRING(StatID.StationID), (long) StatID.StationIndex);
				if (DbPutValues(StatIDTable, DSTATION_IDS_S, NULL, sizeof(StatID), &StatID,
						DSTATION_IDS_S, NULL, sizeof(StatID), &StatID, DB_INSERT, &InsertPlan))
				{	break;
				}
				DgPrintf("Retrying Station Index From %ld\n", (long) StatID.StationIndex);
			}
		} else StatID.StationIndex = -1;
	}
	return StatID.StationIndex;
}

BOOLEAN_F AprsGetStationID(INTEGER_ID_F StationIndex, STATION_ID_F *OwnerID, STATION_ID_F *StationID)
{	BOOLEAN_F Result = FALSE;
	STATION_S Station;
static	POINTER_F GetPlan=NULL;

	Station.StationIndex = StationIndex;
	if (DbGetValues(StationTable, DSTATION_S, "StationIndex=StationIndex", sizeof(Station), &Station,
			DSTATION_S, NULL, sizeof(Station), &Station, &GetPlan))
	{	if (OwnerID) DCOPY(*OwnerID, Station.OwnerID);
		if (StationID) DCOPY(*StationID, Station.StationID);
		Result = TRUE;
	} else
	{	STATION_IDS_S *pStatID;
	static	POINTER_F QueryPlan=NULL;
		POINTER_F Results = DbQuery(StatIDTable, DSTATION_S, "StationIndex=StationIndex", sizeof(Station), &Station, NULL, NULL,
						DSTATION_IDS_S, NULL, sizeof(*pStatID), NULL, NULL, &QueryPlan);
		if (Results)
		{	CURSOR_S *Cursor = DbSetupResultsCursor(Results,HERE);
			pStatID = Cursor->Next(Cursor);
			if (pStatID)
			{	if (OwnerID) DCOPY(*OwnerID, pStatID->OwnerID);
				if (StationID) DCOPY(*StationID, pStatID->StationID);
				Result = TRUE;
			} else
			{	if (OwnerID) DCOPY(*OwnerID, "");
				if (StationID) DCOPY(*StationID, "");
			}
			Cursor->Destroy(Cursor);	
			DbFreeResults(Results);
		} else
		{	if (OwnerID) DCOPY(*OwnerID, "");
			if (StationID) DCOPY(*StationID, "");
		}
	}
	return Result;
}

static BOOLEAN_F AprsMaxPacketIndex(POINTER_F DbRecord,  POINTER_F UserArg)
{	PACKET_S *pPkt = DbRecord;
	INTEGER_ID_F *pIndex = UserArg;
	if (pPkt->PacketIndex > *pIndex) *pIndex = pPkt->PacketIndex;
	return TRUE;
}

INTEGER_ID_F AprsGetNextPacketIndex(void)
{	INTEGER_ID_F MyId;
static	MUTEX_SEMAPHORE_S *Lock = NULL;
static	INTEGER_ID_F NextID = 0;
static	POINTER_F QueryPlan=NULL;

	SmLockMutex(&Lock,HERE);
	if (!NextID)
	{	PACKET_S Pkt;
		Pkt.PacketIndex = 0;
		DbQueryCount(PacketTable, DPACKET_S, NULL, sizeof(Pkt), &Pkt, AprsMaxPacketIndex, &Pkt.PacketIndex, &QueryPlan);
		NextID = Pkt.PacketIndex+1;
	}
	MyId = NextID++;
	if (!MyId) MyId = NextID++;
	SmUnlockMutex(&Lock,HERE);
	return MyId;
}

BOOLEAN_F AprsCheckHopPacket(POINTER_F DbRecord, POINTER_F UserArg)
{	HOP_S *pHop = DbRecord;
	PACKET_S Pkt;
static	POINTER_F GetPlan=NULL;
	return DbGetValues(PacketTable, DHOP_S, "PacketIndex=PacketIndex", sizeof(*pHop), pHop,
				DPACKET_S, "", sizeof(Pkt), &Pkt, &GetPlan) == NULL;
}

VFUNCTION AprsPurgeOrphans(void)
{	HOP_S Hop;
	COUNT_F HopsDeleted;
	COUNT_F HopsStart, HopsNow, Size, Extents;
static	POINTER_F HopPlan=NULL;

DgPrintf("Purging Orphans\n");
	DbGetTableSizing(HopTable, &Size, &HopsStart, &Extents);
	HopsDeleted = DbDeleteRecords(HopTable, DHOP_S, NULL, sizeof(Hop), &Hop, AprsCheckHopPacket, NULL, &HopPlan);
	DbGetTableSizing(HopTable, &Size, &HopsNow, &Extents);
	if (HopsDeleted) DgPrintf("Deleted %ld/%ld Hops, %ld Remaining\n", (long) HopsDeleted, (long) HopsStart, (long) HopsNow);
DgPrintf("Done Purging Orphans\n");
}

/*:AprsConvertUser

*/
static VFUNCTION AprsConvertUser(STRING_F PathName)
{
	if (!DbConvertTable("User0", DAPRSISCE_USER0_S, "StationID", 
				PathName, UserTable))
	{	DgPrintf("Failed To Open User0\n");
	}
}

/*:AprsConvertStation1

*/
static VFUNCTION AprsConvertStation1(STRING_F PathName)
{
	if (!DbConvertTable("Station1", DSTATION1_S, "StationIndex", 
				PathName, StationTable))
	{	DgPrintf("Failed To Open Station1\n");
	}
}

BOOLEAN_F FUNCTION AprsLoadDatabase(STRING_F PathName)
{	ROUTINE("AprsLoadDatabase");
	BOOLEAN_F Created;
static	SECONDARY_DEFINITION_S StatIDSec[] = { {SECONDARY_DEFINITION_MAGIC, "Station", "StationIndex", FALSE} };
static	SECONDARY_DEFINITION_S PosSec[] = { {SECONDARY_DEFINITION_MAGIC, "Coordinate", "Where.Latitude,Where.Longitude", FALSE} };
#ifdef NEW_WAY
static	SECONDARY_DEFINITION_S HopSec[] = { {SECONDARY_DEFINITION_MAGIC, "Packet", "PacketIndex", FALSE} };
static	SECONDARY_DEFINITION_S PacketSec[] = { {SECONDARY_DEFINITION_MAGIC, "Origin", "OriginIndex", FALSE},
						 {SECONDARY_DEFINITION_MAGIC, "Heard", "HeardIndex", FALSE} };
#endif

	StatIDTable = DbLoadTable3("StatIDs0", DSTATION_IDS_S, "OwnerID,StationID", PathName, 0, 0, TRUE, &Created, ACOUNT(StatIDSec), StatIDSec);
	if (!StatIDTable) KILLPROC(-1,"Error loading StatIDTable\n");

	PosTable = DbLoadTable3("Position0", DPOSITION_S, "PositionIndex", PathName, 0, 0, TRUE, &Created, ACOUNT(PosSec), PosSec);
	if (!PosTable) KILLPROC(-1,"Error loading PosTable\n");

	StationTable = DbLoadTable3("Station2", DSTATION_S, "StationIndex", PathName, 0, 0, TRUE, &Created, 0, NULL);
	if (!StationTable) KILLPROC(-1,"Error loading StationTable\n");
	if (Created) AprsConvertStation1(PathName);

#ifdef NEW_WAY
	HopTable = DbLoadTable3("Hop0", DHOP_S, "PacketIndex,Sequence", PathName, 0, 0, TRUE, &Created, ACOUNT(HopSec), HopSec);
#else
	HopTable = DbLoadTable3("Hop0", DHOP_S, "PacketIndex,Sequence", PathName, 0, 0, TRUE, &Created, 0, NULL);
#endif
	if (!HopTable) KILLPROC(-1,"Error loading HopTable\n");

#ifdef NEW_WAY
	PacketTable = DbLoadTable3("Packet1", DPACKET_S, "PacketIndex", PathName, 0, 0, TRUE, &Created, ACOUNT(PacketSec), PacketSec);
#else
	PacketTable = DbLoadTable3("Packet1", DPACKET_S, "PacketIndex", PathName, 0, 0, TRUE, &Created, 0, NULL);
#endif
	if (!PacketTable) KILLPROC(-1,"Error loading PacketTable\n");

#ifndef NEW_WAY
	if (!DbAddSecondaryKey(HopTable, "Packet", "PacketIndex", FALSE))
		ThSprintfErrorString(Routine, HERE, "Failed To Create Packet Secondary On HopTable\n");
	if (!DbAddSecondaryKey(PacketTable, "Origin", "OriginIndex", FALSE))
		ThSprintfErrorString(Routine, HERE, "Failed To Create Origin Secondary On PacketTable\n");
	if (!DbAddSecondaryKey(PacketTable, "Heard", "HeardIndex", FALSE))
		ThSprintfErrorString(Routine, HERE, "Failed To Create Heard Secondary On PacketTable\n");
#endif

	UserTable = DbLoadTable3("User1", DAPRSISCE_USER_S, "StationID", PathName, 0, 0, TRUE, &Created, 0, NULL);
	if (!UserTable) KILLPROC(-1,"Error loading userTable\n");
	if (Created) AprsConvertUser(PathName);

	UserPktTable = DbLoadTable("UserPkt0", DUSER_PACKET_S, "StationID,When", PathName, 0, 0);
	if (!UserPktTable) KILLPROC(-1,"Error loading userPktTable\n");

//	AprsPurgeOrphanPositions(RtNow(NULL));
//	AprsPurgeOrphans();
	DgPrintf("No Longer Purging Orphan Hops!\n");

	return TRUE;
}

VFUNCTION AprsCloseDatabase(void)
{
#ifdef OLD_WAY
DgPrintf("Closing UserPkt\n");
	DbCloseTable(UserPktTable);
DgPrintf("Closing User\n");
	DbCloseTable(UserTable);
DgPrintf("Closing StatID\n");
	DbCloseTable(StatIDTable);
DgPrintf("Closing Station\n");
	DbCloseTable(StationTable);
DgPrintf("Closing Pos\n");
	DbCloseTable(PosTable);
DgPrintf("Closing Packet\n");
	DbCloseTable(PacketTable);
DgPrintf("Closing Hop\n");
	DbCloseTable(HopTable);
#else
	DbCloseAllTables(10000, 10, TRUE);
#endif
}

