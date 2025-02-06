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

#define BUFLEN 512
#define NPACK 10
#define PORT 6369

int main(void)
{
  struct sockaddr_in si_me, si_other;
  int n, s, slen=sizeof(si_other);
  char buf[BUFLEN], *last = NULL;
  int dup=0;

  sock_init();

  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    psock_errno("socket");
  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(PORT);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (struct sockaddr *) &si_me, sizeof(si_me))==-1)
      psock_errno("bind");
  for (;;) {
    if ((n=recvfrom(s, buf, BUFLEN-1, 0, (struct sockaddr *)&si_other, &slen))==-1)
    {   psock_errno("recvfrom()");
	break;
    }
    buf[n] = '\0';
    if (!last || strcmp(buf, last))
    {	if (last) free(last);
	last = strdup(buf);
	if (dup) printf("Ignored %ld Duplicates\n", (long) dup); dup = 0;
	printf("Received packet from %s:%d\nData: %s\n\n", 
          	 inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
    } else dup++;
  }
  soclose(s);
  return 0;
}


