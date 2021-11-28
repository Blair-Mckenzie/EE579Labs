#include <io430.h>
#define LED2 BIT6
#define LED3 (BIT1 + BIT3 + BIT5)
#define SWITCH BIT3
#define BUZZEROUT 0x11
#define YELLOW (BIT1 + BIT3)
#define THREESECONDS 46875     // -- 3 seconds
#define FLASHRATE80 5860  // 80 flashes/min
#define FLASHRATE30 15625 // 30 flashes/min

int count = 0;
int freq1 = 93;
int freq2 = 187;
int currentFreq = 0;
unsigned int i =0;

void IO_init(void);
void switch_init(void);
void timer_init(void);
void three_second_timer();
void startFlashing80();
void startFlashing30();
void driver();
void buzzerNoise();

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Stop WDT
  //Call initialise functions
  BCSCTL2 |= DIVS_3;              // divide smclk by 8
  IO_init();
//  switch_init();
  timer_init();
  __bis_SR_register(GIE); // Enter LPM0 w/ interrupt
  driver();
}

void driver()
{
  for(;;)
  {
    if ((P1IN & SWITCH) == 0)
    {
      P2OUT |= YELLOW;
      three_second_timer();
    }
    else
    {
      startFlashing30();
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
  P1DIR = BIT0;
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
  CCR0 = 3906;    // 1MHz/8/8/3906 ~ 4Hz (period of 0.25s - 1/4=0.25)
  TACTL = TASSEL_2 + MC_1 + ID_3; // SMCLK, Upmode, /8
}

void three_second_timer()
{
  CCR0 = THREESECONDS;
  __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupt
}

void startFlashing80()
{
  CCR0 = FLASHRATE80;
  P2OUT &= ~LED3;
  P2OUT |= BIT1;
}

void startFlashing30()
{
  CCR0 = FLASHRATE30;
}

//// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
  if ((P1IN & SWITCH) == 0)
  {
    if(CCR0 == THREESECONDS)
    {
      startFlashing80();
    }
    if(CCR0 == FLASHRATE80)
    {
//      buzzerNoise();
      P1OUT ^= LED2; // P1.6 LED on
      P2OUT ^= BIT1;
    }
  }
  else
  {
    if(CCR0 == FLASHRATE30)
    {
      P1OUT ^= BIT0;
    }
  }
}

void buzzerNoise()
{
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



