// Il seguente blocco ifdef rappresenta il modo standard di creare macro che semplificano 
// l'esportazione da una DLL. Tutti i file all'interno della DLL sono compilati con il simbolo NIUSB3_CORE_EXPORTS
// definito nella riga di comando. Questo simbolo non deve essere definito in alcun progetto
// che utilizza questa DLL. In questo modo qualsiasi altro progetto i cui file di origine includono questo file vedranno le funzioni 
// NIUSB3_CORE_API come importate da una DLL, mentre la DLL vedr� i simboli
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


//OFFSET ICAPDNA
#define RFA_ICAPDNA_BA					0xFFFA0000
#define RFA_ICAPDNA_UID1				RFA_ICAPDNA_BA + 0x04
#define RFA_ICAPDNA_UID2				RFA_ICAPDNA_BA + 0x05
#define RFA_ICAPDNA_CHECK1				RFA_ICAPDNA_BA + 0x08
#define RFA_ICAPDNA_CHECK2				RFA_ICAPDNA_BA + 0x09
#define RFA_ICAPDNA_CHECK3				RFA_ICAPDNA_BA + 0x0A
#define RFA_ICAPDNA_CHECK4				RFA_ICAPDNA_BA + 0x0B
#define RFA_ICAPDNA_CHECK5				RFA_ICAPDNA_BA + 0x0C
#define RFA_ICAPDNA_CHECK6				RFA_ICAPDNA_BA + 0x0D
#define RFA_ICAPDNA_CHECK7				RFA_ICAPDNA_BA + 0x0E
#define RFA_ICAPDNA_CHECK8				RFA_ICAPDNA_BA + 0x0F
#define RFA_ICAPDNA_WF					RFA_ICAPDNA_BA + 0x00
#define RFA_ICAPDNA_EEPROM				RFA_ICAPDNA_BA + 0x01
#define RFA_ICAPDNA_READEEPROM			RFA_ICAPDNA_BA + 0x07
#define RFA_ICAPDNA_SN					RFA_ICAPDNA_BA + 0x11
#define RFA_ICAPDNA_INFO				RFA_ICAPDNA_BA + 0x10



enum USB_CONNECTION_STATUS
{
	NOT_CONNECTED = 0,
	CONNECTED = 1
} ;


typedef struct NI_HANDLE
{
	void  *device_handle;
	uint32_t board_identifier;
	uint32_t version;
	USB_CONNECTION_STATUS connection_status;
} NI_HANDLE;


typedef struct NI_IIC_HANDLE
{
	NI_HANDLE *dev_con;
	uint32_t __IICBASEADDRESS;
	uint32_t __IICBASEADDRESS_STATUS;
} NI_IIC_HANDLE;


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
NIUSB3_CORE_API int NI_USB3_GetHV(bool *Enable, float *voltage, float *current, NI_HANDLE *handle);
NIUSB3_CORE_API int NI_USB3_SetOffset(bool top, uint32_t DACCODE, NI_HANDLE *handle);
NIUSB3_CORE_API int NI_USB3_SetImpedance(bool R50, NI_HANDLE *handle);

NIUSB3_CORE_API char *ReadFirmwareInformationApp(	NI_HANDLE *handle);


NIUSB3_CORE_API int SECReadActivationStatus(unsigned int *active, unsigned int *trial_counter, unsigned int *trial_expired, NI_HANDLE *handle);
NIUSB3_CORE_API int SECWritekey(int *key, int length, NI_HANDLE *handle);
NIUSB3_CORE_API int SECWriteSN(unsigned int SN, NI_HANDLE *handle);
NIUSB3_CORE_API int SECReadUIDSN(uint64_t *UID, uint32_t *SN, NI_HANDLE *handle);

NIUSB3_CORE_API int NI_USB3_IICUser_OpenController(uint32_t ControlAddress, uint32_t StatusAddress, NI_HANDLE *handle, NI_IIC_HANDLE *IIC_Handle);
NIUSB3_CORE_API int NI_USB3_IICUser_ReadData(uint8_t address, uint8_t *value, int len, uint8_t *value_read, int len_read, NI_IIC_HANDLE *IIC_Handle);
NIUSB3_CORE_API int NI_USB3_IICUser_WriteData(uint8_t address, uint8_t *value, int len, NI_IIC_HANDLE *IIC_Handle);

NIUSB3_CORE_API int NI_USB3_SetIICControllerBaseAddress(uint32_t ControlAddress, uint32_t StatusAddress, NI_HANDLE *handle);

NIUSB3_CORE_API int NI_USB3_GetDT5550_DGBoardInfo(char *model, int *asic_count, int *SN, NI_HANDLE *handle);
NIUSB3_CORE_API int NI_USB3_SetDT5550_DGBoardInfo(char *model, int asic_count, int SN, NI_HANDLE *handle);

NIUSB3_CORE_API int NI_USB3_GetDT5550WTempSens(int address, float *temp, NI_HANDLE *handle);


NIUSB3_CORE_API int NI_USB3_AllocHandle(NI_HANDLE **handle);

#ifdef NIUSB3_CORE_EXPORTS
	uint32_t __IICBASEADDRESS;
	uint32_t __IICBASEADDRESS_STATUS;

#else
#endif