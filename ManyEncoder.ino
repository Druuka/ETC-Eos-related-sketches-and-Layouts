#include <SD.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
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


Encoder myEnc1(22, 23);        // the number of the encoder 1 pins
Encoder myEnc2(24, 25);        // the number of the encoder 2 pins
Encoder myEnc3(26, 27);        // the number of the encoder 3 pins
Encoder myEnc4(28, 29);        // the number of the encoder 4 pins
Encoder myEnc5(30, 31);        // the number of the encoder 5 pins
Encoder myEnc6(32, 33);        // the number of the encoder 6 pins
Encoder myEnc7(34, 35);        // the number of the encoder 7 pins
Encoder myEnc8(36, 37);        // the number of the encoder 8 pins
Encoder myEnc9(38, 39);        // the number of the encoder 9 pins
Encoder myEnc10(40, 41);       // the number of the encoder 10 pins
Encoder myEnc11(42, 43);       // the number of the encoder 11 pins
Encoder myEnc12(44, 45);       // the number of the encoder 12 pins
Encoder myEnc13(46, 47);       // the number of the encoder 13 pins
Encoder myEnc14(48, 49);       // the number of the encoder 14 pins
Encoder myEnc15(14, 15);       // the number of the encoder 15 pins
Encoder myEnc16(16, 17);       // the number of the encoder 16 pins
Encoder myEnc17(18, 19);       // the number of the encoder 17 pins
Encoder myEnc18(20, 21);       // the number of the encoder 18 pins

const int buttonPin1 = 1;      // the number of the pushbutton pin
const int buttonPin2 = 2;     // the number of the pushbutton pin
const int buttonPin3 = ;      // the number of the pushbutton pin
const int buttonPin4 = 9;     // the number of the pushbutton pin
const int buttonPin5 = 8;      // the number of the pushbutton pin
const int buttonPin6 = 9;     // the number of the pushbutton pin
const int buttonPin7 = 8;      // the number of the pushbutton pin
const int buttonPin8 = 9;     // the number of the pushbutton pin
const int buttonPin9 = 8;      // the number of the pushbutton pin
const int buttonPin10 = 9;     // the number of the pushbutton pin
const int buttonPin11 = 8;      // the number of the pushbutton pin
const int buttonPin12 = 9;     // the number of the pushbutton pin
const int buttonPin13 = 8;      // the number of the pushbutton pin
const int buttonPin14 = 9;     // the number of the pushbutton pin
const int buttonPin15 = 8;      // the number of the pushbutton pin
const int buttonPin16 = 9;     // the number of the pushbutton pin
const int buttonPin17 = 8;      // the number of the pushbutton pin
const int buttonPin18 = 9;     // the number of the pushbutton pin
const int buttonPin19 = 8;      // the number of the pushbutton pin
const int buttonPin20 = 9;     // the number of the pushbutton pin









const String HANDSHAKE_QUERY = "ETCOSC?";
const String HANDSHAKE_REPLY = "OK";
long oldPosition1  = -999;
long oldPosition2  = -999;
long oldPosition3  = -999;
long oldPosition4  = -999;
long oldPosition5  = -999;
long oldPosition6  = -999;
long oldPosition7  = -999;
long oldPosition8  = -999;
long oldPosition9  = -999;
long oldPosition10  = -999;
long oldPosition11  = -999;
long oldPosition12  = -999;
long oldPosition13  = -999;
long oldPosition14  = -999;
long oldPosition15  = -999;
long oldPosition16  = -999;
long oldPosition17  = -999;
long oldPosition18  = -999;

int lastButtonState = LOW;   // the previous reading from the input pin
int lastButtonState1 = LOW;   // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers
long lastDebounceTime1 = 0;  // the last time the output pin was toggled
long debounceDelay1 = 50;    // the debounce time; increase if the output flickers

int buttonState = 0;         // variable for reading the pushbutton status
int buttonState1 = 0;         // variable for reading the pushbutton status

char oscMsg1[32];
char oscMsg2[32];
char oscMsg3[32];
char oscMsg4[32];
char oscMsg5[32];
char oscMsg6[32];
char oscMsg7[32];
char oscMsg8[32];
char oscMsg9[32];
char oscMsg10[32];
char oscMsg11[32];
char oscMsg12[32];
char oscMsg13[32];
char oscMsg14[32];
char oscMsg15[32];
char oscMsg16[32];
char oscMsg17[32];
char oscMsg18[32];


byte myMac[6] = {

};
byte myNM[4] = {

};
byte myIP[4] = {

};
byte myGW[4] = {

};
byte RemIP[4] = {

};

int RemPort = 0;      // remote port to transmit to




void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonPin1, INPUT_PULLUP);

//  pinMode(10, OUTPUT);
  //  digitalWrite(10, HIGH);
//  pinMode(51, OUTPUT);

  ShieldSetup ();//setup ethernet shield

 pinMode(4, OUTPUT);
 digitalWrite(4, HIGH);

    SLIPSerial.begin(115200);
// This is a hack around an arduino bug. It was taken from the OSC library examples
    #ifdef BOARD_HAS_USB_SERIAL
      while (!SerialUSB);
      #else
      while (!Serial);
    #endif
 
    // this is necessary for reconnecting a device because it need some timme for the serial port to get open, but meanwhile the handshake message was send from eos
    SLIPSerial.beginPacket();
    SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
    SLIPSerial.endPacket();

}



void ShieldSetup()
{
//  Serial.begin(9600);
  while (!Serial) ;

  if (!SD.begin(4)) Serial.println(F("SD fail"));
  else Serial.println(F("SD ok"));

  File fh = SD.open("settings.txt", FILE_READ);
  char netBuffer[32];

  if (!fh)
  {
    Serial.println(F("SD open fail"));
    return;
  }

  int chPos = 0;
  int lineNo = 0;


  while (fh.available())
  {
    char ch = fh.read();
    if (ch == '\n') {
      chPos = 0;

      switch (lineNo) {
        case 0:
          if (getMAC(netBuffer, myMac)) Serial.println(F("mac ok"));
          break;

        case 2:
          if (getIP(netBuffer, myIP)) Serial.println(F("ip ok"));
          break;

        case 4:
          if (getIP(netBuffer, myNM)) Serial.println(F("NM ok"));
          break;

        case 6:
          if (getIP(netBuffer, myGW)) Serial.println(F("GW ok"));
          break;

        case 8:
          if (getIP(netBuffer, RemIP)) Serial.println(F("CNSL ok"));
          break;

        case 10:
          RemPort = atoi(&netBuffer[0]);
          Serial.print(F("Port "));
          Serial.println(RemPort);
          break;

        case 12:
          strcpy( oscMsg1, netBuffer );
          Serial.print(F("OSC Command1: "));
          Serial.println(oscMsg1);
          break;

        case 14:
          strcpy( oscMsg2, netBuffer );
          Serial.print(F("OSC Command2: "));
          Serial.println(oscMsg2);
          break;

        case 16:
          strcpy( oscMsg3, netBuffer );
          Serial.print(F("OSC Command3: "));
          Serial.println(oscMsg3);
          break;

        case 18:
          strcpy( oscMsg4, netBuffer );
          Serial.print(F("OSC Command4: "));
          Serial.println(oscMsg4);
          break;

        case 20:
          strcpy( oscMsg5, netBuffer );
          Serial.print(F("OSC Command5: "));
          Serial.println(oscMsg5);
          break;
        case 22:
          strcpy( oscMsg6, netBuffer );
          Serial.print(F("OSC Command6: "));
          Serial.println(oscMsg6);
          break;

        case 24:
          strcpy( oscMsg7, netBuffer );
          Serial.print(F("OSC Command7: "));
          Serial.println(oscMsg7);
          break;

        case 26:
          strcpy( oscMsg8, netBuffer );
          Serial.print(F("OSC Command8: "));
          Serial.println(oscMsg8);
          break;

        case 28:
          strcpy( oscMsg9, netBuffer );
          Serial.print(F("OSC Command9: "));
          Serial.println(oscMsg9);
          break;

        case 30:
          strcpy( oscMsg10, netBuffer );
          Serial.print(F("OSC Command10: "));
          Serial.println(oscMsg10);
          break;

        case 32:
          strcpy( oscMsg11, netBuffer );
          Serial.print(F("OSC Command11: "));
          Serial.println(oscMsg11);
          break;

        case 34:
          strcpy( oscMsg12, netBuffer );
          Serial.print(F("OSC Command12: "));
          Serial.println(oscMsg12);
          break;

        case 36:
          strcpy( oscMsg13, netBuffer );
          Serial.print(F("OSC Command13: "));
          Serial.println(oscMsg13);
          break;

        case 38:
          strcpy( oscMsg14, netBuffer );
          Serial.print(F("OSC Command14: "));
          Serial.println(oscMsg14);
          break;

        case 40:
          strcpy( oscMsg15, netBuffer );
          Serial.print(F("OSC Command15: "));
          Serial.println(oscMsg15);
          break;

        case 42:
          strcpy( oscMsg16, netBuffer );
          Serial.print(F("OSC Command16: "));
          Serial.println(oscMsg16);
          break;

        case 44:
          strcpy( oscMsg17, netBuffer );
          Serial.print(F("OSC Command17: "));
          Serial.println(oscMsg17);
          break;

        case 46:
          strcpy( oscMsg18, netBuffer );
          Serial.print(F("OSC Command18: "));
          Serial.println(oscMsg18);
          break;

      }

      lineNo++;
    }
    else if (ch == '\r') {
      // do nothing
    }
    else if (chPos < 32) {
      netBuffer[chPos] = ch;
      chPos++;
      netBuffer[chPos] = 0;
    }
  }

  fh.close();

  int x;

  Serial.print("\r\nmac ");
  for (x = 0; x < 6; x++) {
    Serial.print(myMac[x], HEX);
    if (x < 5) Serial.print(":");
  }

  Serial.print("\r\nip ");
  for (x = 0; x < 4; x++) {
    Serial.print(myIP[x], DEC);
    if (x < 3) Serial.print(".");
  }

  Serial.print("\r\nnetmask ");
  for (x = 0; x < 4; x++) {
    Serial.print(myNM[x], DEC);
    if (x < 3) Serial.print(".");
  }

  Serial.print("\r\ngateway ");
  for (x = 0; x < 4; x++) {
    Serial.print(myGW[x], DEC);
    if (x < 3) Serial.print(".");
  }

  Serial.print("\r\nconsole ");
  for (x = 0; x < 4; x++) {
    Serial.print(RemIP[x], DEC);
    if (x < 3) Serial.print(".");
  }




}



void loop() {
  long newPosition1 = myEnc1.read();
  long newPosition2 = myEnc2.read();
  long newPosition3 = myEnc3.read();
  long newPosition4 = myEnc4.read();
  long newPosition5 = myEnc5.read();
  long newPosition6 = myEnc6.read();
  long newPosition7 = myEnc7.read();
  long newPosition8 = myEnc8.read();
  long newPosition9 = myEnc9.read();
  long newPosition10 = myEnc10.read();
  long newPosition11 = myEnc11.read();
  long newPosition12 = myEnc12.read();
  long newPosition13 = myEnc13.read();
  long newPosition14 = myEnc14.read();
  long newPosition15 = myEnc15.read();
  long newPosition16 = myEnc16.read();
  long newPosition17 = myEnc17.read();
  float newPosition18 = myEnc18.read();
  float outPosition18 = 0.000;
  int reading = digitalRead(buttonPin);
  int reading1 = digitalRead(buttonPin1);



  if (newPosition1 > oldPosition1)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg1);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition1 = newPosition1;

  }

  if (newPosition1 < oldPosition1)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg1);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition1 = newPosition1;

  }


  if (newPosition2 > oldPosition2)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg2);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition2 = newPosition2;

  }

  if (newPosition2 < oldPosition2)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg2);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition2 = newPosition2;

  }



  if (newPosition3 > oldPosition3)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg3);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition3 = newPosition3;

  }

  if (newPosition3 < oldPosition3)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg3);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition3 = newPosition3;

  }



  if (newPosition4 > oldPosition4)
  { //the message wants an OSC address as first argument
    Serial.println(oscMsg4);

    OSCMessage msg(oscMsg4);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition4 = newPosition4;

  }

  if (newPosition4 < oldPosition4)
  { //the message wants an OSC address as first argument
    Serial.println(oscMsg4);

    OSCMessage msg(oscMsg4);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition4 = newPosition4;

  }


  if (newPosition5 > oldPosition5)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg5);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition5 = newPosition5;

  }

  if (newPosition5 < oldPosition5)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg5);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition5 = newPosition5;

  }



  if (newPosition6 > oldPosition6)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg6);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition6 = newPosition6;

  }

  if (newPosition6 < oldPosition6)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg6);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition6 = newPosition6;

  }




  if (newPosition7 > oldPosition7)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg7);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition7 = newPosition7;

  }

  if (newPosition7 < oldPosition7)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg7);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition7 = newPosition7;

  }


  if (newPosition8 > oldPosition8)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg8);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition8 = newPosition8;

  }

  if (newPosition8 < oldPosition8)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg8);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition8 = newPosition8;

  }


  if (newPosition9 > oldPosition9)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg9);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition9 = newPosition9;

  }

  if (newPosition9 < oldPosition9)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg9);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition9 = newPosition9;

  }




  if (newPosition10 > oldPosition10)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg10);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition10 = newPosition10;

  }

  if (newPosition10 < oldPosition10)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg10);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition10 = newPosition10;

  }


  if (newPosition11 > oldPosition11)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg11);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition11 = newPosition11;

  }

  if (newPosition11 < oldPosition11)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg11);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition11 = newPosition11;

  }


  if (newPosition12 > oldPosition12)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg12);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition12 = newPosition12;

  }

  if (newPosition12 < oldPosition12)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg12);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition12 = newPosition12;

  }



  if (newPosition13 > oldPosition13)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg13);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition13 = newPosition13;

  }

  if (newPosition13 < oldPosition13)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg13);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition13 = newPosition13;

  }



  if (newPosition14 > oldPosition14)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg14);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition14 = newPosition14;

  }

  if (newPosition14 < oldPosition14)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg14);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition14 = newPosition14;

  }


  if (newPosition15 > oldPosition15)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg15);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition15 = newPosition15;

  }

  if (newPosition15 < oldPosition15)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg15);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition15 = newPosition15;

  }



  if (newPosition16 > oldPosition16)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg16);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition16 = newPosition16;

  }

  if (newPosition16 < oldPosition16)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg16);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition16 = newPosition16;

  }



  if (newPosition17 > oldPosition17)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg17);
    msg.add("1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition17 = newPosition17;

  }

  if (newPosition17 < oldPosition17)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg17);
    msg.add("-1");
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition17 = newPosition17;

  }


    if (newPosition18 > oldPosition18)
    { //the message wants an OSC address as first argument
      OSCMessage msg(oscMsg18);
      msg.add("1");
      SLIPSerial.beginPacket();
      msg.send(SLIPSerial); // send the bytes to the SLIP stream
      SLIPSerial.endPacket(); // mark the end of the OSC Packet
      msg.empty(); // free space occupied by message
      oldPosition18 = newPosition18;

    }

    if (newPosition18 < oldPosition18)
    { //the message wants an OSC address as first argument
      OSCMessage msg(oscMsg18);
      msg.add("-1");
      SLIPSerial.beginPacket();
      msg.send(SLIPSerial); // send the bytes to the SLIP stream
      SLIPSerial.endPacket(); // mark the end of the OSC Packet
      msg.empty(); // free space occupied by message
      oldPosition18 = newPosition18;

    }
  
static String curMsg;
  int size;
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
    curMsg = String();
  }

  

    if (reading1 != lastButtonState1) {
      // reset the debouncing timer
      lastDebounceTime1 = millis();
    }

    if ((millis() - lastDebounceTime1) > debounceDelay1) {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state:

      // if the button state has changed:
      if (reading1 != buttonState1) {
        buttonState1 = reading1;

        // only toggle the LED if the new button state is HIGH
        if (buttonState1 == HIGH) {
          OSCMessage msg("");
          //  msg.add("0");
          SLIPSerial.beginPacket();
          msg.send(SLIPSerial); // send the bytes to the SLIP stream
          SLIPSerial.endPacket(); // mark the end of the OSC Packet
          msg.empty(); // free space occupied by message
        }
      }
    }

    if (reading != lastButtonState) {
      // reset the debouncing timer
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state:

      // if the button state has changed:
      if (reading != buttonState) {
        buttonState = reading;

        // only toggle the LED if the new button state is HIGH
        if (buttonState == HIGH) {
          //the message wants an OSC address as first argument
          OSCMessage msg("");
          //  msg.add("0");
          SLIPSerial.beginPacket();
          msg.send(SLIPSerial); // send the bytes to the SLIP stream
          SLIPSerial.endPacket(); // mark the end of the OSC Packet
          msg.empty(); // free space occupied by message
        }
      }
    }




    // delay(10);
    lastButtonState = reading;
    lastButtonState1 = reading1;
  
}


void parseOSCMessage(String& msg)
{
  // check to see if this is the handshake string
  if (msg.indexOf(HANDSHAKE_QUERY) != -1)
  {
    // handshake string found!
    SLIPSerial.beginPacket();
    SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
    SLIPSerial.endPacket();


  }
  else
  {
    // prepare the message for routing by filling an OSCMessage object with our message string
    OSCMessage oscmsg;
    oscmsg.fill((uint8_t*)msg.c_str(), (int)msg.length());


  }
}


byte getMAC(char* macBuf, byte* thisMAC) {
  byte thisLen = strlen(macBuf);
  byte thisOctet = 1;

  thisMAC[0] = strtol(&macBuf[0], NULL, 16);

  for (int x = 0; x < thisLen; x++) {
    if (macBuf[x] == ':') {
      thisMAC[thisOctet] = strtol(&macBuf[x + 1], NULL, 16);
      thisOctet++;
    }
  }

  if (thisOctet == 6) return (1);
  else return (0);

}

byte getIP(char* ipBuf, byte* thisIP) {
  byte thisLen = strlen(ipBuf);
  byte thisOctet = 1;

  thisIP[0] = atoi(&ipBuf[0]);

  for (int x = 0; x < thisLen; x++) {
    if (ipBuf[x] == '.') {
      thisIP[thisOctet] = atoi(&ipBuf[x + 1]);
      thisOctet++;
    }
  }

  if (thisOctet == 4) return (1);
  else return (0);
}





