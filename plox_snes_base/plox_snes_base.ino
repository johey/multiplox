// CAN Send Example
//

#include <mcp_can.h>
#include <SPI.h>

#define MODE 1        // 0: hooks up to controllers
// 1: hooks up to base unit

#define CAN0_INT 2    // Set INT to pin 2

#define CTR_CLOCK  0b00010000   // Controller clock pin
#define CTR_LATCH  0b00001000   // Controller latch pin
#define CTR_DATA_0 0b00100000  // Controller 0 data pin
#define CTR_DATA_1 6  // Controller 1 data pin
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

  if (MODE == 0)
  {
    pinMode(CTR_LATCH, OUTPUT);
    pinMode(CTR_CLOCK, OUTPUT);
    pinMode(CTR_DATA_0, INPUT);
    pinMode(CTR_DATA_1, INPUT);
    pinMode(CTR_DATA_2, INPUT);
    pinMode(CTR_DATA_3, INPUT);

    digitalWrite(CTR_LATCH, LOW);
    digitalWrite(CTR_CLOCK, HIGH);
  }
  else {
    DDRD |= 0b11100000;
    PORTD |= 0b11100000;
  }
  noInterrupts();
}

// We cannot use shiftIn() function from arduino, as we need to be able to do things in parallel.
boolean read_controllers()
{
  unsigned int joy_data[JOYS] = {0,0};
  byte joy_pin[4] = {CTR_DATA_0, CTR_DATA_1, CTR_DATA_2, CTR_DATA_3};
  boolean changed = false;

  digitalWrite(CTR_LATCH, HIGH);
  delayMicroseconds(12);
  digitalWrite(CTR_LATCH, LOW);
  delayMicroseconds(6);
  for (int j = 0; j < CLOCKS; j++)
  {
    digitalWrite(CTR_CLOCK, LOW);
    for (int i = 0; i < JOYS; i++)
    {
      joy_data[i] <<= 1;
      joy_data[i] |= !digitalRead(joy_pin[i]);
      //joy_data[i] |= !digitalRead(CTR_DATA_0);
    }
    //delayMicroseconds(6);
    digitalWrite(CTR_CLOCK, HIGH);
    //delayMicroseconds(6);

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

boolean send_baseunit()
{
  unsigned int joy_data[JOYS];
  byte joy_pin[4] = {CTR_DATA_0, CTR_DATA_1, CTR_DATA_2, CTR_DATA_3};

  joy_data[0] = g_joy_data[0];
  //if (digitalRead(CTR_LATCH)) {
  if (PIND & CTR_LATCH) {
    //sprintf(msgString, "joydata: %x", j, g_joy_data[j]);
    //Serial.println(msgString);
    if (joy_data[0] & 0x8000) PORTD &= ~CTR_DATA_0; // write first data bit
    else PORTD |= CTR_DATA_0;
    while (PIND & CTR_LATCH); // wait for latch to finish
    for (int i = 0; i < CLOCKS; i++) {
      while (!(PIND & CTR_CLOCK)); // wait for clock low
      joy_data[0] <<= 1; // shift data
      if (joy_data[0] & 0x8000) PORTD &= ~CTR_DATA_0;
      else PORTD |= CTR_DATA_0;
      delayMicroseconds(10);
      while (PIND & CTR_CLOCK); // wait for clock high
    }
    //*/
  }
  return true;
}

void loop()
{
  if (MODE == 0)
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
        sprintf(msgString, "joy_data(%d): 0x%x", j, g_joy_data[j]);
        Serial.println(msgString);

      }

      // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
      byte sndStat = CAN0.sendMsgBuf(0x100, 0, JOYS * DATA_WIDTH, g_can_data);
      if(sndStat == CAN_OK){
        Serial.println("Message Sent Successfully!");
      } else {
        Serial.println("Error Sending Message...");
      }
    }
    delay(10);
  }
  else 
  {//*
    //if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
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
          //sprintf(msgString, "joy_data[%d]: 0x%x", i, g_joy_data[i]);
          //Serial.println(msgString);
        }
      }

      //Serial.println();
    }
    //*/

    if (send_baseunit())
    {
    }

  }
}

