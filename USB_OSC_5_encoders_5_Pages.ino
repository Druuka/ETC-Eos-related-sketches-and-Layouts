

// Copyright (c) 2017 Electronic Theatre Controls, Inc., http://www.etcconnect.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


/*******************************************************************************

     Electronic Theatre Controls

     lighthack - Box 1

     (c) 2017 by ETC


     This code implements a Pan/Tilt module using two encoders and three
     buttons. The two encoders function as pan and tilt controllers with one
     button being reserved for controlling fine/coarse mode. The other two
     buttons are assigned to Next and Last which make it easy to switch between
     channels.

 *******************************************************************************

    NOTE: UPDATE VERSION_STRING IN DEFINITIONS BELOW WHEN VERSION NUMBER CHANGES

    Revision History

    yyyy-mm-dd   Vxx      By_Who                 Comment

    2017-07-21   1.0.0.1  Ethan Oswald Massey    Original creation

    2017-10-19   1.0.0.2  Sam Kearney            Fix build errors on some
                                                 Arduino platforms. Change
                                                 OSC subscribe parameters

    2017-10-24   1.0.0.3  Sam Kearney            Add ability to scale encoder
                                                 output

    2017-11-22   1.0.0.4  Hans Hinrichsen        Add splash msg before Eos
                                                 connects

    2017-12-07   1.0.0.5  Hans Hinrichsen        Added timeout to disconnect
                                                 and show splash screen again

 ******************************************************************************/

/*******************************************************************************
   Includes
 ******************************************************************************/
#include <SPI.h>
#include <OSCBoards.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>
#include <OSCTiming.h>
#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);
#endif
#include <string.h>

/*******************************************************************************
   Macros and Constants
 ******************************************************************************/


#define ONE_BTN            11
#define TWO_BTN            12
#define THREE_BTN          13
#define SHIFT_BTN           2





#define SUBSCRIBE           ((int32_t)1)
#define UNSUBSCRIBE         ((int32_t)0)

#define EDGE_DOWN           ((int32_t)1)
#define EDGE_UP             ((int32_t)0)

#define FORWARD             0
#define REVERSE             1

// Change these values to switch which direction increase/decrease pan/tilt
#define ENC1_DIR            FORWARD
#define ENC2_DIR            FORWARD
#define ENC3_DIR            FORWARD
#define ENC4_DIR            FORWARD
#define ENC5_DIR            FORWARD

// Use these values to make the encoder more coarse or fine. This controls
// the number of wheel "ticks" the device sends to Eos for each tick of the
// encoder. 1 is the default and the most fine setting. Must be an integer.
//changed to an int so they can be modified at run time.
int ENC1_SCALE = 1;
int ENC2_SCALE = 1;
int ENC3_SCALE = 1;
int ENC4_SCALE = 1;
int ENC5_SCALE = 1;


int sensorPin = A0;
float sensorValue = 0.000;
float outputValue = 0.000;
float sensorVal = 0.000;
float sensorValstring = 0.000;



//added variables to set global "coarse" and "normal" encoder modes
int encNorm = 1;
int encCoarse = 8;

#define SIG_DIGITS          2   // Number of significant digits displayed

#define OSC_BUF_MAX_SIZE    512

const String HANDSHAKE_QUERY = "ETCOSC?";
const String HANDSHAKE_REPLY = "OK";

const String EOS_KEY = "/eos/key/";




//See displayScreen() below - limited to 10 chars (after 6 prefix chars)
const String VERSION_STRING = "1.0.0.5C";

// Change these values to alter how long we wait before sending an OSC ping
// to see if Eos is still there, and then finally how long before we
// disconnect and show the splash screen
// Values are in milliseconds
#define PING_AFTER_IDLE_INTERVAL    5000
#define TIMEOUT_AFTER_IDLE_INTERVAL 15000


char currentChannel[255];
int mode = 1;

static uint32_t debounceTime1 = 0;
static uint32_t debounceTime2 = 0;
static uint32_t debounceTime3 = 0;


/*******************************************************************************
   Local Types
 ******************************************************************************/
enum WHEEL_TYPE { ENC1, ENC2, ENC3, ENC4, ENC5 };

enum WHEEL_MODE { COARSE, FINE };

struct Encoder
{
  uint8_t pinA;
  uint8_t pinB;
  int pinAPrevious;
  int pinBPrevious;
  float pos;
  uint8_t direction;
};
struct Encoder enc1Wheel;
struct Encoder enc2Wheel;
struct Encoder enc3Wheel;
struct Encoder enc4Wheel;
struct Encoder enc5Wheel;

/*******************************************************************************
   Global Variables
 ******************************************************************************/


bool connectedToEos = false;
unsigned long lastMessageRxTime = 0;
bool timeoutPingSent = false;

/*******************************************************************************
   Local Functions
 ******************************************************************************/

/*******************************************************************************
   Issues all our subscribes to Eos. When subscribed, Eos will keep us updated
   with the latest values for a given parameter.

   Parameters:  none

   Return Value: void

 ******************************************************************************/
void issueSubscribes()
{
  // Add a filter so we don't get spammed with unwanted OSC messages from Eos
  OSCMessage filter("/eos/filter/add");
  filter.add("/eos/out/param/*");
  filter.add("/eos/out/ping");
  SLIPSerial.beginPacket();
  filter.send(SLIPSerial);
  SLIPSerial.endPacket();
    // subscribe to Eos pan & tilt updates
    OSCMessage subPan("/eos/subscribe/param/pan/tilt/zoom/edge");
    subPan.add(SUBSCRIBE);
    SLIPSerial.beginPacket();
    subPan.send(SLIPSerial);
    SLIPSerial.endPacket();
  
OSCMessage faderSetup("eos/fader/1/config/1/10");
  SLIPSerial.beginPacket();
faderSetup.send(SLIPSerial);
  SLIPSerial.endPacket();


}

/*******************************************************************************
   Given a valid OSCMessage (relevant to Pan/Tilt), we update our Encoder struct
   with the new position information.

   Parameters:
    msg - The OSC message we will use to update our internal data
    addressOffset - Unused (allows for multiple nested roots)

   Return Value: void

 ******************************************************************************/
void parseNull(OSCMessage& msg, int addressOffset)
{

}


/*******************************************************************************
   Given an unknown OSC message we check to see if it's a handshake message.
   If it's a handshake we issue a subscribe, otherwise we begin route the OSC
   message to the appropriate function.

   Parameters:
    msg - The OSC message of unknown importance

   Return Value: void

 ******************************************************************************/
void parseOSCMessage(String& msg)
{
  // check to see if this is the handshake string
  if (msg.indexOf(HANDSHAKE_QUERY) != -1)
  {
    // handshake string found!
    SLIPSerial.beginPacket();
    SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
    SLIPSerial.endPacket();

    // Let Eos know we want updates on some things
    issueSubscribes();

    // Make our splash screen go away
    connectedToEos = true;
  }
  else
  {
    // prepare the message for routing by filling an OSCMessage object with our message string
    OSCMessage oscmsg;
    oscmsg.fill((uint8_t*)msg.c_str(), (int)msg.length());

  }
}

/*******************************************************************************
   Initializes a given encoder struct to the requested parameters.

   Parameters:
    encoder - Pointer to the encoder we will be initializing
    pinA - Where the A pin is connected to the Arduino
    pinB - Where the B pin is connected to the Arduino
    direction - Determines if clockwise or counterclockwise is "forward"

   Return Value: void

 ******************************************************************************/
void initEncoder(struct Encoder* encoder, uint8_t pinA, uint8_t pinB, uint8_t direction)
{
  encoder->pinA = pinA;
  encoder->pinB = pinB;
  encoder->pos = 0;
  encoder->direction = direction;

  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);

  encoder->pinAPrevious = digitalRead(pinA);
  encoder->pinBPrevious = digitalRead(pinB);
}

/*******************************************************************************
   Checks if the encoder has moved by comparing the previous state of the pins
   with the current state. If they are different, we know there is movement.
   In the event of movement we update the current state of our pins.

   Parameters:
    encoder - Pointer to the encoder we will be checking for motion

   Return Value:
    encoderMotion - Returns the 0 if the encoder has not moved
                                1 for forward motion
                               -1 for reverse motion

 ******************************************************************************/
int8_t updateEncoder(struct Encoder* encoder)
{
  int8_t encoderMotion = 0;
  int pinACurrent = digitalRead(encoder->pinA);
  int pinBCurrent = digitalRead(encoder->pinB);

  // has the encoder moved at all?
  if (encoder->pinAPrevious != pinACurrent)
  {
    // Since it has moved, we must determine if the encoder has moved forwards or backwards
    encoderMotion = (encoder->pinAPrevious == encoder->pinBPrevious) ? -1 : 1;

    // If we are in reverse mode, flip the direction of the encoder motion
    if (encoder->direction == REVERSE)
      encoderMotion = -encoderMotion;
  }
  encoder->pinAPrevious = pinACurrent;
  encoder->pinBPrevious = pinBCurrent;

  return encoderMotion;
}

/*******************************************************************************
   Sends a message to Eos informing them of a wheel movement.

   Parameters:
    type - the type of wheel that's moving (i.e. pan or tilt)
    ticks - the direction and intensity of the movement

   Return Value: void

 ******************************************************************************/
void sendWheelMove(WHEEL_TYPE type, float ticks)
{
  String wheelMsg("/eos/active/wheel");

  if (digitalRead(SHIFT_BTN) == LOW)
    wheelMsg.concat("/fine");
  else
    wheelMsg.concat("/coarse");

  if (type == ENC1) {
      wheelMsg.concat("/1");
  }
  else if (type == ENC2) {
      wheelMsg.concat("/2");
  }
  else if (type == ENC3) {
      wheelMsg.concat("/3");
  }
  else if (type == ENC4) {
      wheelMsg.concat("/4");
  }
  else if (type == ENC5) {
      wheelMsg.concat("/5");
  }  
  else
    // something has gone very wrong
    return;

  OSCMessage wheelUpdate(wheelMsg.c_str());
  wheelUpdate.add(ticks);
  SLIPSerial.beginPacket();
  wheelUpdate.send(SLIPSerial);
  SLIPSerial.endPacket();
}

/*******************************************************************************
   Sends a message to Eos informing them of a key press.

   Parameters:
    down - whether a key has been pushed down (true) or released (false)
    key - the key that has moved

   Return Value: void

 ******************************************************************************/
void sendKeyPress(bool down, String key)
{
  key = "/eos/key/" + key;
  OSCMessage keyMsg(key.c_str());

  //  if (down)
  //    keyMsg.add(EDGE_DOWN);
  //  else
  //    keyMsg.add(EDGE_UP);

  SLIPSerial.beginPacket();
  keyMsg.send(SLIPSerial);
  SLIPSerial.endPacket();
}



/*******************************************************************************
   Checks the status of all the buttons relevant to Eos (i.e. Next & Last)

   NOTE: This does not check the shift key. The shift key is used in tandem with
   the encoder to determine coarse/fine mode and thus does not report to Eos
   directly.

   Parameters: none

   Return Value: void

 ******************************************************************************/
void checkButtons()
{
  static int oneKeyState = HIGH;
  static int twoKeyState = HIGH;
  static int threeKeyState = HIGH;




  // Has the button state changed
  if (digitalRead(ONE_BTN) != oneKeyState)
  {
    oneKeyState = digitalRead(ONE_BTN);
    // Notify Eos of this key press
    if (oneKeyState == LOW)
    {
      debounceTime1 = millis();
    }
    else
    {
      debounceTime1 = 0;
    }
  }

  if (debounceTime1 > 0 && (millis() - debounceTime1 > 10)) {
    // ... set the time stamp to 0 to say we have finished debouncing
    debounceTime1 = 0;
      OSCMessage LoadMsg("/eos/fader/1/1/load");
  SLIPSerial.beginPacket();
  LoadMsg.send(SLIPSerial);
  SLIPSerial.endPacket();
  }


  if (digitalRead(TWO_BTN) != twoKeyState)
  {
    twoKeyState = digitalRead(TWO_BTN);
    if (twoKeyState == LOW)
    {
      debounceTime2 = millis();

    }
    else
    {
      debounceTime2 = 0;

    }
  }

  if (debounceTime2 > 0 && (millis() - debounceTime2 > 10)) {
    // ... set the time stamp to 0 to say we have finished debouncing
    debounceTime2 = 0;
      OSCMessage StopMsg("/eos/fader/1/1/stop");
  SLIPSerial.beginPacket();
  StopMsg.send(SLIPSerial);
  SLIPSerial.endPacket();
  }

  if (digitalRead(THREE_BTN) != threeKeyState)
  {
    threeKeyState = digitalRead(THREE_BTN);
    if (threeKeyState == LOW)
    {
      debounceTime3 = millis();

    }
    else
    {
      debounceTime3 = 0;

    }
  }

  if (debounceTime3 > 0 && (millis() - debounceTime3 > 10)) {
    // ... set the time stamp to 0 to say we have finished debouncing
    debounceTime3 = 0;
      OSCMessage FireMsg("/eos/fader/1/1/fire");
  SLIPSerial.beginPacket();
  FireMsg.send(SLIPSerial);
  SLIPSerial.endPacket();
  }

}


void checkFaders()
{
  sensorValue = analogRead(sensorPin);
  outputValue = sensorValue;
  outputValue = constrain(outputValue, 0, 1000);
  //  String sensorValstring = String(outputValue/10);
  sensorValstring = outputValue / 1000;
  // sensorValstring = sensorValstring + '.' + (outputValue % 1000);


  if (sensorVal == sensorValue)
    //    if( abs(sensorValstring2 - outputValue) < THRESHOLD)
    //value has really changed - do something with the new value
  {

  }
  else if (sensorVal > sensorValue + 3 or sensorVal < sensorValue - 3)
  {
    OSCMessage msg("/eos/fader/1/1");

  SLIPSerial.beginPacket();
      msg.add(sensorValstring);
  msg.send(SLIPSerial);
  SLIPSerial.endPacket();

 
    msg.empty();
  }



  delay(10);
  sensorVal = sensorValue;
}


/*******************************************************************************
   Here we setup our encoder, lcd, and various input devices. We also prepare
   to communicate OSC with Eos by setting up SLIPSerial. Once we are done with
   setup() we pass control over to loop() and never call setup() again.

   NOTE: This function is the entry function. This is where control over the
   Arduino is passed to us (the end user).

   Parameters: none

   Return Value: void

 ******************************************************************************/
void setup()
{
  SLIPSerial.begin(115200);
  // This is a hack around an Arduino bug. It was taken from the OSC library
  //examples
  //#ifdef BOARD_HAS_USB_SERIAL
  //  while (!SerialUSB);
  //#else
  //  while (!Serial);
  //#endif

  // This is necessary for reconnecting a device because it needs some time
  // for the serial port to open, but meanwhile the handshake message was
  // sent from Eos
  SLIPSerial.beginPacket();
  SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
  SLIPSerial.endPacket();
  // Let Eos know we want updates on some things
  issueSubscribes();

  initEncoder(&enc1Wheel, A1, A2, ENC1_DIR);
  initEncoder(&enc2Wheel, A3, A4, ENC2_DIR);
  initEncoder(&enc3Wheel, 5, 6, ENC3_DIR);
  initEncoder(&enc4Wheel, 7, 8, ENC4_DIR);
  initEncoder(&enc5Wheel, 9, 10, ENC5_DIR);



  pinMode(ONE_BTN, INPUT_PULLUP);
  pinMode(TWO_BTN, INPUT_PULLUP);
  pinMode(THREE_BTN, INPUT_PULLUP);
  pinMode(SHIFT_BTN, INPUT_PULLUP);






}

/*******************************************************************************
   Here we service, monitor, and otherwise control all our peripheral devices.
   First, we retrieve the status of our encoders and buttons and update Eos.
   Next, we check if there are any OSC messages for us.
   Finally, we update our display (if an update is necessary)

   NOTE: This function is our main loop and thus this function will be called
   repeatedly forever

   Parameters: none

   Return Value: void

 ******************************************************************************/
void loop()
{
  static String curMsg;
  int size;
  // get the updated state of each encoder
  int32_t enc1Motion = updateEncoder(&enc1Wheel);
  int32_t enc2Motion = updateEncoder(&enc2Wheel);
  int32_t enc3Motion = updateEncoder(&enc3Wheel);
  int32_t enc4Motion = updateEncoder(&enc4Wheel);
  int32_t enc5Motion = updateEncoder(&enc5Wheel);

  // Scale the result by a scaling factor
  enc1Motion *= ENC1_SCALE;
  enc2Motion *= ENC2_SCALE;
  enc3Motion *= ENC3_SCALE;
  enc4Motion *= ENC4_SCALE;
  enc4Motion *= ENC5_SCALE;

  // check for next/last updates
  checkButtons();
  checkFaders();
  // now update our wheels
  if (enc1Motion != 0)
    sendWheelMove(ENC1, enc1Motion);

  if (enc2Motion != 0)
    sendWheelMove(ENC2, enc2Motion);

  if (enc3Motion != 0)
    sendWheelMove(ENC3, enc3Motion);

  if (enc4Motion != 0)
    sendWheelMove(ENC4, enc4Motion);

  if (enc5Motion != 0)
    sendWheelMove(ENC5, enc5Motion);

  // Then we check to see if any OSC commands have come from Eos
  // and update the display accordingly.
  size = SLIPSerial.available();
  if (size > 0)
  {
    // Fill the msg with all of the available bytes
    while (size--)
      curMsg += (char)(SLIPSerial.read());
  }
  if (SLIPSerial.endofPacket())
  {
    parseOSCMessage(curMsg);
    lastMessageRxTime = millis();
    // We only care about the ping if we haven't heard recently
    // Clear flag when we get any traffic
    timeoutPingSent = false;
    curMsg = String();
  }

  if (lastMessageRxTime > 0)
  {
    unsigned long diff = millis() - lastMessageRxTime;
    //We first check if it's been too long and we need to time out
    if (diff > TIMEOUT_AFTER_IDLE_INTERVAL)
    {
      connectedToEos = false;
      lastMessageRxTime = 0;
      timeoutPingSent = false;
    }

    //It could be the console is sitting idle. Send a ping once to
    // double check that it's still there, but only once after 2.5s have passed
    if (!timeoutPingSent && diff > PING_AFTER_IDLE_INTERVAL)
    {
      OSCMessage ping("/eos/ping");
      ping.add("5Encoder"); // This way we know who is sending the ping
      SLIPSerial.beginPacket();
      ping.send(SLIPSerial);
      SLIPSerial.endPacket();
      timeoutPingSent = true;
    }
  }


}

