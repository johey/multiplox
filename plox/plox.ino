// CAN Send Example
//

#include <mcp_can.h>
#include <SPI.h>

#define MODE 0        // 0: hooks up to controllers
                      // 1: hooks up to base unit

#define CAN0_INT 2    // Set INT to pin 2

#define CTR_CLOCK 3   // Controller clock pin
#define CTR_LATCH 4   // Controller latch pin
#define CTR_DATA_0 5  // Controller 0 data pin
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
        pinMode(CTR_DATA, INPUT);

        digitalWrite(CTR_LATCH, LOW);
        digitalWrite(CTR_CLOCK, HIGH);
    }
    else {
        pinMode(CTR_LATCH, INPUT);
        pinMode(CTR_CLOCK, INPUT);
        pinMode(CTR_DATA, OUTPUT);

        digitalWrite(CTR_DATA, HIGH);
    }

}

// We cannot use shiftIn() function from arduino, as we need to be able to do things in parallel.
boolean read_controllers()
{
    unsigned int joy_data[JOYS] = {0, 0, 0, 0};
    byte joy_pin[4] = {CTR_DATA_0, CRT_DATA_1, CRT_DATA_2, CTR_DATA_3};
    boolean changed = false;

    digitalWrite(CTR_LATCH, HIGH);
    delayMicroseconds(12);
    digitalWrite(CTR_LATCH, LOW);
    delayMicroseconds(6);
    for (int j = 0; j < CLOCKS; j++)
    {
        digitalWrite(CRT_CLOCK, LOW);
        for (int i = 0; i < JOYS; i++)
        {
            data[i] << 1;
            data[i] |= digitalRead(joy_pin[i]);
        }
        delayMicroseconds(6);
        digitalWrite(CRT_CLOCK, HIGH);
        delayMicroseconds(6);
    }

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
    return true;
}

void loop()
{
    if (MODE == 0 && read_controllers())
    {
        // Convert joystick data from 32 bit int to 8 bit data array
        for (int j = 0; j < JOYS; j++)
        {
            int joy_data = g_joy_data[0];
            for (int i = 0; i < DATA_WIDTH ; i++)
            {
                g_can_data[(j * DATA_WIDTH) + i] = (byte)(joy_data & 0xff);
                joy_data >> 8;
            }
        }

        // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
        byte sndStat = CAN0.sendMsgBuf(0x100, 0, JOYS * DATA_WIDTH, g_can_data);
        if(sndStat == CAN_OK){
            Serial.println("Message Sent Successfully!");
        } else {
            Serial.println("Error Sending Message...");
        }
    }
    else 
    {
        if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
        {
            CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)

            if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
                sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
            else
                sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);

            Serial.print(msgString);

            if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
                sprintf(msgString, " REMOTE REQUEST FRAME");
                Serial.print(msgString);
            } else {
                for(byte i = 0; i<len; i++){
                    sprintf(msgString, " 0x%.2X", rxBuf[i]);
                    Serial.print(msgString);
                }
            }

            Serial.println();
        }

        if (send_baseunit())
        {
        }

    }
}

