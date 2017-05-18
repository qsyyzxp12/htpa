#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
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

	//Use for loop to test all address between 0x00 and 0xA0
	uint8_t addr;
	for(addr=0x00; addr<0xA0; addr++)
	{
		printf("Addr:");
		printf(" 0x%X\t", addr);
		if(ioctl(i2c_fd, I2C_SLAVE, addr >> 1) < 0)
		{
			perror("ioctl()");
			return 1;
		}

		if(wake_up_i2c(addr) == ERROR)
		{
			fflush(stdout);
			perror("wake_up_i2c()");
			continue;
		}
		else
			printf("wake_up_i2c(): SUCCESS\t");
			

		if(start_i2c(addr) == ERROR)
		{
			fflush(stdout);
			perror("start_i2c()");
			continue;
		}
		else
		{
			printf("start_i2c(): SUCCESS\n");
			sleep(2);
		}
	}

	//Write to and Read from EEPROM
	if(ioctl(i2c_fd, I2C_SLAVE, EEPROM_ADDRESS) < 0)
		return ERROR;

	bzero(read_buff, 3);
	write_EEPROM(0x0000, "END", 3);
	read_EEPROM(0x0000, 3);
	printf("%s\n", read_buff);

	free_memory();
	return 0;
}

void free_memory()
{
	close(i2c_fd);
	free(read_buff);	
	free(write_buff);
}

STATUS start_i2c(uint8_t address)
{
	bzero(write_buff, 3);
	//start
	write_buff[0] = address << 1 + WRITE_BIT;
	write_buff[1] = 0x01;
	write_buff[2] = 0x09;
	if(write(i2c_fd, write_buff, 3) < 0)
		return ERROR;
//	printf("Start\n");

	usleep(DELAY_BETWEEN_WRITE);

	//Send read command
	write_buff[1] = 0x02;
	if(write(i2c_fd, write_buff, 2) < 0)
		return ERROR;
//	printf("Read command sent\n");

//	usleep(DELAY_BETWEEN_WRITE);

	if(read(i2c_fd, read_buff, 2) >= 0)
		printf("read_buff = %s\n", read_buff);
	else
		return ERROR;

	usleep(50000);

	//Send second read command
	if(write(i2c_fd, write_buff, 2) < 0)
		return ERROR;
//	printf("Read command sent\n");
	
	//usleep(DELAY_BETWEEN_WRTIE);

	if(read(i2c_fd, read_buff, 2) >= 0)
		printf("read_buff = %s\n", read_buff);
	else
		return ERROR;

	return SUCCESS;
}

STATUS wake_up_i2c(uint8_t address)
{
	bzero(write_buff, 3);
	//Turn On 						{0x01, 0x01}
	write_buff[0] = address << 1 + WRITE_BIT;
	write_buff[1] = 0x01;
	write_buff[2] = 0x01;
	if(write(i2c_fd, write_buff, 3) < 0)
		return ERROR;
//	printf("Turn On\n");	

	usleep(DELAY_BETWEEN_WRITE);

	//ADC resolution 				{0x04, 0x0c}
	write_buff[1] = 0x03;
	write_buff[2] = 0x0c;
	if(write(i2c_fd, write_buff, 3) < 0)
		return ERROR;
//	printf("ADC resolution\n");

	usleep(DELAY_BETWEEN_WRITE);

	//Top ADC bias current			{0x04, 0x0c}
	write_buff[1] = 0x04;
	write_buff[2] = 0x0c;
	if(write(i2c_fd, write_buff, 3) < 0)
		return ERROR;
//	printf("Top ADC bias current\n");

	usleep(DELAY_BETWEEN_WRITE);

	//Button ADC bias current		{0x05, 0x0c}
	write_buff[1] = 0x05;
	write_buff[2] = 0x0c;
	if(write(i2c_fd, write_buff, 3) < 0)
		return ERROR;
//	printf("Button ADC bias current\n");

	usleep(DELAY_BETWEEN_WRITE);

	//Clock frequence				{0x06, 0x0c}
	write_buff[1] = 0x06;
	write_buff[2] = 0x14;
	if(write(i2c_fd, write_buff, 3) < 0)
		return ERROR;
//	printf("Clock frequence\n");

	usleep(DELAY_BETWEEN_WRITE);

	//Top V of preamplifier			{0x07, 0x0c}
	write_buff[1] = 0x07;
	write_buff[2] = 0x0c;
	if(write(i2c_fd, write_buff, 3) < 0)
		return ERROR;
//	printf("Top V of preamplifier\n");

	usleep(DELAY_BETWEEN_WRITE);

	//Button V of preamplifier		{0x08, 0x0c}
	write_buff[1] = 0x08;
	write_buff[2] = 0x0c;
	if(write(i2c_fd, write_buff, 3) < 0)
		return ERROR;
//	printf("Button V of preamplifier\n");

	usleep(DELAY_BETWEEN_WRITE);

	//Pull up resistor				{0x09, 0x88}
	write_buff[1] = 0x09;
	write_buff[2] = 0x88;
	if(write(i2c_fd, write_buff, 3) < 0)
		return ERROR;
//	printf("Pull up resistor\n");

	usleep(DELAY_BETWEEN_WRITE);
	return SUCCESS;
}

STATUS open_i2c()
{
	i2c_fd = open("/dev/i2c-1", O_RDWR);
	if(i2c_fd < 0)
		return ERROR;
	else if(ioctl(i2c_fd, I2C_SLAVE, CONFIG_AND_SENSOR_ADDRESS) < 0)
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

