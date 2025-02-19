/*
csv2shp - converts a character delimited file to a ESRI shapefile
Copyright (C) 2005 Springs Rescue Mission


LICENSE
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

The GNU General Public License is also available from the web
site <http://www.gnu.org>.


GRATITUDE
Like this program?  Donate at <http://springsrescuemission.org>.


COMPILING INSTRUCTIONS
This program was written and tested using Shapefile C Library version
1.2.10 available at <http://shapelib.maptools.org>.

To compile, copy csv2shp.c to the directory with Shapefile C Library.
Then, compile Shapefile C library.  Then, run something like this:

	gcc -pedantic -Wall -o csv2shp csv2shp.c -I. *o


USAGE NOTES
This program operates on single points only (not polygons or lines).

The input file may be a .csv file (comma separated values) or tab-separated
values, or it may be separated by any other character.  The first row must 
contain column names.  There must be each a column named longitude and 
latitude in the input file.

The .csv parser does not understand text delimiters (e.g. quotation mark).
It parses fields only by the given field delimiter (e.g. comma or tab).
The program has not been tested with null values, and in this case, the
behavior is undefined.  The program will not accept lines with a trailing 
delimiter character.

All columns (including longitude and latitude) in the input file are exported 
to the .dbf file.

The program attempts to find the best type (integer, decimal, string) and
smallest size of the fields necessary for the .dbf file.


SUPPORT
Springs Rescue Mission does not offer any support for this program.


CONTACT INFORMATION
Springs Rescue Mission
5 West Las Vegas St
PO Box 2108
Colorado Springs CO 80901
Web: <http://springsrescuemission.org>
Email: <http://springsrescuemission.org/email.php?recipient=webmaster>

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shapefil.h"
#include "regex.h"

#define MAX_COLUMNS 30

typedef struct column_t {
	DBFFieldType eType;
	int nWidth;
	int nDecimals;
} column;

/* counts the number of occurances of the character in the string */
int strnchr(const char *s, char c)
{
	int n = 0;
	int x = 0;

	for (; x < strlen(s); x++)
	{
		if (c == s[x])
		{
			n++;
		}
	}

	return n;
}

/* Returns a field given by column n (0-based) in a character-
   delimited string s */
char * delimited_column(char *s, char delim, int n)
{
	static char szreturn[4096];
	char szbuffer[4096]; /* a copy of s */
	char * pchar;
	int x;
	char szdelimiter[2]; /* delim converted to string */

	if (strnchr(s, delim) < n)
	{
		fprintf(stderr, "delimited_column: n is too large\n");
		return NULL;
	}

	strcpy(szbuffer, s);
	szdelimiter[0] = delim;
	szdelimiter[1] = '\0';
	x = 0;
	pchar = strtok(szbuffer, szdelimiter);
	while (x < n)
	{	
		pchar = strtok(NULL, szdelimiter);
		x++;
	}

	if (NULL == pchar)
	{
		return NULL;
	}

	strcpy(szreturn, pchar);
	return szreturn;
}

/* Determines the most specific column type.
   The most specific types from most to least are integer, float, string.  */
DBFFieldType str_to_fieldtype(const char *s)
{
	regex_t regex_i;
	regex_t regex_d;

	if (0 != regcomp(&regex_i, "^[0-9]+$", REG_NOSUB|REG_EXTENDED))
	{
		fprintf(stderr, "integer regex complication failed\n");
		exit (EXIT_FAILURE);
	}
	
	if (0 == regexec(&regex_i, s, 0, NULL, 0))
	{
		regfree(&regex_i);
		return FTInteger;
	}

	regfree(&regex_i);

	if (0 != regcomp(&regex_d, "^-?[0-9]+\\.[0-9]+$", REG_NOSUB|REG_EXTENDED))
	{
		fprintf(stderr, "integer regex complication failed\n");
		exit (EXIT_FAILURE);
	}

	if (0 == regexec(&regex_d, s, 0, NULL, 0))
	{
		regfree(&regex_d);
		return FTDouble;
	}

	regfree(&regex_d);

	return FTString;
}

int float_width(const char *s)
{
	regex_t regex_d;
	regmatch_t pmatch[2];
	char szbuffer[4096];

	if (0 != regcomp(&regex_d, "^(-?[0-9]+)\\.[0-9]+$", REG_EXTENDED))
	{
		fprintf(stderr, "integer regex complication failed\n");
		exit (EXIT_FAILURE);
	}

	if (0 != regexec(&regex_d, s, 2, &pmatch[0], 0))
	{
		return -1;
	}

	strncpy(szbuffer, &s[pmatch[1].rm_so], pmatch[1].rm_eo - pmatch[1].rm_so);
	szbuffer[pmatch[1].rm_eo - pmatch[1].rm_so] = '\0';
	regfree(&regex_d);

	return strlen(szbuffer);
}

/* returns the field width */
int str_to_nwidth(const char *s, DBFFieldType eType)
{
	switch (eType)
	{
		case FTString:
		case FTInteger:
		case FTDouble:
			return strlen(s);

		default:
			fprintf(stderr, "str_to_nwidth: unexpected type\n");
			exit (EXIT_FAILURE);
	}
}

/* returns the number of decimals in a real number given as a string s */
int str_to_ndecimals(const char *s)
{
	regex_t regex_d;
	regmatch_t pmatch[2];
	char szbuffer[4096];

	if (0 != regcomp(&regex_d, "^-?[0-9]+\\.([0-9]+)$", REG_EXTENDED))
	{
		fprintf(stderr, "integer regex complication failed\n");
		exit (EXIT_FAILURE);
	}

	if (0 != regexec(&regex_d, s, 2, &pmatch[0], 0))
	{
		return -1;
	}

	strncpy(szbuffer, &s[pmatch[1].rm_so], pmatch[1].rm_eo - pmatch[1].rm_so);
	szbuffer[pmatch[1].rm_eo - pmatch[1].rm_so] = '\0';

	regfree(&regex_d);

	return strlen(szbuffer);
}

/* returns true if f1 is more general than f2, otherwise false */
int more_general_field_type(DBFFieldType t1, DBFFieldType t2)
{
	if (FTInteger == t2 && t1 != FTInteger)
	{
		return 1;
	}

	if (FTDouble == t2 && FTString == t1)
	{
		return 1;
	}
	
	return 0;
}

void strip_crlf (char *line)
{
  /* remove trailing CR/LF */

  if (strchr (line, 0x0D))
    {
      char *pszline;
      pszline = strchr (line, 0x0D);
      pszline[0] = '\0';
    }

  if (strchr (line, 0x0A))
    {
      char *pszline;
      pszline = strchr (line, 0x0A);
      pszline[0] = '\0';
    }
}

int main( int argc, char ** argv )
{
	FILE *csv_f;
	char sbuffer[4096];
	char delimiter;
	int n_columns; /* 1-based */
	int n_line;
	int n_longitude = -1; /* column with x, 0 based */
	int n_latitude = -1; /* column with y, 0 based */
	int x;
	DBFHandle dbf_h;
	SHPHandle shp_h;
	column columns[MAX_COLUMNS + 1];

	printf("csv2shp version 1, Copyright (C) 2005 Springs Rescue Mission\n");

	if (4 != argc)
	{
		fprintf(stderr, "csv2shp comes with ABSOLUTELY NO WARRANTY; for details\n");
		fprintf(stderr, "see csv2shp.c.  This is free software, and you are welcome\n");
		fprintf(stderr, "to redistribute it under certain conditions; see csv2shp.c\n");
		fprintf(stderr, "for details\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "USAGE\n");
		fprintf(stderr, "csv2shp csv_filename delimiter_character shp_filename\n");
		fprintf(stderr, "   csv_filename\n");
		fprintf(stderr, "     columns named longitude and latitude must exist\n");
		fprintf(stderr, "   delimiter_character\n");
		fprintf(stderr, "     one character only\n");
		fprintf(stderr, "   shp_filename\n");
		fprintf(stderr, "     base name, do not give the extension\n");
		return EXIT_FAILURE;
	}

	if (strlen(argv[2]) > 1)
	{
		fprintf(stderr, "delimiter must be one character in length\n");
		return EXIT_FAILURE;
	}

	delimiter = argv[2][0];

	csv_f = fopen(argv[1], "r");

	if (NULL == csv_f)
	{
		perror("could not open csv file");
		exit (EXIT_FAILURE);
	}

	fgets(sbuffer, 4000, csv_f);

	/* check first row */

	strip_crlf(sbuffer);

	if (delimiter == sbuffer[strlen(sbuffer)- 1])
	{
		fprintf(stderr, "lines must not end with the delimiter character\n");
		return EXIT_FAILURE;

	}

	/* count columns and verify consistency*/

	n_columns = strnchr(sbuffer, delimiter);

	if (n_columns > MAX_COLUMNS)
	{
		fprintf(stderr, "too many columns, maximum is %i\n", MAX_COLUMNS);
		return EXIT_FAILURE;
	}

	n_line = 1;

	while (!feof(csv_f))
	{
		n_line++;
		fgets(sbuffer, 4000, csv_f);
		if (n_columns != strnchr(sbuffer, delimiter))
		{
			fprintf(stderr, "Number of columns on row %i does not match number of columns on row 1\n", n_columns);
			return EXIT_FAILURE;
		}
	}

	/* identify longitude and latitude columns */

	fseek(csv_f, 0, SEEK_SET);
	fgets(sbuffer, 4000, csv_f);
	strip_crlf(sbuffer);

	for (x = 0; x <= n_columns; x++)
	{	
		if (0 == strcasecmp("Longitude", delimited_column(sbuffer, delimiter, x)))
		{
			n_longitude = x;
		}

		if (0 == strcasecmp("Latitude", delimited_column(sbuffer, delimiter, x)))
		{
			n_latitude = x;
		}
	}

#ifdef DEBUG
	printf("debug lat/long = %i/%i\n", n_latitude, n_longitude);
#endif

	if (-1 == n_longitude || -1 == n_latitude)
	{
		fprintf(stderr, "The header row must define one each a column named longitude and latitude\n");
		return EXIT_FAILURE;
	}

	/* determine best fit for each column */

	printf ("Anaylzing column types...\n");

#ifdef DEBUG
	printf("debug: string type = %i\n", FTString);
	printf("debug: int type = %i\n", FTInteger);
	printf("debug: double type = %i\n", FTDouble);
#endif
	for (x = 0; x <= n_columns; x++)
	{	
#ifdef DEBUG
		printf("debug: examining column %i\n", x);
#endif
		columns[x].eType = FTInteger;
		columns[x].nWidth = 2;
		columns[x].nDecimals = 0;
	
		fseek(csv_f, 0, SEEK_SET);
		fgets(sbuffer, 4000, csv_f);

		while (!feof(csv_f))
		{
			char szfield[4096];
#ifdef DEBUG
			printf ("column %i, type = %i, w = %i, d = %i\n", x, columns[x].eType, columns[x].nWidth, columns[x].nDecimals);
#endif
			if (NULL == fgets(sbuffer, 4000, csv_f))
			{
				if (!feof(csv_f))
				{
					fprintf(stderr, "error during fgets()\n");
				}
				continue;
			}
			strcpy(szfield, delimited_column(sbuffer, delimiter, x));
			if (more_general_field_type(str_to_fieldtype(szfield), columns[x].eType))
			{
				columns[x].eType = str_to_fieldtype(szfield);
				columns[x].nWidth = 2;
				columns[x].nDecimals = 0;
				fseek(csv_f, 0, SEEK_SET);
				fgets(sbuffer, 4000, csv_f);
				continue;
			}
			if (columns[x].nWidth < str_to_nwidth(szfield, columns[x].eType))
			{
				columns[x].nWidth = str_to_nwidth(szfield, columns[x].eType);
			}
			if (FTDouble == columns[x].eType && columns[x].nDecimals < str_to_ndecimals(szfield))
			{
				columns[x].nDecimals = str_to_ndecimals(szfield);
			}

		}
	}


	/* initilize output files */

	printf ("Initializing output files...\n");

	shp_h = SHPCreate(argv[3], SHPT_POINT);

	dbf_h = DBFCreate(argv[3]);

	if (NULL == dbf_h)
	{
		fprintf(stderr, "DBFCreate failed\n");
		exit (EXIT_FAILURE);
	}

	fseek(csv_f, 0, SEEK_SET);
	fgets(sbuffer, 4000, csv_f);
	strip_crlf(sbuffer);

	for (x = 0; x <= n_columns; x++)
	{	
#ifdef DEBUG
		printf ("debug: final: column %i, type = %i, w = %i, d = %i, name=|%s|\n", x, columns[x].eType, columns[x].nWidth, columns[x].nDecimals, delimited_column(sbuffer, delimiter, x));
#endif
		if (-1 == DBFAddField(dbf_h, delimited_column(sbuffer, delimiter, x), columns[x].eType, columns[x].nWidth, columns[x].nDecimals))
		{
			fprintf(stderr, "DBFFieldAdd failed column %i\n", x + 1);
			exit (EXIT_FAILURE);
		}

	}

	/* write data */

	printf ("Writing data...\n");

	fseek(csv_f, 0, SEEK_SET);
	fgets(sbuffer, 4000, csv_f); /* skip header */

	n_columns = strnchr(sbuffer, delimiter);
	n_line = 1;

	while (!feof(csv_f))
	{
		SHPObject * shp;
		double x_pt;
		double y_pt;
		int shp_i;

		n_line++;
		fgets(sbuffer, 4000, csv_f);
		
		/* write to shape file */
		x_pt = atof(delimited_column(sbuffer, delimiter, n_longitude));
		y_pt = atof(delimited_column(sbuffer, delimiter, n_latitude));

#ifdef DEBUG
		printf("debug: sbuffer=%s", sbuffer);
		printf("debug: x,y = %f, %f\n", x_pt, y_pt);
#endif

		shp = SHPCreateSimpleObject(SHPT_POINT, 1, &x_pt, &y_pt, NULL);
		shp_i = SHPWriteObject(shp_h, -1, shp);
		SHPDestroyObject(shp);

		/* write to dbf */

		for (x = 0; x <= n_columns; x++)
		{	
			char szfield[4096];
			int b;

			strcpy(szfield, delimited_column(sbuffer, delimiter, x));

			switch (columns[x].eType)
			{
				case FTInteger:
					b = DBFWriteIntegerAttribute(dbf_h, shp_i, x, atoi(szfield));
					break;
				case FTDouble:
					b = DBFWriteDoubleAttribute(dbf_h, shp_i, x, atof(szfield));
					break;
				case FTString:
					b = DBFWriteStringAttribute(dbf_h, shp_i, x, szfield);
					break;
				default:
					fprintf(stderr, "unexpected column type %i in column %i\n", columns[x].eType, x);
			}
			
			if (!b)
			{
				fprintf(stderr, "DBFWrite*Attribute failed\n");
				exit (EXIT_FAILURE);
			}
		}
	}

	/* finish up */

	SHPClose(shp_h);
	
        DBFClose(dbf_h);

	return EXIT_SUCCESS;
}
