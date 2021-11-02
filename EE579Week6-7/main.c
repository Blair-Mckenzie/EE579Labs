
#include "io430.h"
#define LOWER_FREQ 150
#define HIGHER_FREQ 300
#define ALTERNATE 31250
#define SWITCH BIT3
void setUpIO(void);
void startTimer(void);
void changeValue(void);

int count = 0;
int freq = 0;

int main(void)
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  setUpIO();
  startTimer();
  return 0;
}

void startTimer(void)
{
  CCTL0 = CCIE;                       // Enable CCR0 Interrupt
  CCR0 = ALTERNATE - 1;               // 4Hz
  TACTL = TASSEL_2 + ID_3 + MC_1;     // SMCLK source, Upmode, /8
  __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupt
}

void setUpIO(void)
{
  ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; // ADC10ON, interrupt enabled
  ADC10CTL1 = INCH_1;                         // input A1
  ADC10AE0 |= 0x02;                           // PA.1 ADC option select
  P1DIR |= 0x01;                              // Set P1.0 to output direction
  P2DIR |= 0x30;                              // Set P2.4 and P2.5 to output direction
  P2OUT |= 0x20;                              // Start P2.5 High
  P2OUT &= ~0x10;                             // Start P2.4 Low
  P1DIR |= BIT6;                              // P1.6 Output
  P1REN |= SWITCH;                            // Enable pullup/pulldown resistors for P1.3
  P1OUT |= SWITCH;                            // Set P1.3 to have pull up resistors
  P1IE |= SWITCH;                             // Enable interrupt on P1.3
  P1IES |= SWITCH;                            // Set interrupt flag on the falling edge of logic level on P1.3
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER_A(void)
{
  if (freq == 0 && count > ALTERNATE)
  {
      freq = 1;
      count = 0;
    
    else
    {
      changeValue();
    }
  }
  else
  {
    if (count > ALTERNATE)
    {
      freq = 0;
      count = 0;
    }
    else
    {
      changeValue();
    }
  }
}

void changeValue()
{
    ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
    CCR0 = ADC10MEM + 1;
    count += CCR0;
    P2OUT ^= 0x30;
}


#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
  P1IFG &= ~SWITCH;     // Clear the interrupt flag for the switch
  P1IE &= ~SWITCH;      // Disable Button interrupt
  WDTCTL = WDT_MDLY_32; // Start and set watchdog timer (WDT) to trigger every 32ms
  IFG1 &= ~WDTIFG;      // Clear the interrupt flag for the WDT
  IE1 |= WDTIE;         // enable WDT interrupt
  P1OUT |= BIT6;        // P1.6 LED On
}

#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void)
{
  IE1 &= ~WDTIE;            // disable Watchdog timer (WDT) interrupt
  IFG1 &= ~WDTIFG;          // clear WDT interrupt flag
  WDTCTL = WDTPW + WDTHOLD; // put WDT back in hold state
  P1IE |= SWITCH;           // Enable interrupts for the switch
}

// ADC10 interrupt service routine
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF); // Clear CPUOFF bit from 0(SR)
}
