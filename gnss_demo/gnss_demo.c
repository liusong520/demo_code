/*
 * ===================================================================================
 *
 *		Author:  liusong`s father;
 *
 * ===================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <pthread.h>
#include <gps.h>
#include <gpsdclient.h>


#define ERR_GPS_DEV_NOT_EXIST		(-2001)
#define ERR_GPS_OPEN_FAIL					(-2002)
#define ERR_GPS_READ_FAIL					(-2003)
#define ERR_GPS_WAITING_FAIL				(-2004)

#define GPS_NAVIGAT_DONE		1		//normal GPS
#define GPS_NAVIGATING				2		//gps is navigating for Latitude,Longitude,Altitude
#define AGPS_MODE						3

struct gps_location_data{
	double time;						/* Unix time in seconds with fractional part */
	double latitude;				/* Latitude in degrees (valid if mode >= 2) */
	double longitude;			/* Longitude in degrees (valid if mode >= 2) */
	double altitude;				/* Altitude in meters (valid if mode == 3) */
} ;

static pthread_mutex_t  gps_lock = PTHREAD_MUTEX_INITIALIZER;
static struct gps_data_t gpsdata={
	.gps_fd = -1,
};

static int verify_gps_data(struct gps_data_t data)
{
	if((data.fix.mode < MODE_2D) ||(data.status == 0))	//check gps status ,and mode
		return GPS_NAVIGATING;

	if (isnan(data.fix.latitude) || isnan(data.fix.longitude) || isnan(data.fix.altitude))	//check nan
		return GPS_NAVIGATING;

	return  GPS_NAVIGAT_DONE;
}

static int start_gps(void)
{
	int  ret=0, cnt=0;

	if(gpsdata.gps_fd != -1)
		return 0;

	while(cnt<5) {
		ret = gps_open("localhost", "2947", &gpsdata);
		if(!ret) {
			break;
		}
		cnt++;
		sleep(1);
	}

	if(cnt == 5) {
		printf("%s fail, error occurred opening gps, code: %d, reason: %s\n", __func__,ret, gps_errstr(ret));
		return ERR_GPS_OPEN_FAIL;
	}

	gps_stream(&gpsdata, WATCH_ENABLE|WATCH_JSON, NULL); 
	printf("%s success \n", __func__);

	return 0;
}

static int read_gps(struct gps_location_data  *location_data, int timeout)
{
	int ret = 0;

	memset(location_data, 0, sizeof(struct gps_location_data));//clear localtion data

	gps_waiting(&gpsdata, timeout);//gps socket wait

	ret = gps_read(&gpsdata);
	printf("gps_read data, status=%d,mode=%d,time=%f,latitude=%f,longitude=%f,altitude=%f\n",	\
		gpsdata.status, gpsdata.fix.mode, gpsdata.fix.time, gpsdata.fix.latitude, gpsdata.fix.longitude, gpsdata.fix.altitude);
	if(ret == -1) {
		printf("error occurred reading gps data, code: %d, reason: %s\n", ret, gps_errstr(ret));
		return ERR_GPS_READ_FAIL;
	}

	ret = verify_gps_data(gpsdata);
	if(ret==GPS_NAVIGAT_DONE) {
		location_data->time = gpsdata.fix.time;
		location_data->latitude = gpsdata.fix.latitude;
		location_data->longitude = gpsdata.fix.longitude;
		location_data->altitude =  gpsdata.fix.altitude;
	}

	return ret;
}

static void stop_gps(void)
{
	if(gpsdata.gps_fd != -1) {
		gps_stream(&gpsdata, WATCH_DISABLE, NULL); 
		gps_close(&gpsdata);
		gpsdata.gps_fd = -1;
	}
}


int Gps_start(int mode, char *attr)
{
	int ret = 0;

	pthread_mutex_lock(&gps_lock);
	ret = start_gps();
	pthread_mutex_unlock(&gps_lock);

	return ret;
}


int Gps_read(struct gps_location_data  *location_data, int timeout)
{
	int ret = 0;

	pthread_mutex_lock(&gps_lock);
	ret = read_gps(location_data, timeout);
	pthread_mutex_unlock(&gps_lock);

	return ret;
}


void Gps_stop(void)
{
	pthread_mutex_lock(&gps_lock);
	stop_gps();
	pthread_mutex_unlock(&gps_lock);

	return;
}


int main(void)
{
	int ret, i;
	struct gps_location_data	location_data;

	printf("gnss_demo\n");
	ret=Gps_start(0, NULL);
	if(ret) 
		return ret;

	for(i=0; i<10; i++) {
		ret=Gps_read(&location_data, 1000000);//waiting gps socket data,time out 1000000us ,1s
		printf("gps navigat  mode %s\n", ret==GPS_NAVIGAT_DONE ? "GPS_NAVIGAT_DONE" : "GPS_NAVIGATING");
		if(ret == GPS_NAVIGAT_DONE)
			printf("location_data latitude=%f, longitude=%f, altitude=%f\n", location_data.latitude, location_data.longitude, location_data.altitude);
	}

	Gps_stop();

	return 0;
}
