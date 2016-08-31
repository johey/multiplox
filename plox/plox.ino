// CAN Send Example
//

#include <mcp_can.h>
#include <SPI.h>

#define CAN0_INT 2    // Set INT to pin 2
#define JOYS 2        // Number of joysticks/controllers
MCP_CAN CAN0(10);     // Set CS to pin 10

/* Register for each controller, where bits are defined as:
   0: active
   1: clock
   2: latch
   3: data
*/
char joy_reg[JOYS];
char joy_data[2][JOYS];

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
}

byte data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

void loop()
{
  // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  byte sndStat = CAN0.sendMsgBuf(0x100, 0, 8, data);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }
  delay(100);   // send data per 100ms
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/