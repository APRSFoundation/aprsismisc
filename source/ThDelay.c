#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <df/base.h>

#include <uf/include/base.h>
#include <uf/source/dgrtns.h>
#include <uf/source/dgprint.h>
#include <uf/source/dgstats.h>
#include <uf/source/dshash.h>
#include <uf/source/dsrtns.h>
#include <uf/source/hprtns.h>
#include <uf/source/rtrtns.h>
#include <uf/source/smrtns.h>
#include <uf/source/thrtns.h>

#include <ci/include/cidef.h>
#include <ci/source/ciinit.h>

#include <aprs/include/thdelay.h>

#include <aprs/source/thdelay.h>


/*:ThSetupAllDelayInfoCursor

	This routine returns a cursor enumerating all purger info structures.
*/
static CURSOR_S * FUNCTION ThSetupAllDelayInfoCursor
(	POINTER_F Dummy,
	CALLER
)
{
	return DsSetupHashContentsCursor(ThDelayInfoHash,MY_CALLER);
}

BOOLEAN_F FUNCTION ThBeginDelayedAction
(	STRING_F Name,		/* Must be static */
	STRING_F Group,		/* Must be static */
	COUNT_F Elapsed,	/* Desired time in seconds */
	COUNT_F TotalCount,	/* Total "things" to do */
	CALLER
)
{	ROUTINE(ThBeginDelayedAction);
	TH_DELAY_INFO_S *Info;

static FIRST_TIME_ONLY_S fInfo = {0};
	if (SmFirstTimeOnly(&fInfo,HERE))
	{	ThDelayInfoHash = DsCreateHash("ThDelayInfo", -1, DTH_DELAY_INFO_S, HERE);
		ThDelayLockHash = DsCreateHash("ThDelayLocks", -1, DMUTEX_SEMAPHORE_S, HERE);
		DgAddFilteredDiagnosticCursor("Thread Delayers",
					"{Name}Name,PassesCompleted=Completed,Active,ETA,LastStart=Start,LastStop=Stop,LastElapsed=Elapsed,Elapsed=Target,TotalCount=Total,CurrentCount=Current,PercentComplete=Percent,msSleep=Sleep,Group,GroupLock",
					ThSetupAllDelayInfoCursor);
		SmEndFirstTimeOnly(&fInfo,HERE);
	}
	Info = DsLookupHashKey(ThDelayInfoHash, Name);
	if (!Info)
	{	Info = HEAP_CALLOC(DsGetHashHeap(ThDelayInfoHash),1,sizeof(*Info));
		Info->Name = Name;
		Info->Group = Group;
		Info->GroupLock = DsLookupHashKey(ThDelayLockHash,Group);
		if (!Info->GroupLock)
		{	STRING_F Temp = DsMakeSubName("ThDelayGroup",Group);
			Info->GroupLock = SmCreateMutexSemaphore(Temp,HERE);
			if (!DsInsertIfNewHashKey(ThDelayLockHash, Temp, Info->GroupLock, HERE))
			{	SmDestroyMutexSemaphore(&Info->GroupLock);
				Info->GroupLock = DsLookupHashKey(ThDelayLockHash,Temp);
				if (!Info->GroupLock)
					ThTerminateProcess(Routine, HERE, -1, "Unresolved GroupLock(%s) Collision!\n", Temp);
			} else SmUnlockMutex(&Info->GroupLock, HERE);
			THREAD_FREE(Temp);
		}
		if (!DsInsertIfNewHashKey(ThDelayInfoHash, Name, Info, HERE))
			ThTerminateProcess(Routine, HERE, -1, "Duplicate ThDelayInfo(%s)\n", Name);
	}
	Info->msStart = RtGetMsec();
	Info->LastStart = RtNow(NULL);
	Info->Elapsed = Elapsed;
	Info->TotalCount = TotalCount;
	Info->PercentComplete = Info->CurrentCount = Info->msSleep = 0;
	Info->Active = TRUE;
	Info->ETA = Info->LastStart + Info->Elapsed;
	return SmLockMutex(&Info->GroupLock, MY_CALLER);
}

BOOLEAN_F FUNCTION ThContinueDelayedAction
(	STRING_F Name,
	COUNT_F Count,		/* Count completed (Cumulative) */
	COUNT_F Delta,		/* Count to do between delays */
	BOOLEAN_F NoSleep,	/* TRUE to not sleep due to light load */
	CALLER
)
{	ROUTINE(ThContinueDelayedAction);
	TH_DELAY_INFO_S *Info = DsLookupHashKey(ThDelayInfoHash, Name);

	if (!Info) KILLPROC(-1,"Missing ThDelayInfo");

	if (!Delta) Delta = 1;
	Info->CurrentCount = Count;
	if (Info->CurrentCount && Info->TotalCount > Info->CurrentCount)
	{	TIMESTAMP_F Now = RtNow(NULL);
		MILLISECS_F msElapsed = (RtGetMsec() - Info->msStart);	/* Time it took to do CurrentCount */
		MILLISECS_F msEach = msElapsed / (double) Info->CurrentCount;	/* Time for each completed */
		MILLISECS_F msRemain = (Info->Elapsed * 1000.0) - msElapsed;	/* Remaining time available */
		COUNT_F cRemain = (Info->TotalCount - Info->CurrentCount);	/* How many still to do? */
		MILLISECS_F msRequired = msEach * cRemain;	/* Time Required */
		MILLISECS_F msExtra = msRemain - msRequired;	/* Extra time available to sleep */

		if (Delta > cRemain) Info->msSleep = msExtra;	/* Sleep all the extra now, no more intervals! */
		else Info->msSleep = msExtra * Delta / cRemain;	/* Split it up over how many intervals we have left */

		Info->PercentComplete = Info->CurrentCount * 100 / Info->TotalCount;

		Info->ETA = Info->LastStart + (TIMESTAMP_F)(((double)(Now-Info->LastStart))*(double)Info->TotalCount/(double)Info->CurrentCount);
		
DgPrintf("%s Sleeping %ld at %ld/%ld (%ld%%) %ld Remaining %ld Extra out of %ld (%ld Each * %ld = %ld Required) ETA:%s\n",
	Name, (long) Info->msSleep, (long) Count, (long) Info->TotalCount, (long) Info->PercentComplete,
	(long) msRemain, (long) msExtra, (long) Info->Elapsed,
	(long) msEach, (long) cRemain, (long) msRequired, ctime(&Info->ETA));

		if (Info->msSleep > 60000L) Info->msSleep = 60000L;
		if (Info->msSleep > 0 && !NoSleep)
		{	SmUnlockMutex(&Info->GroupLock, MY_CALLER);
			ThSleepThread(Info->msSleep);
			SmLockMutex(&Info->GroupLock, MY_CALLER);
		}

	} else Info->msSleep = 0;
	return !CiIsShuttingDown();
}

COUNT_F FUNCTION ThCompleteDelayedAction	/* Returns the elapsed seconds */
(	STRING_F Name,
	CALLER
)
{	ROUTINE(ThCompleteDelayedAction);
	MILLISECS_F Now = RtGetMsec();
	TH_DELAY_INFO_S *Info = DsLookupHashKey(ThDelayInfoHash, Name);

	if (!Info) KILLPROC(-1,"Missing ThDelayInfo");
	SmUnlockMutex(&Info->GroupLock, MY_CALLER);


	Info->Active = FALSE;
	Info->LastStop = RtNow(NULL);
	Info->LastElapsed = (Now - Info->msStart) / 1000;
	Info->PassesCompleted++;
	Info->CurrentCount = Info->TotalCount;
	Info->PercentComplete = 100;
	DgAccumulateStats3("ThDelay", Info->Group, Info->Name, 0, Info->msStart, Now);
	return Info->LastElapsed;	/* Return how long this all took! */
}


