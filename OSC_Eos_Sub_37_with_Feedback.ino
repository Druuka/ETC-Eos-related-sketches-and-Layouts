
#include <SPI.h>         // needed for Arduino versions later than 0018
//#include <Ethernet.h>
//#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <EthernetV2_0.h>
#include <EthernetUdpV2_0.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <EEPROM.h>
//#include <OSCMessage.h>
#include <OSCBundle.h>
//#include <OSCBoards.h>
#include <AFMotor.h>

// DC motor on M2
AF_DCMotor motor(1);
const int wiper        = A0;   //Position of fader relative to GND (Analog 0)
double faderMax        = 0;   //Value read by fader's maximum position (0-1023)
double faderMin        = 0;   //Value read by fader's minimum position (0-1023)
int ledState = LOW;
int ledPin =  5;       //pin 13 on Arduino Uno. Pin 6 on a Teensy++2
int state = 0;
int sensorPin = A0; 
//int sensorPin1 = A1;
double sensorValue = 0;
double outputValue = 0;
double sensorVal = 0;
int sensorValue1 = 0;
int outputValue1 = 0;
int sensorVal1 = 0;
double sensorValstring = 0;
int THRESHOLD = 2; //define a threshold amount
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[6] = {  
  0x90, 0xA2, 0xDA, 0x0E, 0xDA, 0x60 };
byte subnet[] = { 
  255, 255, 0, 0 };
byte gateway[] = { 
  10, 101, 95, 115 };
IPAddress ip(10, 101, 95, 115);
unsigned int localPort = 8000;      // local port to listen on
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,

IPAddress RemIP(10, 101, 95, 101);
unsigned int RemPort = 3501;      // remote port to listen on
char macstr[18];

const byte ID = 0x92; //used to identify if valid data in EEPROM the "know" bit, 
// if this is written in EEPROM the sketch has ran before
// We use this, so that the very first time you'll run this sketch it will use
// the values written above. 
// defining which EEPROM address we are using for what data


// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
#define W5200_CS  10
#define SDCARD_CS 4

void setup() 
{

  ShieldSetup ();//setup ethernet shield
  // start the Ethernet and UDP:
  // Ethernet.begin(mac, ip);
  motor.setSpeed(200);

  motor.run(RELEASE);
  calibrateFader();

}

void ShieldSetup()
{    // Random MAC address stored in EEPROM
  if (EEPROM.read(1) == '#') {
    for (int i = 2; i < 6; i++) {
      mac[i] = EEPROM.read(i);
    }
  } 
  else {
    randomSeed(analogRead(0));
    for (int i = 2; i < 6; i++) {
      mac[i] = random(0, 255);
      EEPROM.write(i, mac[i]);
    }
    EEPROM.write(1, '#');
  }

  int idcheck = EEPROM.read(0);
  if (idcheck != ID){
    //ifcheck id is not the value as const byte ID,
    //it means this sketch has NOT been used to setup the shield before
    //just use the values written in the beginning of the sketch
  }
  if (idcheck == ID){
    //if id is the same value as const byte ID,
    //it means this sketch has been used to setup the shield.
    //So we will read the values out of EERPOM and use them
    //to setup the shield.
    for (int i = 2; i < 6; i++) {
      mac[i] = EEPROM.read(i);
    }

  }



  snprintf(macstr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  // start the Ethernet and UDP:

  pinMode(SDCARD_CS,OUTPUT);
  digitalWrite(SDCARD_CS,HIGH);//Deselect the SD card
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  Udp.begin(localPort);

}


void loop() {
  OSCMsgReceive();
  Subrun();


}

void Subrun(){
  sensorValue = analogRead(sensorPin);
  outputValue = sensorValue; 
  outputValue = constrain(outputValue, 0, 1000); 

  sensorValstring = outputValue/10;
  sensorValstring = sensorValstring -2;
  //reads and dispatches the incoming message


  if (sensorVal == sensorValue) {
    // Do Nothing
  }
  else
  {  
    OSCMessage msg1("/eos/sub/1");
    msg1.add(sensorValstring);

    Udp.beginPacket(RemIP, RemPort);
    msg1.send(Udp);
    Udp.endPacket();
    msg1.empty();
  }
  delay(100);
  sensorVal = sensorValue;

}

void OSCMsgReceive(){
  OSCMessage msgIN;
  int size;
  if((size = Udp.parsePacket())>0){
    while(size--)
      msgIN.fill(Udp.read());
    if(!msgIN.hasError()){
      msgIN.route("/fader/value1",moveFader);
      msgIN.route("/OnOff/toggle1",toggleOnOff);

    }
  }

}

void toggleOnOff(OSCMessage &msg, int addrOffset){
  ledState = (boolean) msg.getFloat(0);
  OSCMessage msgOUT("/OnOff/toggle2");


  motor.run(BACKWARD);
  delay(500);
  motor.run(RELEASE);

  ledState = !ledState;		 // toggle the state from HIGH to LOW to HIGH to LOW ...

  Udp.beginPacket(RemIP, RemPort);
  msgOUT.send(Udp); // send the bytes
  Udp.endPacket(); // mark the end of the OSC Packet
  msgOUT.empty(); // free space occupied by message
}

void moveFader(OSCMessage &msg, int addrOffset){
  state = (boolean) msg.getFloat(0);
 // state = state * 10;
  OSCMessage msgOUT("/Fader/Mov");
      msgOUT.add(state);

  Udp.beginPacket(RemIP, RemPort);
  msgOUT.send(Udp); // send the bytes
  Udp.endPacket(); // mark the end of the OSC Packet
  msgOUT.empty(); // free space occupied by message


  if (state < sensorValue - 10 && state > faderMin) {
    motor.run(BACKWARD);
    while (state < sensorValue - 10) {
    };  //Loops until motor is done moving
    motor.run(RELEASE);
  }

  else if (state > sensorValue + 10 && state < faderMax) {
    motor.run(FORWARD);
    while (state > sensorValue + 10) {
    }; //Loops until motor is done moving
    motor.run(RELEASE);
  }

  motor.run(RELEASE);
  delay(100);
}


void calibrateFader() {
  //Send fader to the top and read max position
  motor.run(FORWARD);
  delay(500);
  motor.run(RELEASE);
  faderMax = analogRead(sensorPin);

  //Send fader to the bottom and read max position
  motor.run(BACKWARD);
  delay(500);
  motor.run(RELEASE);
  faderMin = analogRead(sensorPin);
}







