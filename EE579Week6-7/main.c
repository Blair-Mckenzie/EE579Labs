#include <io430.h>
#define SWITCH BIT3
#define BUZZEROUT 0x30

int count = 0;
int freq1 = 750;
int freq2 = 1500;
int currentFreq = 0;

void ADC_init(void);
void IO_init(void);
void switch_init(void);
void timer_init(void);

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Stop WDT
  ADC_init();
  IO_init();
  switch_init();
  timer_init();
  __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupt
}

void ADC_init()
{
  ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; // ADC10ON, interrupt enabled
  ADC10CTL1 = INCH_1;                         // input A1
  ADC10AE0 |= BIT1;                           // PA.1 ADC option select
}

void IO_init()
{
  P2DIR |= BUZZEROUT; // P2.4, P2.5 outputs
  P2OUT |= 0x20;      // Start P2.5 High
  P2OUT &= ~0x10;     // Start P2.4 Low
  P1DIR |= BIT6;
}

void switch_init()
{
  P1REN |= SWITCH; // Enable pullup/pulldown resistors for P1.3
  P1OUT |= SWITCH; // Set P1.3 to have pull up resistors
  P1IE |= SWITCH;  // Enable interrupt on P1.3
  P1IES |= SWITCH; // Set interrupt flag on the falling edge of logic level on P1.3
}

void timer_init()
{
  TACCTL0 = CCIE; // TACCR0 toggle, interrupt enabled
  CCR0 = 31250;
  TACTL = TASSEL_2 + MC_1 + ID_3; // SMCLK, Contmode, int enabled
}

//// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{

  ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
  count += CCR0;
  if (count >= 31250)
  {
    count = 0;
    if ((P1IN & BIT6) == 0)
    {
      freq1 = ADC10MEM + 1;
    }
    else
    {
      freq2 = ADC10MEM + 1;
    }
    if (currentFreq == 0)
    {
      currentFreq = 1;
      CCR0 = freq2;
    }
    else
    {
      currentFreq = 0;
      CCR0 = freq1;
    }
  }
  P2OUT ^= BUZZEROUT;
}

//// ADC10 interrupt service routine
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF); // Clear CPUOFF bit from 0(SR)
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
  P1IFG &= ~SWITCH;     // Clear the interrupt flag for the switch
  P1IE &= ~SWITCH;      // Disable Button interrupt
  WDTCTL = WDT_MDLY_32; // Start and set watchdog timer (WDT) to trigger every 32ms
  IFG1 &= ~WDTIFG;      // Clear the interrupt flag for the WDT
  IE1 |= WDTIE;         // enable WDT interrupt
  P1OUT ^= BIT6;
}

#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void)
{
  IE1 &= ~WDTIE;            // disable Watchdog timer (WDT) interrupt
  IFG1 &= ~WDTIFG;          // clear WDT interrupt flag
  WDTCTL = WDTPW + WDTHOLD; // put WDT back in hold state
  P1IE |= SWITCH;           // Enable interrupts for the switch
}

