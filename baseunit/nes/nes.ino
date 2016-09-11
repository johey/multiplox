// CAN Send Example
//

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

#define toggle(myPort, myBit) myPort ^= myBit
#define readBit(x) ((*x.pin & x.bit) != 0)

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
  noInterrupts();
}

boolean send_baseunit()
{
  unsigned int joy_data[JOYS];
  table joy[JOYS] = {CTR0_DATA, CTR1_DATA};
  table clk[JOYS] = {CTR0_CLK, CTR1_CLK};
  table latch[JOYS] = {CTR0_LATCH, CTR1_LATCH};

  if (!readBit(latch[0]))
    return false;
   
  joy_data[0] = g_joy_data[0];
  joy_data[1] = g_joy_data[1];

  for (byte j = 0; j < 1; j++) {
    setBit(joy[j], !(joy_data[j] & 0x8000));
    while (readBit(latch[0])); // wait for latch to finish
    for (int i = 0; i < CLOCKS; i++) {
      while (!readBit(clk[0]));
      joy_data[j] <<= 1; // shift data
      setBit(joy[j], !(joy_data[j] & 0x8000));
      while (readBit(clk[0])); // wait for clock high
    }
    setHigh(joy[j]);
  }
  
  return true;
}

void loop()
{
  if (!(PIND & 0b00000100))
  {
    //interrupts();
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    //noInterrupts();

    //if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
    //    sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    //else
    //    sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);

    //Serial.println(msgString);

    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      //    sprintf(msgString, " REMOTE REQUEST FRAME");
      //    Serial.print(msgString);
    } else {
      for (byte i = 0; i < JOYS; i++){
        unsigned long joy_data = 0;
        for (byte j = 0; j < DATA_WIDTH; j++) {
          joy_data <<= 8;
          joy_data |= rxBuf[i * DATA_WIDTH + j];
        }
        g_joy_data[i] = joy_data;
        //g_joy_data[i] = joy_data >> 8 | (joy_data << 8);
        sprintf(msgString, "joy_data[%d]: 0x%x", i, g_joy_data[i]);
        Serial.println(msgString);
      }
    }

    Serial.println();
  }
  //*/

  if (send_baseunit())
  {
  }
}

