#include <TinySlip.h>
#include <Ethernet.h>
#include <OSCBoards.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>
#include <OSCTiming.h>




#define BUFFER_MAX_SIZE 256
unsigned char  parsedSlipBuffer[BUFFER_MAX_SIZE];




#define EOS_CUESTRING_MAX_SIZE      13
char currentChannel[100];
char currentWheel[20];
char currentWheelName[100];
const char* EOS_ACTIVE_CUE = "/eos/out/active/cue";


EthernetClient client;
TinySlip slip( &client );




//the Arduino's IP
IPAddress ip(10, 101, 95, 135);
//destination IP
IPAddress outIp(10, 101, 95, 102);
const unsigned int outPort = 3037;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
}; // you can find this written on the board of some Arduino Ethernets or shields






void setup() {

  Ethernet.init(10);
  Serial.begin(57600);
  Ethernet.begin(mac, ip);
  client.connect(outIp, outPort);
  Serial.println("connecting");
}

void loop() {

  size_t packetLength = slip.parsePacket(parsedSlipBuffer, BUFFER_MAX_SIZE);




  OSCMessage msgIN;
  // IF WE RECEIVED A PACKET
  if ( packetLength > 0 ) {

    msgIN.fill(parsedSlipBuffer, BUFFER_MAX_SIZE);

  }
  parseOSCMessage(msgIN);


  sendOSCMessage();

}


void parseOSCMessage(OSCMessage &msg)
{

  // We will store temporary values in these variables until the entire string has been validated.
  float cCue;

  msg.route(EOS_ACTIVE_CUE, updateActiveCue);
      msg.route("/eos/out/active/chan", updateActiveChannel);
     //msg.route("/eos/out", updateActivity);
  msg.route("/eos/out/active/wheel", updateWheel);
//  msg.route("/eos/out/ping", updatePing);



}


void updateActivity(OSCMessage& msg, int addressOffset)
{
  int length = msg.getDataLength(0);
  msg.getString(0, currentChannel, length);
  char * p = strchr (currentChannel, ' ');  // search for space
  if (p)     // if found truncate at space
    *p = 0;


  //Serial.print("Parsing OSC ");
  //Serial.println(currentChannel);

}



void updateWheel(OSCMessage& msg, int addressOffset)
{
  int length = msg.getDataLength(0);

  //-------//


  char cueAddr[EOS_CUESTRING_MAX_SIZE];
  msg.getAddress(cueAddr, addressOffset + 1, EOS_CUESTRING_MAX_SIZE);
  String addrString(cueAddr);
  int nextSlash = addrString.indexOf('/');

  int cList = addrString.substring(0, nextSlash).toInt();





  //-----//


  msg.getString(0, currentWheelName, length);
  char * p = strchr (currentWheelName, ' ');  // search for space
  if (p)     // if found truncate at space
    *p = 0;
  Serial.print("Current Wheel #");
  Serial.println(cList);
  Serial.print("Current Wheel Name ");
  Serial.println(currentWheelName);
}


void updateActiveChannel(OSCMessage& msg, int addressOffset)
{
  int length = msg.getDataLength(0);
  msg.getString(0, currentChannel, length);
  char * p = strchr (currentChannel, ' ');  // search for space
  if (p)     // if found truncate at space
    *p = 0;
  Serial.print(F("Current Channel :"));
  Serial.println(currentChannel);

}



  void updateActiveCue(OSCMessage& msg, int addressOffset)
  {

        Serial.println("updating Cues");

  char cueAddr[EOS_CUESTRING_MAX_SIZE];
  msg.getAddress(cueAddr, addressOffset + 1, EOS_CUESTRING_MAX_SIZE);
  String addrString(cueAddr);
  int nextSlash = addrString.indexOf('/');
  //  if (nextSlash != -1)
  //  {
    int cList = addrString.substring(0, nextSlash).toInt();
    int cCue = addrString.substring(nextSlash + 1).toInt();

    // Use the OSCMessage library to parse the argument(s)
    if (msg.isFloat(0))
    {
      unsigned int cPct = (unsigned int)(msg.getFloat(0) * 100);
      // All parts of the message have been validated
      // Update the display

      // Current cue and list. Top left of display
  //      lcd.setCursor(0,0);
      Serial.print("C:");
      Serial.print(cList);
      Serial.print("/");
      Serial.println(cCue);
        Serial.print("Cue % ");
        Serial.println(cPct);
  //    }
  }
  }


  void updatePing ()
  {
     OSCMessage msg("/eos/ping");
   msg.add("1");

  slip.beginPacket();

   msg.send(client);

  slip.endPacket();
  delay(20);

  }

  void sendOSCMessage ()
  {

     OSCMessage msg("/eos/ping");
   msg.add("1");

  slip.beginPacket();

   msg.send(client);

  slip.endPacket();
  delay(20);
  }
