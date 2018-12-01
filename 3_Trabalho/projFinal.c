#include <msp430g2553.h>
#include "uart.h" // ATTACH THE UART FILE WITH THE MAIN CODE
#include <string.h>
#define RMC_SENT_LEN 57
#define BTN BIT3
#define SEL BIT4

typedef struct{
	char Time[11];
	char Lat[10];
	char Long[11];
	char Date[7];
} gps;

int gsm(gps *myGPS);//FUNCTION PROTOTYPE
void get_gps(gps *myGPS);
int confirmaAT();
void manda_local();


 

int main(void){
    WDTCTL = WDTPW + WDTHOLD; 
    BCSCTL1 = CALBC1_8MHZ; 
    DCOCTL = CALDCO_8MHZ; 
     
    P1DIR &= ~BTN; 
    P1REN |= BTN; 
    P1OUT |= BTN; 
    P1IES |= BTN; 
    P1IE  |= BTN; 
 
    manda_local();
    
    TA0CTL= TASSEL_1 + ID_1 + MC_2 + TAIE; 
   __bis_SR_register(GIE+LPM3_bits);
   while(1);
} 

void manda_local(){
  gps myGPS;
  
  get_gps(&myGPS);
  gsm(&myGPS);
  
  return;
}

int gsm(gps *myGPS){
      uart_init();
      __enable_interrupt();

      P1OUT |= SEL;
      
      uart_puts((char *)"AT"); // COMMAND FOR INITIALIZING GSM
      uart_putc(0x0A);//ENTER
      uart_putc(0x0D);//CARRIAGE RETURN
      
      if(confirmaAT()!=0)
        return 1;
      
 //     __delay_cycles(10000000);//DELAY...WAIT FOR OK FROM GSM
      uart_puts((char *)"AT+CMGF=1");//COMMUNICATION
      uart_putc(0x0A);
      uart_putc(0x0D);

 
      if(confirmaAT()!=0)
        return 1;
   //  __delay_cycles(10000000);//WAIT FOR OK

      uart_puts((char *)"AT+CMGS=\"61996356120\"");//SEND A MESSAGE TO PARTICULAR NUMBER
      uart_putc(0x0A);
      uart_putc(0x0D);

      uart_puts(myGPS->Date);
      uart_puts(myGPS->Lat);
      uart_puts(myGPS->Long);
      uart_puts(myGPS->Time);

      uart_putc(0x1A);//CTRL Z

      IE2 &= ~UCA0RXIE;
      __disable_interrupt();

      //AFTER HARDWARE CONFIGURATION THE MESSAGE WILL GET SEND

     //ATTACH THE UART FILES OR WRITE THE CODE FOR INIT AND SENDING MESSAGE IN THE SAME FILE...
     return 0;

}
void get_gps(gps *myGPS){
	char temp_sentence[RMC_SENT_LEN];
	char temp_char;
	int sent=0;
	int cont;
	int gps_cont=0;
 
 	uart_init();
	 __enable_interrupt();   
        
        P1OUT &= ~SEL;
        
	for(;;){
        temp_char = uart_getc();  //Collect GPS Data
        if (temp_char == '$') {
            temp_char = uart_getc();
            if (temp_char == 'G') {
                temp_char = uart_getc();
                if (temp_char == 'P') {
                    temp_char = uart_getc();
                    if (temp_char == 'R') {
                        temp_char = uart_getc();
                        if (temp_char == 'M') {
                            temp_char = uart_getc();
                            if (temp_char == 'C') {
                                //Code to save specific portions of GPS DATA Stream
                                 uart_gets(temp_sentence, RMC_SENT_LEN);
                                 for(cont=0;cont<RMC_SENT_LEN;cont++){
                                   if(temp_sentence[cont]==',')
                                     sent++;
                                   switch(sent){
                                   case 1:{
                                     if(temp_sentence[cont]!=','){
                                      *(myGPS->Time+gps_cont)=temp_sentence[cont];
                                      gps_cont++;
                                     }
                                     else{
                                      gps_cont=0;
                                     }
                                     break;
                                   }
                                   case 3:{
                                     if(temp_sentence[cont]!=','){
                                      *(myGPS->Lat+gps_cont)=temp_sentence[cont];
                                      gps_cont++;
                                     }
                                     else{
                                      *(myGPS->Time+gps_cont)='\0';
                                      gps_cont=0;
                                     }
                                     break;  
                                   }
                                   case 4:{
                                     if(temp_sentence[cont]!=','){
                                      *(myGPS->Lat+gps_cont)=temp_sentence[cont];
                                      gps_cont++;
                                     }
                                     else{
                                      *(myGPS->Lat+gps_cont)='-';
                                      gps_cont++;
                                     }
                                     break;  
                                   }
                                   case 5:{
                                     if(temp_sentence[cont]!=','){
                                      *(myGPS->Long+gps_cont)=temp_sentence[cont];
                                      gps_cont++;
                                     }
                                     else{
                                      *(myGPS->Lat+gps_cont)='\0';
                                      gps_cont=0;
                                     }
                                     break; 
                                   }
                                   case 6:{
                                     if(temp_sentence[cont]!=','){
                                      *(myGPS->Long+gps_cont)=temp_sentence[cont];
                                      gps_cont++;
                                     }
                                     else{
                                      *(myGPS->Long+gps_cont)='-';
                                      gps_cont++;
                                     }
                                     break;  
                                   }
                                   case 9:{
                                     if(temp_sentence[cont]!=','){
                                      *(myGPS->Date+gps_cont)=temp_sentence[cont];
                                      gps_cont++;
                                     }
                                     else{
                                      *(myGPS->Long+gps_cont)='\0';
                                      gps_cont=0;
                                     }
                                     break; 
                                   }
                                   case 10:{
                                      *(myGPS->Date+gps_cont)='\0';
                                     cont=RMC_SENT_LEN;
                                   }
                                   }
                                 }
				 IE2 &= ~UCA0RXIE;
                                 __disable_interrupt();

                                 return;
                            }
                        }
                    }
                }
            }
        }
    }
}
 
int confirmaAT(){
  int n;
  char status[11];
  status[0] = '\r';
  while(uart_getc()!='\r');
  for(n=1; n<=9; n++){
    status[n] = uart_getc();
    if(status[n]=='\n'){
    	status[n+1]='\0';
    	n=9;
    }
  }
  
  return strcmp(status, "\r\nOK\r\n");
}
 

#pragma vector=TIMER0_A0_VECTOR 
__interrupt void manda_local_t (void) 
{ 
  static int cont = 0; 
   
  cont++; 
  if(cont>=900){ // para clk 32768 dividido por 2 no timer, 15 contagens será o tempo de 1 min 
    TA0CTL= TACLR;
    TA0CTL=0;
    manda_local(); 
    cont=0; 
  } 
  TA0CTL= TASSEL_2 + ID_3 + MC_2 + TAIE;
} 
 
#pragma vector=PORT1_VECTOR 
__interrupt void manda_local_b(void){
    
    P1IFG &=~BTN; 
    P1IE &= ~BTN; 
    manda_local();
    P1IE |= BTN;
}
