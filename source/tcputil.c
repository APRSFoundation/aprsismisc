#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h>
#include <winsock.h>
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
#define psock_errno(s) printf("%s errno %ld\n", s, (long) h_errno)
#define sock_errno() h_errno

#ifdef UNDER_CE
#define strdup(s) strcpy(malloc(strlen(s)+1),s)
#endif

#include "tcputil.h"

#define DISCONNECTED -1

#define SLOW_CONNECT	 // Define to throttle connects to every 15 seconds */

static int mysocket = DISCONNECTED;
static int changed = 1;
static char *host = NULL;
static int port = 0;
static struct sockaddr_in server = {0};
static int MsgsDisabled = 1;

static unsigned char *SentBuffer = NULL;
static unsigned int SentLength = 0;
static double SentTime = 0;

double RtGetMsec(void);

/*:tcp_status_msg

	This routine will place a message on the top line of the display
	after erasing the first 20 characters.  Note that messages displayed
	with this routine are truncated to 20 characters.
*/
static void tcp_status_msg(TCHAR *Msg)
{
printf("%s\n", Msg); fflush(stdout);
	if (MsgsDisabled) return;
	MessageBox(NULL, Msg, TEXT("tcp_status"),
				MB_OK | MB_ICONINFORMATION);
}

/*:tcp_disable_messages

	This routine will disable TCP/IP status messages.  By default, these
	messages are enabled.
*/
void tcp_disable_messages(void)
{
	MsgsDisabled = 1;
}

/*:tcp_enable_messages

	This routine will re-enabled TCP/IP status messages.
*/
void tcp_enable_messages(void)
{
	MsgsDisabled = 0;
}

/*:tcp_set_port

	This routine sets the port to which TCP/IP connections should be
	established.  This routine must be invoked prior to attempting any
	transmits or receives.
*/
void tcp_set_port(int new_port)
{	port = new_port;
	changed = 1;
}

/*:tcp_set_host

	This routine sets the host to which TCP/IP connections should be
	established.  This routine must be invoked prior to attempting any
	transmits or receives.  The host can be specified as a dot-delimited
	Internet address or hostname.
*/
void tcp_set_host(char *new_host)
{	if (host) free(host);
	host = strdup(new_host);
	changed = 1;
}

/*:tcp_set_host_port

	This routine acts as a single insulation routine for tcp_set_port
	and tcp_set_host.
*/
void tcp_set_host_port(char *new_host, int new_port)
{
	tcp_set_host(new_host);
	tcp_set_port(new_port);
}

/*:tcp_connect

	This routine establishes a connection to the configured TCP/IP host
	and port if a connection does not already exist.  It is automatically
	invoked internally if a connection is required for transmitting or
	receiving.  TRUE is returned if a connection exists.
*/
int tcp_connect(void)
{	DWORD Status;
static HANDLE hmtx;
static int initialized = 0;

#ifdef SLOW_CONNECT
static	double NextConnect = 0.0;
#endif

	if (mysocket != DISCONNECTED) return 1;
	if (!initialized)
	{	sock_init();
		hmtx = CreateMutex(NULL, TRUE, NULL);
		initialized = 1;
	} else
	do
	{	Status = WaitForSingleObject(hmtx, 10000);
		if (Status == WAIT_ABANDONED)
			tcp_status_msg(TEXT("WAIT Abandoned"));
		else if (Status == WAIT_TIMEOUT)
			tcp_status_msg(TEXT("WAIT Timeout"));
		else if (Status == WAIT_FAILED)
		{	DWORD e = GetLastError();
			TCHAR Text[80];
			wsprintf(Text,TEXT("Wait FAILED %ld (0x%lX)"), (long) e, (long) e);
			tcp_status_msg(Text);
		} else if (Status == WAIT_OBJECT_0)
#ifdef VERBOSE
			tcp_status_msg(TEXT("WAIT Object 0"));
#else
			;
#endif
		else
		{	TCHAR Text[80];
			wsprintf(Text,TEXT("Wait Returned %ld"), (long) Status);
			tcp_status_msg(Text);
		}
	} while (Status == WAIT_TIMEOUT);

#ifdef SLOW_CONNECT
	if (RtGetMsec() < NextConnect)
	{	TCHAR Text[80];
		wsprintf(Text,TEXT("Delay Connect For %ld mSec"), (long) (NextConnect-RtGetMsec()));
		tcp_status_msg(Text);
		ReleaseMutex(hmtx);
		return 0;
	}
	NextConnect = RtGetMsec();
	if (NextConnect) NextConnect += 15000; /* Only attempt every 15 seconds! */
#endif

	if (changed)
	{	if (!port || !host)
		{
			ReleaseMutex(hmtx);
			return 0;
		}
tcp_status_msg(TEXT("Resolving..."));
		server.sin_family      = AF_INET;
		server.sin_port        = htons((u_short)port);
		server.sin_addr.s_addr = inet_addr(host);	/* argv[1] = hostname */
		if (server.sin_addr.s_addr == (u_long) -1)
		{	struct hostent *hostnm = gethostbyname(host);
			if (!hostnm)
			{
				ReleaseMutex(hmtx);
				return 0;
			}
			server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
		}
	}
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{	mysocket = DISCONNECTED;
		ReleaseMutex(hmtx);
		return 0;
	}

tcp_status_msg(TEXT("Connecting..."));
	if (connect(mysocket, (struct sockaddr *)&server, sizeof(server)) < 0)
	{	soclose(mysocket);
		mysocket = DISCONNECTED;
tcp_status_msg(TEXT("Connect Failed"));
		ReleaseMutex(hmtx);
		return 0;
	}
tcp_status_msg(TEXT("Connected"));
#ifdef SET_SNDBUF_TO_ZERO
{	int SndBuf;
	int SndLen;
	SndBuf = 0;
	if (setsockopt(mysocket, SOL_SOCKET, SO_SNDBUF, (char *) &SndBuf, sizeof(SndBuf)))
	{	DWORD e = WSAGetLastError();
		TCHAR Text[80];
		wsprintf(Text,TEXT("setsockopt Error %ld"), (long) e);
		tcp_status_msg(Text);
	}
	SndLen = sizeof(SndBuf);
	if (getsockopt(mysocket, SOL_SOCKET, SO_SNDBUF, (char *) &SndBuf, &SndLen))
	{	DWORD e = WSAGetLastError();
		TCHAR Text[80];
		wsprintf(Text,TEXT("getsockopt Error %ld"), (long) e);
		tcp_status_msg(Text);
	} else
	{	TCHAR Text[80];
		wsprintf(Text,TEXT("SndBuf = %ld"), (long) SndBuf);
		tcp_status_msg(Text);
	}
}
#endif

	if (SentBuffer)	/* We may want a time limit on this? */
	{	int n;
		TCHAR Text[4096];
		wsprintf(Text,TEXT("ReSending After %ld msec"), (long) (SentTime?(RtGetMsec()-SentTime):0x7fffffff));
		tcp_status_msg(Text);
#ifdef VERBOSE
		wsprintf(Text,TEXT("ReSending %.*hs"), (int) SentLength, SentBuffer);
		tcp_status_msg(Text);
#endif
		if ((n=send(mysocket, SentBuffer, SentLength, 0)) != (int)SentLength)
		{	tcp_disconnect();
tcp_status_msg(TEXT("re-send Failed"));
			ReleaseMutex(hmtx);
			return 0;
		}
	}

	ReleaseMutex(hmtx);
	return 1;
}

/*:tcp_disconnect

	This routine disconnects any active connection.  It is automatically
	invoked in the event of any errors.  It can be invoked directly to
	force the connection to be reset at any time.
*/
void tcp_disconnect(void)
{
	if (mysocket == DISCONNECTED) return;
tcp_status_msg(TEXT("Lost Connection..."));
	soclose(mysocket);
	mysocket = DISCONNECTED;
}

/*:tcp_transmit_buffer

	This routine transmits the specified buffer contents to the configured
	TCP/IP host and port.  A connection is automatically established if
	one does not exist.

	Successful completion of this routine does not necessarily mean that
	the data has been delivered to the host, only that the local TCP/IP
	stack has accepted the data for transmission.

	This routine automatically appends a newline ('\n') to the transmitted
	buffer to allow packet parsing on the receiving end of the connection.
	The buffer is also prefixed with the configured radio id in Intermec
	format (mA -> p5).

	0 is returned upon successful completion.  Other possible
	error returns are listed below.

<pre>
	 1 - Timeout
	 2 - Connection could not be established or was lost
	 3 - Buffer length is zero
	 4 - Select error
</pre>
*/ 
int tcp_transmit_buffer
(	unsigned int length,
	unsigned char *buffer,
	unsigned int timeout	/* In MilliSeconds */
)
{
static	unsigned short sendmax = 0;
static	char *sendbuf = NULL;
	int n, r;
	fd_set fds;
	struct timeval tv;

	if (!tcp_connect()) return 2;	/* protocol not active */
	if (!length) return 3;		/* zero length buffer */

	if (length+1 > sendmax)
	{	if (sendbuf) free(sendbuf);
		sendmax = length+1;
		sendbuf = malloc(sendmax);
	}
	memcpy(sendbuf,buffer,length);
	sendbuf[length] = '\n';
	length += 1;

	if (SentBuffer) free(SentBuffer);
	SentLength = length;
	SentBuffer = malloc(SentLength);
	memcpy(SentBuffer,sendbuf,SentLength);
	SentTime = RtGetMsec();

	FD_ZERO(&fds);
	FD_SET(mysocket, &fds);
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout%1000)*1000;

	r = select(mysocket+1, NULL, &fds, NULL, &tv);
	if (r == -1) return 4;
	if (r == 0) return 1;
	if (!FD_ISSET(mysocket, &fds)) return 1;

	if ((n=send(mysocket, sendbuf, length, 0)) != (int)length)
	{	tcp_disconnect();
tcp_status_msg(TEXT("send Failed"));
		return 2;
	}
	return 0;	/* communications success */
}

/*:tcp_receive_buffer

	This routine receives a packet of data from the configured TCP/IP
	connection.  A packet is defined as being delimited with newlines
	('\n') which must be appended by the host.

	0 is returned upon successful completion.  Other possible
	error returns are listed below.

<pre>
	 1 - Timeout
	 2 - Connection could not be established or was lost
	 3 - Buffer length is zero
	 4 - Select error
	 5 - Buffer length is < 256 bytes
</pre>
*/
int tcp_receive_buffer
(	unsigned short length,	/* Must be at least 256 */
	unsigned char *buffer,
	unsigned short timeout,	/* In milliseconds */
	unsigned short *retlength
)
{
static	unsigned short pendcount = 0;
static	unsigned short recvlen = 0;
static	char *recvbuf = NULL;
	int n, r;
	fd_set fds;
	struct timeval tv;
#if defined(CK30) || !defined(UNDER_CE)
	unsigned long Available;
#endif

	if (!tcp_connect()) return 2;	/* protocol not active */
	if (!length) return 3;		/* zero length buffer */
	if (length < 256) return 5;	/* Need at least 256 */

/*
	Select seems to flush barcode reads on the CK30 so let's
	just poll the socket for availability and ignore the timeout.
*/
	if (!timeout) timeout = 100;

	for (; timeout >= 100; timeout -= 100)
	if (ioctlsocket(mysocket,FIONREAD,&Available)==0 && Available <= 0)
	{	Sleep(100);
	}

	if (recvlen < length)
	{	recvlen = length + 1;
		recvbuf = realloc(recvbuf,recvlen);
	}

	FD_ZERO(&fds);
	FD_SET(mysocket, &fds);
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout%1000)*1000;

/*printf("Read %ld bytes on socket %ld Timeout %ld\n", (long) length, (long) mysocket, (long) timeout);*/

	if (!timeout)
		tv.tv_usec = 100000;
	do
	{	unsigned short i;
		for (i=0; i<pendcount; i++)
		{	if (recvbuf[i] == '\n' || recvbuf[i] == '\r')
			{	unsigned short use, eat;
				if (i > length)
				{	use = eat = length;
				} else
				{	use = i; eat = i+1;
					if (recvbuf[i-1] == '\r')
						use--;
				}
				memcpy(buffer,recvbuf,use);
				if (retlength) *retlength = use;
				pendcount -= eat;
				if (pendcount)
					memcpy(recvbuf, recvbuf+eat, pendcount);
				return 0;	/* communications success */
			}
		}
		if (pendcount >= recvlen)
			length = pendcount;
		if (pendcount >= length)
		{	memcpy(buffer,recvbuf,length);
			if (retlength) *retlength = length;
			pendcount -= length;
			if (pendcount)
				memcpy(recvbuf, recvbuf+length, pendcount);
			return 0;
		}

		r = select(mysocket+1, &fds, NULL, NULL, &tv);
		if (r == -1) return 4;
		if (r == 0) return 1;
		if (!FD_ISSET(mysocket, &fds)) return 1;

		if ((n=recv(mysocket, recvbuf+pendcount,
				recvlen-pendcount, 0)) <= 0)
		{	DWORD e = WSAGetLastError();
			TCHAR Text[80];
			wsprintf(Text,TEXT("recv Error %ld Sent(%ld)"), (long) e, (long) (SentTime?(RtGetMsec()-SentTime):0));
			tcp_status_msg(Text);
			tcp_disconnect();
			return 2;
		} else pendcount += (unsigned short) n;
		if (SentBuffer) free(SentBuffer);
		SentBuffer = NULL; SentLength = 0; SentTime = 0;
	} while (mysocket);
	return 2;	/* Good returns are all above! */
}


