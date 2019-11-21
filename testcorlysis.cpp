/*
compilation par g++ g++ RCSwitch.o RcOok.cpp Sensor.o testcorlysis.cpp -lwiringPi -o newtestcorlysis
*/

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

time_t ltime, time15mn; //tps en secondes de puis 1970
struct tm * curtime;
FILE *fp, *fp1, *fp2, *fp3;	 // Global var file handle 
char buffer[80], bufdate[80];// buffer date en chaine
char cmd1[255] =  "curl -i -XPOST 'https://corlysis.com:8086/write?db=corbeilles45' -u token:8d380346e550f60163bad83c72c4935a --data-binary \"temperature,place=serre value=";
char cmd2[255] =  "curl -i -XPOST 'https://corlysis.com:8086/write?db=corbeilles45' -u token:8d380346e550f60163bad83c72c4935a --data-binary \"temperature,place=escalier value=";
char cmd3[255] =  "curl -i -XPOST 'https://corlysis.com:8086/write?db=corbeilles45' -u token:8d380346e550f60163bad83c72c4935a --data-binary \"temperature,place=soussol value=";
char cmd[255];
char tempchar1[10], tempchar2[10], tempchar3[10];
 
Sensor  *tabs1[100], *tabs2[100], *tabs3[100]; // tableau des records sur 15mn
int i, i1=0, i2=0, i3=0; // nb de record
struct tm t1_deb, t2_deb, t3_deb; 
float tempi1 = 0, tempi2 = 0, tempi3 = 0;
float humii1 = 0, humii2 = 0, humii3 = 0;

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
	printf(" trame recue %s ",message); 
		Sensor *s = Sensor::getRightSensor(message); // cree un oregonsensor derive de sensor
		if ( s != NULL )
			{
			printf("%s,%f,%f,%d\n",	buffer, s->getTemperature(), s->getHumidity(), s->getChannel() );
			switch (s->getChannel())
				{
				case 1 : if (i1==0){//premiere data
						t1_deb = *curtime; //reference de temps 0mn a 14 15a 29 30 a 44 45 a 59 
						t1_deb.tm_sec = 0; 
						if (t1_deb.tm_min < 15)  t1_deb.tm_min = 0; //de 00:00 a 14:59
							else if (t1_deb.tm_min < 30) t1_deb.tm_min = 15;
								else if (t1_deb.tm_min <45) t1_deb.tm_min = 30;
									else t1_deb.tm_min = 45;
						tabs1[i1] = s;
						i1++;
						strftime(bufdate,80,"%F %T", curtime);// formate ds buffer  la date yyyy-MM-DD HH:MM:SEC
						printf("debut tps 1er record %d %s\n",mktime(&t1_deb), bufdate );
						fflush(stdout);
						}
					else {
					     if ( difftime(ltime, mktime(&t1_deb)) < (60*15) ) //pendant 15mn 
							{
							tabs1[i1] = s;
							i1++;
							} 
						else {  
								strftime(bufdate,80,"%F %T", curtime);// formate ds buffer  la date yyyy-MM-DD HH:MM:SEC
								printf("fin nbre record :%d tps (record +1) %d %s\n",i1,ltime,bufdate ); // on a depasse les 15mn
								fflush(stdout);
								tempi1 = 0.0;
								humii1 = 0.0;
 								for (i=0;i<i1;i++)
									{printf("%f\n",tabs1[i]->getTemperature() );
								//calcul temp
									tempi1 = tempi1 + tabs1[i]->getTemperature();
									humii1 = humii1 + tabs1[i]->getHumidity();
									}
								tempi1 = tempi1/i1;
								humii1 = humii1/i1;
								// ecrit fichier
								fp1 = fopen("/root/RPI_Oregan-master/newjsonI1.txt","a");
								if (fp1 == NULL) {perror("newjson pb"); 
										  exit(1);
										}
								//calcul date fin intervalle 
								time15mn =   mktime(&t1_deb) ;
								time15mn = time15mn  + (60*15) ;
								strftime(bufdate,80,"%F %T",  localtime(&time15mn) );
					fprintf(fp1," {\"datetime\": \"%s\", \"temperature\": \"%.2f\", \"humidity\": \"%.2f\" } \n",
								bufdate, tempi1, humii1 );
			//	system("curl -i -XPOST 'https://corlysis.com:8086/write?db=corbeilles45' -u token:8d380346e550f60163bad83c72c4935a --data-binary \"temperature,place=serre value=20.64\"");
					strcpy(cmd,cmd1);
					sprintf(tempchar1,"%.2f",tempi1);
					strcat( cmd,   tempchar1 );
					strcat( cmd,  "\"" );
					printf("cmd corlysis :%s \n" , cmd);
					system(cmd);
					strcpy(cmd,"");
					printf("on ecrit  {\"datetime\": \"%s\", \"temperature\": \"%.2f\", \"humidity\": \"%.2f\" } \n",
								bufdate, tempi1, humii1 );
								fflush(fp1);
								fclose(fp1);
								// libere tableau
 								for (int i=0;i<i1-1;i++)
									{ printf("delete du %d iemeelt temp=%f:%f\n",i,tabs1[i]->getTemperature(), tabs1[i]->getHumidity() );
										fflush(stdout);
									  delete tabs1[i]; 
										}
								// prepare nouvel intervalle
								//i1 = 0;
								tabs1[0] = tabs1[i1-1]; //tabs1[i] = s; //recupere le dernier envoi
								i1=1;
							t1_deb = *curtime; //reference de temps 0mn a 14 15a 29 30 a 44 45 a 59 
							t1_deb.tm_sec = 0; 
							if (t1_deb.tm_min < 15) t1_deb.tm_min = 0; //de 00:00 a 14:59
							else if (t1_deb.tm_min < 30) t1_deb.tm_min = 15;
								else if (t1_deb.tm_min <45) t1_deb.tm_min = 30;
									else t1_deb.tm_min = 45;
							strftime(bufdate,80,"%F %T", &t1_deb);//  yyyy-MM-DD HH:MM:SEC
							printf("debut tps2 %d, %s\n",mktime(&t1_deb),bufdate );
								tempi1 = 0;
								humii1 = 0;
								}
						}
					break;		
				case 2 :if (i2==0){//premiere data
						t2_deb = *curtime; //reference de temps 0mn a 14 15a 29 30 a 44 45 a 59 
						t2_deb.tm_sec = 0; 
						if (t2_deb.tm_min < 15)  t2_deb.tm_min = 0; //de 00:00 a 14:59
							else if (t2_deb.tm_min < 30) t2_deb.tm_min = 15;
								else if (t2_deb.tm_min <45) t2_deb.tm_min = 30;
									else t2_deb.tm_min = 45;
						tabs2[i2] = s;
						i2++;
						strftime(bufdate,80,"%F %T", curtime);// formate ds buffer  la date yyyy-MM-DD HH:MM:SEC
						printf("debut tps 1er record capteur2 %d %s\n",mktime(&t1_deb), bufdate );
						fflush(stdout);
						}
					else {
					     if ( difftime(ltime, mktime(&t2_deb)) < (60*15) ) //pendant 15mn 
							{
							tabs2[i2] = s;
							i2++;
							} 
						else {  
								strftime(bufdate,80,"%F %T", curtime);// formate ds buffer  la date yyyy-MM-DD HH:MM:SEC
								printf("fin nbre record capteur2 :%d tps (record +1) %d %s\n",i1,ltime,bufdate ); // on a depasse les 15mn
								fflush(stdout);
								tempi2 = 0.0;
								humii2 = 0.0;
 								for (int i=0;i<i2;i++)
									{printf("%f\n",tabs2[i]->getTemperature() );
								//calcul temp
									tempi2 = tempi2 + tabs2[i]->getTemperature();
									humii2 = humii2 + tabs2[i]->getHumidity();
									}
								tempi2 = tempi2/i2;
								humii2 = humii2/i2;
								// ecrit fichier
								fp2 = fopen("/root/RPI_Oregan-master/newjsonI2.txt","a");
								if (fp2 == NULL) {perror("newjson pb"); 
										  exit(1);
										}
								//calcul date fin intervalle 
		// arevoir	strftime(bufdate,80,"%F %T", curtime);// formate ds buffer  la date yyyy-MM-DD HH:MM:SEC
								time15mn =   mktime(&t2_deb) ;
								time15mn = time15mn  + (60*15) ;
								strftime(bufdate,80,"%F %T",  localtime(&time15mn) );
					fprintf(fp2," {\"datetime\": \"%s\", \"temperature\": \"%.2f\", \"humidity\": \"%.2f\" } \n",
								bufdate, tempi2, humii2 );
			//	system("curl -i -XPOST 'https://corlysis.com:8086/write?db=corbeilles45' -u token:8d380346e550f60163bad83c72c4935a --data-binary \"temperature,place=escalier value=17.64\"");

					strcpy(cmd,cmd2);
					sprintf(tempchar2,"%.2f",tempi2);
					strcat( cmd,   tempchar2 );
					strcat( cmd,  "\"" );
					system(cmd);
					printf("cmd corlysis : %s \n", cmd);
					strcpy(cmd,"");

					printf("on ecrit  {\"datetime\": \"%s\", \"temperature\": \"%.2f\", \"humidity\": \"%.2f\" } \n",
								bufdate, tempi2, humii2 );
								fflush(fp2);
								fclose(fp2);
								// libere tableau
 								for (int i=0;i<i2-1;i++)
									{ printf("delete du %d iemeelt temp=%f:%f\n",i,tabs2[i]->getTemperature(), tabs2[i]->getHumidity() );
										fflush(stdout);
									  delete tabs2[i]; 
										}
								// prepare nouvel intervalle
								//i1 = 0;
								tabs2[0] = tabs2[i2-1]; //tabs1[i] = s; //recupere le dernier envoi
								i2=1;
							t2_deb = *curtime; //reference de temps 0mn a 14 15a 29 30 a 44 45 a 59 
							t2_deb.tm_sec = 0; 
							if (t2_deb.tm_min < 15) t2_deb.tm_min = 0; //de 00:00 a 14:59
							else if (t2_deb.tm_min < 30) t2_deb.tm_min = 15;
								else if (t2_deb.tm_min <45) t2_deb.tm_min = 30;
									else t2_deb.tm_min = 45;
							strftime(bufdate,80,"%F %T", &t2_deb);//  yyyy-MM-DD HH:MM:SEC
							printf("debut tps2 capt2 %d, %s\n",mktime(&t2_deb),bufdate );
								tempi2 = 0;
								humii2 = 0;
								}
						}
					break;	
				case 3 :if (i3==0){//premiere data
						t3_deb = *curtime; //reference de temps 0mn a 14 15a 29 30 a 44 45 a 59 
						t3_deb.tm_sec = 0; 
						if (t3_deb.tm_min < 15)  t3_deb.tm_min = 0; //de 00:00 a 14:59
							else if (t3_deb.tm_min < 30) t3_deb.tm_min = 15;
								else if (t3_deb.tm_min <45) t3_deb.tm_min = 30;
									else t3_deb.tm_min = 45;
						tabs3[i3] = s;
						i3++;
						strftime(bufdate,80,"%F %T", curtime);// formate ds buffer  la date yyyy-MM-DD HH:MM:SEC
						printf("debut tps 1er record capteur3 %d %s\n",mktime(&t3_deb), bufdate );
						fflush(stdout);
						}
					else {
					     if ( difftime(ltime, mktime(&t3_deb)) < (60*15) ) //pendant 15mn 
							{
							tabs3[i3] = s;
							i3++;
							} 
						else {  
								strftime(bufdate,80,"%F %T", curtime);// formate ds buffer  la date yyyy-MM-DD HH:MM:SEC
								printf("fin nbre record capteur3 :%d tps (record +1) %d %s\n",i1,ltime,bufdate ); // on a depasse les 15mn
								fflush(stdout);
								tempi3 = 0.0;
								humii3 = 0.0;
 								for (int i=0;i<i3;i++)
									{printf("%f\n",tabs3[i]->getTemperature() );
								//calcul temp
									tempi3 = tempi3 + tabs3[i]->getTemperature();
									humii3 = humii3 + tabs3[i]->getHumidity();
									}
								tempi3 = tempi3/i3;
								humii3 = humii3/i3;
								// ecrit fichier
								fp3 = fopen("/root/RPI_Oregan-master/newjsonI3.txt","a");
								if (fp3 == NULL) {perror("newjson pb capt3"); 
										  exit(1);
										}
								//calcul date fin intervalle 
		// arevoir	strftime(bufdate,80,"%F %T", curtime);// formate ds buffer  la date yyyy-MM-DD HH:MM:SEC
								time15mn =   mktime(&t3_deb) ;
								time15mn = time15mn  + (60*15) ;
								strftime(bufdate,80,"%F %T",  localtime(&time15mn) );
					fprintf(fp3," {\"datetime\": \"%s\", \"temperature\": \"%.2f\", \"humidity\": \"%.2f\" } \n",
								bufdate, tempi3, humii3 );
//				system("curl -i -XPOST 'https://corlysis.com:8086/write?db=corbeilles45' -u token:8d380346e550f60163bad83c72c4935a --data-binary \"temperature,place=soussol value=18.64\"");
					strcpy(cmd,cmd3);
					sprintf(tempchar3,"%.2f",tempi3);
					strcat( cmd,   tempchar3 );
					strcat( cmd,  "\"" );
					system(cmd);
					printf("cmd corlysis : %s \n", cmd);
					strcpy(cmd,"");
					printf("on ecrit  {\"datetime\": \"%s\", \"temperature\": \"%.2f\", \"humidity\": \"%.2f\" } \n",
								bufdate, tempi3, humii3 );
								fflush(fp3);
								fclose(fp3);
								// libere tableau
 								for (int i=0;i<i3-1;i++)
									{ printf("delete du %d iemeelt temp=%f:%f\n",i,tabs3[i]->getTemperature(), tabs3[i]->getHumidity() );
										fflush(stdout);
									  delete tabs3[i]; 
										}
								// prepare nouvel intervalle
								//i1 = 0;
								tabs3[0] = tabs3[i3-1]; //tabs1[i] = s; //recupere le dernier envoi
								i3=1;
							t3_deb = *curtime; //reference de temps 0mn a 14 15a 29 30 a 44 45 a 59 
							t3_deb.tm_sec = 0; 
							if (t3_deb.tm_min < 15) t3_deb.tm_min = 0; //de 00:00 a 14:59
							else if (t3_deb.tm_min < 30) t3_deb.tm_min = 15;
								else if (t3_deb.tm_min <45) t3_deb.tm_min = 30;
									else t3_deb.tm_min = 45;
							strftime(bufdate,80,"%F %T", &t3_deb);//  yyyy-MM-DD HH:MM:SEC
							printf("debut tps2 capt3 %d, %s\n",mktime(&t3_deb),bufdate );
								tempi3 = 0;
								humii3 = 0;
								}
						}
					break;
				default : perror("channel non trouve\n");
				}
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
//			delete s;
		}
		delay(1000);
 	}

}

