/* Prototypes generated Fri Nov  7 23:37:51 2008 */
#ifdef __cplusplus
extern "c"
{
#endif
BOOLEAN_F AprsAuthorize(MESSAGE_S *Req, STRING_F URL, STRING_F Username, STRING_F Password, POINTER_F UserArg);
void myMessageCallback(void *userarg, char *from, char *message);
void myBulletinCallback(void *userarg, char *from, char identifier, char *bulletin);
VFUNCTION AprsQueueXmitPacket(STRING_F Packet);
VFUNCTION AprsInitialize(void);
VFUNCTION AprsStart(void);
VFUNCTION AprsTerminate(void);
int main(int argc, char *argv[]);
#ifdef __cplusplus
}
#endif
