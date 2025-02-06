#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <df/base.h>

#include <uf/include/base.h>
#include <uf/source/dgprint.h>
#include <uf/source/dshash.h>
#include <uf/source/dsqueue.h>
#include <uf/source/dsrtns.h>
#include <uf/source/flrtns.h>
#include <uf/source/hprtns.h>
#include <uf/source/lgrtns.h>
#include <uf/source/rtrtns.h>
#include <uf/source/syrtns.h>
#include <uf/source/tgrtns.h>
#include <uf/source/thrtns.h>

#include <db/include/dbdef.h>
#include <db/source/dbaccess.h>
#include <db/source/message.h>

#include <ci/include/cidef.h>
#include <ci/source/ciinit.h>
#include <ci/source/ciarray.h>
#include <ci/source/ciconn.h>
#include <ci/source/ciconv.h>
#include <ci/source/cidescr.h>

#include <aprs/include/aprsdef.h>
#include <aprs/include/database.h>
#include <aprs/include/messages.h>

#include <aprs/source/aprs.h>

#include <aprs/source/server.h>
#include <aprs/source/services.h>

void cdecl TraceError(HWND hwnd, char *Format, ...)
{	va_list args;
	va_start(args, Format);
	DgVprintf(2000, Format, args);
	va_end(args);
}
void cdecl TraceLog(char *Name, BOOL ForceIt, HWND hwnd, char *Format, ...)
{	va_list args;
	va_start(args, Format);
	DgVprintf(2000, Format, args);
	va_end(args);
}
void cdecl TraceLogThread(char *Name, BOOL ForceIt, char *Format, ...)
{	va_list args;
	va_start(args, Format);
	DgVprintf(2000, Format, args);
	va_end(args);
}
#ifdef OLD_WAY
void cdecl TraceError(HWND hwnd, char *Format, ...) {}
void cdecl TraceLog(char *Name, BOOL ForceIt, HWND hwnd, char *Format, ...) {}
void cdecl TraceLogThread(char *Name, BOOL ForceIt, char *Format, ...) {}
#endif

#define sock_init() 							\
{	WORD wVersionRequested = MAKEWORD(1,1);				\
	WSADATA wsaData;						\
	int err = WSAStartup(wVersionRequested, &wsaData);		\
	if (err != 0)							\
	{	/*printf("WSAStartup Failed With %ld\n", (long) err);*/	\
		exit(-1);						\
	}								\
}
#define soclose(s) closesocket(s)
#define ioctl(s) ioctlsocket(s)
#define psock_errno(s) DgPrintf("%s errno %ld\n", s, (long) h_errno)
#define sock_errno() h_errno

BOOLEAN_F AprsLookupStationLocation
(	STATION_ID_F OwnerID,
	STATION_ID_F StationID,
	INTEGER_ID_F *pStationIndex,
	COORDINATE_S *pLoc,
	INTEGER_ID_F *pIndex
)
{	APRS_LOOKUP_STATION_SRQ Req = {0};
	MESSAGE_S *Msg;
	BOOLEAN_F Result;

	AprsMakeStationID(OwnerID,&Req.OwnerID);
	AprsMakeStationID(StationID,&Req.StationID);
	Msg = CiSendRequestTo("APRS", "APRS_LOOKUP_STATION", &Req, sizeof(Req));
	if (!Msg->Success)
	{	STRING_F Symbol = DsMakeSubName("UNKNOWN", StationID);
		if (!SySymbolTrue(Symbol))
		{	STRING_F Text = CiPopulateResponseStatusText(Msg);
			SySetSymbolValue(Symbol,"1");
			DgPrintf("No Location For Station %.*s:%.*s, Suppressing Messages (Error %s)\n",
				STRING(Req.OwnerID), STRING(Req.StationID), Text);
		}
		if (pStationIndex) *pStationIndex = 0;
		if (pLoc) memset(pLoc,0,sizeof(*pLoc));
		if (pIndex) *pIndex = 0;
		THREAD_FREE(Symbol);
		Result = FALSE;
	} else
	{	APRS_LOOKUP_STATION_SRP *Rsp = Msg->Content;
		if (pStationIndex) *pStationIndex = Rsp->StationIndex;
		if (pLoc) *pLoc = Rsp->Last;
		if (pIndex) *pIndex = Rsp->PositionIndex;
		DgPrintf("Found Station %.*s:%.*s at %.5lf %.5lf\n",
				STRING(Req.OwnerID), STRING(Req.StationID),
				(double) Rsp->Last.Latitude, (double) Rsp->Last.Longitude);
		Result = TRUE;
	}
	CiDestroyMessage(Msg);
	return Result;
}

struct
{	COUNT_F NewUser;
	COUNT_F Upgrade;
	COUNT_F Downgrade;
	COUNT_F ActiveVersions;
	COUNT_F TotalVersions;
} Hourly, LastHourly;
TIMESTAMP_F NextMessage = 0;
TIMESTAMP_F NextDefinition = 0;
HASH_S *VersionHash = NULL;
HASH_S *HourlyHash;

static VFUNCTION CleanupHourlyHash(HASH_S *Hash, POINTER_F Value, POINTER_F UserArg)
{	;	/* No-op */
}

static VFUNCTION UDPHourly(POINTER_F Dummy)
{	ROUTINE(UDPHourly);
	COUNT_F Packets = (RtNow(NULL)%999) + 1;

	STRING_F Definitions[] = { "KJ4ERJ>APZAPM:!2759.80N/08039.54WlVersion Monitor",
							"KJ4ERJ>APZAPM::KJ4ERJ   :PARM.NewUsers,Upgrades,Downgrades,Active Versions,Versions,B1,B2,B3,B4,B5,B6,B7,B8",
							"KJ4ERJ>APZAPM::KJ4ERJ   :UNIT.Stations,Stations,Stations,Unique,Unique,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A",
							"KJ4ERJ>APZAPM::KJ4ERJ   :EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,1,0",
							"KJ4ERJ>APZAPM::KJ4ERJ   :BITS.11111111,APRSISCE/32 Versions (/15 minutes)" };

	if (!NextDefinition) NextDefinition = RtNow(NULL) + 5*60;	/* Delay definitions for 5 minutes after startup */
	if (!NextMessage) NextMessage = RtNow(NULL);
	NextMessage += (15*60) - (RtNow(NULL) % (15*60));	/* Move up to next 15 minute boundary */
	
	DgDefineCountCounter("Hourly.NewUser", &Hourly.NewUser);
	DgDefineCountCounter("Hourly.Upgrade", &Hourly.Upgrade);
	DgDefineCountCounter("Hourly.Downgrade", &Hourly.Downgrade);
	DgDefineCountCounter("Hourly.ActiveVersions", &Hourly.ActiveVersions);
	DgDefineCountCounter("Hourly.TotalVersions", &Hourly.TotalVersions);
	DgDefineTimestampCounter("Hourly.NextTelemetry", &NextMessage);
	DgDefineTimestampCounter("Hourly.NextDefinition", &NextDefinition);
	
	{	char Buffer[256];
		sprintf(Buffer, "KJ4ERJ>APZAPM:T#%03ld,0,0,0,0,0,00000000", (long) Packets);
		if (++Packets > 999) Packets = 1;
		AprsQueueXmitPacket(Buffer);
	}

	while (!CiIsShuttingDown())
	{	TIMESTAMP_F Now = RtNow(NULL);
		if (Now >= NextMessage)
		{	char Buffer[256];
			BOOLEAN_F Changed;
/* T#034,126,157,255,090,081,00000000 */
			sprintf(Buffer, "KJ4ERJ>APZAPM:T#%03ld,%ld,%ld,%ld,%ld,%ld,00000000",
					(long) Packets, (long) LastHourly.NewUser, (long) LastHourly.Upgrade,
					(long) LastHourly.Downgrade, (long) LastHourly.ActiveVersions, (long) LastHourly.TotalVersions);
			if (++Packets > 999) Packets = 1;
			AprsQueueXmitPacket(Buffer);

			Hourly.ActiveVersions = DsGetHashCount(HourlyHash);
			Hourly.TotalVersions = DsGetHashCount(VersionHash);
			Changed = memcmp(&Hourly, &LastHourly, sizeof(Hourly));
			LastHourly = Hourly;
			memset(&Hourly, 0, sizeof(Hourly));
			DsEmptyHash(HourlyHash, CleanupHourlyHash, NULL, HERE);

			if (Changed)
			{	sprintf(Buffer, "KJ4ERJ>APZAPM:T#%03ld,%ld,%ld,%ld,%ld,%ld,00000000",
					(long) Packets, (long) LastHourly.NewUser, (long) LastHourly.Upgrade,
					(long) LastHourly.Downgrade, (long) LastHourly.ActiveVersions, (long) LastHourly.TotalVersions);
				if (++Packets > 999) Packets = 1;
				ThSleepThread(61000);	/* Sleep 61 seconds to allow telemetry to propagate and graph to settle */
				AprsQueueXmitPacket(Buffer);
			}
DgPrintf("Sending: %s\n", Buffer);
			NextMessage += (15*60);
			NextMessage -= NextMessage%(15*60);
		}
		if (Now >= NextDefinition)
		{	INDEX_F i;
			for (i=0; i<ACOUNT(Definitions); i++)
			{
DgPrintf("Sending: %s\n", Definitions[i]);
				AprsQueueXmitPacket(Definitions[i]);
			}
			NextDefinition += 12*60*60;	/* Every 12 hours */
		}
		{	COUNT_F SleepTime = max(1,min(60,(min(NextDefinition,NextMessage) - Now) / 2));
			DgPrintf("%s:Sleeping %ld seconds\n", Routine, SleepTime);
			if (SleepTime > 0) ThSleepThread(SleepTime*1000);
		}
	}
}

static VFUNCTION AprsNotifyQueue(POINTER_F ServerArg)
{	QUEUE_S **Queue = ServerArg;
	STRING_F Packet;

	while (!*Queue)
	{	DgPrintf("AprsNotifyQueue:Waiting for Queue\n");
		ThSleepThread(1000L);
	}

	while (*Queue && (Packet = DsDeQueueElement(*Queue)) != NULL)
	{	INDEX_F c;
		COUNT_F Count = 0;
		struct
		{	COUNT_F Count;
			char Platform[32];
			char Version[16];
			char *Packet;
		} *Counts = NULL;

		for (c=60; c>0; c--)	/* 15 minute collapsed notification */
		{	DgPrintf("AprsNotifyQueue:[%ld]Delaying Notification %ld Waiting...\n",
				(long) c, (long) DsGetQueueCount(*Queue));
			ThSleepThread(60*1000L);	/* Every minute */
		}

		DgPrintf("Coallescing %ld Version Messages\n", (long) DsGetQueueCount(*Queue));

		do
		{	char *safe = THREAD_STRDUP(Packet);
			char *v = strstr(Packet," Running ");	/* Locate the version */
DgPrintf("%s\n", Packet);
			if (v && strlen(v) >= 9+16)
			{	char *p = v + 9;	/* Point to platform */
				v = strchr(p,' ');
				if (!v) v = strchr(p,'\0'); else *v++ = '\0';
				for (c=0; c<Count; c++)
					if (!strncmp(Counts[c].Platform, p, sizeof(Counts[c].Platform))
					&& !strncmp(Counts[c].Version, v, sizeof(Counts[c].Version)))
						break;
				if (c >= Count)
				{	c = Count++;
					Counts = THREAD_REALLOC(Counts,sizeof(*Counts)*Count);
					Counts[c].Packet = safe;
					strncpy(Counts[c].Platform, p, sizeof(Counts[c].Platform));
					strncpy(Counts[c].Version, v, sizeof(Counts[c].Version));
					Counts[c].Count = 0;
DgPrintf("New[%ld] Platform(%.*s) Version(%.*s)\n", (long) Count, STRING(Counts[c].Platform), STRING(Counts[c].Version));
				} else THREAD_FREE(safe);
				Counts[c].Count++;
			} else AprsQueueXmitPacket(Packet);
			STRING_FREE(Packet);
		} while (*Queue && DsGetQueueCount(*Queue) > 1	/* 1 is the one it thinks I'm processing */
		&& (Packet = DsDeQueueElement(*Queue)) != NULL);

		for (c=0; c<Count; c++)
		{	if (Counts[c].Count > 1)
			{	char Buffer[256];
				sprintf(Buffer, "KJ4ERJ>APZAPM::KJ4ERJ-12:%.*s %.*s - %ld\r\n",
					STRING(Counts[c].Platform), STRING(Counts[c].Version), (long) Counts[c].Count);
				AprsQueueXmitPacket(Buffer);
			} else AprsQueueXmitPacket(Counts[c].Packet);
			THREAD_FREE(Counts[c].Packet);
		}
		if (Counts) THREAD_FREE(Counts);
	}
	DgPrintf("AprsNotifyQueue:Terminating...\n");
}

static BOOLEAN_F AprsUpdateUser
(	STRING_F ID,
	TIMESTAMP_F When,
	COORDINATE_S *Where,
	STRING_F IpAddress,
	STRING_F ReverseDNS,
	STRING_F Version	/* May or may not include PROGNAME */
)
{	ROUTINE(AprsUpdateUser);
	BOOLEAN_F Result = FALSE;
	APRSISCE_USER_S User = {0}, oUser = {0};
static	POINTER_F GetPlan=NULL, UpdatePlan=NULL, InsertPlan=NULL, PktPlan=NULL;

	if (!When) When = RtNow(NULL);
	AprsMakeStationID(ID,&User.StationID);
	User.LastHeard = When;
	if (Where) User.Last = *Where;
	User.PacketsReceived = 1;	/* Added in DB */
	DCOPY(User.Version, Version);

	if (DbGetValues(UserTable, DAPRSISCE_USER_S, NULL, sizeof(User), &User,
			DAPRSISCE_USER_S, NULL, sizeof(oUser), &oUser, &GetPlan))
	{	STRING_F UpdateFields = THREAD_STRDUP("LastHeard=LastHeard, PacketsReceived+=PacketsReceived");
		if (Where)
			UpdateFields = DbAddMapping(UpdateFields, "Last.Latitude=Last.Latitude, Last.Longitude=Last.Longitude");
		if (!RtStrnWhite(-1,Version))
		{	if (RtStrnlen(STRING(oUser.Version)) > strlen(Version)	/* old version was longer */
			&& RtStrnstr(STRING(oUser.Version),-1,Version,TRUE))	/* And new version inside (probably missing PROGNAME) */
			{	Version = oUser.Version;	/* Pretend he said it again */
				DCOPY(User.Version, oUser.Version);
			} else if (strncmp(oUser.Version, Version, sizeof(oUser.Version)))	/* Version changed */
				UpdateFields = DbAddMapping(UpdateFields, "Version=Version");
		}
		if (!DbPutValues(UserTable, DAPRSISCE_USER_S, NULL, sizeof(User), &User,
			DAPRSISCE_USER_S, UpdateFields, sizeof(User), &User,
			DB_UPDATE, &UpdatePlan))
			DgPrintf("Failed To Update User %.*s Version %.*s\n", STRING(User.StationID), STRING(User.Version));
		else Result = TRUE;
		THREAD_FREE(UpdateFields);
	} else
	{	DgPrintf("New User %.*s Version %.*s\n", STRING(User.StationID), STRING(User.Version));
		User.FirstHeard = When;
		User.First = User.Last;
		if (!DbPutValues(UserTable, DAPRSISCE_USER_S, NULL, sizeof(User), &User,
				DAPRSISCE_USER_S, NULL, sizeof(User), &User,
				DB_INSERT, &InsertPlan))
			DgPrintf("Failed To Insert New User %.*s\n", STRING(User.StationID));
		else Result = TRUE;
	}

	if (Result)
	{	USER_PACKET_S Pkt = {0};
		DCOPY(Pkt.StationID, User.StationID);
		Pkt.When = User.LastHeard;
		Pkt.Where = User.Last;
		DCOPY(Pkt.IPAddress, IpAddress);
		DCOPY(Pkt.ReverseDNS, ReverseDNS);
		if (!DbPutValues(UserPktTable, DUSER_PACKET_S, NULL, sizeof(Pkt), &Pkt,
				DUSER_PACKET_S, NULL, sizeof(Pkt), &Pkt, DB_INSERT, &PktPlan))
			DgPrintf("Failed To Insert Packet For User %.*s\n", STRING(User.StationID));
	}

	{static	STRING_F Monitors[] = { "KJ4ERJ-12" };
	static	TIMESTAMP_F NextTime;
	static	STATION_ID_F LastStation;
	
		if (!RtStrnWhite(STRING(User.Version)))	/* Only report version changes if we have a version */
		{
			if (!VersionHash) VersionHash = DsCreateHash("APRSISCE:Versions:Active", -1, DSTRING_F, HERE);
			if (!HourlyHash)
			{	HourlyHash = DsCreateHash("APRSISCE:Versions:Hourly", -1, DSTRING_F, HERE);
				ThCreateThread("UDPHourly", UDPHourly, NULL, HERE);
			}
			if (Version && *Version && VersionHash && HourlyHash)
			{	STRING_F MyVersion = DsLookupHashKey(VersionHash, Version);
				if (!MyVersion)
				{	MyVersion = HEAP_STRDUP(DsGetHashHeap(VersionHash), Version);
					DgPrintf("New Detected Version %s\n", MyVersion);
					if (!DsInsertIfNewHashKey(VersionHash, MyVersion, MyVersion, HERE))
					{	ThSprintfErrorString(Routine, HERE, "Failed To Record Version %s\n", MyVersion);
						HEAP_FREE(DsGetHashHeap(VersionHash), MyVersion);
						MyVersion = NULL;
					} else 	Hourly.TotalVersions = DsGetHashCount(VersionHash);

				}
				if (MyVersion)
				{	if (DsInsertIfNewHashKey(HourlyHash, MyVersion, MyVersion, HERE))
					{	DgPrintf("New Hourly Version %s\n", MyVersion);
						Hourly.ActiveVersions = DsGetHashCount(HourlyHash);
					}
				}
			}
			
			if (strncmp(User.Version, oUser.Version, sizeof(User.Version)))
			{	INDEX_F m;
				char Buffer[256];

				for (m=0; m<ACOUNT(Monitors); m++)
				{	sprintf(Buffer,"KJ4ERJ>APZAPM::%-9s:%.*s Running %.*s",
							Monitors[m], STRING(User.StationID), STRING(User.Version));
					if (RtStrnWhite(STRING(oUser.Version)))
					{	strcat(Buffer," NEW USER!");
						//AprsQueueXmitPacket(Buffer);
						Hourly.NewUser++;
					} else
					{	STRING_F oVer = strchr(oUser.Version,' ');
						STRING_F nVer = strchr(User.Version,' ');
						if (!oVer) oVer = strchr(oUser.Version, '\0');
						if (!nVer) nVer = strchr(User.Version, '\0');

						if ((oVer-oUser.Version) != (nVer-User.Version)	/* Length change? */
						|| strncmp(oUser.Version, User.Version, (nVer-User.Version)))	/* Name change? */
						{	sprintf(Buffer+strlen(Buffer)," PLATFORM %.*s", STRING(oUser.Version));
							AprsQueueXmitPacket(Buffer);
DgPrintf("Platform change from(%ld)(%.*s) to(%ld)(%.*s) in(%.*s) and(%.*s)\n",
	(long)(oVer-oUser.Version), (int)(oVer-oUser.Version), oUser.Version,
	(long)(nVer-User.Version), (int)(nVer-User.Version), User.Version,
	STRING(oUser.Version), STRING(User.Version));
						} else if (strcmp(oVer, nVer) > 0)
						{	sprintf(Buffer+strlen(Buffer)," DOWNGRADE%s", oVer);
							//AprsQueueXmitPacket(Buffer);
							Hourly.Downgrade++;
DgPrintf("Downgrade from(%s) to(%s) in(%.*s) and(%.*s)\n",
	oVer, nVer,
	STRING(oUser.Version), STRING(User.Version));
						} else	/* Queue it for routine dupe suppression */
						{
						static QUEUE_S *Queue = NULL;
							if (!Queue) Queue = DsCreateQueue("NotifyQueue", AprsNotifyQueue, &Queue, 0, 1, DSTRING_F, HERE);
							sprintf(Buffer+strlen(Buffer)," Was %.*s", STRING(oUser.Version));
							//if (Queue) DsQueueElement(Queue, STRING_STRDUP(Buffer));
							//else AprsQueueXmitPacket(Buffer);
							Hourly.Upgrade++;
						}
					}
				}
			}
		} else if (Where)	/* Otherwise ancient style, Got coordinates? */
		{	if (!NextTime
			|| NextTime < RtNow(NULL)
			|| strncmp(User.StationID, LastStation, sizeof(User.StationID)))
			{	INDEX_F m;
				char Buffer[256];

				DCOPY(LastStation, User.StationID);
				NextTime = RtNow(NULL)+5*60;
				for (m=0; m<ACOUNT(Monitors); m++)
				{	COORDINATE_S MonLoc;
					if (AprsLookupStationLocation(Monitors[m], Monitors[m], NULL, &MonLoc, NULL))
					{	double Dist, Bearing;
						AprsHaversine(&MonLoc, Where, &Dist, &Bearing);
						sprintf(Buffer,"KJ4ERJ>APZAPM::%-9s:%.*s @ %7.4lf %8.4lf or %.1lfmi@%ld (%s)",
							Monitors[m], STRING(User.StationID),
							(double) Where->Latitude, (double) Where->Longitude,
							(double) Dist, (long) Bearing, Version);
					} else
						sprintf(Buffer,"KJ4ERJ>APZAPM::%-9s:%.*s @ %7.4lf %8.4lf (%s)",
							Monitors[m], STRING(User.StationID),
							(double) Where->Latitude, (double) Where->Longitude, Version);
					AprsQueueXmitPacket(Buffer);
				}
			} else DgPrintf("Suppressing duplicate xmit(%.*s)\n", STRING(LastStation));
		}
	}

	return Result;
}

BOOLEAN_F FUNCTION AprsSvcQueryUserPackets(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_USER_PACKETS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_USER_PACKETS_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F AccessPlan = NULL;

	RtStrnuprTrim(STRING(Svc->StationID));
	Condition = DbAddStringCondition(Condition,STRING(Svc->StationID),"StationID=StationID");

	if (Svc->StartTime) Condition = DbAddCondition(Condition,"When>=StartTime");
	if (Svc->EndTime) Condition = DbAddCondition(Condition,"When<=EndTime");

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	Rsp = DbQueryIntoGrowingResponse(UserPktTable, DAPRS_QUERY_USER_PACKETS_SRQ, Condition,
						sizeof(*Svc), Svc, NULL, NULL,
						DAPRS_QUERY_USER_PACKETS_SRP, "Packets", NULL,
						sizeof(*Rsp), &AccessPlan, HERE);
		if (Rsp)
		{	CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
		} else	CiSendBadResponse2(Req, 204, "APRS::FAIQRYUPK", "Failed To Query User Packets", 0);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcPurgeUserPackets(MESSAGE_S *Req, STRING_F URL)
{	APRS_PURGE_USER_PACKETS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_PURGE_USER_PACKETS_SRP Rsp = {0};
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F AccessPlan = NULL;

	RtStrnuprTrim(STRING(Svc->StationID));
	Condition = DbAddStringCondition(Condition,STRING(Svc->StationID),"StationID=StationID");

	if (Svc->StartTime) Condition = DbAddCondition(Condition,"When>=StartTime");
	if (Svc->EndTime) Condition = DbAddCondition(Condition,"When<=EndTime");

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	Rsp.Count = DbDeleteRecords(UserPktTable, DAPRS_QUERY_USER_PACKETS_SRQ, Condition,
						sizeof(*Svc), Svc, NULL, NULL, &AccessPlan);
		CiSendGoodResponse(Req, &Rsp, sizeof(Rsp));
	}
	THREAD_FREE(Condition);

	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcQueryUsers(MESSAGE_S *Req, STRING_F URL)
{	APRS_QUERY_USERS_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_QUERY_USERS_SRP *Rsp;
	STRING_F Condition = THREAD_STRDUP("");
static	POINTER_F AccessPlan = NULL;

	RtStrnuprTrim(STRING(Svc->StationID));
	Condition = DbAddStringCondition(Condition,STRING(Svc->StationID),"StationID=StationID");

	if (Svc->NewerThan) Condition = DbAddCondition(Condition,"FirstHeard>=NewerThan");
	if (Svc->InactiveSince) Condition = DbAddCondition(Condition,"LastHeard<=InactiveSince");
	if (Svc->ActiveSince) Condition = DbAddCondition(Condition,"LastHeard>=ActiveSince");
	if (Svc->RegisteredSince) Condition = DbAddCondition(Condition,"Registered>=RegisteredSince");

	if (!*Condition && !Svc->ReallyAll)
	{	CiSendBadResponse(Req, 204, "Must Specify One Match Criteria or ReallyAll", 0, NULL);
	} else
	{	Rsp = DbQueryIntoGrowingResponse(UserTable, DAPRS_QUERY_USERS_SRQ, Condition,
						sizeof(*Svc), Svc, NULL, NULL,
						DAPRS_QUERY_USERS_SRP, "Users", NULL,
						sizeof(*Rsp), &AccessPlan, HERE);
		if (Rsp)
		{	CiSendGoodResponse(Req, Rsp, CiGetGrowingResponseSize(Rsp));
			CiDestroyGrowingResponse(Rsp);
		} else	CiSendBadResponse2(Req, 204, "APRS::FAIQRYUSR", "Failed To Query Users", 0);
	}
	THREAD_FREE(Condition);

	return TRUE;
}

BOOLEAN_F FUNCTION AprsSvcRegisterUser(MESSAGE_S *Req, STRING_F URL)
{	APRS_REGISTER_USER_SRQ *Svc = (POINTER_F) Req->Body;
	APRS_REGISTER_USER_SRP Rsp;
static	POINTER_F AccessPlan = NULL;

	RtStrnuprTrim(STRING(Svc->StationID));

	if (RtStrnWhite(STRING(Svc->StationID)))
	{	CiSendBadResponse(Req, 204, "Must Specify Station ID", 0, NULL);
	} else
	{	if (!Svc->When) Svc->When = RtNow(NULL);
		Rsp.Password = "*Not Supported*";
		if (DbPutValues(UserTable, DAPRS_REGISTER_USER_SRQ, NULL, sizeof(*Svc), Svc,
				DAPRS_REGISTER_USER_SRQ, "Registered=When", sizeof(*Svc), Svc, DB_UPDATE, &AccessPlan))
		{	CiSendGoodResponse(Req, &Rsp, sizeof(Rsp));
		} else	CiSendBadResponse2(Req, 204, "APRS::UNK_USER", "Unknown User(%1)", 1, STRING(Svc->StationID));
	}

	return TRUE;
}

static QUEUE_S *RcvdPacketQueue = NULL;

static VFUNCTION AprsDequeueLogRcvdPacket(POINTER_F Dummy)
{	ROUTINE(AprsDequeueLogRcvdPacket);
	APRS_LOG_RECEIVED_PACKET_SRQ *Req;
	SERVER_S *Server = CiSetupServer("APRS");
	COUNT_F LastCount = 0, ThisCount, DoneCount = 0;
	
	while ((Req = DsDeQueueElement(RcvdPacketQueue)) != NULL)
	{	MESSAGE_S *Msg;
		MILLISECS_F msStart = RtGetMsec();
		double Elapsed;

		while (!CiCheckServerConnection(Server,FALSE))
		{	DgPrintf("%s:Waiting For %s %ld in Queue (%.24s)\n",
				Routine, Server->Name,
				(long) DsGetQueueCount(RcvdPacketQueue),
				ctime(&Req->When));
			ThSleepThread(60000L);
		}

		Msg = CiSendRequestTo("APRS", "APRS_LOG_RECEIVED_PACKET", Req, sizeof(*Req));
		if (!Msg->Success)
		{	STRING_F Text = CiPopulateResponseStatusText(Msg);
			DgPrintf("Failed To Log %.24s %s Reason %s\n", ctime(&Req->When), Req->Packet, Text);
			THREAD_FREE(Text);
		}
		CiDestroyMessage(Msg);
		
		DoneCount++;
		ThisCount = DsGetQueueCount(RcvdPacketQueue);
		Elapsed = RtGetMsec()-msStart;
		if ((ThisCount > LastCount && ThisCount >100) || Elapsed > 1000)
		{	DgPrintf("UDP(RcvdPacketQueue) Has %ld (Was %ld) Entries after %.2lf msec LOG_RECEIVED_PACKET (%ld Total)\n",
				(long) ThisCount, (long) LastCount,
				(double) Elapsed,
				(long) DoneCount);
		}
		LastCount = ThisCount;
		
		HEAP_FREE(DsGetQueueHeap(RcvdPacketQueue),Req->ReceivedBy);
		HEAP_FREE(DsGetQueueHeap(RcvdPacketQueue),Req->Packet);
		HEAP_FREE(DsGetQueueHeap(RcvdPacketQueue),Req);
	}
	DgPrintf("%s Terminating\n", Routine);
	ThTerminateThread();
}

static VFUNCTION AprsLogReceivedPacket
(	TIMESTAMP_F Timestamp,
	STRING_F ID,
	STRING_F Packet
)
{static	FIRST_TIME_ONLY_S Info = {0};

	if (SmFirstTimeOnly(&Info,HERE))
	{	if (SySymbolDefined("TCP_APRS"))
		{	DESCRIPTOR_S *Description = CiGetDescription("APRS","APRS_LOG_RECEIVED_PACKET_SRQ",FALSE);
			RcvdPacketQueue = DsCreateQueue("PendingRcvdPackets", AprsDequeueLogRcvdPacket,
							NULL, 0, 1,
							Description, HERE);
		} else DgPrintf("TCP_APRS not defined, NOT forwarding UDP packets\n");
		SmEndFirstTimeOnly(&Info,HERE);
	}
	
	if (RcvdPacketQueue)
	{	APRS_LOG_RECEIVED_PACKET_SRQ *Req;

		Req = HEAP_MALLOC(DsGetQueueHeap(RcvdPacketQueue),sizeof(*Req));
		Req->When = Timestamp?Timestamp:RtNow(NULL);
		Req->ReceivedBy = HEAP_STRDUP(DsGetQueueHeap(RcvdPacketQueue), ID);;
		Req->Packet = HEAP_STRDUP(DsGetQueueHeap(RcvdPacketQueue), Packet);

		DsQueueElement(RcvdPacketQueue, Req);
	}
	AprsForwardPacket(Packet);
}

static QUEUE_S *DigiPacketQueue = NULL;

static VFUNCTION AprsDequeueLogDigiPacket(POINTER_F Dummy)
{	ROUTINE(AprsDequeueLogDigiPacket);
	APRS_LOG_DIGIPEATED_PACKET_SRQ *Req;
	SERVER_S *Server = CiSetupServer("APRS");
	COUNT_F LastCount = 0, ThisCount, DoneCount = 0;
	
	while ((Req = DsDeQueueElement(DigiPacketQueue)) != NULL)
	{	MESSAGE_S *Msg;
		MILLISECS_F msStart = RtGetMsec();
		double Elapsed;

		while (!CiCheckServerConnection(Server,FALSE))
		{	DgPrintf("%s:Waiting For %s %ld in Queue (%.24s)\n",
				Routine, Server->Name,
				(long) DsGetQueueCount(DigiPacketQueue),
				ctime(&Req->When));
			ThSleepThread(60000L);
		}

		Msg = CiSendRequestTo("APRS", "APRS_LOG_DIGIPEATED_PACKET", Req, sizeof(*Req));
		if (!Msg->Success)
		{	STRING_F Text = CiPopulateResponseStatusText(Msg);
			DgPrintf("Failed To Log %.24s %s Reason %s\n", ctime(&Req->When), Req->Packet, Text);
			THREAD_FREE(Text);
		}
		CiDestroyMessage(Msg);
		
		DoneCount++;
		ThisCount = DsGetQueueCount(DigiPacketQueue);
		Elapsed = RtGetMsec()-msStart;
		if ((ThisCount > LastCount && ThisCount >100) || Elapsed > 1000)
		{	DgPrintf("UDP(DigiPacketQueue) Has %ld (Was %ld) Entries after %.2lf msec LOG_DIGIPEATED_PACKET (%ld Total)\n",
				(long) ThisCount, (long) LastCount,
				(double) Elapsed,
				(long) DoneCount);
		}
		LastCount = ThisCount;
		
		HEAP_FREE(DsGetQueueHeap(DigiPacketQueue),Req->ReceivedBy);
		HEAP_FREE(DsGetQueueHeap(DigiPacketQueue),Req->Packet);
		HEAP_FREE(DsGetQueueHeap(DigiPacketQueue),Req);
	}
	DgPrintf("%s Terminating\n", Routine);
	ThTerminateThread();
}

static VFUNCTION AprsLogDigipeatedPacket
(	TIMESTAMP_F Timestamp,
	STRING_F ID,
	STRING_F Packet
)
{static	FIRST_TIME_ONLY_S Info = {0};

	if (SmFirstTimeOnly(&Info,HERE))
	{	if (SySymbolDefined("TCP_APRS"))
		{	DESCRIPTOR_S *Description = CiGetDescription("APRS","APRS_LOG_DIGIPEATED_PACKET_SRQ",FALSE);
			DigiPacketQueue = DsCreateQueue("PendingDigiPackets", AprsDequeueLogDigiPacket,
							NULL, 0, 1,
							Description, HERE);
		} else DgPrintf("TCP_APRS Not Defined, NOT forwarding Digi packets\n");
		SmEndFirstTimeOnly(&Info,HERE);
	}

	if (DigiPacketQueue)
	{	APRS_LOG_DIGIPEATED_PACKET_SRQ *Req;

		Req = HEAP_MALLOC(DsGetQueueHeap(DigiPacketQueue),sizeof(*Req));
		Req->When = Timestamp?Timestamp:RtNow(NULL);
		Req->ReceivedBy = HEAP_STRDUP(DsGetQueueHeap(DigiPacketQueue), ID);;
		Req->Packet = HEAP_STRDUP(DsGetQueueHeap(DigiPacketQueue), Packet);

		DsQueueElement(DigiPacketQueue, Req);
	}
	/* AprsForwardPacket(Packet); */
}

VFUNCTION UDPListener(POINTER_F Dummy)
{	STRING_F UDPPort = Dummy;
	char *e;
	int port;

#define BUFLEN 2048
	struct sockaddr_in si_me, si_other;
	int n, s, slen=sizeof(si_other);
	char buf[BUFLEN], *last = NULL;
	int dup=0;

	port = strtol(UDPPort,&e,10);
	if (*e) DgPrintf("Non-numeric port(%s), Using %ld\n", UDPPort, (long) port);

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		psock_errno("socket");
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (struct sockaddr *) &si_me, sizeof(si_me))==-1)
	{	psock_errno("bind");
	} else
	{

DgPrintf("Listening on UDP Port %ld\n", (long) port);
DgDirectPrintf("APRSISCE", "Listening on UDP Port %ld\n", (long) port);

	while (!CiIsShuttingDown())
	{
		if ((n=recvfrom(s, buf, BUFLEN-1, 0, (struct sockaddr *)&si_other, &slen))==-1)
		{	psock_errno("recvfrom()");
			DgPrintf("recvfrom(UDP) Failed!\n");
			break;
		}
		buf[n] = '\0';
		if (!last || strcmp(buf, last))
		{	struct in_addr xaddr;
			struct hostent *hostnm;
			char *name, *ipadd;
			COUNT_F PrintSize = strlen(buf)+1024;
			MILLISECS_F msStart = RtGetMsec(), msElapsed;

			if (last) free(last);
			last = strdup(buf);	/* Used below as an original copy of buf */
			if (dup) printf("Ignored %ld UDP Duplicates\n", (long) dup); dup = 0;

			ipadd = inet_ntoa(si_other.sin_addr);
			name = SyGetSymbolValue(ipadd);
			if (!name)	/* Only look it up the first time! */
			{	xaddr = si_other.sin_addr;
				hostnm = gethostbyaddr((void*)&xaddr,sizeof(xaddr),AF_INET);
				if (hostnm)
				{	name = hostnm->h_name;
				} else name = ipadd;
				SySetSymbolValue(ipadd, name);
			}
			msElapsed = RtGetMsec()-msStart;
			if (msElapsed > 1000)
				DgBigPrintf(PrintSize, "Received packet from %s:%d (%s) (rDNS:%.2lfmsec)\nData: %s\n\n", 
					inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), name, (double) msElapsed, buf);
			DgDirectBigPrintf("APRSISCE", PrintSize,
				"Received packet from %s:%d (%s)\nData: %s\n\n", 
			  	 inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), name, buf);
			LgSprintfBigEvent("PACKETS",".LOG",-1,"APRSISCE", 0, TRUE, PrintSize,
				"Received packet from %s:%d\nData: %s\n\n", 
			  	 ipadd, ntohs(si_other.sin_port), buf);

/*           1         2         3         4         5         6         7*/
/* 0123456789012345678901234567890123456789012345678901234567890123456789 */
/* 2009-10-06T13:19:36 KJ4ERJ-12@27.99896,-80.66700 (2009/10/06 08:39) */
/* 2009-10-06T13:19:36 K@27.99896,-80.66700 (2009/10/06 08:39) */
			if (strlen(buf) > 32
			&& buf[10] == 'T' && buf[19] == ' '
			&& RtStrnchr(10,&buf[20],'@')	/* Coordinate? */
			&& strchr(RtStrnchr(10,&buf[20],'@'),','))	/* Comma inside Coordinate? */
			{	COORDINATE_S Coord = {0};
				STRING_F Version = "";
				STRING_F ID = &buf[20];
				STRING_F e, pCoord = RtStrnchr(10,&buf[20],'@');
				*pCoord++ = '\0';	/* Null terminate ID */
				Coord.Latitude = strtod(pCoord,&e);
				if (*e == ',')
					Coord.Longitude = strtod(e+1,&e);
				else Coord.Latitude = 0;
				if (*e)	/* More after coordinates? */
				{	char *p1 = strchr(e,'(');
					if (p1)
					{	char *p2 = strchr(p1,')');
						if (p2)
						{	Version = p1+1;
							*p2 = '\0';	/* Null terminate version */
							LgSprintfEvent("PACKETS",".LOG",-1,"APRSISCE", 0, TRUE,
									"%s Running Version %s", ID,  Version);
							DgPrintf("%s Running Version %s", ID,  Version);
							DgDirectPrintf("APRSISCE","%s Running Version %s", ID,  Version);
						}
					}
				}
				AprsUpdateUser(ID, 0, &Coord, ipadd, name, Version);
/* # logresp KJ4ERJ-12 verified, server T2BELGIUM, adjunct "filter m/112 b/AI4GK* b/KJ4ERJ* b/KJ4DXK* b/KJ4OVQ* u/APZW32/APWM* -b/WINLINK" OK - Filter definition updated */
/* # logresp KC5BSD-10 verified, server T2MIDWEST, adjunct "07:43" Missing filter keyword) */
/*           1         2         3         4         5         6         7*/
/* 0123456789012345678901234567890123456789012345678901234567890123456789 */
/* 2010-03-17T03:32:30 KB2HSH (2010/03/16 21:39) # logresp KB2HSH verified, server T2BELGIUM, adjunct "filter m/241" OK - Filter definition updated */
/*                            012345678901234567 */

			} else if (strstr(buf,"# logresp"))
			{	STRING_F pBlank=NULL, pOpen=NULL, pClose=NULL, Version;
				STRING_F ID = &buf[20];

				DgDirectBigPrintf("APRSISCE", PrintSize, "LogResp: %s\n", buf);

				pBlank = RtStrnchr(10,&buf[20],' ');	/* Got space after callsign? */
				if (pBlank) pOpen = RtStrnchr(5,pBlank,'(');	/* Got open ( on version? */
				if (pOpen) pClose = RtStrnchr(40,pOpen,')');	/* Got close ) on version? */

				if (strlen(buf) > 32
				&& buf[10] == 'T' && buf[19] == ' '
				&& pBlank && pOpen && pClose)
				{	*pBlank++ = '\0';	/* Null terminate ID */
					Version = pOpen+1;
					*pClose = '\0';	/* Null terminate version */
					LgSprintfEvent("PACKETS",".LOG",-1,"APRSISCE", 0, TRUE,
							"%s Running Version %s", ID,  Version);
					DgPrintf("%s Running Version %s", ID,  Version);
					DgDirectPrintf("APRSISCE","%s Running Version %s", ID,  Version);
					AprsUpdateUser(ID, 0, NULL, ipadd, name, Version);
				}
/*           1         2         3         4         5         6         7*/
/* 0123456789012345678901234567890123456789012345678901234567890123456789 */
/* 2009-10-06T13:19:36 DEBUG:KJ4ERJ-12:<Pkt Data Here> */
			} else if (strlen(buf) > 32
			&& buf[10] == 'T' && buf[19] == ' '
			&& !strncmp(&buf[20],"DEBUG:",6)	/* Got ID prefix */
			&& RtStrnchr(10,&buf[26],':'))	/* Trailing : after ID? */
			{	STRING_F ID = &buf[26];
				STRING_F pPkt = RtStrnchr(10,ID,':');
				*pPkt++ = '\0';	/* Null terminate ID & point to packet */
				LgSprintfBigEvent("PACKETS", ".LOG", -1, "DEBUG", 0, TRUE, PrintSize,
								"%s %s\n", ID, pPkt);
				DgBigPrintf(PrintSize, "%s %s", ID,  pPkt);
				DgDirectBigPrintf("DEBUG", PrintSize, "%s %s", ID,  pPkt);
/*           1         2         3         4         5         6         7*/
/* 0123456789012345678901234567890123456789012345678901234567890123456789 */
/* 2009-10-06T13:19:36 IGated:KJ4ERJ-12:<Pkt Data Here> */
			} else if (strlen(buf) > 32
			&& buf[10] == 'T' && buf[19] == ' '
			&& (!strncmp(&buf[20],"IGated:",7)	/* Got ID prefix */
				|| !strncmp(&buf[20],"DXrprt:",7)
				|| !strncmp(&buf[20],"Msg2RF:",7)
				|| !strncmp(&buf[20],"Pos2RF:",7)
				|| !strncmp(&buf[20],"ISFltr:",7)
				|| !strncmp(&buf[20],"IStoRF:",7)
				|| !strncmp(&buf[20],"RFtoRF:",7))
			&& RtStrnchr(10,&buf[27],':'))	/* Trailing : after ID? */
			{	STRING_F ID = &buf[27];
				STRING_F pPkt = RtStrnchr(10,ID,':');
				// APRS_LOG_RECEIVED_PACKET_SRQ Req;	/* LOG_DIGIPEATED_PACKET is the same! */
				// MESSAGE_S *Msg;

				*pPkt++ = '\0';	/* Null terminate ID & point to packet */
				if (*pPkt == '[')
				{	char *e;
					long Seq = strtol(pPkt+1,&e,10);
					if (*e == ']')
					{	STRING_F Symbol = DsMakeSubName("UDPSEQ", ID);
						COUNT_F Last = SySymbolTrue(Symbol);
						if (Seq != Last+1)
						{	if (!Last)
								DgPrintf("UDP(%s) Starting Sequence %ld\n", ID, (long) Seq);
							else if (Seq == Last)
								DgPrintf("UDP(%s) Duplicate Sequence %ld\n", ID, (long) Seq);
							else if (Seq > Last)
								DgPrintf("UDP(%s) Missed %ld Packets B4 Sequence %ld\n", ID, (long) Seq-Last-1, Seq);
							else DgPrintf("UDP(%s) Sequence Dropped From From %ld to %ld, Restart?\n", ID, (long) Last, (long) Seq);
						}
						*e = '\0';	/* Null out sequence */
						SySetSymbolValue(Symbol, pPkt+1);	/* And remember for next packet */
						THREAD_FREE(Symbol);
						pPkt = e+1;	/* Packet comes AFTER the [seq] */
					} else DgPrintf("Bad Sequence Received from %s in %s\n", ID, pPkt);
				}

				if (!strncmp(&buf[20],"RFtoRF:",7))	/* Digipeat? */
					AprsLogDigipeatedPacket(0, ID, pPkt);
				else if (!strncmp(&buf[20],"IGated:",7))
					AprsLogReceivedPacket(0, ID, pPkt);
				else
				{	STRING_F Action = THREAD_STRDUP(&buf[20]);
					Action[6] = '\0';
					LgLogEvent("PACKETS", ".LOG", -1, Action, 0, strlen(last), last, FALSE);
					THREAD_FREE(Action);
				}
			} else
			{	DgDirectBigPrintf("APRSISCE", PrintSize, "Unrecognized Data(%s)\n", buf);
				LgLogEvent("PACKETS", ".LOG", -1,"UnRecognized", 0, strlen(buf), buf, FALSE);
			}

		} else dup++;
	}
	}
	soclose(s);
	DgPrintf("UDPListen Exiting!\n");

	if (!CiIsShuttingDown())
	{	DgPrintf("Restaring UDPListener\n");
		ThCreateThread("UDPListener", UDPListener, Dummy, HERE);
	}
}


