#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008 
#include <OSCMessage.h>
#include <LiquidTWI.h>
#include <SD.h>

LiquidTWI lcd(0);

int analogvalue1;
int analogvalue2;
int analogvalue3;
int analogvalue4;


int lastanalogvalue1;
int lastanalogvalue2;
int lastanalogvalue3;
int lastanalogvalue4;

float fadervalue1;
float fadervalue2;
float fadervalue3;
float fadervalue4;


//setup
char oscMsg1[32];
char oscMsg2[32];
char oscMsg3[32];
char oscMsg4[32];

int fader1 = A0;
int fader2 = A1;
int fader3 = A2;
int fader4 = A3;
int fader5 = A4;


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

void setup()
{
  // set up the LCD's number of rows and columns:
  lcd.begin(16, 2);
  lcd.print("Int         Iris");

  ShieldSetup ();//setup ethernet shield

  pinMode(10, OUTPUT);

  Udp.begin(RemPort);


}


void ShieldSetup()
{
  Serial.begin(9600);


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


  Serial.println(F("\r\nStarting ethernet"));
  Ethernet.begin(myMac, myIP, myGW, myGW, myNM);

  Serial.println(Ethernet.localIP());


}




void loop()
{


  //begin fader1


  analogvalue1 = analogRead(fader1);
  fadervalue1 = analogvalue1;
  if (fadervalue1 == lastanalogvalue1) {
    ;
    //      Serial.println("Null");
  }

  else if (fadervalue1 > lastanalogvalue1 + 2) {
    ;

    Serial.println(analogvalue1);
    fadervalue1 = map(fadervalue1, 0, 1023, 0, 1000);
    //    fadervalue1 = fadervalue1 / 4;
    fadervalue1 = fadervalue1 / 10;

    lcd.setCursor(0, 1);
    lcd.print(fadervalue1);
    fadervalue1 = fadervalue1 / 100;

    OSCMessage outMessage(oscMsg1);
    outMessage.add(fadervalue1);
    //    outMessage.send(Udp,RemIP,RemPort);
    Udp.beginPacket(RemIP, RemPort);
    outMessage.send(Udp);
    Udp.endPacket();
    Serial.println(oscMsg1);
    delay(10);
    lastanalogvalue1 = analogvalue1;


  }

  else if (fadervalue1 < lastanalogvalue1 - 2) {
    ;

    Serial.println(analogvalue1);
    fadervalue1 = map(fadervalue1, 0, 1023, 0, 1000);
    //    fadervalue1 = fadervalue1 / 4;
    fadervalue1 = fadervalue1 / 10;

    lcd.setCursor(0, 1);
    lcd.print(fadervalue1);
    fadervalue1 = fadervalue1 / 100;
    OSCMessage outMessage(oscMsg1);
    outMessage.add(fadervalue1);

    //    outMessage.send(Udp,RemIP,RemPort);
    Udp.beginPacket(RemIP, RemPort);
    outMessage.send(Udp);
    Udp.endPacket();
    Serial.println(oscMsg1);
    delay(10);
    lastanalogvalue1 = analogvalue1;


  }


  //begin fader2

  analogvalue2 = analogRead(fader2);
  fadervalue2 = analogvalue2;
  if (fadervalue2 == lastanalogvalue2) {
    ;
    //      Serial.println("Null");
  }

  else if (fadervalue2 > lastanalogvalue2 + 2) {
    ;

    Serial.println(analogvalue2);
    fadervalue2 = map(fadervalue2, 0, 1023, 0, 1000);
    //    fadervalue2 = fadervalue2 / 4;
    fadervalue2 = fadervalue2 / 10;

    lcd.setCursor(10, 1);
    lcd.print(fadervalue2);
    fadervalue2 = fadervalue2 / 100;

    OSCMessage outMessage(oscMsg2);
    outMessage.add(fadervalue2);
    //    outMessage.send(Udp,RemIP,RemPort);
    Udp.beginPacket(RemIP, RemPort);
    outMessage.send(Udp);
    Udp.endPacket();
    Serial.println(oscMsg2);
    delay(10);
    lastanalogvalue2 = analogvalue2;


  }

  else if (fadervalue2 < lastanalogvalue2 - 2) {
    ;

    Serial.println(analogvalue2);
    fadervalue2 = map(fadervalue2, 0, 1023, 0, 1000);
    //    fadervalue2 = fadervalue2 / 4;
    fadervalue2 = fadervalue2 / 10;

    lcd.setCursor(10, 1);
    lcd.print(fadervalue2);
    fadervalue2 = fadervalue2 / 100;

    OSCMessage outMessage(oscMsg2);
    outMessage.add(fadervalue2);
    //    outMessage.send(Udp,RemIP,RemPort);
    Udp.beginPacket(RemIP, RemPort);
    outMessage.send(Udp);
    Udp.endPacket();
    Serial.println(oscMsg2);
    delay(10);
    lastanalogvalue2 = analogvalue2;


  }

  //begin fader3

  analogvalue3 = analogRead(fader3);
  fadervalue3 = analogvalue3;
  if (fadervalue3 == lastanalogvalue3) {
    ;
    //      Serial.println("Null");
  }

  else if (fadervalue3 > lastanalogvalue3) {
    ;

    Serial.println(analogvalue3);
    fadervalue3 = map(fadervalue3, 0, 1023, 0, 1000);
    //    fadervalue3 = fadervalue3 / 4;
    fadervalue3 = fadervalue3 / 1000;
    OSCMessage outMessage(oscMsg3);
    outMessage.add(fadervalue3);
    //    outMessage.send(Udp,RemIP,RemPort);
    Udp.beginPacket(RemIP, RemPort);
    outMessage.send(Udp);
    Udp.endPacket();
    Serial.println(oscMsg3);
    delay(10);
    lastanalogvalue3 = analogvalue3;


  }

  else if (fadervalue3 < lastanalogvalue3) {
    ;

    Serial.println(analogvalue3);
    fadervalue3 = map(fadervalue3, 0, 1023, 0, 1000);
    //    fadervalue3 = fadervalue3 / 4;
    fadervalue3 = fadervalue3 / 1000;
    OSCMessage outMessage(oscMsg3);
    outMessage.add(fadervalue3);
    //    outMessage.send(Udp,RemIP,RemPort);
    Udp.beginPacket(RemIP, RemPort);
    outMessage.send(Udp);
    Udp.endPacket();
    Serial.println(oscMsg3);
    delay(10);
    lastanalogvalue3 = analogvalue3;


  }


  //begin fader4


  analogvalue4 = analogRead(fader4);
  fadervalue4 = analogvalue4;
  if (fadervalue4 == lastanalogvalue4) {
    ;
    //      Serial.println("Null");
  }

  else if (fadervalue4 > lastanalogvalue4) {
    ;

    Serial.println(analogvalue4);
    fadervalue4 = map(fadervalue4, 0, 1023, 0, 1000);
    //    fadervalue4 = fadervalue4 / 4;
    fadervalue4 = fadervalue4 / 1000;
    OSCMessage outMessage(oscMsg4);
    outMessage.add(fadervalue4);
    //    outMessage.send(Udp,RemIP,RemPort);
    Udp.beginPacket(RemIP, RemPort);
    outMessage.send(Udp);
    Udp.endPacket();
    delay(10);
    lastanalogvalue4 = analogvalue4;


  }

  else if (fadervalue4 < lastanalogvalue4) {
    ;

    Serial.println(analogvalue4);
    fadervalue4 = map(fadervalue4, 0, 1023, 0, 1000);
    //    fadervalue4 = fadervalue4 / 4;
    fadervalue4 = fadervalue4 / 1000;
    OSCMessage outMessage(oscMsg4);
    outMessage.add(fadervalue4);
    //    outMessage.send(Udp,RemIP,RemPort);
    Udp.beginPacket(RemIP, RemPort);
    outMessage.send(Udp);
    Udp.endPacket();
    delay(10);
    lastanalogvalue4 = analogvalue4;


  }


  delay(10);
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








