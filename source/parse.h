/* Prototypes generated Thu Jul 31 14:16:33 2008 */
#ifdef __cplusplus
extern "C"
{
#endif
unsigned long CRC32(unsigned char *data, int len);
int IsSameBaseCallsign(char *One, char *Two);
int FromRadix(char *Hex, int Len, unsigned long *pResult, int Radix, char *Chars);
int FromHex(char *Hex, int Len, unsigned long *pResult);
int FromDec(char *Hex, int Len, unsigned long *pResult);
int dwFromDec(char *Hex, int Len, unsigned long *pResult);
int wFromDec(char *Hex, int Len, unsigned short *pResult);
int cFromDec(char *Hex, int Len, unsigned char *pResult);
int newbase91decode(char *s, int len, signed long *l);
long SymbolInt(char Table, char Character);
int APRSSymbolIndex(int Symbol);
long APRSSymbolIndexToInt(int Index);
int GetSymbolByName(char *Name);
char *GetSymbolName(int Symbol);
char *GetDisplayableSymbol(int Symbol);
int IsValidAltNet(char *AltNet);
void set_message_handler(char *CallSign, void (*pMessageCallback)(void *userarg, char *from, char *message), void *userarg);
void set_bulletin_handler(void (*pBulletinCallback)(void *userarg, char *from, char identifier, char *group, char *bulletin), void *userarg);
void parse_route(int pcount, char **pieces, char **entry, char **relay, char **q);
char * parse_aprs(char *InBuf, char **src, char **dst, int *hopCount, char ***Hops, double *rlat, double *rlon, double *alt, int *symbol, char *datatype);
int parse_full_aprs(char *InBuf, APRS_PARSED_INFO_S *Info);
int IsPlatformGeneric(APRS_PLATFORM_V tPlatform);
char *GetPlatformString(APRS_PLATFORM_V tPlatform, char **pGroup);
#ifdef __cplusplus
}
#endif
