/*
        APRS-server.c

<pre>
        Date	Who		What
	111212	L.Deffenbaugh	Adopt from RFB's Trakker.c
</pre>
*/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <df/base.h>

#include <uf/include/base.h>

#include <uf/source/dgprint.h>
#include <uf/source/dsrtns.h>
#include <uf/source/hprtns.h>
#include <uf/source/syrtns.h>
#include <uf/source/smrtns.h>
#include <uf/source/thrtns.h>

#include <uf/source/rtrtns.h>

#include <ci/include/cidef.h>
#include <ci/source/ciinit.h>

#include <aprs/source/server.h>

/*
	The following flack really belongs somewhere in CI with the
	appropriate routines exposed to me here.  This is due to
	initial laziness!
*/
#if defined(__WIN95__) || defined(__NT__)
#define FD_SETSIZE 32
#include <winsock.h>
#define soclose(s) closesocket(s)
#define ioctl(s) ioctlsocket(s)
#define psock_errno(s) printf("%s errno %ld\n", s, (long) h_errno)
#define sock_errno() h_errno
#else

#ifdef __OS2__
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include <sys/ioctl.h>
#ifdef __OS2__
#define BSD_SELECT
#endif
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>

#ifndef __OS2__
#define soclose(s) close(s)
#define psock_errno(s) perror(s)
#endif

#endif
/*
	This is the end of the stuff that really shouldn't be here.
*/

#ifndef FD_SETSIZE
#define FD_SETSIZE 32	/* OS/2 doesn't seem to define this */
#endif
#define BUFFER_SIZE 16384
#define MAX_CLIENT FD_SETSIZE
struct
{	int s;		/* Actual socket for client */
	char *p;	/* IP Address of Partner */
	char *buf;	/* Input Buffer */
	int n;		/* Bytes Received */
} cs[MAX_CLIENT];

static void Disconnect(int c, char *Reason)
{	char *p = strdup(cs[c].p);
	int s = cs[c].s;

	cs[c].s = 0;

	if (s) soclose(s);
	free(p);
}

VFUNCTION AprsForwardPacket(STRING_F Packet)
{	COUNT_F XmitCount = strlen(Packet)+2+1;
	char *XmitBuf = THREAD_MALLOC(XmitCount);
	INDEX_F c;
	COUNT_F n;
	
	XmitCount = sprintf(XmitBuf, "%s\r\n", Packet);

	for (c=0; c<MAX_CLIENT; c++)
	{	if (cs[c].s)	/* Connected? */
		{	ThSetThreadState("Send");
			if ((n=send(cs[c].s, XmitBuf, XmitCount, 0)) < 0)
			{	char Reason[128];
				sprintf(Reason,"send %ld bytes to socket %ld failed with %ld",
					(long) XmitCount-3, (long) cs[c].s, (long) sock_errno());
				psock_errno("Send(cs[c].s)");
	DgPrintf("Send(%ld) Failed, Disconnecting...\n", (long) cs[c].s);
				Disconnect(c, Reason);
			}
		}
	}
	THREAD_FREE(XmitBuf);
}

static void ProcessRead(int c)
{	int n;

	ThSetThreadState("Recv");
	if ((n=recv(cs[c].s, cs[c].buf+cs[c].n, BUFFER_SIZE-cs[c].n-1, 0)) == -1)	/* Extra -1 is room for NULL */
	{	char Reason[128];
		sprintf(Reason,"recv %ld bytes from socket %ld failed with %ld",
				(long) BUFFER_SIZE-cs[c].n-1, (long) cs[c].s, (long) sock_errno());
		psock_errno("Recv(cs[c].s)");
DgPrintf("Recv(%ld) Failed, Disconnecting...\n", (long) cs[c].s);
		Disconnect(c, Reason);
		return;
	}
	if (!n)
	{	char Reason[128];
		sprintf(Reason,"recv zero bytes from socket %ld",
				(long) cs[c].s);
DgPrintf("Recv(%ld) Got Zero Bytes, Disconnecting...\n", (long) cs[c].s);
		Disconnect(c, Reason);
		return;
	}

	ThSetThreadState("Process");
	if (cs[c].buf[cs[c].n] != '#')	// Ignore comments
	{
		while (n>0
		&& (cs[c].buf[cs[c].n+n-1] == '\r'		// Trim \r\n
			|| cs[c].buf[cs[c].n+n-1] == '\n'))
			n--;
		if (n > 0)
			DgBigPrintf(n+80, "Ignoring %ld Bytes of \"%.*s\"\n", (long) n, (int) n, cs[c].buf+cs[c].n);
	}
	cs[c].n += n;
	cs[c].n = 0;
}

/*:RfbDedicatedReader

*/
VFUNCTION RfbDedicatedReader(POINTER_F Dummy)
{	int c = (int) Dummy;

	while (cs[c].s)
	{
#ifdef DEBUG
DgPrintf("Reading socket %ld\n", (long) cs[c].s);
#endif
		ProcessRead(c);
	}
}

/*:RfbTrakkerListen

	This routine executes on a free running thread and is responsible
	for listening for incoming connections from Trakker Radios and
	dispatching data received from them.
*/
VFUNCTION AprsServerListen(POINTER_F PortString)
{	ROUTINE(AprsServerListen);
	struct sockaddr_in client; /* client address information            */
	struct sockaddr_in server; /* server address information            */
	int namelen;               /* length of client name                 */
	int ss;                    /* socket for accepting connections      */

	int c;

static	fd_set rdfds;                  /* read set mask for select() call */
	int width=0;                   /* # bits to be checked for select() call */
	int readysock=0;
static	struct timeval timeout;

	char *e;
	long Port = strtol(PortString, &e, 0);

	ThSetThreadState("Init");
	if (*e)
	{	char Buffer[80];
		sprintf(Buffer,"Port '%s' NonNumeric\n", PortString);
		KILLPROC(-1,Buffer);
	}

	memset(&cs, 0, sizeof(cs));
/*
	First get the server socket set up
*/
	if ((ss = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		psock_errno("Socket(ss)");
		KILLPROC(-1,"Failed To Create Server Socket");
	}

	server.sin_family = AF_INET;
	server.sin_port   = htons((short) Port);
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(ss, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		psock_errno("Bind(ss)");
		KILLPROC(-1,"Failed To Bind Server Socket");
	}

	if (listen(ss, 16) != 0)
	{
		psock_errno("Listen(ss)");
		KILLPROC(-1,"Failed To Listen On Server Socket");
	}

/*
	Now enter an infinite loop checking for what to do via select
*/
	do
	{static time_t NextTime = 0;
		if (time(NULL)>=NextTime)
		{	/* DgPrintf("Listening for connections on Port %d\n", Port); */
			NextTime = time(NULL)+60;
		}
#ifdef SELECTING_FOR_READ
		timeout.tv_sec = 0L;	
		timeout.tv_usec = 10000L;/* 10 msec, this should be shorter than the sleep in RfbTerminate */
#else
		timeout.tv_sec = 1L;	/* 1 second, this should be shorter than the sleep in RfbTerminate */	
		timeout.tv_usec = 0L;
#endif

/*
	Set up the descriptors to watch, the server socket and all connected
	clients
*/
		FD_ZERO( &rdfds );

		FD_SET( ss, &rdfds );
		if ( ss > width ) width = ss;

#ifdef SELECTING_FOR_READ
		for (c=0; c<MAX_CLIENT; c++)
		{	if (cs[c].s)
			{	FD_SET( cs[c].s, &rdfds );
				if ( cs[c].s > width ) width = cs[c].s;
			}
		}
#endif

		width++;	/* Width is relative to 1, sockets are 0 */

		ThSetThreadState("Selecting");
		if ((readysock=select(width, &rdfds, (fd_set *)0, (fd_set *)0, &timeout)) == -1)
		{
			psock_errno("Select(readysock)");
			KILLPROC(-1,"Select(readysock) Failed");
		}

		if (readysock > 0)	/* This says there's not a timeout */
		{
/*
	Check the server socket and accept any new connections
	and make an outbound on their behalf
*/
#ifdef DEBUG
DgPrintf("%ld Sockets Ready\n", (long) readysock);
#endif
			if(FD_ISSET(ss,&rdfds))
			{	for (c=0; c<MAX_CLIENT; c++)
				{	if (!cs[c].s) break;
				}
				if (c>=MAX_CLIENT)
				{	int s;
					DgPrintf("MAX_CLIENT Exceeded\n");
					namelen = sizeof(client);
					ThSetThreadState("AcceptOverflow");
					if ((s=accept(ss, (struct sockaddr *)&client, &namelen)) == -1)
					{	psock_errno("Accept(ss)");
					} else
					{	DgPrintf("TCP Server ignoring %s Port %ld\n",
							inet_ntoa(((struct sockaddr_in*)&client)->sin_addr),
							(long) ntohs(((struct sockaddr_in*)&client)->sin_port));
						soclose(s);
					}
				} else
				{
					int s;
					namelen = sizeof(client);
					ThSetThreadState("Accept");
#define NEW_STUFF
#ifdef NEW_STUFF
					if ((s = accept(ss, (struct sockaddr *)&client, &namelen)) == -1)
					{	psock_errno("Accept(ss)");
						KILLPROC(-1,"Accept(ss) Failed");
					}
					{	static char *greeting = "# aprs64 0.0.0-20240806\r\n";
						ThSetThreadState("NewGreeting");
						if (send(s, greeting, strlen(greeting), 0) < 0)
						{	char Reason[128];
							sprintf(Reason,"send greeting %ld bytes to socket %ld failed with %ld",
								(long) strlen(greeting), (long) s, (long) sock_errno());
							psock_errno("Send(s)");
							DgPrintf("Send greeting(%ld) Failed, Disconnecting...\n", (long) s);
						} else DgPrintf("Greeting sent");
					}
					{	static char *accept = "# logresp KJ4ERJ-RF verified, server APRS64\r\n";
						ThSetThreadState("NewLogResp");
						if (send(s, accept, strlen(accept), 0) < 0)
						{	char Reason[128];
							sprintf(Reason,"send accept %ld bytes to socket %ld failed with %ld",
								(long) strlen(accept), (long) s, (long) sock_errno());
							psock_errno("Send(s)");
							DgPrintf("Send accept(%ld) Failed, Disconnecting...\n", (long) s);
						} else DgPrintf("LogResp sent");
					}
					cs[c].s = s;
#else
					if ((cs[c].s = accept(ss, (struct sockaddr *)&client, &namelen)) == -1)
					{	psock_errno("Accept(ss)");
						KILLPROC(-1,"Accept(ss) Failed");
					}
#endif
					ThSetThreadState("NewRadio");
					if (!cs[c].buf) cs[c].buf = malloc(BUFFER_SIZE);
					cs[c].n = 0;
					if (cs[c].p) STRING_FREE(cs[c].p);
					cs[c].p = STRING_STRDUP(inet_ntoa(((struct sockaddr_in*)&client)->sin_addr));
/*	11-05-2001 LWD
	There should be an ELSE here that blows away any other client
	index that points to this same RadioInfo block.  This will clean
	up "dead" client connections that RFB's TCP stack hasn't yet
	noticed and should allow re-started radios to recover sooner.

	03-07-2002 LWD - Added this else processing to disconnect the
	old connection
*/
					{
						DgPrintf("TCP Server accepted connection %ld from %s Port %ld\n",
							(long) c, cs[c].p,
							(long) ntohs(((struct sockaddr_in*)&client)->sin_port));
#ifndef SELECTING_FOR_READ
						{	STRING_F        Name;
							Name = DsMakeSubName("TCPRead", cs[c].p);
							ThCreateThread(Name, RfbDedicatedReader, (POINTER_F) c, HERE);
							THREAD_FREE(Name);
						}
#endif
					}
				}
			}
/*
	Now check each of the client sockets
*/
#ifdef SELECTING_FOR_READ
			for (c=0; c<MAX_CLIENT; c++)
			if (cs[c].s)
			if(FD_ISSET(cs[c].s,&rdfds))
			{
#ifdef DEBUG
DgPrintf("Reading socket %ld\n", (long) cs[c].s);
#endif
				ProcessRead(c);
			}
#endif
		}
	} while (!CiIsShuttingDown());
}

#ifdef OBSOLETE
/*:RfbTrakkerProcessRead

        This function executes as a free running thread to
        check the receive queue for any pending messages. The
        thread will wait for a message to be queued if the
        receive queue is empty.  These messages should only be
        for fake radios since Trakker messages go directly to
        the proper radio queue.
*/
VFUNCTION RfbTrakkerProcessRead(POINTER_F Dummy)
{       char                            *Buffer;
        COUNT_F                         i, Count = 0;
	COUNT_F				RadioID;
        RFB_RADIO_INFO_S                *RadioInfo = NULL;

        for (;;)
        {       ThSetThreadState("GetMsg");
		Buffer = RfbGetMessageFromRecvQ(&Count);
                if (!Count || !Buffer)
		{	ThSetThreadState("Waiting");
			SmWaitForEvent(&RecvMessageQueued, HERE);
			continue;
		}
		ThSetThreadState("Process");
		if (!RfbParseRadioID(Buffer, Count, &RadioID))
		{	DgPrintf("Failed To Parse RadioID From '%.*s'\n",
				(int) Count, Buffer);
			THREAD_FREE(Buffer);
			continue;
		}

	        RadioInfo = RfbGetRadioInfo(RadioID);
	        if (RadioInfo == NULL)
	        {       RadioInfo = HEAP_CALLOC(DsGetHashHeap(RadioConfig), 1, sizeof(*RadioInfo));
	                RadioInfo->RadioID = RadioID;
	                RfbGetRadioDefaults(RadioInfo);
	        }
		for (i=0; i<Count; i++)
			if (!isdigit(Buffer[i]))
				break;
		ThSetThreadState("Queue");
		RfbQueueToRadioQ(RadioInfo, Count-i, Buffer+i);
		THREAD_FREE(Buffer);
        }
}
#endif
