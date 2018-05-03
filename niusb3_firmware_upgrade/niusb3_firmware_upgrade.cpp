// niusb3_firmware_upgrade.cpp : definisce il punto di ingresso dell'applicazione console.
//


#include "stdafx.h"
#include "../niusb3_core/niusb3_core.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <time.h>
#include <conio.h>
#include <windows.h>


#define BOOTLOADER_BA 0xFFFE0000



using namespace std;

char * readTextFile(char *filename);
char * readBinFile(char *filename, uint32_t *bytes);

vector<string> split(const char *str, char c = ' ')
{
    vector<string> result;

    do
    {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(string(begin, str));
    } while (0 != *str++);

    return result;
}

bool CheckBooloader(bool *bootloader, uint32_t *version,uint32_t *release, 	NI_HANDLE *handle)
{
	uint32_t datarow[8198];
	uint32_t rw;
	uint32_t vd;

	if(NI_USB3_WriteReg(0xFF,BOOTLOADER_BA+8191,handle) != 0) return false;
	if(NI_USB3_ReadData(datarow, 8192, BOOTLOADER_BA, REG_ACCESS, 1000, handle, &rw, &vd)!= 0) return false;
	if (datarow[0] == 0xB1FFAA1B)
	{
		*bootloader = true;
		*version = datarow[1];
		*release = datarow[2];
	}
	else
		*bootloader = false;
	return true;
}

bool Readfirmware(char *fileName, uint32_t length, 	NI_HANDLE *handle)
{
	int i;
	uint32_t temp =0; 
	uint32_t rw;
	uint32_t vd;
	FILE *f = fopen(fileName, "wb");
	if (f==NULL)
		return false;
	printf("Reading back firmware \n");
	
	uint32_t datarow[8198];

	memset(datarow,0,8192);
	for (i=0;i<length;i+=60*512)
	{
		datarow[0] = i;
		datarow[1] = 0x1A;
		NI_USB3_WriteData(datarow, 2, BOOTLOADER_BA+8190, REG_ACCESS, 1000, handle, &rw);
		do
		{
		//	NI_USB3_ReadReg(&temp,0x80000000+8189,handle);
			NI_USB3_ReadData(datarow, 8192, BOOTLOADER_BA, REG_ACCESS, 1000, handle, &rw, &vd);
		}while(datarow[8189]!=1);
	NI_USB3_ReadData(datarow, 8192, BOOTLOADER_BA, REG_ACCESS, 1000, handle, &rw, &vd);
		
		Sleep(50);
		fwrite(datarow,1,60*512,f);
		printf("Progress: %3.1f\r", ((double)i*100.0)/length);
	}
	printf("Read back firmware completed\n");
	fclose(f);
	return true;
}
bool WriteFirmware(char *fileName, 	NI_HANDLE *handle)
{
	uint32_t datarow[8400];
	uint32_t dataread[8400];
	uint32_t temp =0; 
	uint32_t rw;
	uint32_t vd;
	uint32_t bytes;
	uint32_t q;
	char *firmware;
	int i;
	firmware = readBinFile (fileName, &bytes);
	if (bytes>0)
	{
		printf("Firmware: %s, loaded: %d bytes\n",fileName,bytes);
	}
	else
	{
		printf("Unable to load Firmware: %s\n",fileName);
		return false;
	}

	int textLen = bytes;
	int remaining;
	int trfd=0;

	printf("Erasing firmware flash\n");
	NI_USB3_WriteReg(textLen,BOOTLOADER_BA+8190,handle);
	NI_USB3_WriteReg(0x1,BOOTLOADER_BA+8191,handle);	
	Sleep(10);
	
	do
	{
		if (NI_USB3_ReadReg(&temp,BOOTLOADER_BA+8189,handle)!=0) temp=0;
		//NI_USB3_ReadData(datarow, 8192, 0x80000000, REG_ACCESS, 1000, handle, &rw, &vd);
	}while(temp!=1);
	printf("Firmware flash erased\n");

	printf("Starting Program\n");

	remaining = textLen;
	q=0;
	do
	{
		int trfslen = remaining < 512*60 ? remaining : 512*60;
		memset(datarow,0, 8192*sizeof(uint32_t));
		memcpy(datarow,firmware + trfd,trfslen);
		trfd = trfd + trfslen;
		remaining = remaining - trfslen;

		datarow[8190] = q*512*60;
		datarow[8191] = 0xA;
		NI_USB3_WriteData(datarow, 8192, BOOTLOADER_BA, REG_ACCESS, 1000, handle, &rw);
		
		do
		{
			if (NI_USB3_ReadReg(&temp,BOOTLOADER_BA+8189,handle)!=0) temp=0;
			//NI_USB3_ReadData(datarow, 8192, 0x80000000, REG_ACCESS, 1000, handle, &rw, &vd);
		}while((temp!=1) && (temp!=2)) ;
		//while((datarow[8189]!=1) && (datarow[8189]!=2)) ;

		//if (datarow[8189]==2)
		if (temp==2)
		{
			printf("Write operation failed\n");
			return false;
		}
		printf("Progress: %3.1f\r", ((double)trfd*100.0)/textLen);
		fflush(stdout);
		q++;
	} while (remaining>0);

	printf("Firmware uploaded correcly\n");
	return true;
}

bool WriteFirmwareInformation(char *fileName, 	NI_HANDLE *handle)
{
	uint32_t datarow[8192];
	uint32_t dataread[8192];
	uint32_t temp =0; 
	uint32_t rw;
	uint32_t vd;
	uint32_t bytes;
	uint32_t q;
	
	int i;

	
	char *JSONDATA = readTextFile("RegisterFile.json");
	if (JSONDATA)
	{
		int textLen = strlen(JSONDATA);
		int remaining;
		int trfd=0;
		printf("Write Firmware JSON data\n");
		

		printf("Erasing firmware information flash\n");
		NI_USB3_WriteReg(textLen,BOOTLOADER_BA+8190,handle);
		NI_USB3_WriteReg(0x2,BOOTLOADER_BA+8191,handle);	
		Sleep(10);
		uint32_t temp =0; 
		do
		{
			NI_USB3_ReadReg(&temp,BOOTLOADER_BA+8189,handle);
		}while(temp!=1);
		printf("Firmware flash information erased\n");

		printf("Starting Program information\n");
		remaining = textLen;
		q=0;
		do
		{
			int trfslen = remaining < 512*60 ? remaining : 512*60;
			memset(datarow,0, 8192*sizeof(uint32_t));
			memcpy(datarow,JSONDATA + trfd,trfslen);
			trfd = trfd + trfslen;
			remaining = remaining - trfslen;

			datarow[8190] = q*512*60;
			datarow[8191] = 0xB;
			NI_USB3_WriteData(datarow, 8192, BOOTLOADER_BA, REG_ACCESS, 1000, handle, &rw);
			do
			{
				NI_USB3_ReadReg(&temp,BOOTLOADER_BA+8189,handle);
			}while(temp!=1);
			q++;
		} while (remaining>0);
		printf("Program information completed\n");
	}

	return true;
}

char *ReadFirmwareInformation(	NI_HANDLE *handle)
{
		uint32_t datarow[8192];
	uint32_t dataread[8192];
	uint32_t temp =0; 
	uint32_t rw;
	uint32_t vd;
	uint32_t bytes;
	uint32_t q;
	
	int i;
	char *TEXT = (char*) malloc(0x40040 * sizeof(uint32_t));

	printf("Reading firmware information\n");
	memset(TEXT,0,0x40000);
	for (i=0;i<0x40000;i+=60*512)
	{
		datarow[0] = i;
		datarow[1] = 0x1B;
		NI_USB3_WriteData(datarow, 2, BOOTLOADER_BA+8190, REG_ACCESS, 1000, handle, &rw);
		do
		{
			NI_USB3_ReadReg(&temp,BOOTLOADER_BA+8189,handle);
		}while(temp!=1);
		NI_USB3_ReadData((uint32_t*)(TEXT+i), 15*512, BOOTLOADER_BA, REG_ACCESS, 1000, handle, &rw, &vd);
	}

	return TEXT;

}





int _tmain(int argc, _TCHAR* argv[])
{
	char listDev[2048];
	int Count;
	NI_HANDLE handle;

	NI_USB3_Init();
	NI_USB3_ListDevices(listDev, NULL,  &Count);
	
	vector<string> vect = split(listDev, ';');

	NI_USB3_ConnectDevice((char *) vect[0].c_str(), &handle);


	


	bool isBootloader=false;
	uint32_t BL_version;
	uint32_t BL_release;
	CheckBooloader(&isBootloader, &BL_version, &BL_release, &handle);
	if (!isBootloader)
	{
		printf("The board is not running in bootloader mode\n");

		printf("Reading firmware information from app\n");
		char *FirmwareInfo = ReadFirmwareInformationApp(&handle);
		if (FirmwareInfo) 
			printf("%s\n",FirmwareInfo);
		else
			printf("This firmware does not support this feature\n");
		return 1;
	}

	printf("Bootloader verion : %x. Release data: %x\n",BL_version, BL_release);

	//
	//return 0;

	WriteFirmware("DT5550w.bin", &handle);
	Readfirmware("DT5550wRB2.bin", 3000000, &handle);

	WriteFirmwareInformation("RegisterFile.json", &handle);
	printf("%s\n",ReadFirmwareInformation(&handle));
	
	printf("Rebooting with the new firmware\n",BL_version, BL_release);
	NI_USB3_WriteReg(0xFE,BOOTLOADER_BA+8191,&handle);	
	return 0;
}


char * readTextFile(char *filename)
{	
	FILE *f = fopen(filename, "rb");
	if (f==NULL)
		return NULL;
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);

	char *string = (char *) malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;
	return string;
}

char * readBinFile(char *filename, uint32_t *bytes)
{	
	FILE *f = fopen(filename, "rb");
	*bytes = 0;
	if (f==NULL)
		return NULL;
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);

	char *string = (char *) malloc(fsize);
	fread(string, fsize, 1, f);
	fclose(f);
	*bytes = fsize;

	
	return string;
}

