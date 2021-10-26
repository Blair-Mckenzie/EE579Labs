
#include "io430.h"

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                   // Stop WDT
  ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; // ADC10ON, interrupt enabled
  ADC10CTL1 = INCH_1;                         // input A1
  ADC10AE0 |= 0x02;                           // PA.1 ADC option select
  P1DIR |= 0x01;                              // Set P1.0 to output direction
  P2DIR |= 0x30;                              // Set P2.4 and P2.5 to output direction
  P2OUT |= 0x20;                              // Start P2.5 High
  P2OUT &= ~0x10;                             // Start P2.4 Low
  CCTL0 = CCIE;                               // Enable CCR0 Interrupt
  CCR0 = 500;                                 // Toggle every 0.5ms for 1kHz
  TACTL = TASSEL_2 + MC_1;                    // SMCLK source, Upmode
//  __bis_SR_register(LPM0_bits + GIE);

  for (;;)
  {
    ADC10CTL0 |= ENC + ADC10SC;      // Sampling and conversion start
    __bis_SR_register(CPUOFF + GIE); // LPM0, ADC10_ISR will force exit
    if (ADC10MEM < 0x1FF)
      P1OUT &= ~0x01; // Clear P1.0 LED off
    else
      P1OUT |= 0x01; // Set P1.0 LED on
  }
}

// ADC10 interrupt service routine
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF); // Clear CPUOFF bit from 0(SR)
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
  P2OUT ^= 0x30; //Toggle P2.4 and P2.5
}
