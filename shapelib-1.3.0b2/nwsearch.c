#include <stdlib.h>
#include <string.h>
#include "shapefil.h"

void ShowShape(char *FileName, int ObjNum)
{
    SHPHandle	hSHP;
    int		nShapeType, nEntities, i, iPart, bValidate = 0,nInvalidCount=0;
    int         bHeaderOnly = 0;
    const char 	*pszPlus;
    double 	adfMinBound[4], adfMaxBound[4];

/* -------------------------------------------------------------------- */
/*      Open the passed shapefile.                                      */
/* -------------------------------------------------------------------- */
    hSHP = SHPOpen( FileName, "rb" );

    if( hSHP == NULL )
    {
        printf( "Unable to open:%s\n", FileName );
        exit( 1 );
    }

/* -------------------------------------------------------------------- */
/*      Print out the file bounds.                                      */
/* -------------------------------------------------------------------- */
    SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );

    printf( "Shapefile Type: %s   # of Shapes: %d\n\n",
            SHPTypeName( nShapeType ), nEntities );
    
    printf( "File Bounds: (%12.3f,%12.3f,%g,%g)\n"
            "         to  (%12.3f,%12.3f,%g,%g)\n",
            adfMinBound[0], 
            adfMinBound[1], 
            adfMinBound[2], 
            adfMinBound[3], 
            adfMaxBound[0], 
            adfMaxBound[1], 
            adfMaxBound[2], 
            adfMaxBound[3] );

/* -------------------------------------------------------------------- */
/*	Skim over the list of shapes, printing all the vertices.	*/
/* -------------------------------------------------------------------- */
    i = ObjNum;
    {
        int		j;
        SHPObject	*psShape;

        psShape = SHPReadObject( hSHP, i );

        if( psShape == NULL )
        {
            fprintf( stderr,
                     "Unable to read shape %d, terminating object reading.\n",
                    i );
        } else
	{

        if( psShape->bMeasureIsUsed )
            printf( "\nShape:%d (%s)  nVertices=%d, nParts=%d\n"
                    "  Bounds:(%12.3f,%12.3f, %g, %g)\n"
                    "      to (%12.3f,%12.3f, %g, %g)\n",
                    i, SHPTypeName(psShape->nSHPType),
                    psShape->nVertices, psShape->nParts,
                    psShape->dfXMin, psShape->dfYMin,
                    psShape->dfZMin, psShape->dfMMin,
                    psShape->dfXMax, psShape->dfYMax,
                    psShape->dfZMax, psShape->dfMMax );
        else
            printf( "\nShape:%d (%s)  nVertices=%d, nParts=%d\n"
                    "  Bounds:(%12.3f,%12.3f, %g)\n"
                    "      to (%12.3f,%12.3f, %g)\n",
                    i, SHPTypeName(psShape->nSHPType),
                    psShape->nVertices, psShape->nParts,
                    psShape->dfXMin, psShape->dfYMin,
                    psShape->dfZMin,
                    psShape->dfXMax, psShape->dfYMax,
                    psShape->dfZMax );

        if( psShape->nParts > 0 && psShape->panPartStart[0] != 0 )
        {
            fprintf( stderr, "panPartStart[0] = %d, not zero as expected.\n",
                     psShape->panPartStart[0] );
        }

        for( j = 0, iPart = 1; j < psShape->nVertices; j++ )
        {
            const char	*pszPartType = "";

            if( j == 0 && psShape->nParts > 0 )
                pszPartType = SHPPartTypeName( psShape->panPartType[0] );
            
            if( iPart < psShape->nParts
                && psShape->panPartStart[iPart] == j )
            {
                pszPartType = SHPPartTypeName( psShape->panPartType[iPart] );
                iPart++;
                pszPlus = "+";
            }
            else
                pszPlus = " ";

            if( psShape->bMeasureIsUsed )
                printf("   %s (%12.3f,%12.3f, %g, %g) %s \n",
                       pszPlus,
                       psShape->padfX[j],
                       psShape->padfY[j],
                       psShape->padfZ[j],
                       psShape->padfM[j],
                       pszPartType );
            else
                printf("   %s (%12.3f,%12.3f, %g) %s \n",
                       pszPlus,
                       psShape->padfX[j],
                       psShape->padfY[j],
                       psShape->padfZ[j],
                       pszPartType );
        }

        {
            int nAltered = SHPRewindObject( hSHP, psShape );

            if( nAltered > 0 )
            {
                printf( "  %d rings wound in the wrong direction.\n",
                        nAltered );
            }
        }
        
        SHPDestroyObject( psShape );
	}
    }

    SHPClose( hSHP );

}

int main( int argc, char ** argv )

{
    DBFHandle	hDBF;
    int		i, iRecord;
    char	*pszFilename = NULL;
    int		nWidth, nDecimals;
    char	szTitle[12];

    int		fID = -1, fState = -1, fFIPS = -1, fZone = -1, fOffice = -1;
    int		fName = -1, fShortName = -1, fLat = -1, fLon = -1;

/* -------------------------------------------------------------------- */
/*      Handle arguments.                                               */
/* -------------------------------------------------------------------- */

    if (argc > 1)
	pszFilename = argv[1];

/* -------------------------------------------------------------------- */
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
    if( argc < 3 )
    {
	printf( "nwsearch xbase_file ID <ID <ID <...>>>\n" );
        printf( "         where ID is the NWS ID to find\n" );
	exit( 1 );
    }

/* -------------------------------------------------------------------- */
/*      Open the file.                                                  */
/* -------------------------------------------------------------------- */
    hDBF = DBFOpen( pszFilename, "rb" );
    if( hDBF == NULL )
    {
	printf( "DBFOpen(%s,\"r\") failed.\n", argv[1] );
	exit( 2 );
    }
    
/* -------------------------------------------------------------------- */
/*	If there is no data in this file let the user know.		*/
/* -------------------------------------------------------------------- */
    if( DBFGetFieldCount(hDBF) == 0 )
    {
	printf( "There are no fields in this table!\n" );
	exit( 3 );
    }

/* -------------------------------------------------------------------- */
/*	Dump header definitions.					*/
/* -------------------------------------------------------------------- */
        for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
        {
            DBFFieldType	eType;
            const char	 	*pszTypeName;
            char chNativeType;

            chNativeType = DBFGetNativeFieldType( hDBF, i );

            eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
            if( eType == FTString )
                pszTypeName = "String";
            else if( eType == FTInteger )
                pszTypeName = "Integer";
            else if( eType == FTDouble )
                pszTypeName = "Double";
            else if( eType == FTInvalid )
                pszTypeName = "Invalid";

            printf( "Field %d: Type=%c/%s, Title=`%s', Width=%d, Decimals=%d\n",
                    i, chNativeType, pszTypeName, szTitle, nWidth, nDecimals );

		if (!stricmp(szTitle, "ID") && eType == FTString)
			fID = i;
		else if (!stricmp(szTitle, "STATE_ZONE") && eType == FTString)
			fID = i;
		else if (!stricmp(szTitle, "STATE") && eType == FTString)
			fState = i;
		else if (!stricmp(szTitle, "FIPS") && eType == FTString)
			fFIPS = i;
		else if (!stricmp(szTitle, "ZONE") && eType == FTString)
			fZone = i;
		else if (!stricmp(szTitle, "CWA") && eType == FTString)
			fOffice = i;
		else if (!stricmp(szTitle, "WFO") && eType == FTString)
			fOffice = i;
		else if (!stricmp(szTitle, "COUNTYNAME") && eType == FTString)
			fName = i;
		else if (!stricmp(szTitle, "NAME") && eType == FTString)
			fName = i;
		else if (!stricmp(szTitle, "SHORTNAME") && eType == FTString)
			fShortName = i;
		else if (!stricmp(szTitle, "LAT") && eType == FTDouble)
			fLat = i;
		else if (!stricmp(szTitle, "LON") && eType == FTDouble)
			fLon = i;
        }

/* -------------------------------------------------------------------- */
/*	Read all the records 						*/
/* -------------------------------------------------------------------- */
    for( iRecord = 0; iRecord < DBFGetRecordCount(hDBF); iRecord++ )
    {
	if (fID != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fID))	/* got an ID column, check it */
	{	char *ID = DBFReadStringAttribute(hDBF, iRecord, fID);
//printf("[%ld/%ld] ID:%s\n", (long) iRecord, (long) DBFGetRecordCount(hDBF), ID);
		if (stricmp(ID, argv[2]))
			continue;
	} else if (fState != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fState)
	&& fFIPS != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fFIPS))	/* Use state + last 3 of FIPS */
	{	char *State = strdup(DBFReadStringAttribute(hDBF, iRecord, fState));
		char *FIPS = strdup(DBFReadStringAttribute(hDBF, iRecord, fFIPS));
//printf("[%ld/%ld] State:%s FIPS:%s\n", (long) iRecord, (long) DBFGetRecordCount(hDBF), State, FIPS);
		if (strnicmp(State, argv[2], 2)
		|| strnicmp(FIPS+2, argv[2]+2, 3))
		{	free(State); free(FIPS);
			continue;
		}
		free(State); free(FIPS);
	} else
	{	fprintf(stderr,"[%ld/%ld] Need ID(%ld)%s or STATE(%ld)%s/FIPS(%ld)%s to match code(%s), terminating...\n",
			(long) iRecord, (long) DBFGetRecordCount(hDBF),
			fID, DBFIsAttributeNULL(hDBF, iRecord, fID)?"NULL":"",
			fState, DBFIsAttributeNULL(hDBF, iRecord, fState)?"NULL":"",
			fFIPS, DBFIsAttributeNULL(hDBF, iRecord, fFIPS)?"NULL":"", argv[2]);
		continue;
	}

	printf("Found %s At %ld\n", argv[2], (long) iRecord);

	if (fID != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fID))
		printf("ID:%s\n", DBFReadStringAttribute(hDBF, iRecord, fID));
	if (fState != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fState))
		printf("State:%s\n", DBFReadStringAttribute(hDBF, iRecord, fState));
	if (fFIPS != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fFIPS))
		printf("FIPS:%s\n", DBFReadStringAttribute(hDBF, iRecord, fFIPS));
	if (fZone != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fZone))
		printf("Zone:%s\n", DBFReadStringAttribute(hDBF, iRecord, fZone));
	if (fOffice != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fOffice))
		printf("Office:%s\n", DBFReadStringAttribute(hDBF, iRecord, fOffice));
	if (fName != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fName))
		printf("Name:%s\n", DBFReadStringAttribute(hDBF, iRecord, fName));
	if (fShortName != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fShortName))
		printf("ShortName:%s\n", DBFReadStringAttribute(hDBF, iRecord, fShortName));
	if (fLat != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fLat))
		printf("Lat:%.5lf\n", DBFReadDoubleAttribute(hDBF, iRecord, fLat));
	if (fLon != -1 && !DBFIsAttributeNULL(hDBF, iRecord, fLon))
		printf("Lon:%.5lf\n", DBFReadDoubleAttribute(hDBF, iRecord, fLon));

	ShowShape(pszFilename, iRecord);

	printf("\n");

	fflush( stdout );
    }

    DBFClose( hDBF );

    return( 0 );
}
