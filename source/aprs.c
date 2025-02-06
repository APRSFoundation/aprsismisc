/* aprsfl.net:10158 */

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <df/base.h>

#include <uf/include/base.h>
#include <uf/source/dgprint.h>
#include <uf/source/dsarray.h>
#include <uf/source/dsqueue.h>
#include <uf/source/dsrtns.h>
#include <uf/source/flrtns.h>
#include <uf/source/hprtns.h>
#include <uf/source/lgrtns.h>
#include <uf/source/lg2rtns.h>
#include <uf/source/perfrtns.h>
#include <uf/source/rtrtns.h>
#include <uf/source/smrtns.h>
#include <uf/source/syrtns.h>
#include <uf/source/tgrtns.h>
#include <uf/source/thrtns.h>
#include <uf/source/xmlrtns.h>

#include <db/include/dbdef.h>
#include <db/source/dbaccess.h>

#include <ci/include/cidef.h>
#include <ci/include/messages.h>

#include <ci/source/ciarray.h>
#include <ci/source/ciauth.h>
#include <ci/source/ciconn.h>
#include <ci/source/ciconv.h>
#include <ci/source/cidescr.h>
#include <ci/source/cidisp.h>
#include <ci/source/cifile.h>
#include <ci/source/ciinit.h>
#include <ci/source/cinames.h>
#include <ci/source/ciproxy.h>
#include <ci/source/citcp.h>

#define INCL_RXSHV	/* Shared variable support */
#define INCL_RXFUNC	/* External functions support */
#define INCL_RXSUBCOM

#include <rx\source\rexxsaa.h>

#include <ui/include/uidef.h>
#include <ui/source/call.h>
#include <ui/source/init.h>
#include <ui/source/util.h>

#ifdef FUTURE
#include <ht/include/SGML.h>
#include <ht/include/Htext.h>
#include <ht/include/htlist.h>
#include <ht/include/htmlpdtd.h>
#endif

#include <ht/include/HTParse.h>
#include <ht/source/HTParse.h>

#include <aprs/include/database.h>
#include <aprs/include/messages.h>

#include <aprs/source/database.h>
#include <aprs/source/osmimg.h>
#include <aprs/source/parsedef.h>
#include <aprs/source/parse.h>
#include <aprs/source/services.h>

#include <aprs/source/thdelay.h>

#include <aprs/source/server.h>
#include <aprs/source/tcputil.h>
#include <aprs/source/udplistn.h>

#ifdef FUTURE
void ShowPrintable(CALLER)
{	char Output[257] = {0};
	int i;

	for (i=0; i<256; i++)
		Output[i] = isprint(i)?'1':'0';
	DgPrintf("Locale:LC_CTYPE(%s) LC_ALL(%s)\n", setlocale(LC_CTYPE,NULL), setlocale(LC_ALL,NULL));
	DgPrintf("Printable(%s:%ld):%s\n", File, (long) Line, Output);
}
#endif

/*-----------------------------------------------------------------------------
 * This function is called to invoke a named symbol routine and return its value

	Value = AprsCallSymbolRoutine("NamedSymbolRoutine")

 *----------------------------------------------------------------------------*/
static ULONG AprsCallSymbolRoutine
(
        PSZ		name,
        ULONG           argc,
        RXSTRING     	argv[],
        PSZ		stck,
        RXSTRING	*retstr
)
{	ROUTINE(AprsCallSymbolRoutine);
	UI_REXX_INFO_S                  *Info;
	ULONG                           UserData[2];
	STRING_F			Value = NULL;

        if (argc == 0) return 1;

        RexxQueryExit("RxInit", NULL, (PUCHAR)UserData); /* Get User data */
        Info = (UI_REXX_INFO_S *) UserData[0];

DgPrintf("Invoking %.*s\n", argv[0].strlength, argv[0].strptr);
	if (argv[0].strptr[argv[0].strlength]) KILLPROC(-1,"SymbolRoutine Not Null Terminated");
	if (argc >= 2 && argv[1].strptr[argv[1].strlength]) KILLPROC(-1,"SymbolRoutine Args Not Null Terminated");

	Value = UiCallNamedSymbolRoutine(argv[0].strptr, Info->HTMLReq, Info->Form, argc<2?NULL:argv[1].strptr);
	if (!Value) return 1;

	retstr->strptr = Value;
	retstr->strlength = strlen(Value);

	return 0;
}

BOOLEAN_F AprsAuthorize(MESSAGE_S *Req, STRING_F URL, STRING_F Username, STRING_F Password, POINTER_F UserArg)
{	BOOLEAN_F Result = FALSE;

	if (Username && Password)
		Result = !stricmp(Username,"APRSUSER") && !stricmp(Password,"APRSPASS");

	Result = TRUE;	/* Authentication no longer required */

	DgPrintf("AprsAuthorize: User(%s) Pass(%s)\n", Username?Username:"*NULL*", Result?"*SUPPRESSED*":(Password?Password:"*NULL*"));
	return Result;
}

STRING_F CALLSIGN = "KJ4ERJ-M";
STRING_F PASSWORD = "24231";
#define LOGON "user KJ4ERJ-M pass 24231 vers aprs coverage analyzer 0.1"
#define FILTER "#filter r/27.99673/-80.659072/500 b/KJ4ERJ* e/KJ4ERJ* f/KJ4ERJ-12/100 f/KJ4ERJ-JP/10 u/APZA4C"
#define PING "#"

void myMessageCallback(void *userarg, char *from, char *message)
{
	LgSprintfEvent("PACKETS", ".LOG", -1,"MSG", 0, FALSE, "From: %s Msg: %s", from, message);

	if (!strncmp(message, "ack", 3))
	{	DgPrintf("From: %s ACK: %s\n", from, message);
	} else
	{	char *ack = strchr(message, '{');	/* Do we need to ack it? */
		if (ack)
		{	char Buffer[80];
			int len = sprintf(Buffer,"%s>APRS,TCPIP*::%-9s:ack%s\n", CALLSIGN, from, ack+1);
			DgPrintf("Sending Ack: %s\n", Buffer);
			tcp_transmit_buffer(len, Buffer, 1000);
		}
		DgPrintf("From: %s Msg: %s\n", from, message);
	}
}

void myBulletinCallback(void *userarg, char *from, char identifier, char *group, char *bulletin)
{	LgSprintfEvent("PACKETS", ".LOG", -1,"BLT", 0, FALSE, "Bulletin[%c:%s] From: %s Received: %s\n", identifier, group, from, bulletin);
	DgPrintf("Bulletin[%c:%s] From: %s Received: %s\n", identifier, group, from, bulletin);
}

POINTER_ARRAY_S *APRSXmitQueue = NULL;

VFUNCTION AprsQueueXmitPacket(STRING_F Packet)
{	if (!APRSXmitQueue) APRSXmitQueue = DsCreatePointerArray("APRSXmitQueue", NULL, HERE);
	DgPrintf("Queueing(%s)[%ld]\n", Packet, DsGetPointerArrayCount(APRSXmitQueue));
	DgDirectPrintf("XmitQueue", "Queueing(%s)\n", Packet);
	Packet = HEAP_STRDUP(DsGetPointerArrayHeap(APRSXmitQueue),Packet);
	DsAddToPointerArray(APRSXmitQueue, Packet);
}

static QUEUE_S *LogPacketQueue = NULL;

static VFUNCTION AprsDequeueLogPacket(POINTER_F Dummy)
{	ROUTINE(AprsDequeueLogPacket);
	APRS_LOG_PACKET_SRQ *Req;
	SERVER_S *Server = CiSetupServer("APRS");

	while ((Req = DsDeQueueElement(LogPacketQueue)) != NULL)
	{	MESSAGE_S *Msg;

		while (!CiCheckServerConnection(Server,FALSE))
		{	DgPrintf("%s:Waiting For %s %ld in Queue (%.24s)\n",
				Routine, Server->Name,
				(long) DsGetQueueCount(LogPacketQueue),
				ctime(&Req->When));
			ThSleepThread(60000L);
		}

		Msg = CiSendRequestTo("APRS", "APRS_LOG_PACKET", Req, sizeof(*Req));
		if (!Msg->Success)
		{	STRING_F Text = CiPopulateResponseStatusText(Msg);
			DgPrintf("Failed To Log %.24s %s Reason %s\n", ctime(&Req->When), Req->Packet, Text);
			THREAD_FREE(Text);
		}
		CiDestroyMessage(Msg);
		HEAP_FREE(DsGetQueueHeap(LogPacketQueue),Req->Packet);
		HEAP_FREE(DsGetQueueHeap(LogPacketQueue),Req);
	}
	DgPrintf("%s Terminating\n", Routine);
	ThTerminateThread();
}

VFUNCTION AprsLogPacket
(	TIMESTAMP_F Timestamp,
	STRING_F Packet
)
{static	FIRST_TIME_ONLY_S Info = {0};
	APRS_LOG_PACKET_SRQ *Req;

	if (SmFirstTimeOnly(&Info,HERE))
	{	if (SySymbolDefined("TCP_APRS"))
		{
			DESCRIPTOR_S *Description = CiGetDescription("APRS","APRS_LOG_PACKET_SRQ",FALSE);
			LogPacketQueue = DsCreateQueue("PendingLogPackets", AprsDequeueLogPacket,
							NULL, 0, 1,
							Description, HERE);
			DgPrintf("Forwarding APRS packets to %s\n", SyGetSymbolValue("TCP_APRS"));
		} else DgPrintf("TCP_APRS not defined, NOT forwarding packets\n");
		SmEndFirstTimeOnly(&Info,HERE);
	}

	if (LogPacketQueue)
	{
		Req = HEAP_MALLOC(DsGetQueueHeap(LogPacketQueue),sizeof(*Req));
		Req->When = Timestamp?Timestamp:RtNow(NULL);
		Req->Packet = HEAP_STRDUP(DsGetQueueHeap(LogPacketQueue), Packet);

		DsQueueElement(LogPacketQueue, Req);
	}
}

static VFUNCTION AprsPoller(POINTER_F Dummy)
{	STRING_F ServerPort = Dummy;
	char *p, *e;
	int port;
	int status;
	char InBuf[1024];
	unsigned short len;
	char *Filter = SyGetSymbolValueOrDefault("FILTER",FILTER);

	CALLSIGN = SyGetSymbolValueOrDefault("APRSCALL", "KJ4ERJ-M");
	PASSWORD = SyGetSymbolValueOrDefault("APRSPASS", "24231");

	p = strchr(ServerPort,':');
	port = strtol(p+1,&e,10);
	if (*e) DgPrintf("Non-numeric port(%s), Using %ld\n", p+1, (long) port);
	*p = '\0';

	set_message_handler(CALLSIGN, myMessageCallback, NULL);
	set_bulletin_handler(myBulletinCallback, NULL);

	while (!CiIsShuttingDown())
	{	char Buffer[256];

		tcp_set_host_port(ServerPort,port);	/* Force a resolution for round-robin DNS */
		if (!tcp_connect())
		{	DgPrintf("Failed To Connect To %s:%ld\n", ServerPort, (long) port);
			goto sleep;
		}


		sprintf(Buffer,"user %s pass %s vers APRSMonitor v0.1", CALLSIGN, PASSWORD);
		status = tcp_transmit_buffer(strlen(Buffer), Buffer, 10000);
		if (status)
		{	DgPrintf("Transmit(%s) Failed with %ld\n", LOGON, (long) status);
			tcp_disconnect();
			goto sleep;
		}

		if (!RtStrnWhite(-1,Filter))
		{
			status = tcp_transmit_buffer(strlen(Filter), Filter, 10000);
			if (status)
			{	DgPrintf("Transmit(%s) Failed with %ld\n", Filter, (long) status);
				tcp_disconnect();
				goto sleep;
			}
		}

		while ((status=tcp_receive_buffer(sizeof(InBuf)-1,InBuf,15000,&len)) == 0 || status == 1)
		{	if (APRSXmitQueue)
			{	STRING_F Packet;
				while ((Packet = DsIndexPointerArray(APRSXmitQueue,0)) != NULL)
				{	DgPrintf("Transmitting(%s)[%ld]\n", Packet, DsGetPointerArrayCount(APRSXmitQueue));
					status = tcp_transmit_buffer(strlen(Packet), Packet, 5000);
					if (status)
					{	DgPrintf("Transmit(%s) Failed with %ld\n", Packet, (long) status);
					}
					if (!DsRemoveFromPointerArray(APRSXmitQueue, Packet))
						DgPrintf("Failed To Remove(%s) From APRSXmitQueue\n", Packet);
					HEAP_FREE(DsGetPointerArrayHeap(APRSXmitQueue),Packet);
				}
			}

			if (status)
			{	DgPrintf("Pinging due to status %ld...\n", (long) status);
				status = tcp_transmit_buffer(strlen(PING), PING, 10000);
				if (status)
				{	DgPrintf("Transmit(%s) Failed with %ld\n", PING, (long) status);
					tcp_disconnect();
					break;
				}
				continue;
			}

			if (!len) continue;	/* Ignore short buffers */
#ifdef VERBOSE
			DgPrintf("%.*s\n", (int) len, InBuf);
#endif
			if (InBuf[0] == '#') continue;	/* Ignore server comments */

			InBuf[len] = '\0';	/* Guarantee null termination */
			AprsLogPacket(0,InBuf);
		}
		DgPrintf("Receive Failed With %ld, Poller Delaying retry\n", (long) status);
sleep:
		ThSleepThread(30000);
		DgPrintf("Poller Restarting\n", (long) status);
	}

	tcp_disconnect();
}



#ifdef FUTURE
/*
Field Length Meaning
AGWPE Port 1 Bytes [0..n] the least significant value comes in the first byte while the most significant in the second. I.E. Port 2 would be expressed as 0x01 ([4])
Reserved 3 Bytes Usually 0x00 0x00 0x00
DataKind 1 Byte Is the frame code, reflects the purpose of the frame. The meaning of the DataKind DO VARY depending on whether the frame flows from the application to AGWPE or viceversa.
Reserved 1 Byte Usually 0x00
PID 1 Byte Frame PID, it’s usage is valid only under certain frames only. Should be 0x00 when not used.
Reserved 1 Byte Usually 0x00
CallFrom 10 Bytes CallSign FROM of the packet, in ASCII, using the format {CALLSIGN}-{SSID} (i.e. LU7DID-8) it is “null terminated” (it ends with 0x00). ([5])
            The field ALWAYS is 10 bytes long. It’s filled on packets where it has some meaning.
CallTo 10 Bytes CallSign TO of the packet, same as above.
DataLen 4 Bytes Data Length as a 32 bits unsigned integer. If zero means no data follows the header.
User (Reserved) 4 Bytes 32 bits unsigned integer, not used. Reserved for future use.
*/
#endif
typedef struct AGW_HEADER
{	char Port;
	char Unused0[3];
	char DataKind;
	char Unused1;
	char PID;
	char Unused2;
	char CallFrom[10];
	char CallTo[10];
	long DataLen;
	long Reserved;
} AGW_HEADER;

static VFUNCTION AprsAGWMonitor(POINTER_F Dummy)
{	STRING_F Monitor = Dummy;
	STRING_F ServerPort, CallSign;
	char *p, *e;
	TCP_SOCKET_F s = 0;
	int port;
	int InLen = 0;
static	char InBuf[2048];

	if (!RtIsRoutineCall(Monitor, &CallSign, &ServerPort))
	{	DgPrintf("%s Is Not CallSign(IP:Port)\n", Monitor);
		return;
	}
	p = strchr(ServerPort,':');
	port = strtol(p+1,&e,10);
	if (*e) DgPrintf("Non-numeric port(%s), Using %ld\n", p+1, (long) port);
	*p = '\0';

	DgPrintf("AGWMonitoring %s via %s:%ld\n", CallSign, ServerPort, (long) port);

	while (!CiIsShuttingDown())
	{	if (!s)
		{	s = CiTcpConnectSocket(ServerPort, port, NULL, NULL, NULL, HERE);
			if (!s)
			{	DgPrintf("Failed Connection to %s::%ld\n", ServerPort, (long) port);
			} else
			{	AGW_HEADER Head = {0};
				Head.DataKind = 'm';
				if (CiTcpSend(s, (char *) &Head, sizeof(Head), 0) < 0)
				{	DgPrintf("CiTcpSend(m) Failed, Disconnecting\n");
					CiTcpCloseSocket(s);
					s = 0;
				} else InLen = 0;
			}
		}
/*
	Now check each of the client sockets
*/
		else
		{	int n;
			AGW_HEADER *Head;

			n = sizeof(InBuf)-InLen-1;
			if (n <= 0)
			{	DgPrintf("*** Resetting Buffer, Have %ld / %ld bytes\n", (long) InLen, sizeof(InBuf));
				InLen = 0;
				n = sizeof(InBuf)-1;
			}
			if ((n=CiTcpRecv(s, InBuf+InLen, n, 0)) == -1)
			{	DgPrintf("CiTcpRecv returned %ld, disconnecting %ld\n", (long) n, (long) s);
				CiTcpCloseSocket(s);
				s = 0;
			} else if (!n)
			{	DgPrintf("Received 0 bytes, disconnecting %ld\n", (long) s);
				CiTcpCloseSocket(s);
				s = 0;
			}
			InLen += n;

			if (InLen < sizeof(*Head))
			{	DgPrintf("Need Head, Have %ld / %ld Bytes\n", (long) InLen, (long) sizeof(*Head));
				continue;
			}

			InBuf[InLen] = 0;
			Head = (void *) InBuf;
			while (InLen >= sizeof(*Head)+Head->DataLen)
			{	char *Data = &InBuf[sizeof(*Head)];
				char *Fm = strstr(Data,":Fm ");
				char *To = strstr(Data," To ");
				char *Via = strstr(Data," Via ");
				char *End = strstr(Data," <");

				if (Fm && To && End)
				{	char *UI = strstr(Data," <UI");
					char *CR = strstr(Data,"]\r");
					char *CRCR = strstr(Data,"\r\r");
					if (UI && CR && CRCR)
					{	char *Ax25 = malloc(Head->DataLen);
						CR += 2;
						if (Via)
						{	Via += 5;
							sprintf(Ax25, "%.*s>%.*s,%.*s:%.*s\r\n",
								sizeof(Head->CallFrom), Head->CallFrom,
								sizeof(Head->CallTo), Head->CallTo,
								(int) (End-Via), Via,
								(int) (CRCR-CR), CR);
						} else 	sprintf(Ax25, "%.*s>%.*s:%.*s\r\n",
								sizeof(Head->CallFrom), Head->CallFrom,
								sizeof(Head->CallTo), Head->CallTo,
								(int) (CRCR-CR), CR);

#ifdef OLD_WAY
						AprsLogReceivedPacket(0, CallSign, Ax25);
#else
						{	APRS_LOG_RECEIVED_PACKET_SRQ Req;
							MESSAGE_S *Msg;
							Req.When = RtNow(NULL);
							Req.ReceivedBy = CallSign;
							Req.Packet = Ax25;
							Msg = CiSendRequestTo("APRS","APRS_LOG_RECEIVED_PACKET", &Req, sizeof(Req));
							if (!Msg->Success)
							{	STRING_F Text = CiPopulateResponseStatusText(Msg);
								DgPrintf("Failed To Log Packet, Reason %s\n", Text);
								THREAD_FREE(Text);
							}
							CiDestroyMessage(Msg);
						}
#endif
						free(Ax25);
					} else
					{	DgPrintf("Non-UI %ld Bytes AGW(%s)\n", InLen, Data);
					}
					InLen -= sizeof(*Head)+Head->DataLen;
					if (InLen > 0)
					{	DgPrintf("Relocating %ld pending bytes\n", (long) InLen);
						memmove(InBuf, &InBuf[sizeof(*Head)+Head->DataLen], InLen);
					} else if (InLen < 0)
					{	DgPrintf("*** Huh?  Relocating %ld pending bytes from %ld+%ld\n",
							(long) InLen, (long) sizeof(*Head), (long) Head->DataLen);
						InLen = 0;
					}
				} else
				{	DgPrintf("*** Missing From/To/End in %ld Bytes of AGW(%s), Flushing\n", InLen, Data);
					InLen = 0;
				}
			}
			if (InLen) DgPrintf("*** Need Body, Have %ld / %ld Bytes (%ld Payload)\n",
					(long) InLen, sizeof(*Head)+Head->DataLen, (long) Head->DataLen);
		}
	} /* end of for(;;) loop */
/*
	We should never get here anyway!
*/
	CiTcpCloseSocket(s);
	THREAD_FREE(ServerPort); THREAD_FREE(CallSign);

	DgPrintf("AGWMonitor ended successfully\n");
}


static VFUNCTION AprsJavaMonitor(POINTER_F Dummy)
{	STRING_F Monitor = Dummy;
	STRING_F ServerPort, CallSign;
	char *p, *e;
	POINTER_F bs = NULL;
	int port;

	if (!RtIsRoutineCall(Monitor, &CallSign, &ServerPort))
	{	DgPrintf("%s Is Not CallSign(IP:Port)\n", Monitor);
		return;
	}
	p = strchr(ServerPort,':');
	port = strtol(p+1,&e,10);
	if (*e) DgPrintf("Non-numeric port(%s), Using %ld\n", p+1, (long) port);
	*p = '\0';

	DgPrintf("JavaMonitoring %s via %s:%ld\n", CallSign, ServerPort, (long) port);

	while (!CiIsShuttingDown())
	{	if (!bs)	/* Need a connection */
		{	ThSetThreadStateStatic("OpenConnection");
			bs = CiTcpOpenBufferedConnection(ServerPort, port, NULL, NULL, NULL, HERE);
			if (!bs)
			{	DgPrintf("Failed Connection to %s::%ld\n", ServerPort, (long) port);
				ThSetThreadStateStatic("FailedConnection");
				ThSleepThread(5000L);
			}
		} else	/* We have a connection, read the data */
		{	STRING_F Buffer;
			ThSetThreadStateStatic("GetString");
			Buffer = CiTcpGetTimedString2(bs, -1, 1000);
			if (!Buffer)
			{	DgPrintf("CiTcpGetTimedString2 returned NULL, disconnecting\n");
				ThSetThreadStateStatic("GetFailed");
				CiTcpCloseBufferedConnection(bs,NULL);
				bs = NULL;
			} else
			{	STRING_F Packet = strchr(Buffer,']');
				ThSetThreadStateStatic("Processing");
				if (Packet++)	/* Found the ] and move on to the actual packet */
				{	if (Buffer[0] == '[' && Buffer[1] == '>')
					{	LgLogEvent("PACKETS", ".LOG", -1,"XMIT", RtNow(NULL), strlen(Packet), Packet, FALSE);
					} else 
					{
#ifdef OLD_WAY
					AprsLogReceivedPacket(0, CallSign, Packet);
#else
					{	APRS_LOG_RECEIVED_PACKET_SRQ Req;
						MESSAGE_S *Msg;
						Req.When = RtNow(NULL);
						Req.ReceivedBy = CallSign;
						Req.Packet = Packet;
						Msg = CiSendRequestTo("APRS","APRS_LOG_RECEIVED_PACKET", &Req, sizeof(Req));
						if (!Msg->Success)
						{	STRING_F Text = CiPopulateResponseStatusText(Msg);
							DgPrintf("Failed To Log Packet, Reason %s\n", Text);
							THREAD_FREE(Text);
						}
						CiDestroyMessage(Msg);
					}
#endif
					}
				}
				else if (*Buffer && strcmp(Buffer,"\r\n")) DgPrintf("Missing ] in %ld bytes of '%s'\n", (long) strlen(Buffer), Buffer);
				THREAD_FREE(Buffer);
			}
		}
	} /* end of while() loop */

	ThSetThreadStateStatic("Closing");
	if (bs) CiTcpCloseBufferedConnection(bs,NULL);
	THREAD_FREE(ServerPort); THREAD_FREE(CallSign);

	DgPrintf("JavaMonitor ended successfully\n");
	ThSetThreadStateStatic("Exiting");
}

static COUNT_F AprsGetTableValue(COUNT_F Bytes, STRING_F Body, STRING_F Label)
{	COUNT_F Result = 0;
	STRING_F Row = RtStrnstr(Bytes, Body, -1, Label, TRUE);
	if (Row)
	{	STRING_F Value = RtStrnstr(Bytes-(Row-Body), Row, -1, "</TD><TD>", TRUE);
		if (Value)
		{	Value += 9;
			Bytes -= (Value-Body);
			while (Bytes-- && *Value && *Value != '<')
			{	if (isdigit(*Value))
					Result = (Result*10) + (*Value-'0');
				Value++;
			}
		}
	}
	return Result;
}
#ifdef FUTURE
/*
<TR align=right><TD align=left>Packets Gated to Server</TD><TD>74,154</TD></TR>
<TR align=right><TD align=left>Packets Gated to RF</TD><TD>0</TD></TR>
<TR align=right><TD align=left>Messages Gated to RF</TD><TD>0</TD></TR>
*/
#endif

static VFUNCTION AprsJavaTrafficMonitor(POINTER_F Dummy)
{	STRING_F Monitor = Dummy;
	STRING_F ServerPort, CallSign;
	BOOLEAN_F First = TRUE;
	TIMESTAMP_F Last = 0;
	COUNT_F Seq = RtNow(NULL) % 255;
	COUNT_F prevRF = 0, prevIS = 0;
	TIMESTAMP_F NextBeacon = 0;

	if (!RtIsRoutineCall(Monitor, &CallSign, &ServerPort))
	{	DgPrintf("%s Is Not CallSign(IP:Port)\n", Monitor);
		return;
	}

	DgPrintf("JavaTrafficMonitoring %s via %s\n", CallSign, ServerPort);

	while (!CiIsShuttingDown())
	{	MESSAGE_S *Msg;
		ThSetThreadStateStatic("CiGetFrom");
		Msg = CiGetFromTimeout(ServerPort, "/", 15000, NULL, NULL, HERE);
		ThSetThreadStateStatic("Processing");
		if (!Msg->Success)
		{	STRING_F Text = CiPopulateResponseStatusText(Msg);
			DgPrintf("GET(%s) Failed With %s\n", ServerPort, Text);
			THREAD_FREE(Text);
			First = TRUE;	/* Force a complete reset! */
		} else
		{	TIMESTAMP_F Now = RtNow(NULL);
			COUNT_F Bytes;
			STRING_F Body = CiGetPostBody(Msg,&Bytes);
			ThSetThreadStateStatic("Parsing");
			if (!RtStrnstr(Bytes, Body, -1, "IGate Adjunct", TRUE))
			{	DgPrintf("No IGate Adjunct from %s\n", ServerPort);
			} else
			{	char Buffer[132];
				COUNT_F GatedIS = AprsGetTableValue(Bytes, Body, "Packets Gated to Server");
				COUNT_F GatedRF = AprsGetTableValue(Bytes, Body, "Packets Gated to RF");
				COUNT_F MsgsRF = AprsGetTableValue(Bytes, Body, "Messages Gated to RF");
				COUNT_F Recent = AprsGetTableValue(Bytes, Body, "Recently Heard Stations");
				COUNT_F Local = AprsGetTableValue(Bytes, Body, "Local RF Stations");
				COUNT_F Direct = AprsGetTableValue(Bytes, Body, "Directly Heard Stations");
				DgPrintf("%s GatedIS:%ld GatedRF:%ld MsgsRF:%ld Recent:%ld Local:%ld Direct:%ld\n",
					CallSign, (long) GatedIS, (long) GatedRF, (long) MsgsRF,
					(long) Recent, (long) Local, (long) Direct);
				if (First)
				{	First = FALSE;
					sprintf(Buffer,"%s>APZAPM::%-9s:PARM.Direct,Local,Recent,IS-RF,RF-IS,B1,B2,B3,B4,B5,B6,B7,B8",CallSign,CallSign);
					AprsQueueXmitPacket(Buffer);
					sprintf(Buffer,"%s>APZAPM::%-9s:UNIT.Stations,Stations,Stations,Packets/Minute,Packets/Minute,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A",CallSign,CallSign);
					AprsQueueXmitPacket(Buffer);
					if (!stricmp(CallSign,"KJ4ERJ-2"))
						sprintf(Buffer,"%s>APZAPM::%-9s:EQNS.0,1,0,0,1,0,0,1,0,0,0.1,0,0,0.1,0",CallSign,CallSign);
					else	sprintf(Buffer,"%s>APZAPM::%-9s:EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,1,0",CallSign,CallSign);
					AprsQueueXmitPacket(Buffer);
					DgDirectPrintf(CallSign, "%s GatedIS:%ld GatedRF:%ld MsgsRF:%ld Recent:%ld Local:%ld Direct:%ld\n",
						CallSign, (long) GatedIS, (long) GatedRF, (long) MsgsRF,
						(long) Recent, (long) Local, (long) Direct, (long) Now - Last);
				} else if (Last < Now)
				{	double minutes = (double)(Now - Last)/60.0;
					COUNT_F DeltaI, DeltaR;
					if (!stricmp(CallSign,"KJ4ERJ-2"))
					{	DeltaI = (double)(GatedIS-prevIS)*10.0/minutes + 0.5;
						DeltaR = (double)(GatedRF+MsgsRF-prevRF)*10.0/minutes + 0.5;
					} else
					{	DeltaI = (double)(GatedIS-prevIS)/minutes + 0.5;
						DeltaR = (double)(GatedRF+MsgsRF-prevRF)/minutes + 0.5;
					}
					DgDirectPrintf(CallSign, "%s GatedIS:%ld (%ld) Gated+MsgsRF:%ld+%ld (%ld) Recent:%ld Local:%ld Direct:%ld Seconds:%ld\n",
						CallSign, (long) GatedIS, (long) DeltaI, (long) GatedRF, (long) MsgsRF, (long) DeltaR,
						(long) Recent, (long) Local, (long) Direct, (long) Now - Last);

					if (DeltaI < 0) DeltaI = 0;
					if (DeltaR < 0) DeltaR = 0;
					sprintf(Buffer,"%s>APZAPM:T#%03ld,%ld,%ld,%ld,%ld,%ld,00000000",CallSign,(long) Seq++,
						(long) Direct, (long) Local, (long) Recent,
						(long) DeltaR, (long) DeltaI);
					AprsQueueXmitPacket(Buffer);
				}
				if (Now >= NextBeacon)
				{	NextBeacon = Now+60*60;
					sprintf(Buffer,"%s>APZAPM:>Gating details at http://aprs.fi/telemetry/%s",CallSign,CallSign);
					AprsQueueXmitPacket(Buffer);
				}
				Last = Now;
				prevIS = GatedIS;
				prevRF = GatedRF+MsgsRF;
			}
		}
		CiDestroyMessage(Msg);
		ThSetThreadStateStatic("Sleeping");
		ThSleepThread(5*60*1000L);	/* Sleep a few minutes */
	}
	THREAD_FREE(ServerPort); THREAD_FREE(CallSign);

	DgPrintf("JavaTrafficMonitor ended successfully\n");
}

#ifdef FUTURE
/*
 2008-11-05 18:02:27 UTC: KJ4ERJ-12>APZA4C,TCPIP*,qAC,T2NUENGLD::KJ4ERJ-12:PARM.Battery,A2,A3,A4,A5,A/C,Charging,B3,B4,B5,B6,B7,B8
 2008-11-05 18:02:27 UTC: KJ4ERJ-12>APZA4C,TCPIP*,qAC,T2NUENGLD::KJ4ERJ-12:UNIT.Percent,N/A,N/A,N/A,N/A,On,Yes,N/A,N/A,N/A,N/A,N/A,N/A
 2008-11-05 18:02:27 UTC: KJ4ERJ-12>APZA4C,TCPIP*,qAC,T2NUENGLD::KJ4ERJ-12:EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,1,0
 2008-11-05 18:02:27 UTC: KJ4ERJ-12>APZA4C,TCPIP*,qAC,T2NUENGLD::KJ4ERJ-12:BITS.11000000,Battery State Tracking
 2008-11-05 18:02:27 UTC: KJ4ERJ-12>APZA4C,TCPIP*,qAC,T2NUENGLD:T#001,5,0,0,0,0,11000000
 2008-11-05 18:02:37 UTC: KJ4ERJ-12>APZA4C,TCPIP*,qAC,T2NUENGLD:@051802z2759.80N/08039.56W$(FIRST)Lynn's AT&T Tilt
 2008-11-05 18:03:24 UTC: KJ4ERJ-12>APZA4C,TCPIP*,qAC,T2APRSWST:T#002,6,0,0,0,0,11000000
 2008-11-05 18:04:33 UTC: KJ4ERJ-12>APZA4C,TCPIP*,qAC,T2APRSWST:T#003,7,0,0,0,0,11000000
 2008-11-05 18:06:00 UTC: KJ4ERJ-12>APZA4C,TCPIP*,qAC,T2APRSWST:T#004,8,0,0,0,0,11000000
 2008-11-05 18:07:03 UTC: KJ4ERJ-12>APZA4C,TCPIP*,qAC,T2APRSWST:T#005,9,0,0,0,0,11000000
<P>
<TABLE bgColor=#d0d0d0 border=2>
  <TBODY>
  <TR align=middle bgColor=#ffd700><TH colspan=2 align=middle>IGate Adjunct</TH></TR>
  <TR align=middle bgColor=#909090><TH align=left>javAPRSIGate 2.3b05</TH><TH>Copyright &copy; 2006 - Peter Loveall AE5PL</TH></TR>
<TR align=right><TD align=left>IGate Callsign</TD><TD>KJ4ERJ</TD></TR>
<TR align=right><TD align=left>Status</TD><TD>Gating to RF</TD></TR>
<TR align=right><TD colspan=2 bgColor=#909090>&nbsp;</TD></TR>
<TR align=right><TD align=left>Packets Gated to Server</TD><TD>74,154</TD></TR>
<TR align=right><TD align=left>Packets Gated to RF</TD><TD>0</TD></TR>
<TR align=right><TD align=left>Messages Gated to RF</TD><TD>0</TD></TR>
<TR align=right><TD colspan=2 bgColor=#909090>&nbsp;</TD></TR>
<TR align=right><TD align=left>Maximum Digi Hops for Local Stations</TD><TD>1</TD></TR>
<TR align=right><TD align=left>History Time for Station Lists (minutes)</TD><TD>30</TD></TR>
<TR align=right><TD colspan=2 bgColor=#909090>&nbsp;</TD></TR>
<TR align=right><TD align=left>Recently Heard Stations</TD><TD>47</TD></TR>
<TR align=right><TD align=left>Local RF Stations</TD><TD>13</TD></TR>
<TR align=right><TD align=left>Directly Heard Stations</TD><TD>5</TD></TR>
<TR align=right><TD colspan=2 bgColor=#909090>&nbsp;</TD></TR>
<TR align=right><TD align=left>Bytes Sent to RF</TD><TD>69,724</TD></TR>
<TR align=right><TD align=left>Bytes Received from RF</TD><TD>6,253,906</TD></TR>
<TR align=right><TD colspan=2 bgColor=#909090>&nbsp;</TD></TR>
<TR align=right><TD align=left>AGW Interface 4.0b02</TD><TD>Copyright &copy; 2005 - Pete Loveall AE5PL</TD></TR>
<TR align=right><TD align=left>TNC Port Number</TD><TD>1</TD></TR>
</TBODY></TABLE></P>
*/
#endif


static char CharPower(char *Power)
{	char *e;
	long value = strtol(Power,&e,10);

	if ( value < 1 )
    	return '0';

	if ( value >= 1 && value < 4 )
    	return '1';

	if ( value >= 4 && value < 9 )
    	return '2';

	if ( value >= 9 && value < 16 )
	    return '3';

	if ( value >= 16 && value < 25 )
	    return '4';

	if ( value >= 25 && value < 36 )
	    return '5';

	if ( value >= 36 && value < 49 )
	    return '6';

	if ( value >= 49 && value < 64 )
	    return '7';

	if ( value >= 64 && value < 81 )
	    return '8';

	if ( value >= 81 )
	    return '9';

	return '?';
}

static char CharHeight(char *Height)
{	char *e;
	long value = strtol(Height,&e,10);

	return '0'+(long)(log2(((double)value)/10.0));
}

static char CharGain(char *Gain)
{	char *e;
	long value = strtol(Gain,&e,10);

    if (value > 9)
        return '9';

    if (value < 0)
        return '0';

    return '0'+value;
}



static char CharDirection(char *Direction)
{
	if (*Direction < '0' || *Direction > '8') return '?';
	return *Direction;
}

static char *PHG(char *Power, char *Height, char *Gain, char *Direction)
{	char *Output = THREAD_MALLOC(8);
	sprintf(Output, "PHG%c%c%c%c",
		CharPower(Power), CharHeight(Height),
		CharGain(Gain), CharDirection(Direction));
	return Output;
}

static char *RANGE(char *Power, char *Height, char *Gain, char *Direction)
{	char *e;
	double p = strtod(Power,&e);
	double h = strtod(Height,&e);
	double g = pow(10,strtod(Gain,&e)/10);
	double Range = sqrt(2*h*sqrt((p/10)*(g/2)));
	long r = Range+0.5;
	char *Output = THREAD_MALLOC(5);
static	char *Prefix[] = { "R", "NE", "E", "SE", "S", "SW", "W", "NW", "N" };

	if (r>99) r = 99;

	if (*Direction < '0' || *Direction > '9')
		sprintf(Output,"R%02ldm", (long) r);
	else
	{	char *Pre = Prefix[*Direction-'0'];
		sprintf(Output,"%s%02ld%s", Pre, (long) r, strlen(Pre)==1?"m":"");
	}
	return Output;
}

static char *FREQ(char *Freq, char *Tone, char *sPHG, char *sRange)
{	char *e;
	double f = strtod(Freq,&e);
	long t = strtol(Tone,&e,10);
	char *Output = THREAD_MALLOC(max(21,strlen(Freq)+strlen(Tone)+strlen(sPHG)+3));
static	struct
{	double Base;
	char Prefix;
} Freqs[] = {	{ 1200, 'A' }, { 2300, 'B' }, { 2400, 'C' },
		{ 3400, 'D' }, { 5600, 'E' }, { 5700, 'F' },
		{ 5800, 'G' }, { 10100, 'H' }, { 10200, 'I' },
		{ 10300, 'J' }, { 10400, 'K' }, { 10500, 'L' },
		{ 24000, 'M' }, { 24100, 'N' }, { 24200, 'O' } };

	if (f < 1000.0 && t < 1000)
	{	sprintf(Output,"%7.3lfMHz T%03ld %4.4s", (double) f, (long) t, sRange);
		if (Output[0] == ' ') Output[0] = '0';
		if (Output[1] == ' ') Output[1] = '0';
	} else
	{	int i;
		for (i=0; i<ACOUNT(Freqs); i++)
		{	if (f >= Freqs[i].Base && f < Freqs[i].Base+100)
				break;
		}
		if (i<ACOUNT(Freqs) && t < 1000)
		{	sprintf(Output,"%c%6.3lfMHz T%03ld %4.4s", Freqs[i].Prefix, (double) f-Freqs[i].Base, (long) t, sRange);
			if (Output[1] == ' ') Output[1] = '0';
		} else sprintf(Output,"%s %s %s", sPHG, Freq, Tone);
	}
	return Output;
}

static VFUNCTION AprsEchoLinkMonitor(POINTER_F Dummy)
{	STRING_F EchoLinkURL = Dummy;
	STRING_F ServerPort = HTParse(THREAD_HEAP, EchoLinkURL, THREAD_STRDUP(""), PARSE_HOST);
	STRING_F Path = HTParse(THREAD_HEAP, EchoLinkURL, THREAD_STRDUP(""), PARSE_PATH | PARSE_PUNCTUATION);
	BOOLEAN_F First = TRUE;
	POINTER_ARRAY_S *Pkts = NULL;
	TIMESTAMP_F Last = 0;
#ifdef FUTURE
	TIMESTAMP_F NextBeacon = 0;
#endif

	DgPrintf("EchoLinkMonitoring %s\n", EchoLinkURL);

	Pkts = DsCreatePointerArray("EchoLinkQueue", DSTRING_F, HERE);

#define ECHOLINK_REFRESH (24*60)	/* in minutes */
#define ECHOLINK_INJECTION 10		/* Also in minutes */

	while (!CiIsShuttingDown())
	{	MESSAGE_S *Msg;
		COUNT_F StartSeconds = RtGetElapsedSeconds(0);

		ThSetThreadStateStatic("CiGetFrom");
		Msg = CiGetFromTimeout(ServerPort, Path, 15000, NULL, NULL, HERE);
		ThSetThreadStateStatic("Processing");
		if (!Msg->Success)
		{	STRING_F Text = CiPopulateResponseStatusText(Msg);
			DgPrintf("GET(%s/%s) Failed With %s\n", ServerPort, Path, Text);
			THREAD_FREE(Text);
			First = TRUE;	/* Force a complete reset! */
		} else
		{	TIMESTAMP_F Now = RtNow(NULL);
			COUNT_F Bytes, TotalLines;
			STRING_F Body = CiGetPostBody(Msg,&Bytes);
			XML_ELEMENT_S *Records;
			COUNT_F OffLine=0, OnLine=0, Unknown=0;

			ThSetThreadStateStatic("Parsing");

			DgPrintf("Got %ld Bytes of (hopefully) XML\n", (long) Bytes);

			Records = XmlLoadBuffer("EchoLinkNodex", Bytes, Body,
						"stationlist", &TotalLines, HERE);
			if (!Records)
			{	DgPrintf("XML Parse Failed\n");
			} else
			{	STRING_F timestamp = XmlLookupAttributeValue(Records,"timestamp",FALSE);

				DgPrintf("Loaded %ld Bytes of XML, %ld Total Lines, %ld SubElements, timestamp:%s\n",
						(long) Bytes, (long) TotalLines, (long) XmlGetSubElementCount(Records), timestamp);

				ThSetThreadStateStatic("Processing");
				if (XmlGetSubElementCount(Records) == 0)
				{	DgPrintf("No EchoLink stationlist Records Found\n");
				} else if (stricmp(XmlGetElementName(Records),"stationlist"))
				{	DgPrintf("Outer Element is %s, Not stationlist\n", XmlGetElementName(Records));
				} else
				{	XML_ELEMENT_S *Record;
					CURSOR_S *Cursor = XmlSetupSubElementCursor(Records,HERE);
COUNT_F RangeCounts[10] = {0};

					while ((Record=Cursor->Next(Cursor))!=NULL)
					{	if (stricmp(XmlGetElementName(Record),"station"))
							DgPrintf("Line %ld:Found %s Expected station\n",
								XmlGetElementLineNumber(Record), XmlGetElementName(Record));
						else
						{	STRING_F call = XmlGetSubElementValue(Record,"call",FALSE);
							STRING_F location = XmlGetSubElementValue(Record,"location",FALSE);
							STRING_F node = XmlGetSubElementValue(Record,"node",FALSE);
							STRING_F lat = XmlGetSubElementValue(Record,"lat",FALSE);
							STRING_F lon = XmlGetSubElementValue(Record,"lon",FALSE);
							STRING_F freq = XmlGetSubElementValue(Record,"freq",FALSE);
							STRING_F pl = XmlGetSubElementValue(Record,"pl",FALSE);
							STRING_F power = XmlGetSubElementValue(Record,"power",FALSE);
							STRING_F haat = XmlGetSubElementValue(Record,"haat",FALSE);
							STRING_F gain = XmlGetSubElementValue(Record,"gain",FALSE);
							STRING_F directivity = XmlGetSubElementValue(Record,"directivity",FALSE);
							STRING_F status = XmlGetSubElementValue(Record,"status",FALSE);
							STRING_F status_comment = XmlGetSubElementValue(Record,"status_comment",FALSE);
							STRING_F last_update = XmlGetSubElementValue(Record,"last_update",FALSE);
#ifdef FOR_INFO_ONLY
<station>
<call>4X6ZQ-L</call>
<location>Petach-Tikva 145.275</location>
<node>120624</node>
<lat>32.103800</lat>
<lon>-34.870500</lon>
<freq>145.275</freq>
<pl>91</pl>
<power>9</power>
<haat>10</haat>
<gain>0</gain>
<directivity>0</directivity>
<status>N</status>
<status_comment>On  @0041</status_comment>
<last_update>07/22/2009 00:40</last_update>
</station>
#endif

if (status && (*status == 'N' || *status == 'C' || *status == 'B'))
{	char *e;
	double Lat = strtod(lat,&e);
	double Lon = strtod(lon,&e);
	STRING_F sLatLon = APRSLatLon(Lat, Lon, 'E', '0');	/* E0=CircleE, /r=antenna */
	STRING_F sPHG = PHG(power,haat,gain,directivity);
	STRING_F sRange = RANGE(power,haat,gain,directivity);
	STRING_F sFreq = FREQ(freq, pl, sPHG, sRange);
	STRING_F srcCall = THREAD_STRDUP(call);
	STRING_F dashCall = strchr(srcCall,'-');
	double Power = strtod(power,&e);
	double HAAT = strtod(haat,&e);
	double Gain = pow(10,strtod(gain,&e)/10);
	double Range = sqrt(2*HAAT*sqrt((Power/10)*(Gain/2)));
	STRING_F tLocation = strnicmp(location,"In Conference ",14)?location:location+14;

	STRING_F Packet;
	COUNT_F ObjCount;
	char StatID[10];
	STATION_S Station;
static	POINTER_F CountPlan=NULL;

	if (dashCall) *dashCall++ = '\0';	/* Remove -L/-R */
	else dashCall = "?";			/* No dash letter */

	if (status && strlen(status)==1)
	{	switch (*status)
		{
		case 'N': status = "+"; break;
		case 'B': status = "-"; break;
		case 'C': status = "="; break;
		case 'F': status = "x"; break;
		default:
			DgPrintf("*** Node %s Unknown Status Len(%s), Location(%s)\n", call, status, location);
			status = "";
		}
	} else
	{	DgPrintf("*** Node %s Status Len(%s) > 1, Location(%s)\n", call, status, location);
		status = "";
	}

	OnLine++;
	DgPrintf("%ld/%ld:Station:%s Loc:%s Node:%s lat/lon:%s %s Freq:%s(%s) phgd(%s):%s %s %s %s Range:%.1lfm Status:%s(%s) Updated:%s\n",
		(long) XmlGetElementLineNumber(Record), (long) TotalLines,
		call, location, node, lat, lon, freq, pl,
		PHG(power, haat, gain, directivity),
		power, haat, gain, directivity, (double) Range,
		status, status_comment, last_update);

	if (Range > 0 && Range <= 100) RangeCounts[(long)(Range/10)]++;
	else 
		DgPrintf("*** Node %s Object %.*s Range %.2lf Out of Range!\n",
			call, STRING(Station.StationID), (double ) Range);

	sprintf(StatID,"EL-%-.6s",node);
	DCOPY(Station.StationID, StatID);
	ObjCount = DbQueryCount(StatIDTable, DSTATION_S, "StationID=StationID", sizeof(Station), &Station, NULL, NULL, &CountPlan);
	if (ObjCount > 0)
		DgPrintf("*** Node %s Object %.*s Already Exists! (Count=%ld)\n",
			call, STRING(Station.StationID), (long) ObjCount);

	DgPrintf("%s>APELNK,KJ4ERJ*:;EL-%-6.6s*%2.2s%2.2s%2.2sz%s%s %s%s %.*s\n",
		srcCall, node, last_update+3, last_update+11, last_update+14,
		sLatLon, sFreq, status, call, (int) max(0,(int)(43-(strlen(sFreq)+1+strlen(status)+strlen(call)+1))), tLocation);

	Packet = HEAP_MALLOC(DsGetPointerArrayHeap(Pkts),132);
	sprintf(Packet,"ECHOLINK>APELNK,KJ4ERJ*:;EL-%-6.6s*%2.2s%2.2s%2.2sz%s%s %s%s %.*s",
		node, last_update+3, last_update+11, last_update+14,
		sLatLon, sFreq, status, call, (int) max(0,(int)(43-(strlen(sFreq)+1+strlen(status)+strlen(call)+1))), tLocation);
	DgPrintf("%s\n",Packet);
	if (strstr(location,"Palm Bay"))
		DsAddToPointerArray(Pkts, Packet);

	if (strlen(tLocation) > max(0,(int)(43-(strlen(sFreq)+1+strlen(status)+strlen(call)+1))))
		DgPrintf("*** Node %s Location (%s) Truncated to (%.*s)\n",
			call, location,
			(int) max(0,(int)(43-(strlen(sFreq)+1+strlen(status)+strlen(call)+1))), tLocation);

#ifdef OBSOLETE

On receipt, APRS uses the p, h, g and d codes to calculate the usable radio
range (in miles), for plotting a range circle representing the local radio
horizon around the station. The radio range is calculated as follows:
power = p2
Height-above-average-terrain (haat) = 10 x 2h
gain = 10(g/10)
range = –( 2 x haat x –( (power/10) x (gain/2) ) )
Thus, for PHG5132:
power = 52 = 25 watts
haat = 10 x 21 = 20 feet
gain = 10(3/10) = 1.995262
range = –( 2 x 20 x –( (25/10) x (1.995262/2) ) )
~ 7.9 miles

	DgPrintf(";%-9.9s*%.2s%.2s%.2sz%s%s %.36s\n", call, last_update+3, last_update+11, last_update+14, AprLatLon, PHG(power,haat,gain,directivity), location);
	DgPrintf(")%.9s!%s%s %.36s\n", call, AprLatLon, PHG(power,haat,gain,directivity), location);
	DgPrintf("ECHOLINK>APELNK,KJ4ERJ*:;EL-%-6.6s*%.2s%.2s%.2sz%s%s%7.3lfMHz T%03ld %4.4s %s %.36s\n",
		node, last_update+3, last_update+11, last_update+14,
		sLatLon, sPHG, (double) Freq, (long) Tone, sRange, call, location);

WINLINK>APWL2K,TCPIP*,qAC,T2SOCAL:;KD5EOC-10*080956z3308.7 NW09707.5 Wa       145.010MHz 1200 R50m RMSPacket Public
DB0UD  >APRSIS,KJ4ERJ*           :;EL-255256*211554z5125.53NE00604.37E0PHG0000438.700MHz T000 On   DB0UD-R [offline] 

N4ZIQ>APRSIS,KJ4ERJ*:;EL-458245*221323z2815.18NE08119.63W0PHG5530 444.100MHz T123 R35m N4ZIQ-R Saint Cloud, FL 444.100
N4ZWV>APRSIS,KJ4ERJ*:;EL-366193*221322z3132.32NE08724.30W0PHG8430 147.160MHz T167 R31m N4ZWV-R Monroeville, AL  147.16
N5AFY>APRSIS,KJ4ERJ*:;EL-190966*221323z3610.98NE09606.77W0PHG8130 146.490MHz T100 R11m N5AFY-L Tulsa Ok
N5API>APRSIS,KJ4ERJ*:;EL- 48439*090537z3312.69NE09709.04W0PHG4390 441.300MHz T146 R21m N5API-R [offline]
N5EYM>APRSIS,KJ4ERJ*:;EL-185292*221328z3334.88NE08944.87W0PHG6460 146.970MHz T110 R32m N5EYM-R Winona,MS.146.97/110.9
                     012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
                               1         2         3         4         5
#endif



	THREAD_FREE(sLatLon);
	THREAD_FREE(sRange);
	THREAD_FREE(sFreq);
	THREAD_FREE(sPHG);
} else if (status && *status == 'F')
	OffLine++;
else
{	Unknown++;
	DgPrintf("*** %s Unknown Status %s (%s)\n", call, status, status_comment);
}
						}
					}
					Cursor->Destroy(Cursor);
{	int r;
	for (r=0; r<10; r++)
		DgPrintf("Range[%ld] = %ld\n", (long) r, (long) RangeCounts[r]);
}

				}
				XmlFreeElement(Records);
			}
			DgPrintf("%ld Stations Online, %ld Offline, %ld Unknown\n", (long) OnLine, (long) OffLine, (long) Unknown);

#ifdef FUTURE
				if (First)
				{	First = FALSE;
					sprintf(Buffer,"%s>APZAPM::%-9s:PARM.Direct,Local,Recent,IS-RF,RF-IS,B1,B2,B3,B4,B5,B6,B7,B8",CallSign,CallSign);
					AprsQueueXmitPacket(Buffer);
					sprintf(Buffer,"%s>APZAPM::%-9s:UNIT.Stations,Stations,Stations,Packets/Minute,Packets/Minute,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A",CallSign,CallSign);
					AprsQueueXmitPacket(Buffer);
					if (!stricmp(CallSign,"KJ4ERJ-2"))
						sprintf(Buffer,"%s>APZAPM::%-9s:EQNS.0,1,0,0,1,0,0,1,0,0,0.1,0,0,0.1,0",CallSign,CallSign);
					else	sprintf(Buffer,"%s>APZAPM::%-9s:EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,1,0",CallSign,CallSign);
					AprsQueueXmitPacket(Buffer);
					DgDirectPrintf(CallSign, "%s GatedIS:%ld GatedRF:%ld MsgsRF:%ld Recent:%ld Local:%ld Direct:%ld\n",
						CallSign, (long) GatedIS, (long) GatedRF, (long) MsgsRF,
						(long) Recent, (long) Local, (long) Direct, (long) Now - Last);
				} else if (Last < Now)
				{	double minutes = (double)(Now - Last)/60.0;
					COUNT_F DeltaI, DeltaR;
					if (!stricmp(CallSign,"KJ4ERJ-2"))
					{	DeltaI = (double)(GatedIS-prevIS)*10.0/minutes + 0.5;
						DeltaR = (double)(GatedRF+MsgsRF-prevRF)*10.0/minutes + 0.5;
					} else
					{	DeltaI = (double)(GatedIS-prevIS)/minutes + 0.5;
						DeltaR = (double)(GatedRF+MsgsRF-prevRF)/minutes + 0.5;
					}
					DgDirectPrintf(CallSign, "%s GatedIS:%ld (%ld) Gated+MsgsRF:%ld+%ld (%ld) Recent:%ld Local:%ld Direct:%ld Seconds:%ld\n",
						CallSign, (long) GatedIS, (long) DeltaI, (long) GatedRF, (long) MsgsRF, (long) DeltaR,
						(long) Recent, (long) Local, (long) Direct, (long) Now - Last);

					if (DeltaI < 0) DeltaI = 0;
					if (DeltaR < 0) DeltaR = 0;
					sprintf(Buffer,"%s>APZAPM:T#%03ld,%ld,%ld,%ld,%ld,%ld,00000000",CallSign,(long) Seq++,
						(long) Direct, (long) Local, (long) Recent,
						(long) DeltaR, (long) DeltaI);
					AprsQueueXmitPacket(Buffer);
				}
				if (Now >= NextBeacon)
				{	NextBeacon = Now+60*60;
					sprintf(Buffer,"%s>APZAPM:>Gating details at http://aprs.fi/telemetry/%s",CallSign,CallSign);
					AprsQueueXmitPacket(Buffer);
				}
#endif
			Last = Now;

		}
		CiDestroyMessage(Msg);
/*
	Now actually do the injection!
*/
	ThSetThreadStateStatic("Transmitting");
	if (DsGetPointerArrayCount(Pkts))
	{	STRING_F Packet;
		COUNT_F PacketCount = DsGetPointerArrayCount(Pkts);
		COUNT_F SleepTime = ((ECHOLINK_INJECTION) * 60 * 1000L) / PacketCount;
		DgPrintf("%ld EchoLink Packets To Inject over %ld Minutes (%ld msec each)\n",
			(long) PacketCount, (long) (ECHOLINK_INJECTION),
			(long) SleepTime);
		while ((Packet = DsIndexPointerArray(Pkts,0)) != NULL)
		{	DgPrintf("Injecting(%s)\n", Packet);

/* Need to queue for transmit here */
/* AprsQueueXmitPacket(Packet); */

			if (!DsRemoveFromPointerArray(Pkts, Packet))
				DgPrintf("Failed To Remove(%s) From Pkts\n", Packet);
			HEAP_FREE(DsGetPointerArrayHeap(Pkts),Packet);

			ThSleepThread(SleepTime);
		}
	}

		ThSetThreadStateStatic("Sleeping");
		DgPrintf("Sleeping %ld Seconds\n", (long) ECHOLINK_REFRESH*60 - RtGetElapsedSeconds(StartSeconds));
		ThSleepThread((ECHOLINK_REFRESH*60 - RtGetElapsedSeconds(StartSeconds))*1000L);	/* Sleep a few HOURS */
	}
	THREAD_FREE(ServerPort); THREAD_FREE(Path);

	DgPrintf("EchoLinkMonitor ended successfully\n");
}

static POINTER_F AprsPurgeSubDir(STRING_F SubDir, POINTER_F Arg)
{	COUNT_F *pCount = Arg;
	STRING_F sKeepDays = SyGetSymbolValueOrDefault("KEEP_DAYS", "7");
	COUNT_F KeepDays = SyIsValueTrue(sKeepDays);
	TIMESTAMP_F Youngest = RtNow(NULL) - KeepDays*24*60*60;
	COUNT_F MaxSpace = 100*1024*1024;
	TIMESTAMP_F RequiredTime = Youngest;
	TIMESTAMP_F OldZip = RtNow(NULL) - 365*24*60*60;
	COUNT_F DidHere = LgPurgeOldEvents("PACKETS", ".LOG", -1, SubDir, Youngest, MaxSpace, RequiredTime, OldZip);

	if (DidHere)
	{	DgPrintf("Purged(Zipped) %ld Logs in %s (Back to %.24s)\n", (long) DidHere, SubDir, ctime(&Youngest));
		LgSprintfEvent("PACKETS", ".LOG", -1,"PURGE", 0, FALSE,
				"Purged(Zipped) %ld Logs in %s (Back to %.24s)\n", (long) DidHere, SubDir, ctime(&Youngest));
	}
	*pCount += DidHere;
	return Arg;
}

static BOOLEAN_F KeepDaysChanged = FALSE;

static void AprsMonitorKeepDays(STRING_F Symbol, POINTER_F UserArg)
{	KeepDaysChanged = TRUE;
}

static VFUNCTION AprsPurger(POINTER_F Dummy)
{	ROUTINE(AprsPurger);
	COUNT_F PassCount = 0;
	INDEX_F SleepIndex;

#define WAKE_MINUTES 10
#define POS_MINUTES 60
#define ZIP_MINUTES (60*8)

	SyAddSymbolCallback("KEEP_DAYS", AprsMonitorKeepDays, NULL);
	KeepDaysChanged = FALSE;
	
	DgPrintf("Purger Delaying %ld minutes!\n", (long) WAKE_MINUTES);
	for (SleepIndex=0; SleepIndex<WAKE_MINUTES*4; SleepIndex++)
	{	ThSleepThread(15*1000L);
		if (KeepDaysChanged)
		{	KeepDaysChanged = FALSE;
			DgPrintf("Purger Awakening Early due to KEEP_DAYS Change\n");
			break;
		}
	}
	DgPrintf("Purger Running!\n");

	while (!CiIsShuttingDown())
	{	COUNT_F Elapsed = 0;
		STRING_F sKeepDays = SyGetSymbolValueOrDefault("KEEP_DAYS", "7");
		COUNT_F KeepDays = SyIsValueTrue(sKeepDays);
		TIMESTAMP_F Youngest = RtNow(NULL) - KeepDays * 24 * 60 * 60;
	static	POINTER_F QueryPlan = NULL, DeletePlan = NULL, DeletePlan2 = NULL;

		DgPrintf("Purger Keeping %ld Days\n", (long) KeepDays);

		if ((PassCount%(POS_MINUTES/WAKE_MINUTES)) == 0)
		{	COUNT_F PosPurged = AprsPurgeOrphanPositions(Youngest);
			if (PosPurged)
			{	DgPrintf("Purged %ld Orphan Positions\n", (long) PosPurged);
				LgSprintfEvent("PACKETS", ".LOG", -1,"PURGE", 0, FALSE,
						"Purged %ld Orphan Positions\n", (long) PosPurged);
			}
		}

		if (KeepDays /*&& PassCount*/ && (PassCount%(ZIP_MINUTES/WAKE_MINUTES)) == 0)	/* 32*15 = 8 hours */
		{	COUNT_F LogsPurged = 0;
#ifdef OLD_WAY
			LgQuerySubDirs("PACKETS", ".LOG", -1, ".", AprsPurgeSubDir, &LogsPurged, HERE);
#else
			POINTER_ARRAY_S *SubDirs = LgQuerySubDirs("PACKETS", ".LOG", -1, ".", NULL, NULL, HERE);
			if (SubDirs)
			{	STRING_F Text;
				CURSOR_S *Cursor = DsSetupPointerArrayCursor(SubDirs,HERE);
				while ((Text = Cursor->Next(Cursor)) != NULL)
				{	AprsPurgeSubDir(Text, &LogsPurged);
				}
				Cursor->Destroy(Cursor);
				LgFreeQueriedSubDirs(SubDirs);
			} else DgPrintf("LgQuerySubDirs return NULL!\n");
#endif
			if (LogsPurged)
			{	DgPrintf("Purged(Zipped) %ld Total Logs\n", (long) LogsPurged);
				LgSprintfEvent("PACKETS", ".LOG", -1,"PURGE", 0, FALSE,
						"Purged(Zipped) %ld Total Logs\n", (long) LogsPurged);
			}
		}
		{	POINTER_F Results;
			PACKET_S *pPkt, Pkt;
			COUNT_F PacketsPurged = 0, HopsPurged = 0;
			Pkt.When = Youngest;
			Results = DbQuery(PacketTable, DPACKET_S, "When<When", sizeof(Pkt), &Pkt, NULL, NULL,
					DPACKET_S, NULL, sizeof(*pPkt), NULL, NULL, &QueryPlan);
			if (!Results) KILLPROC(-1,"Failed To Query Packets");
			if (DbGetResultsCount(Results))
			{	CURSOR_S *Cursor = DbSetupResultsCursor(Results, HERE);

				DgPrintf("Purging %ld Packets Prior To %.24s\n", (long) DbGetResultsCount(Results), ctime(&Pkt.When));
				LgSprintfEvent("PACKETS", ".LOG", -1,"PURGE", 0, FALSE,
						"Purging %ld Packets Prior To %.24s\n", (long) DbGetResultsCount(Results), ctime(&Pkt.When));

				ThBeginDelayedAction(Routine, "Purger", WAKE_MINUTES*60, DbGetResultsCount(Results), HERE);
				while ((pPkt = Cursor->Next(Cursor)) != NULL)
				{	PacketsPurged += DbDeleteRecord(PacketTable, DPACKET_S, sizeof(*pPkt), pPkt, &DeletePlan);
					HopsPurged += DbDeleteRecords(HopTable, DPACKET_S, "PacketIndex=PacketIndex", sizeof(*pPkt), pPkt, NULL, NULL, &DeletePlan2);
					if (KeepDaysChanged)
					{	DgPrintf("Purger Bailing due to KEEP_DAYS Change\n");
						break;
					}
					if (!(PacketsPurged%1000))
						if (!ThContinueDelayedAction(Routine, PacketsPurged, 1000, FALSE, HERE))
							break;
				}
				Elapsed = ThCompleteDelayedAction(Routine, HERE);
				Cursor->Destroy(Cursor);
			}
			DbFreeResults(Results);

			if (PacketsPurged)
			{	DgPrintf("Purged %ld Packets and %ld Hops Prior To %.24s\n", (long) PacketsPurged, (long) HopsPurged, ctime(&Pkt.When));
				LgSprintfEvent("PACKETS", ".LOG", -1,"PURGE", 0, FALSE,
						"Purged %ld Packets Prior To %.24s\n", (long) PacketsPurged, ctime(&Pkt.When));
			}
		}
		PassCount++;
		for (SleepIndex=0; SleepIndex<WAKE_MINUTES*60-Elapsed; SleepIndex++)
		{	ThSleepThread(1*1000L);
			if (KeepDaysChanged)
			{	DgPrintf("Purger Awakening Early due to KEEP_DAYS Change\n");
				break;
			}
		}
		KeepDaysChanged = FALSE;
		/* ThSleepThread((WAKE_MINUTES*60-Elapsed)*1000); */
	}
}

VFUNCTION AprsInitialize(void)
{	STRING_F ServerName = SyGetSymbolValueOrDefault("SERVER","APRS");
/*
        Set up the directories and passwords for files accessed
        by this server.
*/
        STRING_F FormDir  = SyGetSymbolValue("FORMS");
        STRING_F FileDir  = SyGetSymbolValue("FILES");

#ifdef DEBUGGER_DELAY
	DgPrintf("Delaying startup for debugger\n");
	ShowPrintable(HERE);
	ThSleepThread(15000);
#endif

	AprsLoadDatabase(SyGetSymbolValueOrDefault("DBDIR","."));

	if (!FormDir) FormDir   = FileDir;
	if (!FileDir) FileDir   = FormDir;
	if (!FileDir) FileDir   = FormDir = SyGetSymbolValueOrDefault("FORMDIR","../forms");
/*
        Initialize the user interface file access layer.
*/
	UiInitialize(FormDir, NULL, AprsAuthorize, NULL, FileDir, NULL, AprsAuthorize, NULL);
	CiAddGetURL("/osm...", AprsSendOSM, HERE);

	RexxRegisterFunctionExe("AprsCallSymbolRoutine", AprsCallSymbolRoutine);

	CiSetDefaultAuthorizationRoutine(AprsAuthorize,NULL,"APRS Analyzer v0.1");
	CiSetServerName(ServerName);
	CiSetupProxy();

#define DOIT(m,r) \
	CiAddRequestURL("/APRS_"###m, AprsSvc##r, \
			DAPRS_##m##_SRQ, sizeof(APRS_##m##_SRQ), \
			DAPRS_##m##_SRP, sizeof(APRS_##m##_SRP), HERE)

	DOIT(LOG_PACKET, LogPacket);
	DOIT(LOG_RECEIVED_PACKET, LogReceivedPacket);
	DOIT(LOG_DIGIPEATED_PACKET, LogDigipeatedPacket);
	DOIT(REPARSE_LOG_FILE, ReParseLogFile);
	DOIT(PURGE_PACKETS, PurgePackets);
	DOIT(QUERY_HOPS, QueryHops);
	DOIT(QUERY_PACKETS, QueryPackets);
	DOIT(DELETE_PACKETS, DeletePackets);
	DOIT(LOOKUP_STATION, LookupStation);
	DOIT(QUERY_STATION_INDICES, QueryStationIndices);
	DOIT(QUERY_STATIONS, QueryStations);
	DOIT(QUERY_UNIQUE_IGATES, QueryUniqueIGates);
	DOIT(QUERY_UNIQUE_DESTINATIONS, QueryUniqueDestinations);
	DOIT(QUERY_UNIQUE_PATHS, QueryUniquePaths);
	DOIT(QUERY_UNIQUE_ALT_PATHS, QueryUniqueAltPaths);
	DOIT(QUERY_USERS, QueryUsers);
	DOIT(REGISTER_USER, RegisterUser);
	DOIT(QUERY_USER_PACKETS, QueryUserPackets);
	DOIT(PURGE_USER_PACKETS, PurgeUserPackets);
	DOIT(QUERY_TRAFFIC_RATES, QueryTrafficRates);
	DOIT(CALC_OSM_TILES, CalcOSMTiles);
	DOIT(INJECT_STATION_POSITION, InjectStationPosition);
#undef DOIT

	PfSetupAllProcessDiagnostic();
	PfSetupAllThreadDiagnostic();
}

#ifdef FUTURE
static VFUNCTION AprsMemoryPig(POINTER_F Dummy)
{	ROUTINE(AprsMemoryPig);
	INDEX_F c, i;
	COUNT_F AllocationSize = 8*1024*1024-1;
static	POINTER_F Chunks[65536];

	for (c=0; AllocationSize > 8 && c<ACOUNT(Chunks); c++)
	{	Chunks[c] = THREAD_MALLOC(AllocationSize);
		if ((((TYPE64)Chunks[c])>>32) != 0)
		{	DgPrintf("%s:Chunk[%ld](%ld)=%p\n", Routine, (long) c, (long) AllocationSize, Chunks[c]);
			AllocationSize /= 2;
			AllocationSize--;
		}
	}
	for (i=30; i>0; i--)
	{	DgPrintf("%s:All Memory Allocated.  Final Size %ld in %ld Chunks.  Sleeping %ld Minutes\n", Routine, (long) AllocationSize, (long) c, (long) i);
		ThSleepThread(60000L);
	}
	DgPrintf("%s:Freeing %ld Chunks\n", (long) c);
	for (i=0; i<c; i++)
		THREAD_FREE(Chunks[c]);
	DgPrintf("%s:Freed %ld Chunks, Exiting...\n", (long) c);
}
#endif

VFUNCTION AprsStart(void)
{	STRING_F UDPPort = SyGetSymbolValue("UDPPORT");
	STRING_F ISPort = SyGetSymbolValue("ISPORT");
	STRING_F Server = SyGetSymbolValue("APRSSERVER");
	STRING_F ELMonitor = SyGetSymbolValue("ECHOLINK");
	STRING_F AGWMonitor = SyGetSymbolValue("AGWMONITOR");
	STRING_F javAPRSMonitor = SyGetSymbolValue("JAVAMONITOR");
	STRING_F javaTrafficMonitor = SyGetSymbolValue("TRAFFICMONITOR");
	if (!RtStrnWhite(-1,Server))
		ThCreateThread("AprsPoller", AprsPoller, Server, HERE);
	if (!RtStrnWhite(-1,UDPPort))
	{	ThCreateThread("UDPListener", UDPListener, UDPPort, HERE);
	}
	if (!RtStrnWhite(-1,ISPort))
		ThCreateThread("APRS-IS Listener", AprsServerListen, ISPort, HERE);
	if (!RtStrnWhite(-1,ELMonitor))
		ThCreateThread("EchoLinkMon", AprsEchoLinkMonitor, ELMonitor, HERE);
	if (!RtStrnWhite(-1,AGWMonitor))
		ThCreateThread("AGWMonitor", AprsAGWMonitor, AGWMonitor, HERE);
	if (!RtStrnWhite(-1,javAPRSMonitor))
	{	COUNT_F Count;
		STRING_F *Monitors = RtSplitCommas(javAPRSMonitor,&Count);
		while (Count--)
		{	ThCreateThread(Monitors[Count], AprsJavaMonitor, STRING_STRDUP(Monitors[Count]), HERE);
		}
		RtFreeCommas(Monitors);
	}
	if (!RtStrnWhite(-1,javaTrafficMonitor))
	{	COUNT_F Count;
		STRING_F *Monitors = RtSplitCommas(javaTrafficMonitor,&Count);
		while (Count--)
		{	ThCreateThread(Monitors[Count], AprsJavaTrafficMonitor, STRING_STRDUP(Monitors[Count]), HERE);
		}
		RtFreeCommas(Monitors);
	}
	ThCreateThread("Purger", AprsPurger, NULL, HERE);
#ifdef FUTURE
	ThCreateThread("MemoryPig", AprsMemoryPig, NULL, HERE);
#endif
}

VFUNCTION AprsTerminate(void)
{
	AprsCloseDatabase();
}

int main(int argc, char *argv[])
{
	if (argc == 1)
	{	fprintf(stderr,"Usage: %s <APRSSERVER=Ip:FilterPort> <DBDIR=Dir> <EVENTS=Dir> <FORMS=../files> <FILES=forms> <MAIN_PAGE=/main.htm> <TCPPORT=Port>\n", argv[0]);
		fprintf(stderr,"\t<AGWMONITOR=CallSign(IP:Port)> <JAVAMONITOR=CallSign(IP:14508)> <TRAFFICMONITOR=CallSign(IP:14501)\n");
		return -1;
	}

	CiSyncStartup(argc, argv, "APRS", AprsInitialize, AprsStart, NULL, AprsTerminate);

	return 0;
}


