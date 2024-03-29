#include "RCSwitch.h"
#include "RcOok.h"
#include "Sensor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
int loggingok;	

int main(int argc, char *argv[])
{
int RXPIN = 2;
int TXPIN = -1;

time_t ltime;
struct tm * curtime;
FILE *fp;	 // Global var file handle 
char buffer[80];

if(argc==2) {
	loggingok=1;
	} 
	else {
	  loggingok=0;
	}

if(wiringPiSetup() == -1)
		{
		return 0;
		printf("failed wiring pi\n");
		}
RCSwitch *rc = new RCSwitch(RXPIN,TXPIN);

printf("datetime,temperature,humidity,channel\n");
while (1)
	{
	if ( rc->OokAvailable() ) // une donnee disponible
		{
		char message[100];
		time( &ltime ); //recupere le tps en seconde depuis 1970
		curtime = localtime( &ltime ); //rempli la structure tm
		strftime(buffer,80,"%F %T", curtime);// formate ds buffer  la date yyyy-MM-DD HH:MM:SEC
		rc->getOokCode(message); //message ok
		printf(" %s\n ",message); 

		Sensor *s = Sensor::getRightSensor(message);
		if ( s != NULL )
			{
			printf("%s,%f,%f,%d\n",	buffer, s->getTemperature(), s->getHumidity(), s->getChannel() );
			if(loggingok) {
				fp = fopen(argv[1], "a");
	    	 		if (fp == NULL) {
		      				perror("Failed to open log file!"); // Exit if file open fails
	      					exit(EXIT_FAILURE);
						}
				fprintf(fp," {\"datetime\": \"%s\", \"temperature\": \"%f\", \"humidity\": \"%f\", \"channel\": \"%d\" } \n",
							buffer,	s->getTemperature(), s->getHumidity(), s->getChannel());
				fflush(fp);
				fclose(fp);
  					}

			}
			delete s;
		}
		delay(1000);
 	}

}

