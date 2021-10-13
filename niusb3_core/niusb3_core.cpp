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

NIUSB3_CORE_API int NI_USB3_AllocHandle(NI_HANDLE **handle)
{
	*handle = (NI_HANDLE *)malloc(sizeof(NI_HANDLE));
	return (*handle == 0 ? 1 : 0);
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

	Sleep(2500);

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

	NI_USB3_ReadReg(&(handle->version), 0xFFFEFFFC, handle);
	NI_USB3_ReadReg(&(handle->board_identifier), 0xFFFFFFFB, handle);
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
	uint32_t data_tmp;
	if (NI_USB3_WriteReg(1 << 8, __IICBASEADDRESS + 0, handle) != 0)
		return -1;
		do 
		{
			if (NI_USB3_ReadReg(&data_tmp,__IICBASEADDRESS_STATUS, handle) != 0)
				return -1;
		} while ((data_tmp>>17)==0);


	if (NI_USB3_WriteReg((address << 1) + (1 << 12), __IICBASEADDRESS + 0, handle) != 0)
		return -1;
		do 
		{
			if (NI_USB3_ReadReg(&data_tmp,__IICBASEADDRESS_STATUS, handle) != 0)
				return -1;
		} while ((data_tmp>>17)==0);


	for (i = 0; i<len; i++)
	{
		if (NI_USB3_WriteReg((value[i]) + (1 << 12), __IICBASEADDRESS + 0, handle) != 0)
			return -1;
		do 
		{
			if (NI_USB3_ReadReg(&data_tmp,__IICBASEADDRESS_STATUS, handle) != 0)
				return -1;
		} while ((data_tmp>>17)==0);

	}

	if (NI_USB3_WriteReg((1 << 9), __IICBASEADDRESS + 0, handle) != 0)
		return -1;
		do 
		{
			if (NI_USB3_ReadReg(&data_tmp,__IICBASEADDRESS_STATUS, handle) != 0)
				return -1;
		} while ((data_tmp>>17)==0);


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
	uint32_t data_tmp;

	
	if (NI_USB3_WriteReg(1 << 8, __IICBASEADDRESS + 0, handle) != 0)
		return -1;


	if (NI_USB3_WriteReg((address << 1) + (1 << 12), __IICBASEADDRESS + 0, handle) != 0)
		return -1;
	
	
	do 
	{
		if (NI_USB3_ReadReg(&data_tmp,__IICBASEADDRESS_STATUS, handle) != 0)
			return -1;
	} while ((data_tmp>>17)==0);


	for (i = 0; i<len; i++)
	{
		if (NI_USB3_WriteReg((value[i]) + (1 << 12), __IICBASEADDRESS + 0, handle) != 0)
			return -1;
		
		do 
		{
			if (NI_USB3_ReadReg(&data_tmp,__IICBASEADDRESS_STATUS, handle) != 0)
				return -1;
		} while ((data_tmp>>17)==0);


	}

	if (NI_USB3_WriteReg(1 << 8, __IICBASEADDRESS + 0, handle) != 0)
		return -1;

	if (NI_USB3_WriteReg((address << 1) + 1 + (1 << 12), __IICBASEADDRESS + 0, handle) != 0)
		return -1;
	
	do 
	{
		if (NI_USB3_ReadReg(&data_tmp,__IICBASEADDRESS_STATUS, handle) != 0)
			return -1;
	} while ((data_tmp>>17)==0);


	for (i = 0; i<len_read; i++)
	{
		if (NI_USB3_WriteReg((1 << 11) + (1 << 10), __IICBASEADDRESS + 0, handle) != 0)
			return -1;
		do 
		{
			if (NI_USB3_ReadReg(&data_tmp, __IICBASEADDRESS_STATUS, handle) != 0)
				return -1;

		} while ((data_tmp>>17)==0);
		value_read[i] = data_tmp & 0xFF;
	}

	if (NI_USB3_WriteReg((1 << 9), __IICBASEADDRESS + 0, handle) != 0)
		return -1;

	do 
	{
		if (NI_USB3_ReadReg(&data_tmp,__IICBASEADDRESS_STATUS, handle) != 0)
			return -1;
	} while ((data_tmp>>17)==0);



	return 0;
}


NIUSB3_CORE_API int NI_USB3_IICUser_OpenController(uint32_t ControlAddress, uint32_t StatusAddress, NI_HANDLE *handle, NI_IIC_HANDLE *IIC_Handle)
{
	IIC_Handle->dev_con = handle;
	IIC_Handle->__IICBASEADDRESS = ControlAddress;
	IIC_Handle->__IICBASEADDRESS_STATUS = StatusAddress;

	return 0;
}


NIUSB3_CORE_API int NI_USB3_IICUser_ReadData(uint8_t address, uint8_t *value, int len, uint8_t *value_read, int len_read, NI_IIC_HANDLE *IIC_Handle)
{
	uint32_t data_tmp;
	int i;

	
	if (NI_USB3_WriteReg(1 << 8, IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
		return -1;


	if (NI_USB3_WriteReg((address << 1) + (1 << 12), IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
		return -1;
	

	do 
	{
		if (NI_USB3_ReadReg(&data_tmp,IIC_Handle->__IICBASEADDRESS_STATUS, IIC_Handle->dev_con) != 0)
			return -1;
	} while ((data_tmp>>17)==0);


	for (i = 0; i<len; i++)
	{
		if (NI_USB3_WriteReg((value[i]) + (1 << 12),IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
			return -1;

		do 
		{
			if (NI_USB3_ReadReg(&data_tmp, IIC_Handle->__IICBASEADDRESS_STATUS, IIC_Handle->dev_con) != 0)
				return -1;
		} while ((data_tmp>>17)==0);

	}

	if (NI_USB3_WriteReg(1 << 8, IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
		return -1;

	if (NI_USB3_WriteReg((address << 1) + 1 + (1 << 12), IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
		return -1;
	
	do 
	{
		if (NI_USB3_ReadReg(&data_tmp, IIC_Handle->__IICBASEADDRESS_STATUS, IIC_Handle->dev_con) != 0)
			return -1;
	} while ((data_tmp>>17)==0);


	for (i = 0; i<len_read; i++)
	{
		if (NI_USB3_WriteReg((1 << 11) + (1 << 10), IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
			return -1;
		do 
		{
			if (NI_USB3_ReadReg(&data_tmp, IIC_Handle->__IICBASEADDRESS_STATUS, IIC_Handle->dev_con) != 0)
				return -1;

		} while ((data_tmp>>17)==0);
		value_read[i] = data_tmp & 0xFF;
	}

	if (NI_USB3_WriteReg((1 << 9), IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
		return -1;

	do 
	{
		if (NI_USB3_ReadReg(&data_tmp, IIC_Handle->__IICBASEADDRESS_STATUS, IIC_Handle->dev_con) != 0)
			return -1;
	} while ((data_tmp>>17)==0);



	return 0;

}

NIUSB3_CORE_API int NI_USB3_IICUser_WriteData(uint8_t address, uint8_t *value, int len, NI_IIC_HANDLE *IIC_Handle)
{
	int i;
	uint32_t data_tmp;
	
	if (NI_USB3_WriteReg(1 << 8, IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
		return -1;
	do 
	{
		if (NI_USB3_ReadReg(&data_tmp, IIC_Handle->__IICBASEADDRESS_STATUS, IIC_Handle->dev_con) != 0)
			return -1;
	} while ((data_tmp>>17)==0);


	if (NI_USB3_WriteReg((address << 1) + (1 << 12), IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
		return -1;
	

	do 
	{
		if (NI_USB3_ReadReg(&data_tmp,IIC_Handle->__IICBASEADDRESS_STATUS, IIC_Handle->dev_con) != 0)
			return -1;
	} while ((data_tmp>>17)==0);

	for (i = 0; i<len; i++)
	{
		if (NI_USB3_WriteReg((value[i]) + (1 << 12), IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
			return -1;
		
		do 
		{
			if (NI_USB3_ReadReg(&data_tmp, IIC_Handle->__IICBASEADDRESS_STATUS, IIC_Handle->dev_con) != 0)
				return -1;
		} while ((data_tmp>>17)==0);
	}

	if (NI_USB3_WriteReg((1 << 9), IIC_Handle->__IICBASEADDRESS, IIC_Handle->dev_con) != 0)
		return -1;

	do 
	{
		if (NI_USB3_ReadReg(&data_tmp,IIC_Handle->__IICBASEADDRESS_STATUS, IIC_Handle->dev_con) != 0)
			return -1;
	} while ((data_tmp>>17)==0);
}


NIUSB3_CORE_API int NI_USB3_GetDT5550_DGBoardInfo(char *model, int *asic_count, int *SN, NI_HANDLE *handle)
{
	int i;
	uint8_t vv[16];
	vv[0] = 0;
	if (handle->version < 0x21050500) {


		NI_USB3_IIC_ReadData(0x54, vv, 1, vv, 16, handle);

		for (i = 0; i < 3; i++)
			model[i] = vv[i + 4];

		*asic_count = vv[7];
		*SN = vv[8] + (vv[9] << 8) + (vv[10] << 16) + (vv[11] << 24);
		model[3] = '\0';
		vv[4] = '\0';
		if (strcmp((char *)vv, "NI5W") == 0)
			return 0;
		else
			return 1;

	}
	else {
		uint32_t data;
		char *p;
		char _key[8];
		char _model[8];
		p = (char*)&data;
		NI_USB3_ReadReg(&data, 0xFFFF0030, handle);
		_key[0] = p[0];
		_key[1] = p[1];
		_key[2] = p[2];
		_key[3] = p[3];
		_key[4] = '\0';
		NI_USB3_ReadReg(&data, 0xFFFF0031, handle);
		_model[0] = p[0];
		_model[1] = p[1];
		_model[2] = p[2];
		_model[3] = '\0';
		NI_USB3_ReadReg(&data, 0xFFFF0032, handle);
		*SN = data;
		NI_USB3_ReadReg(&data, 0xFFFF0033, handle);
		*asic_count = data;
		strcpy(model, _model);
		if (strcmp((char *)_key, "NI5W") == 0)
			return 0;
		else
			return 1;
	}
}


NIUSB3_CORE_API int NI_USB3_SetDT5550_DGBoardInfo(char *model, int asic_count, int SN, NI_HANDLE *handle)
{
	int i;


		uint8_t vv[16];
		uint8_t vv2[2];

		vv[0] = 'N';
		vv[1] = 'I';
		vv[2] = '5';
		vv[3] = 'W';

		vv[4] = model[0];
		vv[5] = model[1];
		vv[6] = model[2];

		vv[7] = asic_count;

		vv[8] = (SN >> 0) & 0xFF;
		vv[9] = (SN >> 8) & 0xFF;
		vv[10] = (SN >> 16) & 0xFF;
		vv[11] = (SN >> 24) & 0xFF;
	if (handle->version < 0x21050500) {
		for (i = 0; i < 16; i++)
		{
			vv2[0] = i;
			vv2[1] = vv[i];
			NI_USB3_IIC_WriteData(0x54, vv2, 2, handle);
			Sleep(15);
		}
	}
	else {
		for (i = 0; i < 16; i++) {
			uint32_t status;
			do {
				NI_USB3_ReadReg(&status, 0xFFFF0100, handle);
			} while (status & 0x1 == 0);
			Sleep(10);
			NI_USB3_WriteReg((i << 8) + (uint32_t)vv[i], 0xFFFF0034, handle);
		}
	}
	return 0;
}

NIUSB3_CORE_API int NI_USB3_SetHV(bool Enable, float voltage, NI_HANDLE *handle)
{
	uint8_t vv[16];
	if (handle->version < 0x21050500) {
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
		return 0;
	}
	else {
		NI_USB3_WriteReg((uint32_t)(voltage * 10000), 0xFFFF0002, handle);
		NI_USB3_WriteReg((uint32_t)((voltage+8) * 10000), 0xFFFF0009, handle);
		NI_USB3_WriteReg(0, 0xFFFF0003, handle);
		NI_USB3_WriteReg(Enable == true ? 1 : 0, 0xFFFF0000, handle);
		return 0;
	}
}


NIUSB3_CORE_API int NI_USB3_GetHV(bool *Enable, float *voltage, float *current, NI_HANDLE *handle)
{
	if (handle->version < 0x21050500) {
		uint8_t vv[16];
		uint32_t *data;
		data = (uint32_t*)vv;
		vv[0] = 0;
		vv[1] = 0;
		NI_USB3_IIC_ReadData(0x73, vv, 2, vv, 4, handle);
		*Enable = vv[0] > 0 ? true : false;
		vv[0] = 231;
		vv[1] = 1;
		NI_USB3_IIC_ReadData(0x73, vv, 2, vv, 4, handle);

		*voltage = ((float)*data) / 10000.0;

		vv[0] = 232;
		vv[1] = 1;
		NI_USB3_IIC_ReadData(0x73, vv, 2, vv, 4, handle);
		*current = ((float)*data) / 10000.0;
		if (*current > 10000) *current = 0;
		return 0;
	}
	else {
		uint32_t data;
		int32_t *datai;
		datai = (int32_t*)&data;
		NI_USB3_ReadReg(&data, 0xFFFF0010, handle);
		*Enable = data & 0x1 ? true : false;
		NI_USB3_ReadReg(&data, 0xFFFF0011, handle);
		*voltage = ((double)data) / 10000.0;
		NI_USB3_ReadReg(&data, 0xFFFF0012, handle);
		*current = ((double)*datai) / 10.0;
		return 0;
	}

}



NIUSB3_CORE_API int NI_USB3_GetDT5550WTempSens(int address, float *temp, NI_HANDLE *handle)
{
	if (handle->version < 0x21050500) {
		uint8_t vv[16];
		uint32_t *data;
		data = (uint32_t*)vv;
		vv[0] = 0;
		NI_USB3_IIC_ReadData(0x48 + address, vv, 1, vv, 2, handle);
		int32_t value;
		value = (vv[0] << 4) + ((vv[1] >> 4) & 0xF);
		if (value > 0x800) value = -(0xFFF - value);
		*temp = ((float)value) / 2048.0 * 128.0;
		return 0;
	}
	else {
		uint32_t data;
		address = (address > 1 ? 1 : address);
		NI_USB3_ReadReg(&data, 0xFFFF0020 + address, handle);
		*temp = ((double)data) / 16.0;
		return 0;
	}
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




/*
	Write Security EEPROM
*/

NIUSB3_CORE_API int SECWriteWord(unsigned int address, unsigned int word, NI_HANDLE *handle)
{
	
	if(NI_USB3_WriteReg(0, RFA_ICAPDNA_WF , handle) != 0)
		return -1;

	if(NI_USB3_WriteReg(1 << 24, RFA_ICAPDNA_EEPROM, handle ) != 0)
		return -1;

	if(NI_USB3_WriteReg(1, RFA_ICAPDNA_WF, handle) != 0)
		return -1;

	if(NI_USB3_WriteReg(0, RFA_ICAPDNA_WF, handle) != 0)
		return -1;
	Sleep (10);
	if(NI_USB3_WriteReg((address << 16) + (2 << 24), RFA_ICAPDNA_EEPROM,handle ) != 0)
		return -1;

	if(NI_USB3_WriteReg(1, RFA_ICAPDNA_WF, handle ) != 0)
		return -1;

	if(NI_USB3_WriteReg(0, RFA_ICAPDNA_WF, handle ) != 0)
		return -1;

	Sleep (10);

	if(NI_USB3_WriteReg(word + (address << 16) + (4 << 24), RFA_ICAPDNA_EEPROM, handle ) != 0)
		return -1;

	if(NI_USB3_WriteReg(1, RFA_ICAPDNA_WF, handle ) != 0)
		return -1;

	if(NI_USB3_WriteReg(0, RFA_ICAPDNA_WF, handle ) != 0)
		return -1;

	Sleep (10);
	
	return 0;
}




/*
	Read Security EEPROM
*/

NIUSB3_CORE_API int SECReadWord(unsigned int address, unsigned int *word, NI_HANDLE *handle)
{
	unsigned int w;
	if(NI_USB3_WriteReg(0, RFA_ICAPDNA_WF, handle) != 0)
		return -1;

	if(NI_USB3_WriteReg((address << 16) + (8 << 24), RFA_ICAPDNA_EEPROM, handle ) != 0)
		return -1;

	if(NI_USB3_WriteReg(1, RFA_ICAPDNA_WF, handle ) != 0)
		return -1;

	if(NI_USB3_WriteReg(0, RFA_ICAPDNA_WF, handle ) != 0)
		return -1;

	Sleep (10);

	if(NI_USB3_ReadReg(&w, RFA_ICAPDNA_READEEPROM, handle ) != 0)
		return -1;

	if(NI_USB3_WriteReg(0, RFA_ICAPDNA_WF, handle) != 0)
		return -1;

	if(NI_USB3_WriteReg(1, RFA_ICAPDNA_WF, handle) != 0)
		return -1;

	if(NI_USB3_WriteReg(0, RFA_ICAPDNA_WF, handle) != 0)
		return -1;

	*word = w & 0xFFFF;
	
	return 0;
}



NIUSB3_CORE_API int SECReadUIDSN(uint64_t *UID, uint32_t *SN, NI_HANDLE *handle)
{
	unsigned int TTT[20];
	unsigned int v;
	uint32_t read_data;
	uint32_t valid_data;

	SECReadWord(16,&v, handle);

	NI_USB3_ReadData(TTT, 18, RFA_ICAPDNA_BA, REG_ACCESS , 100, handle, &read_data, &valid_data);


	*UID = (((UINT64)TTT[5]) << 32) + ((UINT64)TTT[4]); 
	*SN = v;
	return 0;
}



NIUSB3_CORE_API int SECWriteSN(unsigned int SN, NI_HANDLE *handle)
{
	return SECWriteWord(16,SN, handle);
	
}


NIUSB3_CORE_API int SECWritekey(int *key, int length, NI_HANDLE *handle)
{
	int i;
	if (length > 16)
		return -1;
	for (i=0;i<length-1;i+=2)
	{
		SECWriteWord(i,key[i+1],handle);
		SECWriteWord(i+1,key[i],handle);
		Sleep(5);
	}
	return 0;
}


NIUSB3_CORE_API int SECReadActivationStatus(unsigned int *active, unsigned int *trial_counter, unsigned int *trial_expired, NI_HANDLE *handle)
{
	unsigned int TTT[20];
	uint32_t read_data;
	uint32_t valid_data;

	NI_USB3_ReadData(TTT, 18, RFA_ICAPDNA_BA, REG_ACCESS , 100, handle, &read_data, &valid_data);


	*active = TTT[3] & 0x01;
	*trial_counter = (TTT[3] >> 3) & 0xFFF;
	*trial_expired = !( (TTT[3] >> 2) & 0x01);
	return 0;
}


