/* Prototypes generated Fri May 18 03:56:48 2018 */
#ifdef __cplusplus
extern "c"
{
#endif
BOOLEAN_F AprsLookupStationLocation( STATION_ID_F OwnerID, STATION_ID_F StationID, INTEGER_ID_F *pStationIndex, COORDINATE_S *pLoc, INTEGER_ID_F *pIndex);
BOOLEAN_F FUNCTION AprsSvcQueryUserPackets(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcPurgeUserPackets(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcQueryUsers(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcRegisterUser(MESSAGE_S *Req, STRING_F URL);
VFUNCTION UDPListener(POINTER_F Dummy);
#ifdef __cplusplus
}
#endif
