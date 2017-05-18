//
// Created by hamlet on 6/1/16.
//

#ifndef AMOBILEEXAMPLE_LEPROM_H
#define AMOBILEEXAMPLE_LEPROM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "LEPTON_Types.h"
#include "LEPTON_SDK.h"
#include "LEPTON_SYS.h"

#define EEPROM_OK           1
#define EEPROM_OPEN_FAIL    -1
#define EEPROM_IOCTL_FAIL   -2
#define EEPROM_READ_FAIL    -3
#define EEPROM_WRITE_FAIL   -4
#define EEPROM_SERIAL_WRONG -5

#define EEPROM_DEVICE_ADDR  (0xA0)

#define PARAM_A             1
#define PARAM_B             2

extern int openEEDevice();
extern void closeEEDevice();
extern int writeSerialNum(LEP_SYS_FLIR_SERIAL_NUMBER_T SerialNum);
extern int checkSerialNum(LEP_SYS_FLIR_SERIAL_NUMBER_T SerialNum);
extern void encode(unsigned char *pSrc, unsigned char *pDst, int byteToEncode);
extern int writeToEEPRom(unsigned char regAddr, unsigned char *pData, int byteToWrite);
extern int readFromEEPRom(unsigned char regAddr, unsigned char *pData, int byteToRead);
extern int readParameter(int param, int *pValue);
extern int writeParameter(int param, int value);

#ifdef __cplusplus
}
#endif

#endif //AMOBILEEXAMPLE_LEPROM_H
