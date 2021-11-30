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

void IO_init();
void startThreeSeconds();
void configureTimer1();
void startFlashing80();
void init();