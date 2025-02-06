#ifdef CRCTESTING
/* KD4PCU>R8RP3R,WC4PEM-14,WIDE1*,WIDE3-3:`m;ul w>/]"42}146.700MHz= (0xB2B785C2) */
/* KD4PCU>R8RP3R,WC4PEM-14,WIDE1,WX4MLB-3*,WIDE3-2:`m;ul w>/]"42}146.700MHz= (0xB2B785C2) */
/* KD4PCU>R8RP3R,WC4PEM-14,WIDE1,WC4PEM-10,WC4PEM-13*,WIDE3-1:`m;ul w>/]"42}146.700MHz= (0xB2B785C2) */
#endif

#define WIN_LEAN_AND_MEAN
#include <windows.h>

#include <math.h>
#include <ctype.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#define APRPARSE
#include "parse.c"

void cdecl TraceError(HWND hwnd, char *Format, ...)
{	va_list args;
	va_start(args, Format);
	vprintf(Format, args);
	va_end(args);
}
void cdecl TraceLog(char *Name, BOOL ForceIt, HWND hwnd, char *Format, ...)
{	va_list args;
	va_start(args, Format);
	vprintf(Format, args);
	va_end(args);
}
void cdecl TraceLogThread(char *Name, BOOL ForceIt, char *Format, ...)
{	va_list args;
	va_start(args, Format);
	vprintf(Format, args);
	va_end(args);
}

char *APRSLatLon(double Lat, double Lon, char Table, char Code, int addDigits, int daoDigits, char **pDAO)
{static	char OutBuf[48];
static char DAO[16];
	char *p;
	char NS = (Lat<0)?'S':'N';
	char EW = (Lon<0)?'W':'E';
	char lat[33], lon[33];
	int latLen, lonLen;
	double fLat, fLon;

	if (Lat < 0) Lat = -Lat;
	if (Lon < 0) Lon = -Lon;
	fLat = floor(Lat);
	fLon = floor(Lon);

	DAO[0] = '\0';
	if (!daoDigits)
		sprintf(OutBuf, "%02ld%*.*lf%c%c%03ld%*.*lf%c%c",
						(long) fLat, 5+addDigits, 2+addDigits,
						(double) (Lat - fLat)*60.0, NS, (char) Table,
						(long) fLon, 5+addDigits, 2+addDigits,
						(double) (Lon - fLon)*60.0, EW, (char) Code);
	else
	{	if (daoDigits > 2) daoDigits = 2;
		latLen = sprintf(lat, "%02ld%*.*lf",
						(long) fLat, 5+addDigits+daoDigits, 2+addDigits+daoDigits,
						(double) (Lat - fLat)*60.0);
		lonLen = sprintf(lon, "%03ld%*.*lf",
						(long) fLon, 5+addDigits+daoDigits, 2+addDigits+daoDigits,
						(double) (Lon - fLon)*60.0);

		sprintf(OutBuf,"%.*s%c%c%.*s%c%c",
						latLen-daoDigits, lat, NS, (char) Table,
						lonLen-daoDigits, lon, EW, (char) Code);
		if (daoDigits == 1)
			sprintf(DAO, "!W%c%c!", lat[2+5+addDigits], lon[3+5+addDigits]);
		else if (daoDigits == 2)
		{	printf("Lat: %s -> %ld or %ld (%c) Lon: %s -> %ld or %ld (%c)  (%s)\n",
						lat, (long) atoi(&lat[2+5+addDigits]), (long) (atoi(&lat[2+5+addDigits])/1.10), (char) (atoi(&lat[2+5+addDigits])/1.10+'!'),
						lon, (long) atoi(&lon[3+5+addDigits]), (long) (atoi(&lon[3+5+addDigits])/1.10), (char) (atoi(&lon[3+5+addDigits])/1.10+'!'),
						OutBuf);
			sprintf(DAO, "!w%c%c!", (char)(atoi(&lat[2+5+addDigits])/1.10)+'!',(char)(atoi(&lon[3+5+addDigits])/1.10)+'!'); 
		}
	}
	for (p=OutBuf; *p; p++)
		if (*p == ' ' && (p-OutBuf)!=8+addDigits && (p-OutBuf)!=18+addDigits*2)
			*p = '0';

	if (pDAO) *pDAO = DAO;
	return OutBuf;
}

void ShowMe(char *Pkt)
{	APRS_PARSED_INFO_S Info = {0};
	char *src, *dst, *obj, *relay, *igate, *q, datatype;
	double lat, lon, alt;
	int symbol;
	char *LatLon;
	int hopCount = 0;
	int len = strlen(Pkt);
	char *e, *Temp = malloc(len+1+sizeof(void*));
	char **Hops = NULL;

#ifdef NOT_SURE_WHY
	if (strchr(Pkt,' ')) *strchr(Pkt,' ') = 0x7F;
	if (strchr(Pkt,' ')) *strchr(Pkt,' ') = 0x7D;
#endif

	memset(Temp,0xff,len+1+sizeof(void*));
	strcpy(Temp,Pkt);
	e = Temp + len+1;

	if (!parse_full_aprs(Temp, &Info))
		printf("Parse(%s) Failed!   However...\nFailure: %s\n", Pkt, Info.ParseError);
	{	printf("src:%.*s obj:%.*s dst:%.*s Platform:%.*s Mic-E:%s%s\n",
			sizeof(Info.srcCall), Info.srcCall,
			sizeof(Info.objCall), Info.objCall,
			sizeof(Info.dstCall), Info.dstCall,
			sizeof(Info.Platform), Info.Platform,
			Info.Valid&APRS_MICE_MESSAGE_VALID?Info.MicEMessage:"",
			Info.Valid&APRS_NWS_VALID?" NWS":"");
		printf("CleanComment:%.*s Capability:%.*s Status:%.*s %s\n",
			sizeof(Info.CleanComment), Info.CleanComment,
			sizeof(Info.Capabilities), Info.Capabilities,
			sizeof(Info.StatusReport), Info.StatusReport, Info.MessageCapable?"MESSAGES!":"");
		if (Info.ObjectKilled) printf("**** KILLED OBJECT! ****\n");
		obj = Info.Valid&(APRS_ITEM_VALID|APRS_OBJECT_VALID)?Info.objCall:Info.srcCall;
		if (Info.Valid)
		{	int h;
			hopCount = Info.Path.hopCount;
			Hops = malloc(sizeof(*Hops)*(Info.Path.hopCount+1));
			for (h=0; h<Info.Path.hopCount; h++)
			{	Hops[h] = Info.Path.Hops[h];
			}
			Hops[h] = NULL;	/* just for grins */
			src = Info.srcCall;
			dst = Info.dstCall;
		} else
		{	src = dst = "";
			hopCount = 0;
		}
		if (Info.Valid & APRS_LATLON_VALID || TRUE)
		{	lat = Info.lat;
			lon = Info.lon;
		} else lat = lon = 0.0;
		if (Info.Valid & APRS_ALTITUDE_VALID)
			alt = Info.alt;
		else alt = 0.0;
		if (Info.Valid & APRS_SYMBOL_VALID)
			symbol = Info.symbol;
		else if (Info.Valid & APRS_SYMBOL_DEFAULTED)
			symbol = Info.symbol;
		else symbol = 0;
		if (Info.Valid & APRS_DATATYPE_VALID)
			datatype = Info.datatype;
		else datatype = 0;

		if (hopCount && Hops)
		{	int h;
			for (h=0; h<hopCount; h++)
				printf("Hop[%ld] = %s\n", (long) h, Hops[h]);
		} else printf("HopCount = %ld\n", (long) hopCount);

		LatLon = APRSLatLon(lat, lon, ' ', ' ', 4, 0, NULL);


		parse_route(hopCount, Hops, &igate, &relay, &q);
		printf("From: %s (Obj: %s) To: %s Via: %s and %s (q=%s) Datatype:%c\n",
			src, obj, dst, relay, igate, q, Info.datatype);
		printf("%s at %.6lf %.6lf (%s)(Ambig:%ld %ld) (Spd:%.2lf Cse:%ld %s) (Alt: %.2lfm) (datatype %c)\n",
			obj, lat, lon, LatLon, (long) Info.latAmbiguity, (long) Info.lonAmbiguity,
			(double) Info.speed, (long) Info.course, Info.Valid&APRS_CRSSPD_VALID?"Valid":"NOT Valid",
			alt, isprint(datatype)?datatype:'?');
		printf("Symbol:0x%lX %s %c%s%c (%s)\n",
			symbol, (symbol>>8)&0xff?"Alternate":"Primary",
			isprint(symbol&0xff)?(symbol&0xff):'?',
			((symbol>>16)&0xff)?" Over ":"",
			((symbol>>16)&0xff)?(isprint((symbol>>16)&0xff)?((symbol>>16)&0xff):'?'):' ',
			GetDisplayableSymbol(symbol));
		printf("CRC32: 0x%lX\n", (unsigned long) Info.CRC32);
		if (Info.Valid & APRS_WEATHER_VALID) printf("HAS WEATHER!\n");

	}
	if (hopCount && Hops)
	{	int h;
		for (h=0; h<hopCount; h++)
			printf("Hop[%ld] = %s\n", (long) h, Hops[h]);
		free(Hops);
	} else printf("HopCount = %ld\n", (long) hopCount);
	for (e=Temp+len+1; e<Temp+len+1+sizeof(void*); e++)
		if ((long)*e&0xff != 0xff)
			printf("e[%ld] Stomped with 0x%02lX\n",
				(long) (e-(Temp+len+1)), (long)*e&0xff);

}


int main(int argc, char *argv[])
{static char *DefArgs[] = { "AprParse.exe",
				"VK2JNG-12>S5QLZL,VK1RGI-1*,WIDE2-2,qAR,VK1KCM-5:'M4\x1c""l \x1c""K\\]146.950MHz G'Day to all APRS users.=",
				"AI4LE-1>RXQY9S,WC4PEM-14*,WIDE3-2:`m> nfvv/>\"47 \r\n",
				"KE5CAB-11>APT311,WIDE3-3:/150014h3559.06N/09701.31WO318/000/A=000918",
				"KE5CAB-11>3U5Y0V,WIDE3-3:`}Y;l!HO/\"7%}",
				"WB4ATV>8PT6T,RGLAY,WIDE:`lB-l ?v/]\"3]}",
				"WB4ATV>R8PT6T,RELAY,WIDE:`lB-l ?v/]\"3^}" };
#ifdef BUSTED_MIC_E
2010-03-09 11:24:19 UTC VK2JNG-12: 95 bytes
0x00 V K 2 J N G - 1 2 > S 5 Q L Z L , V K 1 R G I - 1 * , W I D E 2 
     564b324a4e472d31323e5335514c5a4c2c564b315247492d312a2c5749444532
0x20 - 2 , q A R , V K 1 K C M - 5 : ' M 4 1cl   1cK \ ] 1 4 6 . 9 5 
     2d322c7141522c564b314b434d2d353a274d341c6c201c4b5c5d3134362e3935
0x40 0 M H z     G ' D a y   t o   a l l   A P R S   u s e r s . = 
     304d487a2020472744617920746f20616c6c20415052532075736572732e3d

2010-03-09 11:24:19 UTC: VK2JNG-12>S5QLZL,VK1RGI-1*,WIDE2-2,qAR,VK1KCM-5:'M4<0x1c>l <0x1c>K\]146.950MHz G'Day to all APRS users.=
   type: location
   srccallsign: VK2JNG-12
   dstcallsign: S5QLZL
   latitude: -35.25 °
   longitude: 149.4166666666667 °
   course: 0 °
   speed: 0 km/h
   symboltable: \
   symbolcode: K
   mbits: 101
   posresolution: 18520 m
   posambiguity: 3
   comment: ]146.950MHz G'Day to all APRS users.=

aprs.fi has 35 15.00S 149 25.00E
aprsis32 has 35 10.00S 149 24.00E

MicE(1)  VK2JNG-12 (146.950MHz}G'Day to all APRS users.=)
MicE  VK2JNG-12  (146.950MHz}G'Day to all APRS users.=)
Converting rx_lat(3510.00) rx_lon(14924.00)
src:VK2JNG-12 obj: dst:S5QLZL Comment:146.950MHz}G'Day to all APRS users.= Platform:
Hop[0] = VK2JNG-12
Hop[1] = S5QLZL
Hop[2] = VK1RGI-1*
Hop[3] = WIDE2-2
Hop[4] = qAR
Hop[5] = VK1KCM-5

#endif


	int a;

	if (argc < 2)
	{	char *DAO;
		char *LatLon;
		char Buffer[256];
		char *Owner = "KJ4ERJ-12";
		char x = 'D';
		char RFID[] = "1234567890";
		char Comment[] = "Test RFID Reader";
		char Station[] = "LynnsOffc";
		char srcCall[] = "KJ4ERJ-OF";
		double lat = 28+31.234567/60.0;
		double lon = -(80+37.654321/60.0);
#ifndef STRING
#define STRING(s) sizeof(s),s
#endif

		LatLon = APRSLatLon(lat, lon, 'R', 'A', 0, 1, &DAO);
		printf("RFID:Moving(%s) RFI%c(%.*s) To(%s)(%s) Via(%.*s)(%.*s)\n",
					Owner, x, STRING(Comment),
					LatLon, DAO, STRING(Station), STRING(srcCall));

		sprintf(Buffer,"%s>APRFI%c,WIDE2-2:!%s%.*s@%.*s%.*s %s",
				Owner, x,
				LatLon, STRING(RFID), STRING(Station),
				STRING(Comment), DAO);
		ShowMe(Buffer);

		LatLon = APRSLatLon(lat, lon, 'R', 'A', 0, 2, &DAO);
		printf("RFID:Moving(%s) RFI%c(%.*s) To(%s)(%s) Via(%.*s)(%.*s)\n",
					Owner, x, STRING(Comment),
					LatLon, DAO, STRING(Station), STRING(srcCall));

		sprintf(Buffer,"%s>APRFI%c,WIDE2-2:!%s%.*s@%.*s%.*s %s",
				Owner, x,
				LatLon, STRING(RFID), STRING(Station),
				STRING(Comment), DAO);
		ShowMe(Buffer);
	}

	if (argc < 2)
	{	argc = (sizeof(DefArgs) / sizeof(DefArgs[0]));
		argv = malloc(sizeof(*argv)*argc);
		for (a=0; a<argc; a++)
			argv[a] = strdup(DefArgs[a]);
	}
	for (a=1; a<argc; a++)
	{	ShowMe(argv[a]);
	}
	return 0;
}

