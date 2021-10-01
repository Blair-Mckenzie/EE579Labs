#include <io430.h>
#define LED2 BIT6
#define LED3 (BIT1 + BIT3 + BIT5)
#define RED (BIT1)
#define YELLOW (BIT1 + BIT3)
#define SWITCH BIT3
#define TIMER 12288     // -- 3 seconds
#define FLASHRATE 80000 // 80 flashes/min
#define LED_DELAY 65000 // ~~ 0.75s delay

// Function Declarations
void setUpIO(void);
void configureTimer(void);
void startTimer(void);
void startAndConfigureFlashing(void);
void driver(void);

unsigned int i; //Global counter

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Stop WDT
  setUpIO();
  i = 0;
  __bis_SR_register(GIE); // Enter LPM3 w/ interrupt
  configureTimer();
  driver(); // Main control of program
  return 0;
}

void driver(void)
{
  for (;;)
  {
    if ((P1IN & SWITCH) == 0)
    {                  // Check the switch is depressed
      P2OUT |= YELLOW; // Set the RGB Led to yellow
      startTimer();    // Start the 3 second timer
    }
    else
    {
      P2OUT &= ~LED3;   // P2.1,P2.3,P2.5 LED Off
      P1OUT &= ~LED2;   // P1.6 LED Off
      CCR0 = 0;         // Timer Off
      configureTimer(); // Reset Configuration
      i = 0;            // Reset Global Counter
    }
  }
}

void setUpIO(void)
{
  P1DIR |= LED2;  // P1.6 Output
  P1OUT &= ~LED2; // P1.6 LED Off
  P2DIR |= LED3;  // P2.1,P2.3,P2.5 output
  P2OUT &= ~LED3; // P2.1,P2.3,P2.5 LED Off
  return;
}

void configureTimer(void)
{
  CCTL0 = CCIE;           // CCR0 interrupt enabled
  CCR0 = (TIMER)-1;       // Set CCR0 to 3 seconds
  __bis_SR_register(GIE); // Global Enable interrupt
  return;
}

void startTimer(void)
{
  TACTL = TASSEL_1 + ID_3 + MC_1; // ACLK Clock Source (32768Hz), Divider of 8, UpMode
}

void startAndConfigureFlashing(void)
{
  CCTL0 = CCIE;                       // CCR0 interrupt enabled
  CCR0 = (FLASHRATE)-1;               // CCR0 set to toggle rate of 80/minute
  TACTL = TASSEL_1 + MC_1;            // ACLK Clock Source, UpMode
  __bis_SR_register(LPM3_bits + GIE); // Enter LPM3 w/ interrupt
  return;
}

// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(TIMER0_A0_VECTOR))) Timer_A(void)
#else
#error Compiler not supported!
#endif
{
  i++;
  if ((P1IN & SWITCH) == 0)
  {
    if (i == 1)
    {                              // First Time Interrupt fires (i.e. after three seconds)
      CCR0 = 0;                    // Timer Off
      P2OUT &= ~LED3;              // P2.1,P2.3,P2.5 LED Off
      startAndConfigureFlashing(); // Different timer setup for flashing
    }
    P1OUT ^= LED2; // P1.6 LED on
    for (i = 0; i < LED_DELAY; i++)
      ;           // Delay for alternating LEDs
    P2OUT ^= RED; // P2.1 LED on (Red)
  }
  else
  {
    driver();       // Back to main loop
    P2OUT &= ~LED3; // P2.1,P2.3,P2.5 LED Off
    P1OUT &= ~LED2; // P1.6 LED Off
  }
}
