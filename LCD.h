void ledNACK(void);
void i2cConfig(uint8_t isMaster, uint8_t ownAddr, uint16_t baudRate_kHz);
uint8_t i2cWrite (uint8_t slaveAddr, uint8_t * data, uint16_t nBytes);
void i2cWriteByte(uint8_t slaveAddr, uint8_t data);
void lcdWriteNibble(uint8_t nibble, uint8_t RS);
void lcdWriteByte(uint8_t byte, uint8_t RS);
void delay_time(uint32_t time_us);
void lcdInit(void);
void lcdWriteString(char vetor[]);
