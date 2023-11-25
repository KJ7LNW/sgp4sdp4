/* Unit SGP_In */
/*           Author:  Dr TS Kelso */
/* Original Version:  1992 Jun 25 */
/* Current Revision:  1999 Nov 27 */
/*          Version:  2.10 */
/*        Copyright:  1992-1999, All Rights Reserved */

/* Ported to C by N. Kyriazis  April 6  2001 */

#define SGP4SDP4_CONSTANTS
#include "sgp4sdp4.h"

/* Calculates the checksum mod 10 of a line from a TLE set and */
/* returns 1 if it compares with checksum in column 68, else 0.*/
/* tle_set is a character string holding the two lines read    */
/* from a text file containing NASA format Keplerian elements. */
  int
Checksum_Good( char *tle_set )
{
  int i, check_digit, value, checksum = 0;

  for(i = 0; i < 68; i++)
  {
	if( (tle_set[i] >= '0') && (tle_set[i] <= '9') )
	  value = tle_set[i] - '0';
	else if( tle_set[i] == '-' )
	  value = 1;
	else
	  value = 0;

	checksum += value;
  } /* End for(i = 0; i < 68; i++) */

  checksum %= 10;
  check_digit = tle_set[68] - '0';

  return( checksum == check_digit );
} /* Function Checksums_Good */

/*------------------------------------------------------------------*/

/* Carries out various checks on a TLE set to verify its validity */
/* tle_set is a character string holding the two lines read    */
/* from a text file containing NASA format Keplerian elements. */
  int
Good_Elements( char *tle_set )
{
  /* Verify checksum of both lines of a TLE set */
  if( !Checksum_Good(&tle_set[0]) || !Checksum_Good(&tle_set[69]) )
	return (0);
  /* Check the line number of each line */
  if( (tle_set[0] != '1') || (tle_set[69] != '2') )
	return (0);
  /* Verify that Satellite Number is same in both lines */
  if( strncmp( &tle_set[2], &tle_set[71], 5 ) != 0 )
	return (0);
  /* Check that various elements are in the right place */
  if( (tle_set[ 23] != '.') ||
	  (tle_set[ 34] != '.') ||
	  (tle_set[ 80] != '.') ||
	  (tle_set[ 89] != '.') ||
	  (tle_set[106] != '.') ||
	  (tle_set[115] != '.') ||
	  (tle_set[123] != '.') ||
	  (strncmp(&tle_set[61], " 0 ", 3) != 0)
	)
	return (0);

  return(1);
}  /* Function Good_Elements */

/*------------------------------------------------------------------*/

/* Converts the strings in a raw two-line element set  */
/* to their intended numerical values. No processing   */
/* of these values is done, e.g. from deg to rads etc. */
/* This is done in the select_ephemeris() function.    */
  void
Convert_Satellite_Data( char *tle_set, tle_t *tle )
{
  char buff[15];
  int i;

  /* Terminate white space on the right side of the name */
  for (i = strlen(tle->sat_name)-1; i >= 0 && isspace((int)tle->sat_name[i]); i--)
	  tle->sat_name[i] = 0;

  /** Decode Card 1 **/
  /* Satellite's catalogue number */
  strncpy( buff, &tle_set[2],5 );
  buff[5] = '\0';
  tle->catnr = atoi(buff);

  /* International Designator for satellite */
  strncpy( tle->idesg, &tle_set[9],8 );
  tle->idesg[8] = '\0';
  for (i = 7; i && isspace((int)tle->idesg[i]); i--)
	  tle->idesg[i] = '\0';

  /* Satellite's epoch */
  strncpy( buff, &tle_set[18],14 );
  buff[14] = '\0';
  tle->epoch = atof(buff);

  /* Satellite's First Time Derivative */
  strncpy( buff, &tle_set[33],10 );
  buff[10]='\0';
  tle->xndt2o = atof(buff);

  /* Satellite's Second Time Derivative */
  strncpy( buff, &tle_set[44],1 );
  buff[1] = '.';
  strncpy( &buff[2], &tle_set[45],5 );
  buff[7] = 'E';
  strncpy( &buff[8], &tle_set[50],2 );
  buff[10]='\0';
  tle->xndd6o = atof(buff);

  /* Satellite's bstar drag term */
  strncpy( buff, &tle_set[53],1 );
  buff[1] = '.';
  strncpy( &buff[2], &tle_set[54],5 );
  buff[7] = 'E';
  strncpy( &buff[8], &tle_set[59],2 );
  buff[10]='\0';
  tle->bstar = atof(buff);

  /* Element Number */
  strncpy( buff, &tle_set[64],4 );
  buff[4]='\0';
  tle->elset = atoi(buff);

  /** Decode Card 2 **/
  /* Satellite's Orbital Inclination (degrees) */
  strncpy( buff, &tle_set[77], 8 );
  buff[8]='\0';
  tle->xincl = atof(buff);

  /* Satellite's RAAN (degrees) */
  strncpy( buff, &tle_set[86], 8 );
  buff[8]='\0';
  tle->xnodeo = atof(buff);

  /* Satellite's Orbital Eccentricity */
  buff[0] = '.';
  strncpy( &buff[1], &tle_set[95], 7 );
  buff[8]='\0';
  tle->eo = atof(buff);

  /* Satellite's Argument of Perigee (degrees) */
  strncpy( buff, &tle_set[103], 8 );
  buff[8]='\0';
  tle->omegao = atof(buff);

  /* Satellite's Mean Anomaly of Orbit (degrees) */
  strncpy( buff, &tle_set[112], 8 );
  buff[8]='\0';
  tle->xmo = atof(buff);

  /* Satellite's Mean Motion (rev/day) */
  strncpy( buff, &tle_set[121], 10 );
  buff[10]='\0';
  tle->xno = atof(buff);

  /* Satellite's Revolution number at epoch */
  strncpy( buff, &tle_set[132], 5 );
  buff[5]='\0';
  double rvn = atof(buff);
  tle->revnum = (int)rvn;

} /* Procedure Convert_Satellite_Data */

/*------------------------------------------------------------------*/

/* Opens TLE source file and reads one (first!) TLE set  */
/* including the name line 0. If only the two Keplerian  */
/* element lines exist, this function will fail. Returns */
/* -1 if file open fails and -2 if TLE set is not valid  */
  int
Input_Tle_Set( char *tle_file, tle_t *tle)
{
  int chr;  /* Used for inputting characters */

  char sat_name[sizeof(tle->sat_name)]; /* Temp var for satellite name */
  char tle_set[139]; /* Two lines of a TLE set */

  /* File pointer for opening TLE source file */
  FILE *fp;

  /* Open TLE file, abort on failure */
  if( (fp = fopen( tle_file, "r")) == NULL )
	return(-1);

  /* Read the satellite's name */
  fgets(sat_name, sizeof(sat_name), fp);
  strncpy(tle->sat_name, sat_name, sizeof(tle->sat_name));
  tle->sat_name[sizeof(tle->sat_name)-1] = 0;

  /* Read in first line of TLE set */
  fgets( tle_set, 70, fp );

  /* Dump CR/LF, put back last character */
  while( ((chr = fgetc(fp)) == CR) || (chr == LF) );
  ungetc(chr,fp);

  /* Read in second line of TLE set and terminate string */
  fgets( &tle_set[69], 70, fp );
  tle_set[138]='\0';

  fclose(fp);

  /* Check TLE set and abort if not valid */
  if( !Good_Elements(tle_set) )
	return(-2);

  /* Convert the TLE set to orbital elements */
  Convert_Satellite_Data( tle_set, tle );

  return(0);
} /* End of Input_Tle_Set() */

/*------------------------------------------------------------------*/

/* Selects the apropriate ephemeris type to be used */
/* for predictions according to the data in the TLE */
/* It also processes values in the tle set so that  */
/* they are apropriate for the sgp4/sdp4 routines   */
  void
select_ephemeris(tle_t *tle)
{
  double ao,xnodp,dd1,dd2,delo,temp,a1,del1,r1;

  /* Preprocess tle set */
  tle->xnodeo *= de2ra;
  tle-> omegao *= de2ra;
  tle->xmo *= de2ra;
  tle->xincl *= de2ra;
  temp = twopi/xmnpda/xmnpda;
  tle->xno = tle->xno*temp*xmnpda;
  tle->xndt2o *= temp;
  tle->xndd6o = tle->xndd6o*temp/xmnpda;
  tle->bstar /= ae;

  /* Period > 225 minutes is deep space */
  dd1 = (xke/tle->xno);
  dd2 = tothrd;
  a1 = pow(dd1, dd2);
  r1 = cos(tle->xincl);
  dd1 = (1.0-tle->eo*tle->eo);
  temp = ck2*1.5f*(r1*r1*3.0-1.0)/pow(dd1, 1.5);
  del1 = temp/(a1*a1);
  ao = a1*(1.0-del1*(tothrd*.5+del1*
		(del1*1.654320987654321+1.0)));
  delo = temp/(ao*ao);
  xnodp = tle->xno/(delo+1.0);

  /* Select a deep-space/near-earth ephemeris */
  if (twopi/xnodp/xmnpda >= .15625)
	SetFlag(DEEP_SPACE_EPHEM_FLAG);
  else
	ClearFlag(DEEP_SPACE_EPHEM_FLAG);

  return;
} /* End of select_ephemeris() */

/*------------------------------------------------------------------*/

