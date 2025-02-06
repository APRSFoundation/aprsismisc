/* Prototypes generated Thu Jul 31 14:17:02 2008 */
#ifdef __cplusplus
extern "c"
{
#endif
void tcp_disable_messages(void);
void tcp_enable_messages(void);
void tcp_set_port(int new_port);
void tcp_set_host(char *new_host);
void tcp_set_host_port(char *new_host, int new_port);
int tcp_connect(void);
void tcp_disconnect(void);
int tcp_transmit_buffer( unsigned int length, unsigned char *buffer, unsigned int timeout );
int tcp_receive_buffer( unsigned short length, unsigned char *buffer, unsigned short timeout, unsigned short *retlength);
#ifdef __cplusplus
}
#endif
