#include<stdio.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<linux/types.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<stdint.h>

#include"htpa.h"


int i2c_fd;
uint8_t* read_buff;
uint8_t* write_buff;

int main()
{
	read_buff = malloc(READ_BUFF_LEN);
	write_buff = malloc(WRITE_BUFF_LEN);
	bzero(write_buff, WRITE_BUFF_LEN);
	bzero(read_buff, READ_BUFF_LEN);

	if(open_i2c() == ERROR)
	{
		perror("open_i2c()");
		free_memory();
		return 1;
	}

	if(wake_up_i2c() == ERROR)
	{
		perror("wake_up_i2c()");
		return 1;
	}		

	if(start_i2c() == ERROR)
	{
		perror("start_i2c()");
		return 1;
	}
/*
	//Write to and Read from EEPROM
	if(ioctl(i2c_fd, I2C_SLAVE, EEPROM_ADDRESS) < 0)
		return ERROR;

	bzero(read_buff, 3);
	write_EEPROM(0x0000, "END", 3);
	read_EEPROM(0x0000, 3);
	printf("%s\n", read_buff);
*/
	free_memory();
	return 0;
}

void free_memory()
{
	close(i2c_fd);
	free(read_buff);	
	free(write_buff);
}

STATUS i2c_read(int len, uint8_t cmd, uint8_t addr)
{
	struct i2c_rdwr_ioctl_data ioctl_data;
	struct i2c_msg msg[2];
	ioctl_data.nmsgs = 2;
	ioctl_data.msgs = msg;

	(ioctl_data.msgs[0]).len = sizeof(cmd);
	(ioctl_data.msgs[0]).addr = addr;
	(ioctl_data.msgs[0]).flags = 0;
	(ioctl_data.msgs[0]).buf = &cmd;
	(ioctl_data.msgs[1]).len = 1;
	(ioctl_data.msgs[1]).addr = addr;
	(ioctl_data.msgs[1]).flags = I2C_M_RD;
	(ioctl_data.msgs[1]).buf = read_buff;

	int ret = ioctl(i2c_fd, I2C_RDWR, &ioctl_data);
	if(ret < 0)
	{
		perror("i2c_read/ioctl()");
		return ERROR;
	}
	printf("read_buff = 0x%X\n", read_buff[0]);
	return SUCCESS;
}

STATUS i2c_write(int len, uint8_t* str, uint8_t addr)
{
	struct i2c_rdwr_ioctl_data ioctl_data;
	struct i2c_msg msg[1];
	ioctl_data.nmsgs = 1;
	ioctl_data.msgs = msg;

	(ioctl_data.msgs[0]).len = len;
	(ioctl_data.msgs[0]).addr = addr;
	(ioctl_data.msgs[0]).flags = 0;
	(ioctl_data.msgs[0]).buf = str;
	
	int ret = ioctl(i2c_fd, I2C_RDWR, &ioctl_data);
	if(ret < 0)
	{
		perror("i2c_write/ioctl()");
		return ERROR;
	}
	usleep(DELAY_BETWEEN_WRITE);
}

STATUS start_i2c()
{
	uint8_t write_buff[2] = {0};
	write_buff[0] = 0x02;
	write_buff[1] = 0x09;
	i2c_write(2, write_buff, CONFIG_AND_SENSOR_ADDR);

	i2c_read(1, 0x02, CONFIG_AND_SENSOR_ADDR);

	usleep(80000);

	i2c_read(1, 0x02, CONFIG_AND_SENSOR_ADDR);
}

STATUS wake_up_i2c()
{
	uint8_t write_buff[2] = {0};

	//Turn On 						{0x01, 0x01}
	write_buff[0] = 0x01;
	write_buff[1] = 0x01;
	i2c_write(2, write_buff, CONFIG_AND_SENSOR_ADDR);
//	printf("Turn On\n");

	//ADC resolution 				{0x04, 0x0c}
	write_buff[0] = 0x03;
	write_buff[1] = 0x0c;
	i2c_write(2, write_buff, CONFIG_AND_SENSOR_ADDR);
//	printf("ADC resolution\n");


	//Top ADC bias current			{0x04, 0x0c}
	write_buff[1] = 0x04;
	write_buff[2] = 0x0c;
	i2c_write(2, write_buff, CONFIG_AND_SENSOR_ADDR);
//	printf("Top ADC bias current\n");

	//Button ADC bias current		{0x05, 0x0c}
	write_buff[1] = 0x05;
	write_buff[2] = 0x0c;
	i2c_write(2, write_buff, CONFIG_AND_SENSOR_ADDR);
//	printf("Button ADC bias current\n");

	//Clock frequence				{0x06, 0x0c}
	write_buff[1] = 0x06;
	write_buff[2] = 0x14;
	i2c_write(2, write_buff, CONFIG_AND_SENSOR_ADDR);
//	printf("Clock frequence\n");

	//Top V of preamplifier			{0x07, 0x0c}
	write_buff[1] = 0x07;
	write_buff[2] = 0x0c;
	i2c_write(2, write_buff, CONFIG_AND_SENSOR_ADDR);
//	printf("Top V of preamplifier\n");

	//Button V of preamplifier		{0x08, 0x0c}
	write_buff[1] = 0x08;
	write_buff[2] = 0x0c;
	i2c_write(2, write_buff, CONFIG_AND_SENSOR_ADDR);
//	printf("Button V of preamplifier\n");

	//Pull up resistor				{0x09, 0x88}
	write_buff[1] = 0x09;
	write_buff[2] = 0x88;
	i2c_write(2, write_buff, CONFIG_AND_SENSOR_ADDR);

	return SUCCESS;
}

STATUS open_i2c()
{
	i2c_fd = open("/dev/i2c-1", O_RDWR);
	if(i2c_fd < 0)
		return ERROR;
	else if(ioctl(i2c_fd, I2C_SLAVE, CONFIG_AND_SENSOR_ADDR) < 0)
		return ERROR;
	
	return SUCCESS;
}

STATUS write_bytes(int fd, uint8_t* str, short len)
{
	uint8_t* rest_str = str;
	while(len > 0)
	{
		int write_len = write(fd, rest_str, len);
		if(write_len < 0)
			return ERROR;
		len -= write_len;
		rest_str += write_len;
	}
}

STATUS read_bytes(int fd, uint8_t* buff, short len)
{
	bzero(read_buff, READ_BUFF_LEN);
	while(len)
	{
		int read_len = read(fd, buff, len);
		if(read_len < 0)
			return ERROR;
		len -= read_len;
		buff += read_len;
	}
	return SUCCESS;
}

STATUS write_EEPROM(uint16_t addr_offset, uint8_t* str, short len)
{	
	uint8_t addr_offset_high_3bit = (addr_offset >> 8) & 0x07; //0000 0111
	uint8_t addr_offset_low_bit = (uint8_t)addr_offset;

	bzero(write_buff, len + 2);
	write_buff[0] = (EEPROM_ADDRESS + addr_offset_high_3bit) << 1 + WRITE_BIT;
	write_buff[1] = addr_offset_low_bit;
	memcpy(write_buff+2, str, len);

	int write_bytes = write(i2c_fd, write_buff, len + 2);
	if(write_bytes < 0)
		return ERROR;
	usleep(DELAY_BETWEEN_WRITE);
}

STATUS read_EEPROM(uint16_t addr_offset, short len)
{
	uint8_t addr_offset_high_3bit = (uint8_t)((addr_offset >> 8) & 0x07); //0000 0111
	uint8_t addr_offset_low_bit = (uint8_t)addr_offset;
	
	uint8_t control_bytes[2] = {(EEPROM_ADDRESS + addr_offset_high_3bit) << 1 + WRITE_BIT, addr_offset_low_bit};
	if(write(i2c_fd, control_bytes, 2) < 0)
		return ERROR;

	bzero(read_buff, READ_BUFF_LEN);	
	if(read_bytes(i2c_fd, read_buff, len) == ERROR)
		return ERROR;
	
	return SUCCESS;			
}

