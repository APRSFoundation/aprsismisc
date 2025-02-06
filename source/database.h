/* Prototypes generated Wed Feb 25 17:27:17 2009 */
#ifdef __cplusplus
extern "c"
{
#endif
COUNT_F AprsPurgeOrphanPositions(TIMESTAMP_F Youngest);
INTEGER_ID_F AprsGetPositionIndex(COORDINATE_S *Coord);
BOOLEAN_F AprsGetPosition(INTEGER_ID_F PositionIndex, COORDINATE_S *Coord);
INTEGER_ID_F AprsGetStationIndex(STATION_ID_F OwnerID, STATION_ID_F StationID, BOOLEAN_F AllowCreate);
BOOLEAN_F AprsGetStationID(INTEGER_ID_F StationIndex, STATION_ID_F *OwnerID, STATION_ID_F *StationID);
INTEGER_ID_F AprsGetNextPacketIndex(void);
BOOLEAN_F AprsCheckHopPacket(POINTER_F DbRecord, POINTER_F UserArg);
VFUNCTION AprsPurgeOrphans(void);
BOOLEAN_F FUNCTION AprsLoadDatabase(STRING_F PathName);
VFUNCTION AprsCloseDatabase(void);
#ifdef __cplusplus
}
#endif
