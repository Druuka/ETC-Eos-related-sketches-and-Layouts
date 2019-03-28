//THESE ARE THE LIBRARIES THAT NEED TO BE INSTALLED FOR THIS TO WORK
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <OSCMessage.h>
#include <SD.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

Encoder myEnc1(6, 7);        // the number of the encoder 1 pins
Encoder myEnc2(8, 9);        // the number of the encoder 2 pins


const int buttonPin = A0;      // the number of the pushbutton pin
const int buttonPin1 = A1;     // the number of the pushbutton pin
const int ledPin = 3;
const int ledPin1 = 5;
long oldPosition1  = -999;
long oldPosition2  = -999;


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


EthernetUDP Udp;
//SETUP FOR PIN DEFINITION
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonPin1, INPUT_PULLUP);

  //LEDs on BUTTONS
  pinMode(ledPin, OUTPUT);
  analogWrite(ledPin, 67);
  pinMode(ledPin1, OUTPUT);
  analogWrite(ledPin1, 67);

  //ETHERNET ENABLE
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);


  ShieldSetup ();//setup ethernet shield


  Udp.begin(RemPort);


}


//THIS SECTION LOADS SETTINGS.TXT FILE INTO LOCAL MEMORY AND SETS UP ETHERNET
void ShieldSetup()
{
  Serial.begin(9600);
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
      //DIAG INFO FROM SD CARD
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
  //THIS PRINTS OUT DIAGNOSTIC INFO IF YOU ARE CONNECTED TO THE ARDUINO IDE

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


  Serial.println(F("\r\nStarting ethernet"));
  Ethernet.begin(myMac, myIP, myGW, myGW, myNM);

  Serial.println(Ethernet.localIP());

}

//MAIN LOOP OF PROGRAM THIS IS WHERE THE MAGIC HAPPENS

void loop() {
  long newPosition1 = myEnc1.read();
  long newPosition2 = myEnc2.read();

  int reading = digitalRead(buttonPin);
  int reading1 = digitalRead(buttonPin1);

  //ENCODER ONE OSCMSG1

  if (newPosition1 > oldPosition1)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg1);
    msg.add("1");
    Udp.beginPacket(RemIP, RemPort);
    msg.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition1 = newPosition1;

  }

  if (newPosition1 < oldPosition1)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg1);
    msg.add("-1");
    Udp.beginPacket(RemIP, RemPort);
    msg.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition1 = newPosition1;

  }


  //ENCODER 2 OSCMSG2

  if (newPosition2 > oldPosition2)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg2);
    msg.add("1");
    Udp.beginPacket(RemIP, RemPort);
    msg.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition2 = newPosition2;

  }

  if (newPosition2 < oldPosition2)
  { //the message wants an OSC address as first argument
    OSCMessage msg(oscMsg2);
    msg.add("-1");
    Udp.beginPacket(RemIP, RemPort);
    msg.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    oldPosition2 = newPosition2;

  }

  //BUTTON 1 OSCMSG3

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

      // only toggle the LED if the new button state is LOW
      if (buttonState == LOW) {
        //the message wants an OSC address as first argument
        OSCMessage msg(oscMsg3);
        msg.add("1");
        analogWrite(ledPin, 255);
        Udp.beginPacket(RemIP, RemPort);
        msg.send(Udp); // send the bytes to the SLIP stream
        Udp.endPacket(); // mark the end of the OSC Packet
        msg.empty(); // free space occupied by message
      }
      if (buttonState == HIGH) {
        //the message wants an OSC address as first argument
        OSCMessage msg(oscMsg3);
        msg.add("0");
         analogWrite(ledPin, 67);
       Udp.beginPacket(RemIP, RemPort);
        msg.send(Udp); // send the bytes to the SLIP stream
        Udp.endPacket(); // mark the end of the OSC Packet
        msg.empty(); // free space occupied by message
      }
    }
  }

  //BUTTON 2 OSCMSG4

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

      // only toggle the LED if the new button state is LOW
      if (buttonState1 == LOW) {
        OSCMessage msg(oscMsg4);
        msg.add("1");
        analogWrite(ledPin1, 255);
        Udp.beginPacket(RemIP, RemPort);
        msg.send(Udp); // send the bytes to the SLIP stream
        Udp.endPacket(); // mark the end of the OSC Packet
        msg.empty(); // free space occupied by message
      }
      if (buttonState1 == HIGH) {
        //the message wants an OSC address as first argument
        OSCMessage msg(oscMsg4);
        msg.add("0");
        analogWrite(ledPin1, 67);
        Udp.beginPacket(RemIP, RemPort);
        msg.send(Udp); // send the bytes to the SLIP stream
        Udp.endPacket(); // mark the end of the OSC Packet
        msg.empty(); // free space occupied by message
      }
    }
  }


  lastButtonState = reading;
  lastButtonState1 = reading1;

}


//THIS IS PART OF THE SETUP THAT CONVERTS TEXT FROM THE SD CARD INTO NUMBERS FOR THE MAC ADDRESS AND IP ADDRESS


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





