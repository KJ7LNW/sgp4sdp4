/*
 *  main.c          April 10  2001
 *
 *  A skeleton main() function to demonstrate the use of
 *  the SGP4() and SDP4() routines in the sgp4sdp4.c file.
 *  The TLE set to be used is read from the file amateur.txt
 *  downloaded from Dr Kelso's website at www.celestrak.com.
 *  Please note that this is a simple application and can only
 *  read the first TLE kep set, including line 0 with the Name.
 */

#include "sgp4sdp4.h"

/* Main program */
  int
main(void)
{
  /* TLE source file */
  char tle_file[] = "./amateur.txt";

  /* Observer's geodetic co-ordinates.      */
  /* Lat North, Lon East in rads, Alt in km */
  geodetic_t obs_geodetic = {0.6056, 0.5761, 0.15, 0.0};

  /* Two-line Orbital Elements for the satellite */
  tle_t tle ;

  /* Zero vector for initializations */
  vector_t zero_vector = {0,0,0,0};

  /* Satellite position and velocity vectors */
  vector_t vel = zero_vector;
  vector_t pos = zero_vector;
  /* Satellite Az, El, Range, Range rate */
  vector_t obs_set;

  /* Solar ECI position vector  */
  vector_t solar_vector = zero_vector;
  /* Solar observed azi and ele vector  */
  vector_t solar_set;

  /* Calendar date and time (UTC) */
  struct tm utc;

  /* Satellite's predicted geodetic position */
  geodetic_t sat_geodetic;

  double
	tsince,            /* Time since epoch (in minutes) */
	jul_epoch,         /* Julian date of epoch          */
	jul_utc,           /* Julian UTC date               */
	eclipse_depth = 0, /* Depth of satellite eclipse    */
	/* Satellite's observed position, range, range rate */
	sat_azi, sat_ele, sat_range, sat_range_rate,
	/* Satellites geodetic position and velocity */
	sat_lat, sat_lon, sat_alt, sat_vel,
	/* Solar azimuth and elevation */
	sun_azi, sun_ele;

  /* Used for storing function return codes */
  int flg;

  char
	ephem[5],       /* Ephemeris in use string  */
	sat_status[12]; /* Satellite eclipse status */


  /* Input one (first!) TLE set from file */
  flg = Input_Tle_Set(tle_file, &tle);

  /* Abort if file open fails */
  if( flg == -1 )
  {
	printf(" File open failed - Exiting!\n");
	exit(-1);
  }

  /* Print satellite name and TLE read status */
  printf(" %s: ", tle.sat_name);
  if( flg == -2 )
  {
	printf("TLE set bad - Exiting!\n");
	exit(-2);
  }
  else
	printf("TLE set good - Happy Tracking!\n");

  /* Printout of tle set data for tests if needed */
  /*  printf("\n %s %s %i  %i  %i\n"
	  " %14f %10f %8f %8f\n"
	  " %8f %8f %9f %8f %8f %12f\n",
	  tle.sat_name, tle.idesg, tle.catnr, tle.elset, tle.revnum,
	  tle.epoch, tle.xndt2o, tle.xndd6o, tle.bstar,
	  tle.xincl, tle.xnodeo, tle.eo, tle.omegao, tle.xmo, tle.xno);
   */

  /** !Clear all flags! **/
  /* Before calling a different ephemeris  */
  /* or changing the TLE set, flow control */
  /* flags must be cleared in main().      */
  ClearFlag(ALL_FLAGS);

  /** Select ephemeris type **/
  /* Will set or clear the DEEP_SPACE_EPHEM_FLAG       */
  /* depending on the TLE parameters of the satellite. */
  /* It will also pre-process tle members for the      */
  /* ephemeris functions SGP4 or SDP4 so this function */
  /* must be called each time a new tle set is used    */
  select_ephemeris(&tle);

  do  /* Loop */
  {
	/* Get UTC calendar and convert to Julian */
	UTC_Calendar_Now(&utc);
	jul_utc = Julian_Date(&utc);

	/* Convert satellite's epoch time to Julian  */
	/* and calculate time since epoch in minutes */
	jul_epoch = Julian_Date_of_Epoch(tle.epoch);
	tsince = (jul_utc - jul_epoch) * xmnpda;

	/* Copy the ephemeris type in use to ephem string */
	if( isFlagSet(DEEP_SPACE_EPHEM_FLAG) )
	  strcpy(ephem,"SDP4");
	else
	  strcpy(ephem,"SGP4");

	/* Call NORAD routines according to deep-space flag */
	if( isFlagSet(DEEP_SPACE_EPHEM_FLAG) )
	  SDP4(tsince, &tle, &pos, &vel);
	else
	  SGP4(tsince, &tle, &pos, &vel);

	/* Scale position and velocity vectors to km and km/sec */
	Convert_Sat_State( &pos, &vel );

	/* Calculate velocity of satellite */
	Magnitude( &vel );
	sat_vel = vel.w;

	/** All angles in rads. Distance in km. Velocity in km/s **/
	/* Calculate satellite Azi, Ele, Range and Range-rate */
	Calculate_Obs(jul_utc, &pos, &vel, &obs_geodetic, &obs_set);

	/* Calculate satellite Lat North, Lon East and Alt. */
	Calculate_LatLonAlt(jul_utc, &pos, &sat_geodetic);

	/* Calculate solar position and satellite eclipse depth */
	/* Also set or clear the satellite eclipsed flag accordingly */
	Calculate_Solar_Position(jul_utc, &solar_vector);
	Calculate_Obs(jul_utc,&solar_vector,&zero_vector,&obs_geodetic,&solar_set);

	if( Sat_Eclipsed(&pos, &solar_vector, &eclipse_depth) )
	  SetFlag( SAT_ECLIPSED_FLAG );
	else
	  ClearFlag( SAT_ECLIPSED_FLAG );

	/* Copy a satellite eclipse status string in sat_status */
	if( isFlagSet( SAT_ECLIPSED_FLAG ) )
	  strcpy( sat_status, "Eclipsed" );
	else
	  strcpy( sat_status, "In Sunlight" );

	/* Convert and print satellite and solar data */
	sat_azi = Degrees(obs_set.x);
	sat_ele = Degrees(obs_set.y);
	sat_range = obs_set.z;
	sat_range_rate = obs_set.w;
	sat_lat = Degrees(sat_geodetic.lat);
	sat_lon = Degrees(sat_geodetic.lon);
	sat_alt = sat_geodetic.alt;

	sun_azi = Degrees(solar_set.x);
	sun_ele = Degrees(solar_set.y);

	printf("\n Date: %02d/%02d/%04d UTC: %02d:%02d:%02d  Ephemeris: %s"
		"\n Azi=%6.1f Ele=%6.1f Range=%8.1f Range Rate=%6.2f"
		"\n Lat=%6.1f Lon=%6.1f  Alt=%8.1f  Vel=%8.3f"
		"\n Stellite Status: %s - Depth: %2.3f"
		"\n Sun Azi=%6.1f Sun Ele=%6.1f\n",
		utc.tm_mday, utc.tm_mon, utc.tm_year,
		utc.tm_hour, utc.tm_min, utc.tm_sec, ephem,
		sat_azi, sat_ele, sat_range, sat_range_rate,
		sat_lat, sat_lon, sat_alt, sat_vel,
		sat_status, eclipse_depth,
		sun_azi, sun_ele);

	sleep(1);
  }  /* End of do */
  while( 1 );

} /* End of main() */

/*------------------------------------------------------------------*/

