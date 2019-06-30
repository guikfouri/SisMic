#include <msp430.h> 
#include <stdint.h>
#include <LibClock/clock.h>

void uartInit(uint32_t baudRate, uint8_t isMSB, uint8_t par, uint8_t nStops);
uint8_t uartGet(char * data);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	clockInit();

	P4DIR |= BIT7;
	P4OUT &= ~BIT7;

	uartInit(9600, 0, 0, 1);

	char string[10] = {2};

	if(uartGet(string)){
	    P4OUT ^= BIT7;
	}
	
	while(1);

	return 0;
}

void uartInit(uint32_t baudRate, uint8_t isMSB, uint8_t par, uint8_t nStops){
    UCA0CTLW0 = UCSWRST;                    //Reset que habilita as configura��es
                                            //O registro de CTLW agrupa CTL0 e CTL1, onde CTL1 � o LSByte

    UCA0CTLW0 |= (isMSB ? BITD : 0) |
                 (par ? BITF : 0) |
                 (par == 1 ? BITE : 0) |
                 (nStops == 2 ? BITB : 0) |         //At� aqui, s�o registros do CTL1
                 UCMODE_0 |                 //Modo UART simples: 1 tx e 1 rx
                 UCSYNC;

    uint32_t brDiv = (1000000 << 3)/baudRate;       //Ao fazer a rota��o antes da divis�o, obt�m-se um resultado que contempla as 3 casas fracion�rias nos bits menos significativos
    UCA0BRW = brDiv >> 3;                   //Parte inteira
    UCA0MCTL = (brDiv & 0x7) << 1;          //Filtra a parte fracion�ria e posiciona corretamente no registro
                                            //Modo oversamplig desligado;

    P3SEL |= BIT4;                          //Utiliza o pino dedicado para recep��o do UCA0

    UCA0CTLW0 &= ~UCSWRST;                  //Fim da configura��o
}

uint8_t uartGet(char * data){
    TA1CTL = TASSEL__ACLK | MC__CONTINUOUS | ID__4 | TACLR;     //Espera at� 8 segundos para a primeira comunica��o
    while(!(UCA0IFG & UCRXIFG) && !(TA1CTL & TAIFG));

    if(TA1CTL & TAIFG)
        return 1;                           //Se se passaram 8s e n�o foi recebido nenhum dado, retorna -1

    *data++ = UCA0RXBUF;                    //Leu o primeiro elemento

    while((*data != '\n') || (*data != '\0') || (*data != '\r')){
        while(!(UCA0IFG & UCRXIFG));
        *data++ = UCA0RXBUF;                //Continua lendo os dados recebidos
    }

    TA1CTL = MC__STOP;

    return 0;                               //Indica que a leitura foi feita adequadamente;
}

