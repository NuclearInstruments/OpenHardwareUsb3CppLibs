

#include "stdafx.h"
#include "../niusb3_core/niusb3_core.h"
#include "ClockGeneratorCDCE65005.h"
#include "i2c.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <time.h>
#include <conio.h>
#include <windows.h>

using namespace std;

unsigned int gray_to_bin(unsigned int Gray, int nbit)
{
	unsigned int Bin=0;
	for (int i = 0; i < nbit; ++ i) {
		unsigned short j = nbit-1 - i;
		unsigned short m = 1 << j;
		Bin |= ((Bin >> 1) & m) ^ (Gray & m);
	}
	return Bin;

}

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





#define READ_BA 0x80010000
#define READ_STATUS 0x80020000
#define READ_POSITION 0x80020001
#define CONFIG_TRIGGER_MODE 0x80020002
#define CONFIG_PRETRIGGER 0x80020003
#define CONFIG_TRIGGER_LEVEL 0x80020004
#define CONFIG_ARM 0x80020005
#define CONFIG_DECIMATOR 0x80020006


#define I_READ_BA 0x80000000
#define I_READ_STATUS 0x80000001
#define I_CONFIG_TRIGGER_MODE 0x80000002
#define I_CONFIG_T0MASK 0x80000003
#define I_CONFIG_WAIT 0x80000004
#define I_CONFIG_ARM 0x80000005
#define I_CONFIG_SYNC 0x80000006

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

int _tmain(int argc, _TCHAR* argv[])
{
	NI_HANDLE handle;
	FILE *fp;
	char listDev[2048];
	uint32_t datarow[8192];
	uint32_t dataread[8192];
	uint32_t *data_words = (uint32_t*) malloc(100000000 * sizeof(uint32_t));//[96000];
	char *TEXT = (char*) malloc(1000000 * sizeof(uint32_t));//[96000];
	int Count;
	uint32_t rw;
	uint32_t vd;
	uint32_t single_reg;
	uint8_t vv[16];
	uint8_t vvr[16];
	int i;
	int q;
	int transferlen =0;
	uint32_t temp =0; 
	NI_USB3_Init();
	NI_USB3_ListDevices(listDev, NULL,  &Count);
	
	vector<string> vect = split(listDev, ';');

	NI_USB3_ConnectDevice((char *) vect[0].c_str(), &handle);

	transferlen=8192;
	q=0;

		for (int z = 0; z<transferlen; z++)
		{
			datarow[z] = q++;
		}

	//Ram Test. Perform a write/read of a RAM area to test transfer errors
	while(1)
	{

	
		NI_USB3_WriteData(datarow, transferlen, 0x80000000, REG_ACCESS, 1000, &handle, &rw);
		NI_USB3_ReadData(dataread, transferlen, 0x80000000, REG_ACCESS, 1000, &handle, &rw, &vd);
		
		for (i = 0; i<transferlen; i++)
		{
			if (datarow[i] != dataread[i])
				printf("errore %d   --     %X     %X\n", i, datarow[i], dataread[i]);
		}
		printf(".");
		fflush(stdout);
	}
	

	//Initialize oscilloscpoe
	
	NI_USB3_WriteReg(0, CONFIG_DECIMATOR, &handle);
	NI_USB3_WriteReg(100, CONFIG_PRETRIGGER, &handle);
	NI_USB3_WriteReg(0x82, CONFIG_TRIGGER_MODE, &handle);
	NI_USB3_WriteReg(1000, CONFIG_TRIGGER_LEVEL, &handle);
	
	NI_USB3_WriteReg(0, CONFIG_ARM, &handle);
	NI_USB3_WriteReg(1, CONFIG_ARM, &handle);
	NI_USB3_ReadReg(&single_reg, READ_STATUS, &handle);
	NI_USB3_ReadReg(&single_reg, READ_POSITION, &handle);
	
	NI_USB3_ReadData(data_words, 4096*32, READ_BA, REG_ACCESS, 1000, &handle, &rw, &vd);
	
	

	

	return 0;
}

 