#include <msp430.h>
#include <stdint.h>
#include <LibClock/clock.h>

#define instruction 0x00
#define caracter    BIT0
#define EN          BIT2
#define BT          BIT3

#define lcdAddress  0x27

void ledNACK(void){
    P1DIR |= BIT0;
    P1OUT |= BIT0;
}

void i2cConfig(uint8_t isMaster, uint8_t ownAddr, uint16_t baudRate_kHz){

    P3SEL |= (BIT1 | BIT0);             //P3.1 como SCL e P3.0 SDA
    P3REN |= (BIT1 | BIT0);
    P3OUT |= (BIT1 | BIT0);
    P3DIR &= ~(BIT1 | BIT0);

    UCB0CTL1 = UCSWRST;                 //Inicio da ConfiguraÁ„o

    UCB0CTL0 = UCMODE_3 | UCSYNC;
    if(isMaster){
        UCB0CTL0 |= UCMST;
        UCB0CTL1 |= UCSSEL__SMCLK;          //Selecionar SM clock @1MHz
        UCB0BRW = 1000/baudRate_kHz;
    }
    UCB0I2COA = ownAddr;            //UCBxI2COA = endere√ßo da interface em quest√£o
    UCB0CTL1 &= ~UCSWRST;
}

uint8_t i2cWrite (uint8_t slaveAddr, uint8_t * data, uint16_t nBytes){
    UCB0I2CSA = slaveAddr;                  //UCBxI2CSA = endere√ßo do escravo
    UCB0CTL1 |= (UCTR | UCTXSTT);           //Flag de transmission e da start

    while(!(UCB0IFG & UCTXIFG));        //Espera esvaziar o buffer
    UCB0TXBUF = *data++;                //Coloca os dados no buffer
    nBytes--;

    //while(*pont_addr & UCTXSTT);          //Setamos o start, porÈm o bit sÛ ser· resetado quando o start e a transmiss„o  ocorrerem
    while(!(UCB0IFG & UCTXIFG) && !(UCB0IFG & UCNACKIFG));

    if(UCB0IFG & UCNACKIFG){                //Se receber NACK
        UCB0CTL1 |= UCTXSTP;                //Seta o Stop
        while(UCB0CTL1 & UCTXSTP);          //Espera o Stop zerar
        return 1;                           //Retorna 1 sinalizando NACK
    }

    while(nBytes--){
        while(!(UCB0IFG & UCTXIFG));      //Espera esvaziar o buffer
        UCB0TXBUF = *data++;                        //Coloca os dados no buffer
    }

    while(!(UCB0IFG & UCTXIFG));  //Espera esvaziar o buffer pela √∫ltima vez
    UCB0IFG &= ~UCTXIFG;          //Se a fun√ß√£o for chamada de novo, a flag j√° estar√° zerada

    UCB0CTL1 |= UCTXSTP;                    //Seta o Stop
    while(UCB0CTL1 & UCTXSTP);              //Espera o stop zerar

    return 0;                               //Retorna 0 sinalizando ACK
}

void i2cWriteByte(uint8_t slaveAddr, uint8_t data){
    if(i2cWrite(slaveAddr, &data, 1)){
        ledNACK();
    }
}

void lcdWriteNibble(uint8_t nibble, uint8_t RS){
    i2cWriteByte(lcdAddress, (nibble<<4) |BT|0 |0|RS);
    i2cWriteByte(lcdAddress, (nibble<<4) |BT|EN|0|RS);
    i2cWriteByte(lcdAddress, (nibble<<4) |BT|0 |0|RS);
}

void lcdWriteByte(uint8_t byte, uint8_t RS){
    lcdWriteNibble(byte>>4, RS);
    lcdWriteNibble(byte & 0x0F, RS);
}

void delay_time(uint32_t time_us){
    TA2CTL = TASSEL__SMCLK | MC__CONTINUOUS| TACLR;
    TA2CCR2 = time_us - 1;
    while(!(TA2CTL & TAIFG));
    TA2CTL = MC__STOP;
}

void lcdInit(void){
    lcdWriteNibble(0x3, instruction);
    lcdWriteNibble(0x3, instruction);
    lcdWriteNibble(0x3, instruction);       //Garante o lcd no modo 8bits

    lcdWriteNibble(0x2, instruction);       //Lcd no modo 4bits

    lcdWriteByte(0x28, instruction);
    lcdWriteByte(0x14, instruction);        //Cursor move para direita
    lcdWriteByte(0x08, instruction);
    lcdWriteByte(0x06, instruction);        //Desloca o caracter para a direita e mantem o display fixo
    lcdWriteByte(0x0C, instruction);        //ConfiguraÁ„o de display e cursor
    lcdWriteByte(0x01, instruction);        //Limpar display
    delay_time(1600);
}

void lcdWriteString(char vetor[]){
    int i = 0;
    char c = vetor[i++];
    while(c != '\0'){
        lcdWriteByte(c, 1);
        c = vetor[i++];
    }
}
