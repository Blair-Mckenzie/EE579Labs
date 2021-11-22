#include <io430.h>
#define LED2 BIT6
#define LED3 (BIT1 + BIT3 + BIT5)
#define SWITCH BIT3
#define BUZZEROUT 0x11
#define YELLOW (BIT1 + BIT3)
#define THREESECONDS 46875     // -- 3 seconds
#define FLASHRATE 5860  // 80 flashes/min

int count = 0;
int freq1 = 93;
int freq2 = 187;
int currentFreq = 0;

void IO_init(void);
void switch_init(void);
void timer_init(void);
void three_second_timer();
void startFlashing();

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Stop WDT
  //Call initialise functions
  BCSCTL2 |= DIVS_3;              // divide smclk by 8
  IO_init();
//  switch_init();
  timer_init();
  __bis_SR_register(GIE); // Enter LPM0 w/ interrupt
  for(;;)
  {
    if ((P1IN & SWITCH) == 0)
    {
      P2OUT |= YELLOW;
      three_second_timer();
    }
    else
    {
      P2OUT &= ~LED3;   // P2.1,P2.3,P2.5 LED Off
      P1OUT &= ~LED2;   // P1.6 LED Off
    }
  }
}

void IO_init()
{
  P2DIR |= BUZZEROUT; // P2.4, P2.5 outputs
  P2DIR |= LED3;      // P2.1,P2.3,P2.5 output
  P1DIR |= BIT6;      // P1.6 output
  P2OUT |= 0x1;       // Start P2.0 High
  P2OUT &= ~0x10;     // Start P2.4 Low
  P2OUT &= ~LED3;    // P2.1,P2.3,P2.5 LED Off
//  P2OUT |= BIT1;      // Start P2.1 High
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
//  TACCTL1 = CCIE;                
//  TACCTL2 = CCIE;                
  CCR0 = 3906;    // 1MHz/8/31250 ~ 4Hz (period of 0.25s - 1/4=0.25)
//  CCR1 = 5860;
//  CCR2 = 3906;
  TACTL = TASSEL_2 + MC_1 + ID_3; // SMCLK, Upmode, /8
}

void three_second_timer()
{
  CCR1 = THREESECONDS;
  __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupt
}

void startFlashing()
{
  
}

//// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{

//  if(CCR0 == (FLASHRATE-1))
//  {
//    P1OUT ^= LED2; // P1.6 LED on
//  }
  if(CCR1 == THREESECONDS)
  {
    startFlashing();
  }
  count += CCR0;              // Increment count with current value in CCR0
  if (count >= 3906)         // Toggle rate of frequencies
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

// Timer A1 Interrupt Vector (TA0IV) handler
//#pragma vector=TIMER0_A1_VECTOR
//__interrupt void Timer_A1(void)
//{
//  switch( TA0IV )
//  {
//  case  2:
//    P2OUT ^= BIT1;
//    P1OUT ^= BIT6;
//    CCR1 += 5860;
//    break;
//  case  4:
//    
//           break;
//  case 10: 
//           break;
//  default: break;
//  }
//}

//#pragma vector = PORT1_VECTOR
//__interrupt void Port1_ISR(void)
//{
//  P1IFG &= ~SWITCH;     // Clear the interrupt flag for the switch
//  P1IE &= ~SWITCH;      // Disable Button interrupt
//  WDTCTL = WDT_MDLY_32; // Start and set watchdog timer (WDT) to trigger every 32ms
//  IFG1 &= ~WDTIFG;      // Clear the interrupt flag for the WDT
//  IE1 |= WDTIE;         // enable WDT interrupt
////  P1OUT ^= BIT6;        // Toggle P1.6 
//}
//
//#pragma vector = WDT_VECTOR
//__interrupt void WDT_ISR(void)
//{
//  IE1 &= ~WDTIE;            // disable Watchdog timer (WDT) interrupt
//  IFG1 &= ~WDTIFG;          // clear WDT interrupt flag
//  WDTCTL = WDTPW + WDTHOLD; // put WDT back in hold state
//  P1IE |= SWITCH;           // Enable interrupts for the switch
//}

