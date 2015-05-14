#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "gps.h"


static unsigned char *gpsDev = "/dev/gps"; 

int parseData(char **pointOne,char **pointTwo)
{
    //定位pointOne位置
    char temp;
    temp = *(*pointOne)++;
    while(temp != ',')
    {
        if(temp == '*' || temp == '$' || temp == '\n' || temp == '\0')
            return -1;
        temp = *(*pointOne)++;
    }
    //定位pointTwo位置
    *pointTwo = *pointOne;
    temp = *(*pointTwo)++;

    while(temp != ',' && temp != '*')
    {
        if(temp == '$' || temp == '\n' || temp == '\0')
            return -1;
        temp = *(*pointTwo)++;
    }
    //返回1表示已经读到语句结尾处
    if(temp == '*')
        return 1;
    return 0;
}

int getGPGGAData(GPGGA *gpGGA, char *sentence)
{
    char *pointOne = NULL;
    char *pointTwo = NULL;
    char tempBuf[14][20];
    int i = 0;
    int result = -1;
    if((pointOne = strstr(sentence,"$GPGGA")) == NULL)
        return ERROR_GET_GPGGA;

    pointOne++;
    for(i=0;i<14;i++)
    {
        memset(tempBuf[i],0x00,sizeof(tempBuf[i]));
        result = parseData(&pointOne,&pointTwo);
        if(result >= 0)
        {
            memcpy(tempBuf[i],pointOne,pointTwo-pointOne-1);
        }
        if(result == -1 || result == 1)
            break;
    }

    if(result >= 0)
    {
        strcpy(gpGGA->UTCTime,tempBuf[0]);
        gpGGA->LatitudePositionFieldNumber = atof(tempBuf[1]);
        gpGGA->Latitude = tempBuf[2][0];
        gpGGA->LongitudePositionFieldNumber = atof(tempBuf[3]);
        gpGGA->Longitude = tempBuf[4][0];
        gpGGA->GPSQuality = atoi(tempBuf[5]);
        gpGGA->NumberOfSatellitesInUse = atoi(tempBuf[6]);
        gpGGA->HorizontalDilutionOfPrecision = atof(tempBuf[7]);
        gpGGA->AntennaAltitudeMeters = atof(tempBuf[8]);
        gpGGA->GeoidalSeparationMeters = atof(tempBuf[10]);
        gpGGA->AgeOfDifferentialGPSDataSeconds = atof(tempBuf[12]);
        gpGGA->DifferentialReferenceStationID = atoi(tempBuf[13]);
        result = SUCCESS_GPS;
    }
    else
    	result = ERROR_GET_GPGGA;
    return result;
}


int getGPGSAData(GPGSA *gpGSA, char *sentence)
{
    char *pointOne = NULL;
    char *pointTwo = NULL;
    char tempBuf[17][20];
    int i = 0;
    int result = -1;
    if((pointOne = strstr(sentence,"$GPGSA")) == NULL)
    	return ERROR_GET_GPGSA;

    pointOne++;
    for(i=0;i<17;i++)
    {
        memset(tempBuf[i],0x00,sizeof(tempBuf[i]));
        result = parseData(&pointOne,&pointTwo);
        if(result >= 0)
            memcpy(tempBuf[i],pointOne,pointTwo-pointOne-1);
        if(result == -1 || result == 1)
            break;
    }

    if(result >= 0)
    {
        gpGSA->OperatingMode = tempBuf[0][0];
        gpGSA->FixMode = atoi(tempBuf[1]);
        int j;
        for(j=0;j<12;j++)
            gpGSA->SatelliteNumber[j] = atoi(tempBuf[j+2]);
        gpGSA->PDOP = atof(tempBuf[14]);
        gpGSA->HDOP = atof(tempBuf[15]);
        gpGSA->VDOP = atof(tempBuf[16]);
        result = SUCCESS_GPS;
    }
    else
    	result = ERROR_GET_GPGSA;
    return result;
}



int getGPGSVData(GPGSV gpGSV[], char *sentence)
{
    char *pointOne = NULL;
    char *pointTwo = NULL;
    char tempBuf[19][20];
    int i,j,k;
    int result = -1;
    int count = 0;


    pointOne = sentence;
    for(j=0;j<6;j++)
    {
        if((pointOne = strstr(pointOne,"$GPGSV")) == NULL)
        {
            if(j == 0)
                result = ERROR_GET_GPGSV;
            return result;
        }
        pointOne++;
        for(i=0;i<19;i++)
            memset(tempBuf[i],0x00,sizeof(tempBuf[i]));

        for(i=0;i<19;i++)
        {
            result = parseData(&pointOne,&pointTwo);
            if(result >= 0)
                memcpy(tempBuf[i],pointOne,pointTwo-pointOne-1);
            if(result == -1 || result == 1)
                break;
        }

        if(result >= 0)
        {
            gpGSV[j].TotalNumberOfMessages = atoi(tempBuf[0]);
            gpGSV[j].MessageNumber = atoi(tempBuf[1]);
            gpGSV[j].NumberOfSatellites = atoi(tempBuf[2]);

            count = (i-3)/4;
            if((i-3)%4 > 0)
                count++;
            for(k=0;k<count;k++)
            {
                gpGSV[j].SatellitesInView[k].SatelliteNumber = atoi(tempBuf[k*4+3]);
                gpGSV[j].SatellitesInView[k].Elevation = atoi(tempBuf[k*4+4]);
                gpGSV[j].SatellitesInView[k].AzimuthDegrees = atoi(tempBuf[k*4+5]);
                gpGSV[j].SatellitesInView[k].SNR = atoi(tempBuf[k*4+6]);
            }
            result = SUCCESS_GPS;
        }
        else
        	result = ERROR_GET_GPGSV;
    }

    return result;
}



int getGPGLLData(GPGLL *gpGLL, char *sentence)
{
    char *pointOne = NULL;
    char *pointTwo = NULL;
    char tempBuf[6][20];
    int i = 0;
    int result = -1;
    if((pointOne = strstr(sentence,"$GPGLL")) == NULL)
        return ERROR_GET_GPGLL;
    pointOne++;
    for(i=0;i<6;i++)
    {
        memset(tempBuf[i],0x00,sizeof(tempBuf[i]));
        result = parseData(&pointOne,&pointTwo);
        if(result >= 0)
            memcpy(tempBuf[i],pointOne,pointTwo-pointOne-1);

        if(result == -1 || result == 1)
            break;
    }

    if(result >= 0)
    {
        gpGLL->LatitudePositionFieldNumber = atof(tempBuf[0]);
        gpGLL->Latitude = tempBuf[1][0];
        gpGLL->LongitudePositionFieldNumber = atof(tempBuf[2]);
        gpGLL->Longitude = tempBuf[3][0];
        strcpy(gpGLL->UTCTime,tempBuf[4]);
        gpGLL->IsDataValid = tempBuf[5][0];
        result = SUCCESS_GPS;
    }
    else
    	result = ERROR_GET_GPGLL;
    return result;
}



int getGPRMCData(GPRMC *gpRMC, char *sentence)
{
    char *pointOne = NULL;
    char *pointTwo = NULL;
    char tempBuf[11][20];
    int i = 0;
    int result = -1;
    if((pointOne = strstr(sentence,"$GPRMC")) == NULL)
        return ERROR_GET_GPRMC;
    pointOne++;
    for(i=0;i<11;i++)
    {
        memset(tempBuf[i],0x00,sizeof(tempBuf[i]));
        result = parseData(&pointOne,&pointTwo);
        if(result >= 0)
            memcpy(tempBuf[i],pointOne,pointTwo-pointOne-1);
        if(result == -1 || result == 1)
            break;
    }

    if(result >= 0)
    {
        strcpy(gpRMC->UTCTime,tempBuf[0]);
        gpRMC->IsDataValid = tempBuf[1][0];
        gpRMC->LatitudePositionFieldNumber = atof(tempBuf[2]);
        gpRMC->Latitude = tempBuf[3][0];
        gpRMC->LongitudePositionFieldNumber = atof(tempBuf[4]);
        gpRMC->Longitude = tempBuf[5][0];
        gpRMC->SpeedOverGroundKnots = atof(tempBuf[6]);
        gpRMC->TrackMadeGoodDegreesTrue = atof(tempBuf[7]);
        strcpy(gpRMC->Date,tempBuf[8]);
        gpRMC->MagneticVariation = atof(tempBuf[9]);
        gpRMC->MagneticVariationDirection = tempBuf[10][0];
        result = SUCCESS_GPS;
    }
    else
    	result = ERROR_GET_GPRMC;
    return result;
}



int getGPVTGData(GPVTG *gpVTG, char *sentence)
{
    char *pointOne = NULL;
    char *pointTwo = NULL;
    char tempBuf[8][20];
    int i = 0;
    int result = -1;
    if((pointOne = strstr(sentence,"$GPVTG")) == NULL)
        return ERROR_GET_GPVTG;
    pointOne++;
    for(i=0;i<8;i++)
    {
        memset(tempBuf[i],0x00,sizeof(tempBuf[i]));
        result = parseData(&pointOne,&pointTwo);
        if(result >= 0)
            memcpy(tempBuf[i],pointOne,pointTwo-pointOne-1);
        if(result == -1 || result == 1)
            break;
    }

    if(result >= 0)
    {
        gpVTG->TrackDegreesTrue = atof(tempBuf[0]);
        gpVTG->TrackDegreesMagnetic = atof(tempBuf[2]);
        gpVTG->SpeedKnots = atof(tempBuf[4]);
        gpVTG->SpeedKilometersPerHour = atof(tempBuf[6]);
        result = SUCCESS_GPS;
    }
    else
    	result = ERROR_GET_GPVTG;
    return result;
}



int getGPZDAData(GPZDA *gpZDA, char *sentence)
{
    char *pointOne = NULL;
    char *pointTwo = NULL;
    char tempBuf[6][20];
    int i = 0;
    int result = -1;
    if((pointOne = strstr(sentence,"$GPZDA")) == NULL)
        return ERROR_GET_GPZDA;
    pointOne++;
    for(i=0;i<6;i++)
    {
        memset(tempBuf[i],0x00,sizeof(tempBuf[i]));
        result = parseData(&pointOne,&pointTwo);
        if(result >= 0)
            memcpy(tempBuf[i],pointOne,pointTwo-pointOne-1);
        if(result == -1 || result == 1)
            break;
    }

    if(result >= 0)
    {
        strcpy(gpZDA->UTCTime,tempBuf[0]);
        gpZDA->Day = atoi(tempBuf[1]);
        gpZDA->Month = atoi(tempBuf[2]);
        gpZDA->Year = atoi(tempBuf[3]);
        gpZDA->LocalHourDeviation = atoi(tempBuf[4]);
        gpZDA->LocalMinutesDeviation = atoi(tempBuf[5]);
        result = SUCCESS_GPS;
    }
    else
    	result = ERROR_GET_GPZDA;
    return result;
}


int readDev(unsigned char *sentence,int length)
{

	int fd, ret;
	fd = open(gpsDev,O_RDWR|O_NOCTTY);

	if(fd < 0)
		return ERROR_OPEN_GPSDEV;
	ret = read(fd,sentence,length);	
	close(fd);
	if(ret <= 0)
		ret = ERROR_READ_GPSDEV;
	return ret;
}

int getGPSData(char *cmd, void *gpsStruct)
{
    int result = -1;
	unsigned char sentence[2048];
    memset(sentence,0,sizeof(sentence));
    result = readDev(sentence,sizeof(sentence));
	if(result < 0)
		return result;

	if(strstr(cmd,"GPGGA"))
        result = getGPGGAData((GPGGA*)gpsStruct,sentence);
    else if(strstr(cmd,"GPGSA"))
        result = getGPGSAData((GPGSA*)gpsStruct,sentence);
    else if(strstr(cmd,"GPGSV"))
        result = getGPGSVData((GPGSV*)gpsStruct,sentence);
    else if(strstr(cmd,"GPGLL"))
        result = getGPGLLData((GPGLL*)gpsStruct,sentence);
    else if(strstr(cmd,"GPRMC"))
        result = getGPRMCData((GPRMC*)gpsStruct,sentence);
    else if(strstr(cmd,"GPVTG"))
        result = getGPVTGData((GPVTG*)gpsStruct,sentence);
    else if(strstr(cmd,"GPZDA"))
        result = getGPZDAData((GPZDA*)gpsStruct,sentence);
    return result;
}
