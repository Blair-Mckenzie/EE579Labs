#include <io430.h>
#define LED1 BIT0
#define LED2 BIT6
#define LED3 (BIT1 + BIT3 + BIT5)
#define SWITCH BIT3
#define BUZZEROUT 0x11
#define YELLOW (BIT1 + BIT3)
#define THREESECONDS 46875     // -- 3 seconds
#define FLASHRATE80 5860  // 80 flashes/min
#define FLASHRATE30 15625 // 30 flashes/min
#define BUZZERRATE 3906

int count = 0;
int freq1 = 93;
int freq2 = 187;
int currentFreq = 0;
unsigned int i;
unsigned int greenLedCount = 0;

void IO_init(void);
void startThreeSeconds();
void configureTimer1();
void startFlashing80();
void driver();
void setUpandStartWatchDog();
void init();

int main(void)
{
//  WDTCTL = WDTPW + WDTHOLD; // Stop WDT
  init();
  return 0;
}
void init()
{
  i = 0;
  setUpandStartWatchDog();
  BCSCTL2 |= DIVS_3;                    // divide smclk by 8
  IO_init();
  __bis_SR_register(GIE);		// Enter Low power mode 0 with interrupts enabled
  driver();
}

void driver()
{
  for(;;)
  {
    if ((P1IN & SWITCH) == 0)
    {
      configureTimer1();
      P2OUT |= YELLOW;
      startThreeSeconds();  
    }
    else
    {
      P2OUT &= ~LED3;   // P2.1,P2.3,P2.5 LED Off
      P1OUT &= ~LED2;   // P1.6 LED Off
      TA0CCR0 =0;
      TA1CCR0 =0;
      i = 0;
    }
  }
}
void IO_init()
{
  P2DIR |= BUZZEROUT;   // P2.4, P2.5 outputs
  P2DIR |= LED3;        // P2.1,P2.3,P2.5 output
  P2OUT |= 0x1;         // Start P2.0 High
  P2OUT &= ~0x10;       // Start P2.4 Low
  P2OUT &= ~LED3;       // P2.1,P2.3,P2.5 LED Off
  
  P1DIR |= 0x41;        // P1.0,P1.6 output
  P1OUT &= ~LED1;        // Start P1.0 High
  P1OUT &= ~LED2;       // Start P1.6 Low
}

void setUpandStartWatchDog()
{
  WDTCTL = WDT_ADLY_250;                // 250ms interval   
  IE1 |= WDTIE;                         // Enable WDT interrupts in the status register
}

void configureTimer1()
{
  TA1CCR0 |= BUZZERRATE;	        // Counter value
  TA1CCTL0 |= CCIE;			// Enable Timer1_A interrupts
  TA1CCR1 = FLASHRATE30;
  TA1CTL = TASSEL_2 + MC_1 + ID_3;      // SMCLK, Upmode, /8
}

void startThreeSeconds()
{
    TA0CCTL0 |= CCIE;
    TA0CCR0 = THREESECONDS;
    TA0CTL = TASSEL_2 + MC_1 + ID_3; // SMCLK, Upmode, /8 
    __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupt
}

void startFlashing80()
{
  P2OUT |= BIT1;
  TA0CCTL0 = CCIE;
  TA0CCR0 = FLASHRATE80;
  TA0CTL = TASSEL_2 + MC_1 + ID_3; // SMCLK, Upmode, /8
  __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupt
}

//// Timer0 A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void)
{
  i++;
  if ((P1IN & SWITCH) == 0)
  {
    if(i == 1)
    {
      TA0CCR0 = 0;
      P2OUT &= ~LED3;              // P2.1,P2.3,P2.5 LED Off
      startFlashing80();
    }
    if(i == 8)
    {
      WDTCTL = WDT_ADLY_1000;
    }
    P1OUT ^= LED2; // P1.6 LED on
    P2OUT ^= BIT1;
  }
  else
  {
    P2OUT &= ~LED3; // P2.1,P2.3,P2.5 LED Off
    P1OUT &= ~LED2; // P1.6 LED Off
    init();
  }
}

#pragma vector=TIMER1_A0_VECTOR     // Timer1 A0 interrupt service routine
 __interrupt void Timer1_A0 (void) 
{
  if ((P1IN & SWITCH) == 0)
  {
    count += TA1CCR0;          // Increment count with current value in TA1CCR0
    if (count >= BUZZERRATE)   // Toggle rate of frequencies
    {
      count = 0;                  
      if (currentFreq == 0)     //Check current frequency, switch TA1CCR0 to have other frequency
      {
        currentFreq = 1;
        TA1CCR0 = freq2;
      }
      else
      {
        currentFreq = 0;
        TA1CCR0 = freq1;
      }
    }
    P2OUT ^= BUZZEROUT;         // Toggle Buzzer Pins 
  }
  else
  {
    init();
  }
}
#pragma vector = WDT_VECTOR  // Interrupt Service Routine (ISR) for Watchdog Timer
__interrupt void flashLed(void) 
{ 
      P1OUT ^= BIT0;
}