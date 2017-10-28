#include <mcp_can.h>
#include <SPI.h>

#define CAN0_INT 2    // Set INT to pin 2

typedef struct {
  volatile uint8_t *port;
  volatile uint8_t *pin;
  byte bit;
} table;

#define CTR0_CLK   (table) { &PORTD, &PIND,  0x08 }
#define CTR0_LATCH (table) { &PORTD, &PIND,  0x10 }
#define CTR0_DATA  (table) { &PORTD, &PIND,  0x20 }
#define CTR1_CLK   (table) { &PORTD, &PIND,  0x40 }
#define CTR1_LATCH (table) { &PORTD, &PIND,  0x80 }
#define CTR1_DATA  (table) { &PORTB, &PINB,  0x01 }


#define setHigh(x) *x.port |= x.bit
#define setLow(x) *x.port &= ~x.bit
#define setBit(x, val) {if (val) *(x.port) |= x.bit; else *(x.port) &= ~(x.bit);}

#define toggle(x) (*x.pin ^= x.bit)
#define readBit(x) ((*x.pin & x.bit) != 0)
#define isLow(x) ((*x.pin & x.bit) == 0)
#define isHigh(x) ((*x.pin & x.bit) != 0)

#define JOYS 2        // Number of joysticks/controllers
#define CLOCKS 8     // Number of pulses to send to controllers
#define DATA_WIDTH 2  // Number of bytes read from each controller

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string
MCP_CAN CAN0(10);     // Set CS to pin 10


unsigned int g_joy_data[JOYS];
byte g_can_data[JOYS * DATA_WIDTH];

unsigned long last;

void setup()
{
  Serial.begin(115200);

  // Initialize MCP2515 running at 8MHz with a baudrate of 1000kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) 
    Serial.println("MCP2515 Initialized Successfully!");
  else 
    Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
  pinMode(CAN0_INT, INPUT);   // Configuring pin for /INT input

  DDRD |= CTR0_DATA.bit;
  DDRB |= CTR1_DATA.bit;
  setHigh(CTR0_DATA);
  setHigh(CTR1_DATA);
  last = micros();
}

void send_baseunit()
{
  unsigned int data0, data1;
  noInterrupts();

  data0 = g_joy_data[0];
  data1 = g_joy_data[1];
  while(isLow(CTR0_LATCH));
 
  for (byte i = 0; i < 32; i++) {
    while(1) {
    //*
    if (isHigh(CTR0_LATCH)) {
      data0 = g_joy_data[0];
      data1 = g_joy_data[1];
      data0 <<= 1; // shift data
      data1 <<= 1;
      setBit(CTR0_DATA, !(data0 & 0x8000));
      setBit(CTR1_DATA, !(data1 & 0x8000));
      while (isHigh(CTR0_LATCH));
    }
    //*/
    //*
    if (isLow(CTR1_CLK)) {
      while (isLow(CTR1_CLK));
      data1 <<= 1;
      setBit(CTR1_DATA, !(data1 & 0x8000));
      //continue;
      break;
    }
    //*/
    if (isLow(CTR0_CLK)) {
      while (isLow(CTR0_CLK));
      data0 <<= 1;
      setBit(CTR0_DATA, !(data0 & 0x8000));
      //continue;
      break;
    }
  }  
  }
  interrupts();
}

void readCan() {
  setLow(CTR1_DATA);
  if (CAN0.readMsgBuf(&rxId, &len, rxBuf) != CAN_OK) return;      // Read data: len = data length, buf = data byte(s)
  sprintf(msgString, "micros-last=%lu", micros() - last);
  Serial.println(msgString);
  last = micros();
  for (byte i = 0; i < JOYS; i++){
    unsigned long joy_data = 0;
    for (byte j = 0; j < DATA_WIDTH; j++) {
      joy_data <<= 8;
      joy_data |= rxBuf[i * DATA_WIDTH + j];
    }
    g_joy_data[i] = joy_data;
  }
  setHigh(CTR1_DATA);
}

void requestControllers() {
  while (PIND & 0b00000100) {
    CAN0.sendMsgBuf(0x200, 0, 0, NULL);
  }
}

void loop()
{
  requestControllers();
  readCan();
  send_baseunit();
}

