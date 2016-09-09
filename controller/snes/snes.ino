// CAN Send Example
//

#include <mcp_can.h>
#include <SPI.h>

#define CAN0_INT 2    // Set INT to pin 2

#define CTR_CLOCK  0b00001000   // Controller clock pin
#define CTR_LATCH  0b00010000   // Controller latch pin
#define CTR_DATA_0 0b00100000  // Controller 0 data pin
#define CTR_DATA_1 0b01000000  // Controller 1 data pin
#define CTR_DATA_2 7  // Controller 2 data pin
#define CTR_DATA_3 8  // Controller 3 data pin

#define JOYS 2        // Number of joysticks/controllers
#define CLOCKS 15     // Number of pulses to send to controllers
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

  DDRD |= 0b00011000;
  PORTD |= 0b00011000;
  noInterrupts();
}

// We cannot use shiftIn() function from arduino, as we need to be able to do things in parallel.
boolean read_controllers()
{
  unsigned int joy_data[4] = {0, 0, 0, 0};
  byte joy_pin[4] = {CTR_DATA_0, CTR_DATA_1, CTR_DATA_2, CTR_DATA_3};
  boolean changed = false;

  //digitalWrite(CTR_LATCH, HIGH);
  PORTD |= CTR_LATCH;
  delayMicroseconds(12);
  //digitalWrite(CTR_LATCH, LOW);
  PORTD &= ~CTR_LATCH;
  delayMicroseconds(6);
  for (int j = 0; j < CLOCKS; j++)
  {
    //digitalWrite(CTR_CLOCK, LOW);
    PORTD &= ~CTR_CLOCK;
    for (int i = 0; i < JOYS; i++)
    {
      joy_data[i] <<= 1;
      //joy_data[i] |= !digitalRead(joy_pin[i]);
      if (PIND & joy_pin[i]) joy_data[i] &= ~1; else joy_data[i] |= 1;
    }
    delayMicroseconds(6);
    //digitalWrite(CTR_CLOCK, HIGH);
    PORTD |= CTR_CLOCK;
    delayMicroseconds(6);

  }
  //sprintf(msgString, "joy_data(0): 0x%x", joy_data[0]);
  //Serial.println(msgString);

  for (int i = 0; i < JOYS; i++)
  {
    if (g_joy_data[i] != joy_data[i])
    {
      g_joy_data[i] = joy_data[i];
      changed = true;
    }
  }

  return changed;
}

void loop()
{
  if (read_controllers())
  {
    // Convert joystick data from 32 bit int to 8 bit data array
    for (int j = 0; j < JOYS; j++)
    {
      int joy_data = g_joy_data[j];
      for (int i = DATA_WIDTH - 1; i >= 0; i--)
      {
        g_can_data[(j * DATA_WIDTH) + i] = (byte)(joy_data & 0xff);
        joy_data >>= 8;
      }
      //sprintf(msgString, "joy_data(%d): 0x%x", j, g_joy_data[j]);
      //Serial.println(msgString);

    }

    // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
    byte sndStat = CAN0.sendMsgBuf(0x100, 0, JOYS * DATA_WIDTH, g_can_data);
    if(sndStat == CAN_OK) {
      //Serial.println("Message Sent Successfully!");
    } else {
      //Serial.println("Error Sending Message...");
    }
  }
}

