#include <stdio.h>
#include <sys/types.h>

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


int tcp_send_udp(char *To, int Port, int Len, char *Buffer, int Attempts)
{	struct sockaddr_in si_other;
	int s, i, slen=sizeof(si_other);

	sock_init();

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{	printf("socket(UDP) Failed!\n");
		psock_errno("socket");
		return 0;
	} else printf("Created UDP Socket %ld\n", (long) s);

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(Port);
	si_other.sin_addr.s_addr = inet_addr(To);
	if (si_other.sin_addr.s_addr == (u_long) -1)
	{	struct hostent *hostnm = gethostbyname(To);
		if (!hostnm)
		{	printf("No Translation for %s\n", To);
			return 0;
		}
		si_other.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
	}
	for (i=0; i<Attempts; i++)
	{	printf("Sending %ld/%ld\n", (long) i+1, (long) Attempts);
		if (sendto(s, Buffer, Len, 0, (struct sockaddr *) &si_other, slen)==-1)
		{	printf("sendto(%s:%ld) '%.*s' Failed\n", To, (long) Port, (int) Len, Buffer);
			return 0;
		}
		Sleep(100);
	}
	soclose(s);
	printf("Sent(%.*s) To %s:%ld\n", (int) Len, Buffer, To, (long) Port);
	return 1;
}

int main(void)
{	return tcp_send_udp("ldeffenb.dnsalias.net", 6369, 7, "KJ4ERJ", 10);
}
