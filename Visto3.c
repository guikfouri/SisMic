#include <msp430.h> 
#include <stdint.h>
#include <LibClock/clock.h>
#include <LibLCD/LCD.h>

uint8_t isMaster = 1;
uint8_t updateLCD = 0;
uint32_t SumData[2] = {0}, dataOut[2], charOut[2];
uint8_t j = 0, i = 0;

void ConfigPorts(uint8_t A1, uint8_t A2);
void TimeSamples_ms(uint16_t tempo_centenas_de_milionesimo);
void atualizaLCD(uint32_t * dado);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    clockInit();

    i2cConfig(isMaster, 0x12, 200);         //Taxa de comunicação I²C à 200kHz
    lcdInit();

    ConfigPorts(0, 1);                      //Portas 1 e 0 selecionadas
    TimeSamples_ms(625);                    //Intervalo entre as amostragens de 62,5ms

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;                         //Configurando o LED para indicar a taxa de conversão

    __enable_interrupt();

    lcdWriteString("A0: ");                 //Escrve na primeira linha
    lcdWriteByte(0xC0, 0);                  //Leva o cursor para a segunda linha
    lcdWriteString("A1: ");

    while(1){
        while(!updateLCD);
        updateLCD = 0;
        dataOut[0] = SumData[0];
        dataOut[1] = SumData[1];
        atualizaLCD(dataOut);
        P1OUT ^= BIT0;                      //Inverte o LED após toda atualização do LCD
    }

    return 0;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA00ISR(){
    ADC12CTL0 |= ADC12ENC;
    ADC12CTL0 &= ~ADC12SC;
    ADC12CTL0 |= ADC12SC;                   //Gera o flanco de subida a cada contagem do timer de 62,5ms
}

#pragma vector = ADC12_VECTOR
__interrupt void ADCISR(){
    SumData[0] += ADC12MEM0;
    SumData[1] += ADC12MEM1;
    if(++i == 8){                           //Acumulando a os valores de 8 amostragens de cada canal para realizar a média
        updateLCD = 1;                      //Atualiza LCD quando as 8 medições foram realizadas
        i = 0;
    }
}

void ConfigPorts(uint8_t A1, uint8_t A2){
    ADC12CTL0 &= ~ADC12ENC;                 //Desabilita as conversões: necessário para realizar as configurações

    ADC12CTL0 |= ADC12SHT0_3 |              //Escolhendo a quantidade de batidas de clock que garante a amostragem: 32 batidas de clock;
                 ADC12ON;                   //Ligando o conversor

    ADC12CTL1 = ADC12CSTARTADD_0 |          //Escolhendo o buffer de memória zero.
                ADC12SHS_0 |                //O trigger será dado pelo registro 'SC'
                ADC12SHP |                  //Usar o timer interno do conversor
                ADC12SSEL_0 |               //Clock interno de 5MHz
                ADC12CONSEQ_1;              //Conversão de mais de um canal

    ADC12CTL2 = ADC12TCOFF | ADC12RES_2;    //Conversor de temperatura desligado e resolução de 12 bits

    ADC12MCTL0 = A1 & 0x7;                  //Indicando o canal de entrada
    ADC12MCTL1 = A2 & 0x7 | ADC12EOS;       //Segundo canal é o fim da sequência

                                            //As configurações de referência são deixadas no modo padrão;

    P6SEL |= 1 << (A1 & 0x7);               //Habilita o pino correspondente à porta escolhida.
    P6SEL |= 1 << (A2 & 0x7);

    ADC12IE = ADC12IE1;                     //Habilita interrupções oriundas do canal 1 de memória


    ADC12CTL0 |= ADC12ENC;                  //Configuração concluída
}

void TimeSamples_ms(uint16_t tempo_centenas_de_milionesimo){
    TA0CCTL0 = CCIE;
    TA0CCR0 = (tempo_centenas_de_milionesimo * 100) - 1;
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR;
}

void atualizaLCD(uint32_t * dado){
    SumData[0] = SumData[1] = 0;                        //Já permite que SumData receba os novos valores medidos

    while(j < 2){
        lcdWriteByte((0x84 + (j*0x40)), 0);             //Leva o cursor para a posição adequada a depender do canal.

        dado[j] *= 3300;
        dado[j] /= (4095*8);                            //Tira a média das medidas do canal e converte para mV

        charOut[j] = dado[j]/1000;
        charOut[j] += 0x30;
        lcdWriteByte(charOut[j], 1);

        lcdWriteByte(',', 1);

        charOut[j] = (dado[j]%1000)/100;
        charOut[j] += 0x30;
        lcdWriteByte(charOut[j], 1);

        charOut[j] = (dado[j]%100)/10;
        charOut[j] += 0x30;
        lcdWriteByte(charOut[j], 1);

        charOut[j] = (dado[j]%10);
        charOut[j] += 0x30;
        lcdWriteByte(charOut[j], 1);                //Aqui termina de printar em decimal: d,ddd

//// Tranformação pra hexadecimal ////

        lcdWriteByte((0x8E + (j*0x40)), 0);         //Leva o cursor para a posição do último byte do valor em hexadecimal

        charOut[j] = dado[j]%16;
        if(charOut[j] <= 9){
            charOut[j] += 0x30;
        }
        else charOut[j] += 0x37;
        lcdWriteByte(charOut[j], 1);                //Deve-se fazer a avaliação se o algarismo em hexadecimal é letra ou número

        lcdWriteByte((0x8D + (j*0x40)), 0);         //Leva o cursor para a posição anterior

        charOut[j] = (dado[j]/16)%16;
        if(charOut[j] <= 9){
            charOut[j] += 0x30;
        }
        else charOut[j] += 0x37;
        lcdWriteByte(charOut[j], 1);

        lcdWriteByte((0x8C + (j*0x40)), 0);

        charOut[j] = (dado[j]/(16*16))%16;
        if(charOut[j] <= 9){
            charOut[j] += 0x30;
        }
        else charOut[j] += 0x37;
        lcdWriteByte(charOut[j], 1);                //Obtenção do valor em hexadecimal e mudança do cursor foi realizada até o final

        j++;
    }
    j = 0;
}
