#include <io430.h>
#define SWITCH BIT3
#define BUZZEROUT 0x11
#define FLASHRATE 11718  // 80 flashes/min

int count = 0;
int freq1 = 750;
int freq2 = 1500;
int currentFreq = 0;

void IO_init(void);
void switch_init(void);
void timer_init(void);

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Stop WDT
  //Call initialise functions
//  BCSCTL2 |= SELS + DIVS_2;              // divide clk by 8
  IO_init();
  switch_init();
  timer_init();
  __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupt
}

void IO_init()
{
  P2DIR |= BUZZEROUT; // P2.4, P2.5 outputs
  P1DIR |= BIT6;      // P1.6 output
  P2OUT |= 0x1;       // Start P2.0 High
  P2OUT &= ~0x10;     // Start P2.4 Low
  P1OUT &= ~BIT6;     // Start P1.6 Low
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
  CCR0 = 31250;    // 1MHz/8/31250 ~ 4Hz (period of 0.25s - 1/4=0.25)
  TACTL = TASSEL_2 + MC_1 + ID_3; // SMCLK, Upmode, /8
}

//// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{

//  if(CCR0 == (FLASHRATE-1))
//  {
//    P1OUT ^= LED2; // P1.6 LED on
//  }
  count += CCR0;              // Increment count with current value in CCR0
  if (count >= 31250)          // Toggle rate of frequencies
  {
    count = 0;                  
    if (currentFreq == 0)     //Check current frequency, switch CCR0 to have other frequency
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
  P2OUT ^= BUZZEROUT;         // Toggle Buzzer Pins
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
  P1IFG &= ~SWITCH;     // Clear the interrupt flag for the switch
  P1IE &= ~SWITCH;      // Disable Button interrupt
  WDTCTL = WDT_MDLY_32; // Start and set watchdog timer (WDT) to trigger every 32ms
  IFG1 &= ~WDTIFG;      // Clear the interrupt flag for the WDT
  IE1 |= WDTIE;         // enable WDT interrupt
  P1OUT ^= BIT6;        // Toggle P1.6 
}

#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void)
{
  IE1 &= ~WDTIE;            // disable Watchdog timer (WDT) interrupt
  IFG1 &= ~WDTIFG;          // clear WDT interrupt flag
  WDTCTL = WDTPW + WDTHOLD; // put WDT back in hold state
  P1IE |= SWITCH;           // Enable interrupts for the switch
}

