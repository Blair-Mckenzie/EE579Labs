
#include "io430.h"
#define LED2 BIT6
#define LED3 (BIT1+ BIT3 + BIT5)
#define RED (BIT1)
#define YELLOW (BIT1 + BIT3)
#define SWITCH BIT3
#define TIMER (3u * (32768/8)) //Change this as per needed

void configureTimerA(void);
//unsigned int count;

int main( void ){
  WDTCTL = WDTPW + WDTHOLD;     // Stop watchdog timer to prevent time out reset
  //Set MCLK = SMCLK = 1MHz
//  BCSCTL1 = CALBC1_1MHZ;
//  DCOCTL = CALDCO_1MHZ;
  
  P1DIR |= LED2;                // Set P1.6 to output direction
  P2DIR |= LED3;                // Set P2.1,P2.3,P2.5 to output direction
  P1OUT = 0;
//  _enable_interrupts();
  for (;;){
    if((P1IN & SWITCH)==0){
      P2OUT |= YELLOW;
      configureTimerA();
      P2OUT |= RED;
    }else{
      P2OUT = 0;
    }
  } 
  return 0;
}

void configureTimerA(void){
  // TASSELx bits 9-8, IDx bits 7-6, MCx bits 5-4
  // ACLK as clock source, divider 8, countup, interrupt enabled
  TACTL = TASSEL_1 | ID_3 | MC_1| TAIE;
  TACCR0 = TIMER; 
  TACCTL0 = CCIE;
 __bis_SR_register(GIE); 
  LPM1;         //CPU, MCLK are disabled, ACLK is active
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A(void){
  switch(TAIV){
    case 0x02:
    P2OUT = 0;
    TACTL = MC_0;
    break;
  }
}