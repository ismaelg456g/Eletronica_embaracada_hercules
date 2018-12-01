#include "msp430g2553.h" 
#include "uart.c"  
#include <string.h> 
#include "lcd.c" 
#define RMC_SENT_LEN 57 
#define BTN BIT3 
 
typedef struct{ 
 char Time[12]; 
 char Lat[14]; 
 char Long[15]; 
 char Date[8]; 
} gps; 
 
void get_gps(gps *myGPS); 
void manda_local(); 
void confirma();
 
int main(void){ 
    WDTCTL = WDTPW + WDTHOLD; 
    BCSCTL1 = CALBC1_8MHZ; 
    DCOCTL = CALDCO_8MHZ; 
     
    P1DIR &= ~BTN; 
    P1REN |= BTN; 
    P1OUT |= BTN; 
    P1IES |= BTN; 
    P1IE  |= BTN; 
 
    InitLCD();
    CLR_DISPLAY; 
    Send_String("GPS:");
    Send_String(" ok");
    confirma();
    manda_local();
    
    TA0CTL= TASSEL_2 + ID_3 + MC_2 + TAIE; 
   __bis_SR_register(GIE+LPM0_bits);
   while(1);
}  
 
void manda_local(){ 
  gps myGPS;
  int cont;
   
  get_gps(&myGPS); 
  CLR_DISPLAY;
  for(cont=0;cont<6;cont++){
    Send_Data(myGPS.Time[cont]);
    if(((cont%2)!=0)&&(cont!=5))
      Send_Data(':');
  }
  confirma();
  CLR_DISPLAY;
  for(cont=0;cont<6;cont++){
    Send_Data(myGPS.Date[cont]);
    if(((cont%2)!=0)&&(cont!=5))
      Send_Data('/');
  }
  confirma();
  CLR_DISPLAY;
  Send_String(myGPS.Lat); 
  confirma();
  CLR_DISPLAY;
  Send_String(myGPS.Long); 
  confirma();
  P1IFG &=~BTN; 
  
  return; 
} 
 
void get_gps(gps *myGPS){ 
 char temp_sentence[RMC_SENT_LEN]; 
 char temp_char; 
 int sent=0;
 int cont;
 int gps_cont=0;
 
 uart_init();
 __enable_interrupt();        
 for(;;){ 
        temp_char = uart_getc(); 
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

void confirma(){
  Atraso_us(50000);
  while((P1IN&BTN)==0);
  while((P1IN&BTN)!=0);
}

#pragma vector=TIMER0_A0_VECTOR 
__interrupt void manda_local_t (void) 
{ 
  static int cont = 0; 
   
  cont++; 
  if(cont>=1000){ // para clk 32768 dividido por 2 no timer, 15 contagens será o tempo de 1 min 
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
