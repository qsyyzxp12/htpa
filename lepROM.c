//
// Created by hamlet on 6/1/16.
//

#include "lepROM.h"

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int m_iDevice = -1;
unsigned char m_key[8];

#define SNX_I2C_R8D8_MODE	(0)
#define SNX_I2C_R8D16_MODE	(1)
#define SNX_I2C_R16D8_MODE	(2)
#define SNX_I2C_R16D16_MODE	(3)

#define PARAM_A_ADDR (0x10)
#define PARAM_B_ADDR (0x20)

static int snx_i2c_open(char *dev)
{
    int fd = -1;
    fd = open(dev, O_RDWR);
    if (fd < 0)
    {
        printf("open %s failed\n", dev);
        //exit(1);
    }
    else
        printf("open %s ok\n", dev);
    return fd;
}

static int snx_i2c_close(int fd)
{
        return close(fd);
}

static int snx_i2c_burst_write(int fd, int chip_addr, int start_addr, int len, void *data, int mode)
{
    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data ioctl_data;
    int ret = 0;
    int i;
    __u8 *val;
    
    if(len <= 0) {
        ret = -1;
        printf("[SNX_I2C] Wrong len (%d)\n", len);
        return ret;
    }

    val = (__u8 *) malloc(sizeof(__u8) * (len + 1) * 2);

    memset(val, 0x0, (sizeof(__u8) * (len + 1) * 2));

    msgs[0].addr = chip_addr;
    msgs[0].flags = 0;

    switch (mode)
    {
        case SNX_I2C_R8D8_MODE:
            msgs[0].len = 1 + len;
            msgs[0].buf = val;
            val[0] = start_addr;
            for (i = 0; i < len; i++) {
                __u8 *new_data = (__u8 *) data;
                val[i+1] = new_data[i];
            }
            break;

        case SNX_I2C_R8D16_MODE:
            msgs[0].len = 1 + (len * 2);
            msgs[0].buf = val;
            val[0] = start_addr;
            for (i = 0; i < len; i++) {
                __u16 *new_data = (__u16 *) data;
                val[i+1] = (__u8)(new_data[i] >> 8);
                val[i+2] = (__u8)(new_data[i] & 0xff);
            }
            break;

        case SNX_I2C_R16D8_MODE:
            msgs[0].len = 2 + len;
            msgs[0].buf = val;
            val[0] = (__u8)(start_addr >> 8);
            val[1] = (__u8)(start_addr & 0xff);

            for (i = 0; i < len; i++) {
                __u8 *new_data = (__u8 *) data;
                val[i+2] = new_data[i];
            }
            break;

        case SNX_I2C_R16D16_MODE:
            msgs[0].len = 2 + (len * 2);
            msgs[0].buf = val;
            val[0] = (__u8)(start_addr >> 8);
            val[1] = (__u8)(start_addr & 0xff);
            for (i = 0; i < len; i++) {
                __u16 *new_data = (__u16 *) data;
                val[i+1] = (__u8)(new_data[i] >> 8);
                val[i+2] = (__u8)(new_data[i] & 0xff);
            }
            break;
        default:
            printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
            break;

    }
    
    
    ioctl_data.nmsgs = 1;
    ioctl_data.msgs = msgs;

#ifdef DEBUG

    printf("----- Writing Data -------\n\n");
    printf("-- \tchipaddr: 0x%x, reg: 0x%x\n", chip_addr, start_addr);
    for (i = 0; i < msgs[0].len ; i ++) {
        printf("-- \t msgs.data[%d] = 0x%x\n", i, val[i]);
    }
#endif

    ret = ioctl(fd, I2C_RDWR, &ioctl_data);
    if (ret < 0) {
        printf("%s: ioctl return: %d\n", __func__, ret);
    }
    

    return ret;
}

static int snx_i2c_write(int fd, int chip_addr, int addr, int data, int mode)
{
    int ret = 0;
    ret = snx_i2c_burst_write(fd, chip_addr, addr, 1, &data, mode);

    return ret;
}

static int snx_i2c_burst_read(int fd, int chip_addr, int start_addr, int len, void *data, int mode)
{
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data ioctl_data;
    int ret = 0;
    int i;
    __u8 *val;

    if(len <= 0)
    {
        ret = -1;
        //printf("[SNX_I2C] Wrong len (%d)\n", len);
        return ret;
    }

    printf("[SNX_I2C] snx_i2c_burst_read start...\n");
    printf("[SNX_I2C] read at chip_addr: (%x)\n", chip_addr);
    printf("[SNX_I2C] read at start_addr: (%x)\n", start_addr);

    val = (__u8 *) malloc(sizeof(__u8) * len * 2);

    memset(val, 0x0, (sizeof(__u8) * len * 2));

    msgs[0].addr = chip_addr;
    msgs[0].flags = 0;

    switch (mode)
    {
        case SNX_I2C_R8D8_MODE:
        case SNX_I2C_R8D16_MODE:
            msgs[0].len = 1;
            msgs[0].buf = val;
            val[0] = start_addr;
            break;

        case SNX_I2C_R16D8_MODE:
        case SNX_I2C_R16D16_MODE:
            msgs[0].len = 2;
            msgs[0].buf = val;
            val[0] = (__u8)(start_addr >> 8);
            val[1] = (__u8)(start_addr & 0xff);
            break;
        default:
            //printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
            break;

    }

    // write
    printf("[SNX_I2C] Msg[0] buf...\n");   
    for(i = 0; i < msgs[0].len; ++i)
    {
    	printf("val[%d] : %d \n", i, val[i]);
    }

    msgs[1].addr = chip_addr;
    msgs[1].flags = I2C_M_RD;

    switch (mode)
    {
        case SNX_I2C_R8D8_MODE:
        case SNX_I2C_R16D8_MODE:

            msgs[1].len = len;
            msgs[1].buf = val;
            break;

        case SNX_I2C_R8D16_MODE:
        case SNX_I2C_R16D16_MODE:

            msgs[1].len = len * 2;
            msgs[1].buf = val;
            break;
        default:
            //printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
            break;

    }


    ioctl_data.nmsgs = 2;
    ioctl_data.msgs = msgs;
    ret = ioctl(fd, I2C_RDWR, &ioctl_data);
    if (ret < 0)
    {
        printf("%s: ioctl return: %d\n", __func__, ret);
    }

    for (i = 0; i < len; i++) {
        switch (mode)
        {
            case SNX_I2C_R8D8_MODE:
            case SNX_I2C_R16D8_MODE:

                {
                    __u8 *new_data = (__u8 *) data;

                    new_data[i] = (__u8)val[i];

                }
                break;

            case SNX_I2C_R8D16_MODE:
            case SNX_I2C_R16D16_MODE:
                {
                    __u16 *new_data = (__u16 *) data;

                    new_data[i] = (__u16)val[i*2];
                    new_data[i] = (new_data[i]  << 8 )| (__u16)val[ (i*2 + 1)];
                }
                break;
            default:
                //printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
                break;

        }

    }

    printf("[SNX_I2C] Msg[1] buf...\n");   
    for(i = 0; i < msgs[1].len; ++i)
    {
    	printf("val[%d] : %d \n", i, val[i]);
    }


    printf("[SNX_I2C] snx_i2c_burst_read End...\n");
    return ret;
}

static int snx_i2c_read(int fd, int chip_addr, int addr, int mode)
{
    int value = 0;

    snx_i2c_burst_read(fd, chip_addr, addr, 1, &value, mode);

    return value;
}
//////////////////////////////////////////////////////////////
int openEEDevice()
{
    int iRst = EEPROM_OK;

    m_key[0] = 0x60;
    m_key[1] = 0x35;
    m_key[2] = 0x71;
    m_key[3] = 0x09;
    m_key[4] = 0x85;
    m_key[5] = 0x27;
    m_key[6] = 0x65;
    m_key[7] = 0x24;

    m_iDevice = snx_i2c_open("/dev/i2c-1"/*, O_RDWR*/);
    //m_iDevice = open("/dev/i2c-0", O_RDWR);
    if (m_iDevice < 0)
        iRst = EEPROM_OPEN_FAIL;

    return iRst;
}

void closeEEDevice()
{
	if (m_iDevice >= 0)
    snx_i2c_close(m_iDevice);
}

int writeSerialNum(LEP_SYS_FLIR_SERIAL_NUMBER_T SerialNum)
{
    unsigned char *pSerialNum = (unsigned char*)(&SerialNum);
    unsigned char pEncode[8];
    
    encode(pSerialNum, (unsigned char *)pEncode, 8);    
    int iRst = writeToEEPRom((unsigned char)0x00, (unsigned char *)pEncode, (int) 8);
    return iRst;
}

int checkSerialNum(LEP_SYS_FLIR_SERIAL_NUMBER_T SerialNum)
{
    int iRst = EEPROM_OK;
    unsigned long long *pSerialNum = &SerialNum;
    unsigned char *pSerialPtr = (unsigned char*)(pSerialNum);
    unsigned char pEncode[8];

    encode(pSerialPtr, (unsigned char *)pEncode, 8);

    unsigned char pEEPRom[8];
    iRst = readFromEEPRom((unsigned char)0x00, (unsigned char *)pEEPRom, 8);

    int i;
    for(i = 0; i < 8; ++i)
    {
        if (pEncode[i] != pEEPRom[i])
        {
            iRst = EEPROM_SERIAL_WRONG;
            break;
        }
    }

    return iRst;
}

int readParameter(int param, int *pValue)
{
    unsigned char regAddr = PARAM_A_ADDR;
    if (param == PARAM_B)
        regAddr = PARAM_B_ADDR;

    unsigned char pEEPRom[4];
    int iRst = readFromEEPRom(regAddr, pEEPRom, 4);

    int i=0;
    for (i=0;i<4;i++) {
        printf("val[%d] = %d\n", i, pEEPRom[i]);
    }

    if (iRst == EEPROM_OK)
    {
        int *pAddr = (int *) pEEPRom;
        *pValue = *pAddr;
    }
    return  iRst;

}

int writeParameter(int param, int value)
{
    unsigned char regAddr = PARAM_A_ADDR;
    if (param == PARAM_B)
        regAddr = PARAM_B_ADDR;
    printf("write regAddr: 0x%x\n", regAddr);

    unsigned char *pEEPRom;

    pEEPRom = (unsigned char *)&value;
    printf("write value: 0x%x\n", pEEPRom[0]);
    int iRst = writeToEEPRom(regAddr, pEEPRom, 4);

    return iRst;
}

void encode(unsigned char *pSrc, unsigned char *pDst, int byteToEncode)
{
    int i, j;
    unsigned char *pTmp = (unsigned char*) malloc(byteToEncode);
    for(i = 0; i < byteToEncode; ++i)
    {
        unsigned char iKey = m_key[i % 8];
        pTmp[i] = pSrc[i] % iKey;
    }

    for(i = 0; i < byteToEncode; ++i)
    {
        int iValue = pTmp[i];
        for(j = 0; j < i; ++j)
        {
            iValue += pTmp[j] * j;
        }

        pDst[i] = iValue % m_key[i % 8];
    }

    free(pTmp);
}

int writeToEEPRom(unsigned char regAddr, unsigned char *pData, int byteToWrite)
{
    int iRst = EEPROM_OK;

    if (m_iDevice == -1)
        return EEPROM_OPEN_FAIL;

    //int i;
    //unsigned char* txdata = (unsigned char*)malloc(byteToWrite + 1);
    //txdata[0] = regAddr;
    //for(i = 0; i < byteToWrite; ++i)
    //    txdata[i + 1] = pData[i];

    //int iWrited = write(m_iDevice, txdata, byteToWrite + 1);
	int iWrited = snx_i2c_burst_write(m_iDevice, EEPROM_DEVICE_ADDR, regAddr, byteToWrite, pData, SNX_I2C_R8D8_MODE);
    if (iWrited < 0)
        iRst = EEPROM_WRITE_FAIL;

    //free(txdata);

    return iRst;
}

int readFromEEPRom(unsigned char regAddr, unsigned char *pData, int byteToRead)
{
    int iRst = EEPROM_OK;

    if (m_iDevice == -1)
        return EEPROM_OPEN_FAIL;

    //unsigned char* txdata = (unsigned char*)malloc(1);
    //unsigned char* rxdata = (unsigned char*)malloc(byteToRead);

    //int i;
    //txdata[0] = regAddr;
    //int iWrited = write(m_iDevice, txdata, 1);
    //if (iWrited < 0)
    //    iRst = EEPROM_WRITE_FAIL;
    //else
    //{
        int iReaded = snx_i2c_burst_read(m_iDevice, EEPROM_DEVICE_ADDR, regAddr, byteToRead, pData, SNX_I2C_R8D8_MODE);
        if (iReaded < 0)
            iRst = EEPROM_READ_FAIL;
        //else
        //{
        //    for(i = 0; i < byteToRead; ++i)
        //        pData[i] = rxdata[i];
        //}
    //}

    //free(txdata);
    //free(rxdata);

    return iRst;
}



/**/
