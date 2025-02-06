#ifndef GOT_TH_DELAY
#define GOT_TH_DELAY

#include <df/base.h>
#include <uf/include/dsdef.h>

#include <df/global.h>

typedef struct TH_DELAY_INFO_S
{	STRING_F	Name;		/* Which purger this is */
	STRING_F	Group;		/* Exclusivity Group */
	MUTEX_SEMAPHORE_S *GroupLock;	/* Group Lock Semaphore */
	COUNT_F		PassesCompleted;/* Passes Completed */
	TIMESTAMP_F	LastStart;	/* Time of last start */
	TIMESTAMP_F	LastStop;	/* Time of last completion */
	COUNT_F		LastElapsed;	/* Total time of last pass */
	COUNT_F		Elapsed;	/* Target Duration of execution */
	BOOLEAN_F	Active;		/* TRUE if currently active */
	TIMESTAMP_F	ETA;		/* ETA based on Current vs Total and LastStart */
	COUNT_F		TotalCount;	/* Count of items to do */
	COUNT_F		CurrentCount;	/* Count of completed items */
	COUNT_F		PercentComplete;/* Within the current pass */
	COUNT_F		msSleep;	/* Calculated sleep time */
	MILLISECS_F	msStart;	/* Start time in Milliseconds */
} TH_DELAY_INFO_S;

extern DESCRIPTOR_S DTH_DELAY_INFO_S[];
GLOBAL_STORAGE HASH_S *ThDelayLockHash INIT(=NULL);
GLOBAL_STORAGE HASH_S *ThDelayInfoHash INIT(=NULL);

#endif /* GOT_TH_DELAY */

