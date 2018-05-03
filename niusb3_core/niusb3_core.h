// Il seguente blocco ifdef rappresenta il modo standard di creare macro che semplificano 
// l'esportazione da una DLL. Tutti i file all'interno della DLL sono compilati con il simbolo NIUSB3_CORE_EXPORTS
// definito nella riga di comando. Questo simbolo non deve essere definito in alcun progetto
// che utilizza questa DLL. In questo modo qualsiasi altro progetto i cui file di origine includono questo file vedranno le funzioni 
// NIUSB3_CORE_API come importate da una DLL, mentre la DLL vedrà i simboli
// definiti con questa macro come esportati.
#pragma once
#ifdef NIUSB3_CORE_EXPORTS
#define NIUSB3_CORE_API extern "C" __declspec(dllexport)
#else
#define NIUSB3_CORE_API extern "C" __declspec(dllimport)
#endif

#include <stdlib.h>
#include <stdint.h>


#define FLASH_PAGE_SIZE			512
#define FLASH_SECTOR_SIZE   0x40000

#define FIRMWARE_BL 				0
#define FIRMWARE_BA 				0x400000
#define FIRMWARE_LEN 				0x800000 - FLASH_SECTOR_SIZE
#define FIRMWARE_FIRMWARE_INFO_BA 	FIRMWARE_BA + FIRMWARE_LEN
#define FIRMWARE_FIRMWARE_INFO_LEN  FLASH_SECTOR_SIZE
#define FIRMWARE_FIRMWARE_PARAM		0xC00000
#define FIRMWARE_PARAMSIZE			0x400000

#define FLASH_CONTROLLER_BA 0xFFFE0000
#define FLASH_CONTROLLER_REG_ADDR 0xF001
#define FLASH_CONTROLLER_REG_CNTRL 0xF000
#define FLASH_CONTROLLER_REG_MAGIC 0xFFFF


enum USB_CONNECTION_STATUS
{
	NOT_CONNECTED = 0,
	CONNECTED = 1
} ;


typedef struct NI_HANDLE
{
	void  *device_handle;
	USB_CONNECTION_STATUS connection_status;
} NI_HANDLE;

enum USB_BUS_MODE
{
	REG_ACCESS = 0,
	STREAMING = 1
} ;

NIUSB3_CORE_API extern int nniusb3_core;

NIUSB3_CORE_API int fnniusb3_core(void);
NIUSB3_CORE_API int NI_USB3_ListDevices(char *ListOfDevice, char *model,  int *Count);
NIUSB3_CORE_API int NI_USB3_ConnectDevice(char *serial_number, NI_HANDLE *handle);
NIUSB3_CORE_API int NI_USB3_CloseConnection(NI_HANDLE *handle);
NIUSB3_CORE_API int NI_USB3_Init();

NIUSB3_CORE_API int NI_USB3_WriteData(uint32_t *data, uint32_t count, 
										uint32_t address, USB_BUS_MODE bus_mode, 
										uint32_t timeout_ms, NI_HANDLE *handle, 
										uint32_t *written_data);
NIUSB3_CORE_API int NI_USB3_ReadData(uint32_t *data, uint32_t count, 
										uint32_t address, USB_BUS_MODE bus_mode, 
										uint32_t timeout_ms, NI_HANDLE *handle, 
										uint32_t *read_data, uint32_t *valid_data);
NIUSB3_CORE_API int NI_USB3_WriteReg(uint32_t data, uint32_t address, NI_HANDLE *handle);
NIUSB3_CORE_API int NI_USB3_ReadReg(uint32_t *data, uint32_t address, NI_HANDLE *handle);


NIUSB3_CORE_API int NI_USB3_SetHV(bool Enable, float voltage, NI_HANDLE *handle);
NIUSB3_CORE_API int NI_USB3_SetOffset(bool top, uint32_t DACCODE, NI_HANDLE *handle);
NIUSB3_CORE_API int NI_USB3_SetImpedance(bool R50, NI_HANDLE *handle);

NIUSB3_CORE_API char *ReadFirmwareInformationApp(	NI_HANDLE *handle);

#ifdef NIUSB3_CORE_EXPORTS
uint32_t __IICBASEADDRESS;
uint32_t __IICBASEADDRESS_STATUS;

#else
#endif