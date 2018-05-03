#pragma once
#include "../niusb3_core/niusb3_core.h"


int IIC_WriteData(uint8_t address, uint8_t *value, int len, NI_HANDLE *handle);
int IIC_ReadData(uint8_t address, uint8_t *value, int len, uint8_t *value_read, int len_read, NI_HANDLE *handle);
int SetHV(bool Enable, float voltage, NI_HANDLE *handle);
int SetOffset(bool top, float voltage, NI_HANDLE *handle);
int SetImpedance(bool R50, NI_HANDLE *handle);