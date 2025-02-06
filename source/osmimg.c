#include <df/base.h>

#include <uf/include/base.h>
#include <uf/source/dgprint.h>
#include <uf/source/dgstats.h>
#include <uf/source/hprtns.h>
#include <uf/source/rtrtns.h>
#include <uf/source/thrtns.h>

#include <ci/include/cidef.h>

#include <ci/source/ciconv.h>

#include <aprs/source/osmimg.h>

BOOLEAN_F FUNCTION AprsSendOSM
(	MESSAGE_S *Req,
	STRING_F URL
)
{
	BOOLEAN_F TrackIt = TRUE;

	{	STRING_F Zoom, Lat, Lon;
		MILLISECS_F Start;
		MESSAGE_S *Msg;

		if (TrackIt) Start = RtGetMsec();

		Zoom = CiGetReqFormValue(Req, "Zoom");
		Lat = CiGetReqFormValue(Req, "Lat");
		Lon = CiGetReqFormValue(Req, "Lon");

DgPrintf("Getting panel /%s/%s/%s For %s From tile.openstreetmap.org\n", Zoom, Lat, Lon, URL);

		Msg = CiGetFrom("tile.openstreetmap.org", "/17/36168/54911.png", NULL,NULL,HERE);
		if (!Msg->Success)
		{	CiPassthruBadResponse(Req, Msg);
		} else
		{	COUNT_F Size;
			POINTER_F Body = CiGetPostBody(Msg, &Size);
			CiSendGoodExpiredResponse(Req, Body, Size);
		}
		CiDestroyMessage(Msg);
#ifdef FUTURE
VFUNCTION CiPassthruBadResponse( MESSAGE_S *Req, 	MESSAGE_S *Rsp );

POINTER_F CiGetPostBody( MESSAGE_S *Req, COUNT_F *Count);

http://tile.openstreetmap.org/18/72337/109822.png
http://tile.openstreetmap.org/17/36168/54911.png
http://tah.openstreetmap.org/Tiles/tile/17/36168/54911.png
BGD_DECLARE(gdImagePtr) gdImageCreateFromPngPtr (int size, void *data)
#endif
		if (TrackIt)
		{	DgAccumulateStats("AprsSendOSM",Start,RtGetMsec());
		}
		THREAD_FREE(Zoom); THREAD_FREE(Lat); THREAD_FREE(Lon);
	}
	return TRUE;
}

