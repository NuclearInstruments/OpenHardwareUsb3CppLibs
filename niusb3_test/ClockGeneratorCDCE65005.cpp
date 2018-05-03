
#include "stdafx.h"
#include "ClockGeneratorCDCE65005.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <time.h>
#include <conio.h>
#include <windows.h>


#define RFA_CLKGEN_BA						0xFFFFFF00

#define RFA_CLKGEN_STROBE					0x09
#define RFA_CLKGEN_CFG0						0x00
#define RFA_CLKGEN_CFG1						0x01
#define RFA_CLKGEN_CFG2      				0x02
#define RFA_CLKGEN_CFG3      				0x03
#define RFA_CLKGEN_CFG4      				0x04
#define RFA_CLKGEN_CFG5      				0x05
#define RFA_CLKGEN_CFG6      				0x06
#define RFA_CLKGEN_CFG7      				0x07

#define RFA_CLKGEN_CFG8      				0x08
//USE TEXAS INSTRUMENTS TOOL TO CALCULATE REGISTER VALUES

int CfgClockGenerator(NI_HANDLE *handle)
{
	//CFG0
	if (NI_USB3_WriteReg(0xEB0A0320, RFA_CLKGEN_BA + RFA_CLKGEN_CFG0, handle) != 0) //83800320
		return -1;

	Sleep(5);
	//CFG1

	if (NI_USB3_WriteReg(0xEB140301, RFA_CLKGEN_BA + RFA_CLKGEN_CFG1, handle) != 0) //83800301
		return -1;

	Sleep(5);
	//CFG2
	if (NI_USB3_WriteReg(0xEB020302, RFA_CLKGEN_BA + RFA_CLKGEN_CFG2, handle) != 0) //EB0C0302
		return -1;

	Sleep(5);
	//CFG3 EB020303  EB0A0303
	if (NI_USB3_WriteReg(0xEB020303, RFA_CLKGEN_BA + RFA_CLKGEN_CFG3, handle) != 0)
		return -1;
	Sleep(5);

	//CFG4
	if (NI_USB3_WriteReg(0x68860314, RFA_CLKGEN_BA + RFA_CLKGEN_CFG4, handle) != 0)
		return -1;

	Sleep(5);

	//CFG5
	if (NI_USB3_WriteReg(0x10100B25, RFA_CLKGEN_BA + RFA_CLKGEN_CFG5, handle) != 0)
		return -1;

	Sleep(5);
	//CFG6
	if (NI_USB3_WriteReg(0x80BE1B66, RFA_CLKGEN_BA + RFA_CLKGEN_CFG6, handle) != 0)
		return -1;

	Sleep(5);
	//CFG7
	if (NI_USB3_WriteReg(0x950037F7, RFA_CLKGEN_BA + RFA_CLKGEN_CFG7, handle) != 0)
		return -1;

	Sleep(5);
	//CFG8
	if (NI_USB3_WriteReg(0x20009D98, RFA_CLKGEN_BA + RFA_CLKGEN_CFG8, handle) != 0)
		return -1;
	Sleep(5);


	//STROBE
	if (NI_USB3_WriteReg(1, RFA_CLKGEN_BA + RFA_CLKGEN_STROBE, handle) != 0)
		return -1;
	if (NI_USB3_WriteReg(0, RFA_CLKGEN_BA + RFA_CLKGEN_STROBE, handle) != 0)
		return -1;

	if (NI_USB3_WriteReg(0x84BE1B66, RFA_CLKGEN_BA + RFA_CLKGEN_CFG6, handle) != 0)
		return -1;


	if (NI_USB3_WriteReg(1, RFA_CLKGEN_BA + RFA_CLKGEN_STROBE, handle) != 0)
		return -1;
	if (NI_USB3_WriteReg(0, RFA_CLKGEN_BA + RFA_CLKGEN_STROBE, handle) != 0)
		return -1;

	if (NI_USB3_WriteReg(0x80BE1B66, RFA_CLKGEN_BA + RFA_CLKGEN_CFG6, handle) != 0)
		return -1;

	if (NI_USB3_WriteReg(1, RFA_CLKGEN_BA + RFA_CLKGEN_STROBE, handle) != 0)
		return -1;
	if (NI_USB3_WriteReg(0, RFA_CLKGEN_BA + RFA_CLKGEN_STROBE, handle) != 0)
		return -1;

	return 0;
}
