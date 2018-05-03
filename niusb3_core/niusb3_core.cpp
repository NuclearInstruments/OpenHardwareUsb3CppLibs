// niusb3_core.cpp: definisce le funzioni esportate per l'applicazione DLL.
//

#include "stdafx.h"
#include "niusb3_core.h"
#include "FTD3XX.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>


// Esempio di variabile esportata
NIUSB3_CORE_API int nniusb3_core=0;

// Esempio di funzione esportata.
NIUSB3_CORE_API int fnniusb3_core(void)
{
	return 42;
}

NIUSB3_CORE_API int NI_USB3_Init()
{
	return 0;
}

NIUSB3_CORE_API int NI_USB3_ConnectDevice(char *serial_number, NI_HANDLE *handle)
{
	FT_STATUS ftStatus;
	ftStatus = FT_Create(
        serial_number,
        FT_OPEN_BY_SERIAL_NUMBER,
		(FT_HANDLE *) &handle->device_handle
        );

	if (ftStatus == FT_OK)
		FT_CycleDevicePort(handle->device_handle);

	Sleep(500);

	FT_Close(handle->device_handle);

	ftStatus = FT_Create(
		serial_number,
		FT_OPEN_BY_SERIAL_NUMBER,
		(FT_HANDLE *)&handle->device_handle
	);


	if (ftStatus == FT_OK)
	{
      ftStatus = FT_ResetDevicePort(
        handle->device_handle
        );

	 
	}
	handle->connection_status = CONNECTED;
	return ftStatus;
}

NIUSB3_CORE_API int NI_USB3_CloseConnection(NI_HANDLE *handle)
{
	if (handle->connection_status == CONNECTED)
	{
		FT_Close(handle->device_handle);
		handle->connection_status = NOT_CONNECTED;
		return 0;
	}
	else
		return 1;
}

NIUSB3_CORE_API int NI_USB3_WriteData(uint32_t *data, uint32_t count, uint32_t address, USB_BUS_MODE bus_mode, uint32_t timeout_ms, NI_HANDLE *handle, uint32_t *written_data)
{

	FT_STATUS ftStatus; 

	UCHAR acWriteBuf[255];
	UCHAR *data_pointer;
	ULONG ulBytesWritten = 0;
	int32_t pTotalData = count;
	int32_t timeoutns = timeout_ms * 100000;
	if (handle->connection_status != CONNECTED)
	{
		return -1;
	}
	//Imposta il timeout sulla pipe

	ftStatus = FT_SetPipeTimeout(
    handle->device_handle,
    0x02,
    timeout_ms * 1.5
    );
	if (FT_FAILED(ftStatus))
	{
		if (written_data!=NULL) *written_data = -1;
		return ftStatus;
	}


	//header
	acWriteBuf[0] = 0xFF;	acWriteBuf[1] = 0x00;	acWriteBuf[2] = 0xAB;	acWriteBuf[3] = (0xF0 + (bus_mode << 1));
	acWriteBuf[4] = 0xCD;	acWriteBuf[5] = 0xAB;	acWriteBuf[6] = 0xCD;	acWriteBuf[7] = 0xAB;
	acWriteBuf[8] = 0xF1;	acWriteBuf[9] = 0xCA;	acWriteBuf[10] = 0x60;	acWriteBuf[11] = 0x0D;

	//timeout
	acWriteBuf[12] = ((timeoutns >> 24)&0xFF);;	acWriteBuf[13] = ((timeoutns >> 16)&0xFF);;	acWriteBuf[14] = ((timeoutns >> 8)&0xFF);;	acWriteBuf[15] = ((timeoutns)&0xFF);;

	//address
	acWriteBuf[16] = ((address >> 24)&0xFF);	acWriteBuf[17] = ((address >> 16)&0xFF);	acWriteBuf[18] = ((address >> 8)&0xFF);	acWriteBuf[19] = ((address)&0xFF);

	//patload len
	acWriteBuf[20] = ((pTotalData >> 24)&0xFF);	acWriteBuf[21] = ((pTotalData >> 16)&0xFF);	acWriteBuf[22] = ((pTotalData >> 8)&0xFF);	acWriteBuf[23] = ((pTotalData)&0xFF);

	ftStatus = FT_WritePipe(handle->device_handle, 0x02, acWriteBuf, 24, &ulBytesWritten, NULL); 
	
	if (FT_FAILED(ftStatus))
	{
		if (written_data!=NULL) *written_data = -1;
		return ftStatus;
	}

	data_pointer = (UCHAR *) data;

	ftStatus = FT_WritePipe(handle->device_handle, 0x02, data_pointer, count*4, &ulBytesWritten, NULL); 

	if (written_data!=NULL) *written_data = ulBytesWritten;

	if (FT_FAILED(ftStatus))
	{
		return ftStatus;	
	}


	return 0;

}




NIUSB3_CORE_API int NI_USB3_ReadData(uint32_t *data, uint32_t count, uint32_t address, USB_BUS_MODE bus_mode, uint32_t timeout_ms, NI_HANDLE *handle, uint32_t *read_data, uint32_t *valid_data)
{

	FT_STATUS ftStatus; 

	UCHAR acWriteBuf[255];
	UCHAR *acReadBuf;
	UCHAR *data_pointer;
	ULONG ulBytesWritten = 0;
	int32_t pTotalData = count;
	int32_t timeoutns = timeout_ms * (100000.0/65536.0);
	timeoutns = timeoutns < 1 ? 1 : timeoutns;

	ULONG ulBytesRead = 0;
	ULONG cnt;
	ULONG pos;

	//FT_AbortPipe(handle, 0x82);

	if (handle->connection_status != CONNECTED)
	{
		return -1;
	}
	//Imposta il timeout sulla pipe
	
	ftStatus = FT_SetPipeTimeout(
    handle->device_handle,
    0x82,
    (int)timeout_ms * 5 + 100
    );
	if (FT_FAILED(ftStatus))
	{
		if (read_data!=NULL) *read_data = -1;
		return ftStatus;
	}
	

	//header
	acWriteBuf[0] = 0xFF;	acWriteBuf[1] = 0x00;	acWriteBuf[2] = 0xAB;	acWriteBuf[3] = (0xF1 + (bus_mode << 1));
	acWriteBuf[4] = 0xCD;	acWriteBuf[5] = 0xAB;	acWriteBuf[6] = 0xCD;	acWriteBuf[7] = 0xAB;
	acWriteBuf[8] = 0xF1;	acWriteBuf[9] = 0xCA;	acWriteBuf[10] = 0x60;	acWriteBuf[11] = 0x0D;

	//timeout
	acWriteBuf[12] = ((timeoutns >> 24)&0xFF);;	acWriteBuf[13] = ((timeoutns >> 16)&0xFF);;	acWriteBuf[14] = ((timeoutns >> 8)&0xFF);;	acWriteBuf[15] = ((timeoutns)&0xFF);;

	//address
	acWriteBuf[16] = ((address >> 24)&0xFF);	acWriteBuf[17] = ((address >> 16)&0xFF);	acWriteBuf[18] = ((address >> 8)&0xFF);	acWriteBuf[19] = ((address)&0xFF);

	//patload len
	acWriteBuf[20] = ((pTotalData >> 24)&0xFF);	acWriteBuf[21] = ((pTotalData >> 16)&0xFF);	acWriteBuf[22] = ((pTotalData >> 8)&0xFF);	acWriteBuf[23] = ((pTotalData)&0xFF);

	//Send request
	ftStatus = FT_WritePipe(handle->device_handle, 0x02, acWriteBuf, 24, &ulBytesWritten, NULL); 
	
	if (FT_FAILED(ftStatus))
	{
		if (read_data!=NULL) *read_data = -1;
		return ftStatus;
	}
	const clock_t begin_time = clock();

	//Read data
	acReadBuf = (UCHAR *) data;

	cnt = (count+2) * 4;
	pos = 0; 
	
	while (cnt > 0)
	{
		ftStatus = FT_ReadPipe(handle->device_handle, 0x82, &acReadBuf[pos], cnt, &ulBytesRead, NULL);
		if (FT_FAILED(ftStatus))
		{
			if (read_data!=NULL) *read_data = pos;
			return ftStatus;
		}
		cnt = cnt - ulBytesRead;
		pos = pos + ulBytesRead;
	
		/*if (float(clock() - begin_time) / CLOCKS_PER_SEC > timeout_ms / 1000)
			return 0x10000000;*/
	}
	
	//Extract valid data
	uint32_t  *preal;
	uint32_t  *keyword;
	keyword = (uint32_t *) &acReadBuf[count*4];
	preal = (uint32_t *) &acReadBuf[count*4+4];
	if (*preal == 0xF1CA600D)
		preal = (uint32_t *) &acReadBuf[count*4+8];

	if (read_data!=NULL)  *read_data = pos;
	if (valid_data!=NULL) *valid_data = *preal;
	
	if (*keyword == 0xF1CA600D)
		return 0;
	else
		return 2;
}

NIUSB3_CORE_API int NI_USB3_WriteReg(uint32_t data, uint32_t address, NI_HANDLE *handle)
{
	FT_STATUS ftStatus; 
	uint32_t written_data;
	ftStatus = NI_USB3_WriteData(&data, 1, address, REG_ACCESS , 1000, handle, &written_data);
	
	if (FT_FAILED(ftStatus))
		return -1;

	if (written_data>0)
		return 0;
	else 
		return -1;
}


NIUSB3_CORE_API int NI_USB3_ReadReg(uint32_t *data, uint32_t address, NI_HANDLE *handle)
{
	FT_STATUS ftStatus; 
	uint32_t read_data;
	uint32_t dummy_vector[4];
	ftStatus = NI_USB3_ReadData(dummy_vector, 1, address, REG_ACCESS , 1000, handle, NULL, &read_data);
	*data = dummy_vector[0];
	if (FT_FAILED(ftStatus))
		return -1;

	if (read_data>0)
		return 0;
	else 
		return -1;
}



NIUSB3_CORE_API int NI_USB3_ListDevices(char *ListOfDevice, char *model,  int *Count)
{
	FT_STATUS ftStatus; 
	FT_DEVICE_LIST_INFO_NODE *devInfo; 
	DWORD numDevs; 

	// 
	// create the device information list 
	// 
	ftStatus = FT_CreateDeviceInfoList(&numDevs); 
	if (ftStatus != FT_OK) { 
	   return -1;
	} 

	// 
	// allocate storage for list based on numDevs 
	// 
	devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs); 
	ListOfDevice[0] = '\0';
	// 
	// get the device information list 
	// 
	*Count = 0;
	ftStatus = FT_GetDeviceInfoList(devInfo,&numDevs); 
	if (ftStatus == FT_OK) { 
	for (int i = 0; i < numDevs; i++) {  
		bool valid=true;
		
		if (model!= NULL)
			if (strcmp(devInfo[i].Description, model) == 0)
				valid = true;
			else
				valid = false;

		if (valid == true)
			sprintf(ListOfDevice + strlen(ListOfDevice), "%s;",devInfo[i].SerialNumber); 
			(*Count)++;
	   } 
	}

	
	return ftStatus;
}



NIUSB3_CORE_API int NI_USB3_IIC_WriteData(uint8_t address, uint8_t *value, int len, NI_HANDLE *handle)
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
	if (NI_USB3_WriteReg(1 << 8, __IICBASEADDRESS + 0, handle) != 0)
		return -1;
	Sleep(5);

	if (NI_USB3_WriteReg((address << 1) + (1 << 12), __IICBASEADDRESS + 0, handle) != 0)
		return -1;
	Sleep(5);

	for (i = 0; i<len; i++)
	{
		if (NI_USB3_WriteReg((value[i]) + (1 << 12), __IICBASEADDRESS + 0, handle) != 0)
			return -1;
		Sleep(5);
	}

	if (NI_USB3_WriteReg((1 << 9), __IICBASEADDRESS + 0, handle) != 0)
		return -1;

	Sleep(5);

	return 0;
}

NIUSB3_CORE_API int NI_USB3_SetIICControllerBaseAddress(uint32_t ControlAddress, uint32_t StatusAddress, NI_HANDLE *handle)
{
	__IICBASEADDRESS = ControlAddress;
	__IICBASEADDRESS_STATUS = StatusAddress;
	return 0;
}
NIUSB3_CORE_API int NI_USB3_IIC_ReadData(uint8_t address, uint8_t *value, int len, uint8_t *value_read, int len_read, NI_HANDLE *handle)
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
	if (NI_USB3_WriteReg(1 << 8, __IICBASEADDRESS + 0, handle) != 0)
		return -1;


	if (NI_USB3_WriteReg((address << 1) + (1 << 12), __IICBASEADDRESS + 0, handle) != 0)
		return -1;
	Sleep(5);

	for (i = 0; i<len; i++)
	{
		if (NI_USB3_WriteReg((value[i]) + (1 << 12), __IICBASEADDRESS + 0, handle) != 0)
			return -1;
		Sleep(5);
	}

	if (NI_USB3_WriteReg(1 << 8, __IICBASEADDRESS + 0, handle) != 0)
		return -1;

	if (NI_USB3_WriteReg((address << 1) + 1 + (1 << 12), __IICBASEADDRESS + 0, handle) != 0)
		return -1;
	Sleep(5);

	for (i = 0; i<len_read; i++)
	{
		if (NI_USB3_WriteReg((1 << 11) + (1 << 10), __IICBASEADDRESS + 0, handle) != 0)
			return -1;
		Sleep(5);
	}

	if (NI_USB3_WriteReg((1 << 9), __IICBASEADDRESS + 0, handle) != 0)
		return -1;

	Sleep(5);

	return 0;
}


NIUSB3_CORE_API int NI_USB3_SetHV(bool Enable, float voltage, NI_HANDLE *handle)
{
	uint8_t vv[16];

	vv[0] = 1;
	vv[1] = 2;
	vv[2] = 0;
	vv[3] = 0;
	vv[4] = 0;
	vv[5] = 0;
	NI_USB3_IIC_WriteData(0x73, vv, 6, handle);

	vv[0] = 6;
	vv[1] = 2;
	vv[2] = 10;
	vv[3] = 0;
	vv[4] = 0;
	vv[5] = 0;
	NI_USB3_IIC_WriteData(0x73, vv, 6, handle);

	vv[0] = 2;
	vv[1] = 3;
	vv[2] = 0;
	vv[3] = 0;
	vv[4] = 0;
	vv[5] = 0;
	memcpy(&vv[2], &voltage, 4);
	NI_USB3_IIC_WriteData(0x73, vv, 6, handle);

	vv[0] = 0;
	vv[1] = 2;
	vv[2] = Enable == true ? 1 : 0;
	vv[3] = 0;
	vv[4] = 0;
	vv[5] = 0;
	NI_USB3_IIC_WriteData(0x73, vv, 6, handle);

}


NIUSB3_CORE_API int NI_USB3_SetOffset(bool top, uint32_t DACCODE, NI_HANDLE *handle)
{
	uint8_t vv[16];
	uint8_t address;
	uint32_t voltageI = DACCODE;
	if (top == true) address = 0x61; else address = 0x60;
	vv[0] = (voltageI >> 8) & 0xFF;
	vv[1] = (voltageI >> 0) & 0xFF;
	return NI_USB3_IIC_WriteData(address, vv, 2, handle);

}


NIUSB3_CORE_API int NI_USB3_SetImpedance(bool R50, NI_HANDLE *handle)
{
	uint8_t vv[16];

	vv[0] = 0x03;
	vv[1] = 0x00;
	NI_USB3_IIC_WriteData(0x18, vv, 2, handle);

	vv[0] = 0x01;
	if (R50 == false)
		vv[1] = 0x12;
	else
		vv[1] = 0x00;

	NI_USB3_IIC_WriteData(0x18, vv, 2, handle);

}



NIUSB3_CORE_API char *ReadFirmwareInformationApp(	NI_HANDLE *handle)
{
		uint32_t datarow[512];
	uint32_t dataread[512];
	uint32_t temp =0; 
	uint32_t rw;
	uint32_t vd;
	uint32_t bytes;
	uint32_t q;
	
	int i;
	char *TEXT = (char*) malloc(0x40040 * sizeof(uint32_t));

	NI_USB3_ReadReg(&temp,FLASH_CONTROLLER_BA + FLASH_CONTROLLER_REG_MAGIC,handle);
	if (temp != 0x1234ABBA)
		return NULL;

	
	memset(TEXT,0,0x40000);
	for (i=0;i<0x40000;i+=256)
	{

		NI_USB3_WriteReg(FIRMWARE_FIRMWARE_INFO_BA+ i,FLASH_CONTROLLER_BA+FLASH_CONTROLLER_REG_ADDR,handle);
		NI_USB3_WriteReg(5,FLASH_CONTROLLER_BA+FLASH_CONTROLLER_REG_CNTRL,handle);
		do
		{
			NI_USB3_ReadReg(&temp,FLASH_CONTROLLER_BA+FLASH_CONTROLLER_REG_CNTRL,handle);
		}while(temp!=0);
		NI_USB3_ReadData((uint32_t*)(TEXT+i), 64, FLASH_CONTROLLER_BA, REG_ACCESS, 1000, handle, &rw, &vd);
		if (TEXT[i+255] == '\0')
			break;
	}

	return TEXT;

}