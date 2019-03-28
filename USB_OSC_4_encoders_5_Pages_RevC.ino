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

    This code adds to the initial with the addition of 2 more encoders bringing the total to 4 and adding 5 pages of encoder layouts
    This box is setup for 8 buttons plus using each encoder as a button bringing the total button inputs to 12
    Next/Last/Shift/Page1/Page2/Page3/Page4/Page5/Encoder1/Encoder2/Encoder3/Encoder4
    This sketch is intended for a Teensy 3.5 
    This sketch will not work on an uno. 
    It should work on a Mega2560 but would need pin changes for inputs.
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
/*   2018-10-17   1.0.0.5B Andrew Webberley       Added Multiple pages with attribute on command line with encoder press. 
                                                  Changed how buttons are treated
                                                  Hold encoder for super coarse mode.
                                                  2X40 Character Display
                                                  Teensy 3.5
                                                  If you want to use with consoles you must either spoof the PID/VID of an Uno or install Teensy serial driver
                                                  

 */                                                 
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
#include <LiquidCrystal.h>
#include <string.h>

/*******************************************************************************
   Macros and Constants
 ******************************************************************************/
#define LCD_CHARS           40
#define LCD_LINES           2   // Currently assume at least 2 lines

#define NEXT_BTN            37
#define LAST_BTN            38
#define SHIFT_BTN           39
#define MODE1_BTN           24
#define MODE2_BTN           25
#define MODE3_BTN           26
#define MODE4_BTN           27
#define MODE5_BTN           28
#define ENC1_BTN            33
#define ENC2_BTN            34
#define ENC3_BTN            35
#define ENC4_BTN            36


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

// Use these values to make the encoder more coarse or fine. This controls
// the number of wheel "ticks" the device sends to Eos for each tick of the
// encoder. 1 is the default and the most fine setting. Must be an integer.
//#define ENC1_SCALE          1
//#define ENC2_SCALE          1
//#define ENC3_SCALE          1
//#define ENC4_SCALE          1
//changed to an int so they can be modified at run time.
int ENC1_SCALE = 1;
int ENC2_SCALE = 1;
int ENC3_SCALE = 1;
int ENC4_SCALE = 1;
//added variables to set global "coarse" and "normal" encoder modes
int encNorm = 1;
int encCoarse = 8;
#define SIG_DIGITS          2   // Number of significant digits displayed

#define OSC_BUF_MAX_SIZE    512

const String HANDSHAKE_QUERY = "ETCOSC?";
const String HANDSHAKE_REPLY = "OK";

//See displayScreen() below - limited to 10 chars (after 6 prefix chars)
const String VERSION_STRING = "1.0.0.5B";

// Change these values to alter how long we wait before sending an OSC ping
// to see if Eos is still there, and then finally how long before we
// disconnect and show the splash screen
// Values are in milliseconds
#define PING_AFTER_IDLE_INTERVAL    5000
#define TIMEOUT_AFTER_IDLE_INTERVAL 15000
String enc1Name = "PAN";
String enc2Name = "TILT";
String enc3Name = "EDGE";
String enc4Name = "ZOOM";
char currentChannel[255];
int mode = 1;

static uint32_t debounceTime1 = 0;
static uint32_t debounceTime2 = 0;
static uint32_t debounceTime11 = 0;
static uint32_t debounceTime12 = 0;
static uint32_t debounceTime13 = 0;
static uint32_t debounceTime14 = 0;

/*******************************************************************************
   Local Types
 ******************************************************************************/
enum WHEEL_TYPE { ENC1, ENC2, ENC3, ENC4 };

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

/*******************************************************************************
   Global Variables
 ******************************************************************************/

// initialize the library with the numbers of the interface pins
const int rs = 2, rw = 3, en = 5, d4 = 6, d5 = 7, d6 = 8, d7 = 9;
LiquidCrystal lcd(rs, rw, en, d4, d5, d6, d7);

bool updateDisplay = false;
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
  filter.add("/eos/out/active/chan");
  SLIPSerial.beginPacket();
  filter.send(SLIPSerial);
  SLIPSerial.endPacket();


  if (mode == 1) {
    // subscribe to Eos pan & tilt updates
    OSCMessage subOne("/eos/subscribe/param/pan/tilt/zoom/edge");
    subOne.add(SUBSCRIBE);
    SLIPSerial.beginPacket();
    subOne.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subTwo("/eos/subscribe/param/cyan/magenta/yellow/cto");
    subTwo.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subTwo.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subThree("/eos/subscribe/param/thrust_a/angle_a/thrust_c/angle_c");
    subThree.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subThree.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subFour("/eos/subscribe/param/thrust_b/angle_b/thrust_d/angle_d");
    subFour.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subFour.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subFive("/eos/subscribe/param/gobo_select/gobo_ind\\spd/gobo_select_2/gobo_ind\\spd_2");
    subFive.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subFive.send(SLIPSerial);
    SLIPSerial.endPacket();
  }

  if (mode == 2) {
    // subscribe to Eos color updates
    OSCMessage subOne("/eos/subscribe/param/pan/tilt/zoom/edge");
    subOne.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subOne.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subTwo("/eos/subscribe/param/cyan/magenta/yellow/cto");
    subTwo.add(SUBSCRIBE);
    SLIPSerial.beginPacket();
    subTwo.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subThree("/eos/subscribe/param/thrust_a/angle_a/thrust_c/angle_c");
    subThree.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subThree.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subFour("/eos/subscribe/param/thrust_b/angle_b/thrust_d/angle_d");
    subFour.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subFour.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subFive("/eos/subscribe/param/gobo_select/gobo_ind\\spd/gobo_select_2/gobo_ind\\spd_2");
    subFive.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subFive.send(SLIPSerial);
    SLIPSerial.endPacket();

  }
  if (mode == 3) {
    // subscribe to Eos shutter A/C updates
    OSCMessage subOne("/eos/subscribe/param/pan/tilt/zoom/edge");
    subOne.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subOne.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subTwo("/eos/subscribe/param/cyan/magenta/yellow/cto");
    subTwo.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subTwo.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subThree("/eos/subscribe/param/thrust_a/angle_a/thrust_c/angle_c");
    subThree.add(SUBSCRIBE);
    SLIPSerial.beginPacket();
    subThree.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subFour("/eos/subscribe/param/thrust_b/angle_b/thrust_d/angle_d");
    subFour.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subFour.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subFive("/eos/subscribe/param/gobo_select/gobo_ind\\spd/gobo_select_2/gobo_ind\\spd_2");
    subFive.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subFive.send(SLIPSerial);
    SLIPSerial.endPacket();


  }
  if (mode == 4) {
    // subscribe to Eos shutter B/D updates
    OSCMessage subOne("/eos/subscribe/param/pan/tilt/zoom/edge");
    subOne.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subOne.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subTwo("/eos/subscribe/param/cyan/magenta/yellow/cto");
    subTwo.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subTwo.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subThree("/eos/subscribe/param/thrust_a/angle_a/thrust_c/angle_c");
    subThree.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subThree.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subFour("/eos/subscribe/param/thrust_b/angle_b/thrust_d/angle_d");
    subFour.add(SUBSCRIBE);
    SLIPSerial.beginPacket();
    subFour.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subFive("/eos/subscribe/param/gobo_select/gobo_ind\\spd/gobo_select_2/gobo_ind\\spd_2");
    subFive.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subFive.send(SLIPSerial);
    SLIPSerial.endPacket();



  }
  if (mode == 5) {
    // subscribe to Eos GOBO updates
    OSCMessage subOne("/eos/subscribe/param/pan/tilt/zoom/edge");
    subOne.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subOne.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subTwo("/eos/subscribe/param/cyan/magenta/yellow/cto");
    subTwo.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subTwo.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subThree("/eos/subscribe/param/thrust_a/angle_a/thrust_c/angle_c");
    subThree.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subThree.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subFour("/eos/subscribe/param/thrust_b/angle_b/thrust_d/angle_d");
    subFour.add(UNSUBSCRIBE);
    SLIPSerial.beginPacket();
    subFour.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subFive("/eos/subscribe/param/gobo_select/gobo_ind\\spd/gobo_select_2/gobo_ind\\spd_2");
    subFive.add(SUBSCRIBE);
    SLIPSerial.beginPacket();
    subFive.send(SLIPSerial);
    SLIPSerial.endPacket();


  }
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

void parseenc1Update(OSCMessage& msg, int addressOffset)
{
  enc1Wheel.pos = msg.getOSCData(0)->getFloat();
  connectedToEos = true; // Update this here just in case we missed the handshake
  updateDisplay = true;
}

void parseenc2Update(OSCMessage& msg, int addressOffset)
{
  enc2Wheel.pos = msg.getOSCData(0)->getFloat();
  connectedToEos = true; // Update this here just in case we missed the handshake
  updateDisplay = true;
}


void parseenc3Update(OSCMessage& msg, int addressOffset)
{
  enc3Wheel.pos = msg.getOSCData(0)->getFloat();
  connectedToEos = true; // Update this here just in case we missed the handshake
  updateDisplay = true;
}

void parseenc4Update(OSCMessage& msg, int addressOffset)
{
  enc4Wheel.pos = msg.getOSCData(0)->getFloat();
  connectedToEos = true; // Update this here just in case we missed the handshake
  updateDisplay = true;
}

void parseChannelUpdate(OSCMessage& msg, int addressOffset)
{
  int length = msg.getDataLength(0);
  msg.getString(0, currentChannel, length);
  char * p = strchr (currentChannel, ' ');  // search for space
  if (p)     // if found truncate at space
    *p = 0;
  connectedToEos = true; // Update this here just in case we missed the handshake
  updateDisplay = true;
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
    updateDisplay = true;
  }
  else
  {
    // prepare the message for routing by filling an OSCMessage object with our message string
    OSCMessage oscmsg;
    oscmsg.fill((uint8_t*)msg.c_str(), (int)msg.length());
    // route pan/tilt messages to the relevant update function
    if (mode == 1); {
      oscmsg.route("/eos/out/param/pan", parseenc1Update);
      oscmsg.route("/eos/out/param/tilt", parseenc2Update);
      oscmsg.route("/eos/out/param/edge", parseenc3Update);
      oscmsg.route("/eos/out/param/zoom", parseenc4Update);
      oscmsg.route("/eos/out/active/chan", parseChannelUpdate);

    }

    if (mode == 2); {
      oscmsg.route("/eos/out/param/cyan", parseenc1Update);
      oscmsg.route("/eos/out/param/magenta", parseenc2Update);
      oscmsg.route("/eos/out/param/yellow", parseenc3Update);
      oscmsg.route("/eos/out/param/cto", parseenc4Update);
      oscmsg.route("/eos/out/active/chan", parseChannelUpdate);



    }
    if (mode == 3); {
      oscmsg.route("/eos/out/param/thrust_a", parseenc1Update);
      oscmsg.route("/eos/out/param/angle_a", parseenc2Update);
      oscmsg.route("/eos/out/param/thrust_c", parseenc3Update);
      oscmsg.route("/eos/out/param/angle_c", parseenc4Update);
      oscmsg.route("/eos/out/active/chan", parseChannelUpdate);




    }
    if (mode == 4); {
      oscmsg.route("/eos/out/param/thrust_b", parseenc1Update);
      oscmsg.route("/eos/out/param/angle_b", parseenc2Update);
      oscmsg.route("/eos/out/param/thrust_d", parseenc3Update);
      oscmsg.route("/eos/out/param/angle_d", parseenc4Update);
      oscmsg.route("/eos/out/active/chan", parseChannelUpdate);



    }

    if (mode == 5); {
      oscmsg.route("/eos/out/param/gobo_select", parseenc1Update);
      oscmsg.route("/eos/out/param/gobo_ind/spd", parseenc2Update);
      oscmsg.route("/eos/out/param/gobo_select_2", parseenc3Update);
      oscmsg.route("/eos/out/param/gobo_ind/spd_2", parseenc4Update);
      oscmsg.route("/eos/out/active/chan", parseChannelUpdate);


    }
  }
}
/*******************************************************************************
   Updates the display with the latest pan and tilt positions.

   Parameters:  none

   Return Value: void

 ******************************************************************************/
void displayStatus()
{
  //  lcd.clear();

  if (!connectedToEos)
  {
    // display a splash message before the Eos connection is open
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(String("4 Enc v" + VERSION_STRING).c_str());
    lcd.setCursor(0, 1);
    lcd.print("waiting for eos");
  }
  else
  {


    lcd.setCursor(10, 1);
    lcd.print("|");
    lcd.setCursor(20, 0);
    lcd.print("|");
    lcd.setCursor(20, 1);
    lcd.print("|");
    lcd.setCursor(30, 0);
    lcd.print("|");
    lcd.setCursor(30, 1);
    lcd.print("|");


    // put the cursor at the begining of the first line
    lcd.setCursor(0, 0);
    lcd.print("               ");
    lcd.setCursor(0, 0);
    lcd.print(enc1Name);
    lcd.setCursor(0, 1);
    lcd.print("          ");
    lcd.setCursor(0, 1);
    lcd.print(enc1Wheel.pos, SIG_DIGITS);
    //    lcd.print("  ");

    //find out how many digits in the current channel and offset the start to format properly
    int offSet = strlen(currentChannel);
    lcd.setCursor(9 - offSet / 2, 0);
    lcd.print("(");
    lcd.print(currentChannel);
    lcd.print(")");

    //find the length of the current float to set the start position
    float tempFloat = (enc2Wheel.pos, SIG_DIGITS);
    int offSet1 = sizeof(tempFloat);
    offSet1 = (offSet1 + 2);
    lcd.setCursor(15, 0);
    lcd.print("     ");
    lcd.setCursor(15, 0);
    lcd.print(enc2Name);
    lcd.setCursor(11, 1);
    lcd.print("         ");
    lcd.setCursor(19 - offSet1, 1);
    lcd.print(enc2Wheel.pos, SIG_DIGITS);
    //    lcd.print("  ");

    // put the cursor at the begining of the second line
    lcd.setCursor(21, 0);
    lcd.print("         ");
    lcd.setCursor(21, 0);
    lcd.print("  ");
    lcd.print(enc3Name);
    lcd.setCursor(21, 1);
    lcd.print("         ");
    lcd.setCursor(21, 1);
    lcd.print("  ");
    lcd.print(enc3Wheel.pos, SIG_DIGITS);
    //    lcd.print("  ");

    // put the cursor at the begining of the second line
    lcd.setCursor(31, 0);
    lcd.print("         ");
    lcd.setCursor(31, 0);
    lcd.print("  ");    lcd.print(enc4Name);
    lcd.setCursor(31, 1);
    lcd.print("         ");
    lcd.setCursor(31, 1);
    lcd.print("  ");
    lcd.print(enc4Wheel.pos, SIG_DIGITS);
    //    lcd.print("  ");

  }

  updateDisplay = false;
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
  String wheelMsg("/eos/wheel");

  if (digitalRead(SHIFT_BTN) == LOW)
    wheelMsg.concat("/fine");
  else
    wheelMsg.concat("/coarse");

  if (type == ENC1) {
    if (mode == 1)
      wheelMsg.concat("/pan");
    else if (mode == 2)
      wheelMsg.concat("/cyan");
    else if (mode == 3)
      wheelMsg.concat("/thrust_a");
    else if (mode == 4)
      wheelMsg.concat("/thrust_b");
    else if (mode == 5)
      wheelMsg.concat("/gobo_select");


  }
  else if (type == ENC2) {
    if (mode == 1)
      wheelMsg.concat("/tilt");
    else if (mode == 2)
      wheelMsg.concat("/magenta");
    else if (mode == 3)
      wheelMsg.concat("/angle_a");
    else if (mode == 4)
      wheelMsg.concat("/angle_b");
    else if (mode == 5)
      wheelMsg.concat("/gobo_ind\\spd");
  }
  else if (type == ENC3) {
    if (mode == 1)
      wheelMsg.concat("/edge");
    else if (mode == 2)
      wheelMsg.concat("/yellow");
    else if (mode == 3)
      wheelMsg.concat("/thrust_c");
    else if (mode == 4)
      wheelMsg.concat("/thrust_d");
    else if (mode == 5)
      wheelMsg.concat("/gobo_select_2");
  }
  else if (type == ENC4) {
    if (mode == 1)
      wheelMsg.concat("/zoom");
    else if (mode == 2)
      wheelMsg.concat("/cto");
    else if (mode == 3)
      wheelMsg.concat("/angle_c");
    else if (mode == 4)
      wheelMsg.concat("/angle_d");
    else if (mode == 5)
      wheelMsg.concat("/gobo_ind\\spd_2");

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
   Sends a message to Eos informing them of a attribute key press.

   Parameters:
    down - whether a key has been pushed down (true) or released (false)
    key - the key that has moved

   Return Value: void

 ******************************************************************************/
void sendParamPress(String key)
{
  String  key1 = "/eos/cmd";    // + key;
  String keyMsg(key1.c_str());
  key = key;

  if (key == "ENC1") {
    if (mode == 1)
      keyMsg.concat("/pan");
    else if (mode == 2)
      keyMsg.concat("/cyan");
    else if (mode == 3)
      keyMsg.concat("/thrust_a");
    else if (mode == 4)
      keyMsg.concat("/thrust_b");
    else if (mode == 5)
      keyMsg.concat("/gobo_select");

  }
  else if (key == "ENC2") {
    if (mode == 1)
      keyMsg.concat("/tilt");
    else if (mode == 2)
      keyMsg.concat("/magenta");
    else if (mode == 3)
      keyMsg.concat("/angle_a");
    else if (mode == 4)
      keyMsg.concat("/angle_b");
    else if (mode == 5)
      keyMsg.concat("/gobo_ind\\spd");
  }
  else if (key == "ENC3") {
    if (mode == 1)
      keyMsg.concat("/edge");
    else if (mode == 2)
      keyMsg.concat("/yellow");
    else if (mode == 3)
      keyMsg.concat("/thrust_c");
    else if (mode == 4)
      keyMsg.concat("/thrust_d");
    else if (mode == 5)
      keyMsg.concat("/gobo_select_2");
  }
  else if (key == "ENC4") {
    if (mode == 1)
      keyMsg.concat("/zoom");
    else if (mode == 2)
      keyMsg.concat("/cto");
    else if (mode == 3)
      keyMsg.concat("/angle_c");
    else if (mode == 4)
      keyMsg.concat("/angle_d");
    else if (mode == 5)
      keyMsg.concat("/gobo_ind\\spd_2");

  }
  else
    // something has gone very wrong
    return;


  OSCMessage keyMsg1(keyMsg.c_str());


  //    keyMsg1.add(EDGE_DOWN);

  SLIPSerial.beginPacket();
  keyMsg1.send(SLIPSerial);
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
  static int nextKeyState = HIGH;
  static int lastKeyState = HIGH;
  static int mode1KeyState = HIGH;
  static int mode2KeyState = HIGH;
  static int mode3KeyState = HIGH;
  static int mode4KeyState = HIGH;
  static int mode5KeyState = HIGH;

  static int enc1KeyState = HIGH;
  static int enc2KeyState = HIGH;
  static int enc3KeyState = HIGH;
  static int enc4KeyState = HIGH;


  // Has the button state changed
  if (digitalRead(NEXT_BTN) != nextKeyState)
  {
    nextKeyState = digitalRead(NEXT_BTN);
    // Notify Eos of this key press
    if (nextKeyState == LOW)
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
    sendKeyPress(false, "NEXT");
  }


  if (digitalRead(LAST_BTN) != lastKeyState)
  {
    lastKeyState = digitalRead(LAST_BTN);
    if (lastKeyState == LOW)
    {
      debounceTime2 = millis();

      //      sendKeyPress(false, "LAST");
      //      lastKeyState = HIGH;
    }
    else
    {
      debounceTime2 = 0;

      //      sendKeyPress(true, "LAST");
      //      lastKeyState = LOW;
    }
  }

  if (debounceTime2 > 0 && (millis() - debounceTime2 > 10)) {
    // ... set the time stamp to 0 to say we have finished debouncing
    debounceTime2 = 0;
    sendKeyPress(false, "LAST");
  }


  if (digitalRead(MODE1_BTN) != mode1KeyState)
  {
    if (mode1KeyState == LOW)
    {
      enc1Name = "PAN";
      enc2Name = "TILT";
      enc3Name = "EDGE";
      enc4Name = "ZOOM";
      mode = 1;
      mode1KeyState = HIGH;
      issueSubscribes();
      lcd.clear();
    }
    else
    {

      mode1KeyState = LOW;
    }
  }


  if (digitalRead(MODE2_BTN) != mode2KeyState)
  {
    if (mode2KeyState == LOW)
    {
      enc1Name = "CYN";
      enc2Name = "MAG";
      enc3Name = "YEL";
      enc4Name = "CTO";
      mode = 2;
      mode2KeyState = HIGH;
      issueSubscribes();
      lcd.clear();
    }
    else
    {

      mode2KeyState = LOW;
    }
  }

  if (digitalRead(MODE3_BTN) != mode3KeyState)
  {
    if (mode3KeyState == LOW)
    {
      enc1Name = "A THR";
      enc2Name = "A ANG";
      enc3Name = "C THR";
      enc4Name = "C ANG";
      mode = 3;
      mode3KeyState = HIGH;
      issueSubscribes();
      lcd.clear();
    }
    else
    {

      mode3KeyState = LOW;
    }
  }

  if (digitalRead(MODE4_BTN) != mode4KeyState)
  {
    if (mode4KeyState == LOW)
    {
      enc1Name = "B THR";
      enc2Name = "B ANG";
      enc3Name = "D THR";
      enc4Name = "D ANG";
      mode = 4;
      mode4KeyState = HIGH;
      issueSubscribes();
      lcd.clear();
    }
    else
    {

      mode4KeyState = LOW;
    }
  }

  if (digitalRead(MODE5_BTN) != mode5KeyState)
  {
    if (mode5KeyState == LOW)
    {
      enc1Name = "GOBO1";
      enc2Name = "G1ROT";
      enc3Name = "GOBO2";
      enc4Name = "G2ROT";
      mode = 5;
      mode5KeyState = HIGH;
      issueSubscribes();
      lcd.clear();
    }
    else
    {

      mode5KeyState = LOW;
    }
  }

  if (digitalRead(ENC1_BTN) != enc1KeyState)
  {
    enc1KeyState = digitalRead(ENC1_BTN);

    if (enc1KeyState == HIGH)
    {
      debounceTime11 = millis();
             ENC1_SCALE = encNorm;  //added for encoder fine/coarse

    }
    else
    {
      debounceTime11 = 0;
             ENC1_SCALE = encCoarse;  //added for encoder fine/coarse

    }
  }

  if (debounceTime11 > 0 && (millis() - debounceTime11 > 10)) {
    // ... set the time stamp to 0 to say we have finished debouncing
    debounceTime11 = 0;
    sendParamPress("ENC1");

  }




  if (digitalRead(ENC2_BTN) != enc2KeyState)
  {
    enc2KeyState = digitalRead(ENC2_BTN);

    if (enc2KeyState == HIGH)
    {
      debounceTime12 = millis();
             ENC2_SCALE = encNorm;    //added for encoder fine/coarse

    }
    else
    {
      debounceTime12 = 0;
             ENC2_SCALE = encCoarse;  //added for encoder fine/coarse


    }
  }

  if (debounceTime12 > 0 && (millis() - debounceTime12 > 10)) {
    // ... set the time stamp to 0 to say we have finished debouncing
    debounceTime12 = 0;
    sendParamPress("ENC2");

  }


  if (digitalRead(ENC3_BTN) != enc3KeyState)
  {
    enc3KeyState = digitalRead(ENC3_BTN);

    if (enc3KeyState == HIGH)
    {
      debounceTime13 = millis();
             ENC3_SCALE = encNorm;    //added for encoder fine/coarse

    }
    else
    {
      debounceTime13 = 0;
             ENC3_SCALE = encCoarse;  //added for encoder fine/coarse


    }
  }

  if (debounceTime13 > 0 && (millis() - debounceTime13 > 10)) {
    // ... set the time stamp to 0 to say we have finished debouncing
    debounceTime13 = 0;
    sendParamPress("ENC3");

  }


  if (digitalRead(ENC4_BTN) != enc4KeyState)
  {
    enc4KeyState = digitalRead(ENC4_BTN);

    if (enc4KeyState == HIGH)
    {
      debounceTime14 = millis();
             ENC4_SCALE = encNorm;  //added for encoder fine/coarse


    }
    else
    {
      debounceTime14 = 0;
             ENC4_SCALE = encCoarse;  //added for encoder fine/coarse


    }
  }

  if (debounceTime14 > 0 && (millis() - debounceTime14 > 10)) {
    // ... set the time stamp to 0 to say we have finished debouncing
    debounceTime14 = 0;
    sendParamPress("ENC4");

  }
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

    initEncoder(&enc1Wheel, A0, A1, ENC1_DIR);
    initEncoder(&enc2Wheel, A2, A3, ENC2_DIR);
    initEncoder(&enc3Wheel, A4, A5, ENC3_DIR);
    initEncoder(&enc4Wheel, A6, A7, ENC4_DIR);

    lcd.begin(LCD_CHARS, LCD_LINES);
    lcd.clear();

    pinMode(NEXT_BTN, INPUT_PULLUP);
    pinMode(LAST_BTN, INPUT_PULLUP);
    pinMode(SHIFT_BTN, INPUT_PULLUP);
    pinMode(MODE1_BTN, INPUT_PULLUP);
    pinMode(MODE2_BTN, INPUT_PULLUP);
    pinMode(MODE3_BTN, INPUT_PULLUP);
    pinMode(MODE4_BTN, INPUT_PULLUP);
    pinMode(MODE5_BTN, INPUT_PULLUP);
    pinMode(ENC1_BTN, INPUT_PULLUP);
    pinMode(ENC2_BTN, INPUT_PULLUP);
    pinMode(ENC3_BTN, INPUT_PULLUP);
    pinMode(ENC4_BTN, INPUT_PULLUP);

    displayStatus();



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

    // Scale the result by a scaling factor
    enc1Motion *= ENC1_SCALE;
    enc2Motion *= ENC2_SCALE;
    enc3Motion *= ENC3_SCALE;
    enc4Motion *= ENC4_SCALE;

    // check for next/last updates
    checkButtons();

    // now update our wheels
    if (enc1Motion != 0)
      sendWheelMove(ENC1, enc1Motion);

    if (enc2Motion != 0)
      sendWheelMove(ENC2, enc2Motion);

    if (enc3Motion != 0)
      sendWheelMove(ENC3, enc3Motion);

    if (enc4Motion != 0)
      sendWheelMove(ENC4, enc4Motion);

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
        updateDisplay = true;
        timeoutPingSent = false;
      }

      //It could be the console is sitting idle. Send a ping once to
      // double check that it's still there, but only once after 5s have passed
      if (!timeoutPingSent && diff > PING_AFTER_IDLE_INTERVAL)
      {
        OSCMessage ping("/eos/ping");
        ping.add("4Enc_hello"); // This way we know who is sending the ping
        SLIPSerial.beginPacket();
        ping.send(SLIPSerial);
        SLIPSerial.endPacket();
        timeoutPingSent = true;
      }
    }

    if (updateDisplay)
      displayStatus();
  }
