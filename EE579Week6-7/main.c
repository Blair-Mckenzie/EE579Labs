//******************************************************************************
//  MSP430G2452 Multiple freq. PWM Demo - Timer_A, Toggle P1.0-2, P1.4
//
//  Description: Use Timer_A CCRx units and overflow to generate three
//  independent PWM freqs and duty cycles. For demonstration, CCR0 and CCR1 output
//  units are optionally selected with port pins P1.1, P1.2, and P1.4 in toggle
//  mode. Interrupts are also enabled with all CCRx units,
//  software loads offset to next interval only - as long as the interval offset
//  is aded to CCRx, toggle rate is generated in hardware. Timer_A overflow ISR
//  is used to toggle P1.0 with software. 
//
//  ACLK = n/a, MCLK = SMCLK = TACLK = default DCO ~1MHz
//  As coded and assuming ~1MHz DCO, toggle rates are:
//  P1.1 = CCR0 ~ 1MHz/(2*100) = ~5kHz
//  P1.2 = CCR1 ~ 1MHz/(2*200) = ~2.5kHz
//  P1.4 = CCR2 ~ 1MHz/(2*500) = ~1kHz
//
//  P1.0 = overflow ~ 1MHz/2/(2*65536) = ~4Hz
//
//               MSP430G2452
//            -----------------
//        /|\|              XIN|-
//         | |                 |
//         --|RST          XOUT|-
//           |                 |
//           |         P1.1/TA0|--> TACCR0
//           |         P1.2/TA1|--> TACCR1
//           |         P1.4/TA2|--> TACCR2
//           |             P1.0|--> Overflow/software
//
//******************************************************************************


#include <io430.h>
#define SWITCH BIT3
#define BUZZEROUT 0x30

void ADC_init(void);
void IO_init(void);
void switch_init(void);
void timer_init(void);

int i;
int k;
int lowerFreq = 475;
int upperFreq = 400;
int freqChange = 0;

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Stop WDT

  //Calibrate DCO for 1MHz operation
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  //
  //  P2SEL |= 0x30;                            // P2.4 - P2.5 option select Timer_A output
  //  P1SEL2 |= 0x10;                           // P1.4 option select Timer_A output
  //  P1DIR |= 0x17;                            // P1.0 - P1.2, P1.4 outputs
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
  ADC10AE0 |= 0x02;                           // PA.1 ADC option select
}

void IO_init()
{
  P1DIR |= BIT0 + BIT6;
  P2DIR |= BUZZEROUT; // P2.4, P2.5 outputs
  P2OUT |= 0x20;      // Start P2.5 High
  P2OUT &= ~0x10;     // Start P2.4 Low
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
  TACCTL0 = OUTMOD_4 + CCIE;             // TACCR0 toggle, interrupt enabled
  TACCTL1 =   CCIE;             // TACCR1 toggle, interrupt enabled
  TACCTL2 = CCIE;             // TACCR2 toggle, interrupt enabled
  TACTL = TASSEL_2 + MC_2 + ID_2 + TAIE; // SMCLK, Contmode, int enabled
}

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{
  //    ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
  //    TACCR0 = ADC10MEM + 1;                // reload period
  //    P2OUT ^= 0x30;
}

// Timer A1 Interrupt Vector (TA0IV) handler
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1(void)
{
  ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
  switch (TA0IV)
  {
  case 2:
    if (freqChange == 0)
    {
      i = ADC10MEM + 1;
      TACCR1 += i; // reload period
      P2OUT ^= BUZZEROUT;
    }
    else
    {
      TACCR1 += lowerFreq; // reload period
      P2OUT ^= BUZZEROUT;
    }
    break;
  case 4:
    if (freqChange == 0)
    {
      TACCR2 += upperFreq; // reload period
      P2OUT ^= BUZZEROUT;
    }
    else
    {
      k = ADC10MEM + 1;
      TACCR2 +=k ; // reload period
      P2OUT ^= BUZZEROUT;
    }
    break;
  case 10:
    P2OUT ^= BUZZEROUT; //Overflow configured for 4Hz
    break;
  default:
    break;
  }
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
  P1IFG &= ~SWITCH;     // Clear the interrupt flag for the switch
  P1IE &= ~SWITCH;      // Disable Button interrupt
  WDTCTL = WDT_MDLY_32; // Start and set watchdog timer (WDT) to trigger every 32ms
  IFG1 &= ~WDTIFG;      // Clear the interrupt flag for the WDT
  IE1 |= WDTIE;         // enable WDT interrupt
  if (freqChange == 0)
  {
    freqChange = 1;
  }
  else
  {
    freqChange = 0;
  }
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


//#define LOWER_FREQ 150
//#define HIGHER_FREQ 300
//#define ALTERNATE 31250
//#define SWITCH BIT3
//void setUpIO(void);
//void startTimer(void);
//void changeValue(void);
//
//int count = 0;
//int freq = 0;
//
//int main(void)
//{
//  // Stop watchdog timer to prevent time out reset
//  WDTCTL = WDTPW + WDTHOLD;
//  setUpIO();
//  startTimer();
//  return 0;
//}
//
//void startTimer(void)
//{
//  CCTL0 = CCIE;                       // Enable CCR0 Interrupt
//  CCR0 = ALTERNATE - 1;               // 4Hz
//  TACTL = TASSEL_2 + ID_3 + MC_1;     // SMCLK source, Upmode, /8
//  __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupt
//}
//
//void setUpIO(void)
//{
//  ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; // ADC10ON, interrupt enabled
//  ADC10CTL1 = INCH_1;                         // input A1
//  ADC10AE0 |= 0x02;                           // PA.1 ADC option select
//  P1DIR |= 0x01;                              // Set P1.0 to output direction
//  P2DIR |= 0x30;                              // Set P2.4 and P2.5 to output direction
//  P2OUT |= 0x20;                              // Start P2.5 High
//  P2OUT &= ~0x10;                             // Start P2.4 Low
//  P1DIR |= BIT6;                              // P1.6 Output
//  P1REN |= SWITCH;                            // Enable pullup/pulldown resistors for P1.3
//  P1OUT |= SWITCH;                            // Set P1.3 to have pull up resistors
//  P1IE |= SWITCH;                             // Enable interrupt on P1.3
//  P1IES |= SWITCH;                            // Set interrupt flag on the falling edge of logic level on P1.3
//}
//
//#pragma vector = TIMER0_A0_VECTOR
//__interrupt void TIMER_A(void)
//{
//  if (freq == 0 && count > ALTERNATE)
//  {
//      freq = 1;
//      count = 0;
//    
//    else
//    {
//      changeValue();
//    }
//  }
//  else
//  {
//    if (count > ALTERNATE)
//    {
//      freq = 0;
//      count = 0;
//    }
//    else
//    {
//      changeValue();
//    }
//  }
//}
//
//void changeValue()
//{
//    ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
//    CCR0 = ADC10MEM + 1;
//    count += CCR0;
//    P2OUT ^= 0x30;
//}
//
//
//#pragma vector = PORT1_VECTOR
//__interrupt void Port1_ISR(void)
//{
//  P1IFG &= ~SWITCH;     // Clear the interrupt flag for the switch
//  P1IE &= ~SWITCH;      // Disable Button interrupt
//  WDTCTL = WDT_MDLY_32; // Start and set watchdog timer (WDT) to trigger every 32ms
//  IFG1 &= ~WDTIFG;      // Clear the interrupt flag for the WDT
//  IE1 |= WDTIE;         // enable WDT interrupt
//  P1OUT |= BIT6;        // P1.6 LED On
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
//

