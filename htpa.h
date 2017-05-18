#define CONFIG_AND_SENSOR_ADDR 	0x34 >> 1
#define EEPROM_ADDRESS 			0xA0 >> 1
#define DELAY_BETWEEN_WRITE	 	10000
#define READ_BUFF_LEN 			32
#define WRITE_BUFF_LEN 			32
#define WRITE_BIT 				0

typedef enum{SUCCESS, ERROR}STATUS;
/*
struct i2c_msg
{
	unsigned short addr;
	unsigned short flags;
#define I2C_M_TEN 	0x0010
#define I2C_M_RD 	0x0001
	unsigned short len;
	unsigned char *buf;
};*/

void free_memory();
STATUS open_i2c();
STATUS start_i2c();
STATUS wake_up_i2c();
STATUS write_EEPROM(uint16_t addr_offset, uint8_t* str, short len);
STATUS read_EEPROM(uint16_t addr_offset, short len);
STATUS write_bytes(int fd, uint8_t* str, short len);
STATUS read_bytes(int fd, uint8_t* buff, short len);
