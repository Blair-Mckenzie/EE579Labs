
#include "io430.h"
#define LED2  BIT6
#define SWITCH BIT3
#define FLASHRATE 384  // 40 flashes/min
#define WAIT 15360     // 30 seconds

void startFlashing(void);
void startTimer(void);
void setUpIO(void);


int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  setUpIO();
  startFlashing();
  return 0;
}
 
void setUpIO(void)
{
  BCSCTL3 |= LFXT1S0+XCAP_3;      // set clk to 32768HZ
  BCSCTL1 |= DIVA_3;              // divide clk by 8
  P1DIR |= LED2;                  // P1.6 Output
  P1REN|=SWITCH;                  // Enable pullup/pulldown resistors for P1.3
  P1OUT|=SWITCH;                  // Set P1.3 to have pull up resistors
  P1IE|=SWITCH;                   // Enable interrupt on P1.3
  P1IES|=SWITCH;                  // Set interrupt flag on the falling edge of logic level on P1.3
  return;
}

void startFlashing(void)
{
  CCTL0 = CCIE;                       // CCR0 interrupt enabled
  CCR0 = (FLASHRATE)-1;               // CCR0 set to toggle rate of 40/minute
  TACTL = TASSEL_1 + ID_3 + MC_1;     // ACLK Clock Source, /8, UpMode
  __bis_SR_register(LPM3_bits + GIE); // Enter LPM3 w/ interrupt
  return;
}

void startTimer(void)
{
    CCR0 = WAIT -1;
    TACTL = TASSEL_1 + ID_3 + MC_1;     // ACLK Clock Source, /8, UpMode
    __bis_SR_register(LPM3_bits + GIE); // Enter LPM3 w/ interrupt
    return;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{

  if(CCR0 == (FLASHRATE-1))
  {
    P1OUT ^= LED2; // P1.6 LED on
  }
  if(CCR0 == (WAIT-1))
  {
    setUpIO();
    startFlashing();
  } 
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
  P1IFG &= ~ SWITCH;                    // Clear the interrupt flag for the switch
  P1IE &= ~ SWITCH;                     // Disable Button interrupt
  WDTCTL = WDT_MDLY_32;                 // Start and set watchdog timer (WDT) to trigger every 32ms
  IFG1 &= ~ WDTIFG;                     // Clear the interrupt flag for the WDT
  IE1 |= WDTIE;                         // enable WDT interrupt
  P1OUT &= ~LED2;                       // P1.6 LED Off
  startTimer();
}

#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void)
{
  IE1 &= ~WDTIE;                        // disable Watchdog timer (WDT) interrupt
  IFG1 &= ~WDTIFG;                      // clear WDT interrupt flag
  WDTCTL = WDTPW + WDTHOLD;             // put WDT back in hold state
  P1IE |= SWITCH;                       // Enable interrupts for the switch
  P1IES|=SWITCH;                        // Set interrupt flag on falling edge
}

