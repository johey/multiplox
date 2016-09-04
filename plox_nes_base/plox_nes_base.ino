// CAN Send Example
//

#include <mcp_can.h>
#include <SPI.h>

#define CAN0_INT 2    // Set INT to pin 2

#define CTR_CLOCK  0b00001000   // Controller clock pin
#define CTR_LATCH  0b00010000   // Controller latch pin
#define CTR_DATA_0 0b01000000  // Controller 0 data pin
#define CTR_DATA_1 0b01000000  // Controller 0 data pin

#define CTR_DATA_2 7  // Controller 2 data pin
#define CTR_DATA_3 8  // Controller 3 data pin

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

    DDRD |= 0b11100000;
    PORTD |= 0b11100000;
    noInterrupts();
}

boolean send_baseunit()
{
    unsigned int joy_data[JOYS];
    byte joy_pin[4] = {CTR_DATA_0, CTR_DATA_1, CTR_DATA_2, CTR_DATA_3};

    if (!(PIND & CTR_LATCH))
      return false;
      
    joy_data[0] = g_joy_data[0];

    if (joy_data[0] & 0x8000) PORTD &= ~CTR_DATA_0; // write first data bit
    else PORTD |= CTR_DATA_0;
    while (PIND & CTR_LATCH); // wait for latch to finish
    for (int i = 0; i < CLOCKS; i++) {
        while (!(PIND & CTR_CLOCK)); // wait for clock low
        //delayMicroseconds(3);
        joy_data[0] <<= 1; // shift data
        if (joy_data[0] & 0x8000) PORTD &= ~CTR_DATA_0;
        else PORTD |= CTR_DATA_0;
        while (PIND & CTR_CLOCK); // wait for clock high
    }
    PORTD |= CTR_DATA_0;
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

