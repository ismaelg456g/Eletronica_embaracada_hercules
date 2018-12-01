#include "lcd.h"

void Atraso_us(volatile unsigned int us)
{
	TA1CCR0 = us-1;
	TA1CTL = TASSEL_2 + ID_3 + MC_1 + TAIE;
	while((TA1CTL & TAIFG)==0);
	TA1CTL = TACLR;
	TA1CTL = 0;
}

void Send_Nibble(volatile unsigned char nibble, volatile unsigned char dados, volatile unsigned int microsegs)
{
	LCD_OUT |= E;
	LCD_OUT &= ~(RS + D4 + D5 + D6 + D7);
	LCD_OUT |= RS*(dados==DADOS) +
		D4*((nibble & BIT0)>0) +
		D5*((nibble & BIT1)>0) +
		D6*((nibble & BIT2)>0) +
		D7*((nibble & BIT3)>0);
	LCD_OUT &= ~E;
	Atraso_us(microsegs);
}

void Send_Byte(volatile unsigned char byte, volatile unsigned char dados, volatile unsigned int microsegs)
{
	Send_Nibble(byte >> 4, dados, microsegs/2);
	Send_Nibble(byte & 0xF, dados, microsegs/2);
}

void Send_Data(volatile unsigned char byte)
{
	Send_Byte(byte, DADOS, DATA_DLY);
}

void Send_String(char str[])
{
	while((*str)!='\0')
	{
		Send_Data(*(str++));
	}
}

void Send_Int(int n)
{
	int casa, dig;
	if(n==0)
	{
		Send_Data('0');
		return;
	}
	if(n<0)
	{
		Send_Data('-');
		n = -n;
	}
	for(casa = 10000; casa>n; casa /= 10);
	while(casa>0)
	{
		dig = (n/casa);
		Send_Data(dig+'0');
		n -= dig*casa;
		casa /= 10;
	}
}

void InitLCD(void)
{
	unsigned char CMNDS[] = {0x20, 0x14, 0xC, 0x6};
	unsigned int i;
	// Atraso de 10ms para o LCD fazer o boot
	Atraso_us(10000);
	LCD_DIR |= D4+D5+D6+D7+RS+E;
	Send_Nibble(0x2, COMANDO, CMND_DLY);
	for(i=0; i<4; i++)
		Send_Byte(CMNDS[i], COMANDO, CMND_DLY);
	CLR_DISPLAY;
	POS0_DISPLAY;
}
