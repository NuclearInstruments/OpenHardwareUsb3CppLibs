#include "stdafx.h"
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
#define RFA_IIC_BA							0x80000001




int IIC_WriteData(uint8_t address, uint8_t *value, int len, NI_HANDLE *handle)
{
	int i;
	/*
	if (NI_USB3_WriteReg(0, RFA_IIC_BA + 0, handle) != 0)
	return -1;
	Sleep(5);
	if (NI_USB3_WriteReg(1<<15, RFA_IIC_BA + 0, handle) != 0)
	return -1;
	Sleep(5);
	if (NI_USB3_WriteReg(0, RFA_IIC_BA + 0, handle) != 0)
	return -1;	*/
	Sleep(5);
	if (NI_USB3_WriteReg(1 << 8, RFA_IIC_BA + 0, handle) != 0)
		return -1;
	Sleep(5);

	if (NI_USB3_WriteReg((address << 1) + (1 << 12), RFA_IIC_BA + 0, handle) != 0)
		return -1;
	Sleep(5);

	for (i = 0;i<len;i++)
	{
		if (NI_USB3_WriteReg((value[i]) + (1 << 12), RFA_IIC_BA + 0, handle) != 0)
			return -1;
		Sleep(5);
	}

	if (NI_USB3_WriteReg((1 << 9), RFA_IIC_BA + 0, handle) != 0)
		return -1;

	Sleep(5);

	return 0;
}


int IIC_ReadData(uint8_t address, uint8_t *value, int len, uint8_t *value_read, int len_read, NI_HANDLE *handle)
{
	int i;
	/*
	if (NI_USB3_WriteReg(0, RFA_IIC_BA + 0, handle) != 0)
	return -1;
	Sleep(5);
	if (NI_USB3_WriteReg(1<<15, RFA_IIC_BA + 0, handle) != 0)
	return -1;
	Sleep(5);
	if (NI_USB3_WriteReg(0, RFA_IIC_BA + 0, handle) != 0)
	return -1;	*/
	Sleep(5);
	if (NI_USB3_WriteReg(1 << 8, RFA_IIC_BA + 0, handle) != 0)
		return -1;


	if (NI_USB3_WriteReg((address << 1) + (1 << 12), RFA_IIC_BA + 0, handle) != 0)
		return -1;
	Sleep(5);

	for (i = 0; i<len; i++)
	{
		if (NI_USB3_WriteReg((value[i]) + (1 << 12), RFA_IIC_BA + 0, handle) != 0)
			return -1;
		Sleep(5);
	}

	if (NI_USB3_WriteReg(1 << 8, RFA_IIC_BA + 0, handle) != 0)
		return -1;

	if (NI_USB3_WriteReg((address << 1) + 1 + (1 << 12), RFA_IIC_BA + 0, handle) != 0)
		return -1;
	Sleep(5);

	for (i = 0; i<len_read; i++)
	{
		if (NI_USB3_WriteReg((1 << 11) + (1 << 10), RFA_IIC_BA + 0, handle) != 0)
			return -1;
		Sleep(5);
	}

	if (NI_USB3_WriteReg((1 << 9), RFA_IIC_BA + 0, handle) != 0)
		return -1;

	Sleep(5);

	return 0;
}


int SetHV(bool Enable, float voltage, NI_HANDLE *handle)
{
	uint8_t vv[16];

	vv[0] = 1;
	vv[1] = 2;
	vv[2] = 0;
	vv[3] = 0;
	vv[4] = 0;
	vv[5] = 0;
	IIC_WriteData(0x73, vv, 6, handle);

	vv[0] = 6;
	vv[1] = 2;
	vv[2] = 10;
	vv[3] = 0;
	vv[4] = 0;
	vv[5] = 0;
	IIC_WriteData(0x73, vv, 6, handle);

	vv[0] = 2;
	vv[1] = 3;
	vv[2] = 0;
	vv[3] = 0;
	vv[4] = 0;
	vv[5] = 0;
	memcpy(&vv[2], &voltage, 4);
	IIC_WriteData(0x73, vv, 6, handle);

	vv[0] = 0;
	vv[1] = 2;
	vv[2] = Enable == true ? 1 : 0;
	vv[3] = 0;
	vv[4] = 0;
	vv[5] = 0;
	IIC_WriteData(0x73, vv, 6, handle);

}


int SetOffset(bool top, float voltage, NI_HANDLE *handle)
{
	uint8_t vv[16];
	uint8_t address;
	uint32_t voltageI = (int32_t)((-voltage + 2.0) / 4.0 * 4095);
	if (top == true) address = 0x61; else address = 0x60;
	vv[0] = (voltageI >> 8) & 0xFF;
	vv[1] = (voltageI >> 0) & 0xFF;
	return IIC_WriteData(address, vv, 2, handle);

}


int SetImpedance(bool R50, NI_HANDLE *handle)
{
	uint8_t vv[16];

	vv[0] = 0x03;
	vv[1] = 0x00;
	IIC_WriteData(0x18, vv, 2, handle);

	vv[0] = 0x01;
	if (R50 == false)
		vv[1] = 0x12;
	else
		vv[1] = 0x00;

	IIC_WriteData(0x18, vv, 2, handle);

}