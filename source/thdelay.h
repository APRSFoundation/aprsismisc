/* Prototypes generated Wed Sep 10 13:02:43 2008 */
#ifdef __cplusplus
extern "c"
{
#endif
BOOLEAN_F FUNCTION ThBeginDelayedAction( STRING_F Name, 	STRING_F Group, 	COUNT_F Elapsed, COUNT_F TotalCount, CALLER);
BOOLEAN_F FUNCTION ThContinueDelayedAction( STRING_F Name, COUNT_F Count, 	COUNT_F Delta, 	BOOLEAN_F NoSleep, CALLER);
COUNT_F FUNCTION ThCompleteDelayedAction ( STRING_F Name, CALLER);
#ifdef __cplusplus
}
#endif
