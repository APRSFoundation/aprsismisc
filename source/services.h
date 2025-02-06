/* Prototypes generated Mon Oct  2 16:46:27 2017 */
#ifdef __cplusplus
extern "c"
{
#endif
char *GetSymbolName(int Symbol);
VFUNCTION AprsMakeStationID(STRING_F ID, STATION_ID_F *rID);
VFUNCTION AprsBoundingBox(COORDINATE_S *Where, COORDINATE_S *Min, COORDINATE_S *Max);
VFUNCTION AprsHaversine(COORDINATE_S *From, COORDINATE_S *To, double *Dist, double *Bearing);
BOOLEAN_F AprsGetStationLocation( STATION_ID_F OwnerID, STATION_ID_F StationID, INTEGER_ID_F *pStationIndex, COORDINATE_S *pLoc, INTEGER_ID_F *pIndex);
BOOLEAN_F FUNCTION AprsRelayToAnyTrak(STRING_F obj, double lat, double lon);
COUNT_F AprsGetLogLength(STRING_F Packet);
BOOLEAN_F FUNCTION AprsSvcLogPacket(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcLogReceivedPacket(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcLogDigipeatedPacket(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcReParseLogFile(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcPurgePackets(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcQueryPackets(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcDeletePackets(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcQueryHops(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcQueryStations(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcLookupStation(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcQueryStationIndices(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcQueryUniqueIGates(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcQueryUniqueDestinations(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcQueryUniquePaths(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcQueryUniqueAltPaths(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcQueryTrafficRates(MESSAGE_S *Req, STRING_F URL);
BOOLEAN_F FUNCTION AprsSvcCalcOSMTiles(MESSAGE_S *Req, STRING_F URL);
char *APRSLatLon(double Lat, double Lon, char Table, char Code);
char *APRSAltitude(int Valid, double Alt);
char *APRSHeadSpeed(int Valid, double Hdg, double Spd);
BOOLEAN_F FUNCTION AprsSvcInjectStationPosition(MESSAGE_S *Req, STRING_F URL);
#ifdef __cplusplus
}
#endif
