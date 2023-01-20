//las direcciones ip se deben cambia en el setup y en la funcion verifica
//
#include <EtherCard.h> //Usa por defecto pin 10 de Atmega328 P, pero el diseño de ese pin est{a} sobre el pin 12 . En este momento se modifica la tarjeta pero se podr{i}a modificar la libreria.
#include <stdlib.h>    
#include <EEPROM.h>
#include "melody.h"
#define VOLTS_IN_A0 A0   
#define VOLTS_IN_A1 A1   
#define TRIGER_TO_A0 A2 
#define TRIGER_TO_A1 A3 
#define SERIAL_RX 0
#define SERIAL_TX 1

#define SOFT_RESET 5   
#define BUZZER 6
#define PLUGED_DEVICE_CTL 7
#define SAMPLING_DELAY_MS 2 
#define SAMPLE_QUANTITY 10
#define CALIBRATE_BY_SOFTWARE_VOLT_VALUE 25
#define CALIBRATE_BY_SOFTWARE_M 0.037606837606837605
#define CALIBRATE_BY_SOFTWARE_B 0.14871794871794908
#define BATTERYSYSTEM 12
#define SYSTEM1 12
#define SYSTEM2 24
#define MAC {0x74, 0x69, 0x69, 0x2D, 0x30, 0x3B}

#define VPS_SERVER_IP "146.71.79.111"
#define TCP_PORT_FOR_HTTP_API 8013
#define PATH "volts"
const char websiteTarget[] PROGMEM = "myvpsserveraccount.com";
const long POSTrequestInterval = 15000; //(milliseconds)
unsigned long previousMillisForPOSTrequest = 0;
/////////////////////////////////////////////////////////////////////////
// PIN Connections (Using Arduino UNO/NANO/MINI/PRO):
//   VCC -   3.3V     ///
//   GND -    GND     ///
//   SCK - [Arduino-Pin 13] ///Ping 17 on chip
//   SO  - [Arduino-Pin 12] ///Ping 16 on chip
//   SI  - [Arduino-Pin 11] ///Ping 15 on chip
//   CS  - [Arduino-Pin 10] ///ping 14 on chip) Legacy===Arduino pin 8 =>12 on chip "!important:" for Atmega328P (Cs=8..por error) (My mistake! it must be Cs=Arduino pin10(ping 14 en atmega))
/////////////////////////////////////////////////////////////////////////
// Default network config                                             //
byte mymac[] = MAC;                  //
byte myip[] = {192, 168, 254, 114};   //
byte gwip[] = {192, 168, 254, 1};     //
byte static_dns[] = {8, 8, 8, 8};    //
byte netmask[] = {255, 255, 255, 0}; //

int reboot = 0;

const char website[] PROGMEM = "www.google.com";
const long interval = 300000; //(milliseconds)  300000=>5 minutes 
byte Ethernet::buffer[400];
unsigned long previousMillis = 0;
unsigned long time;
BufferFiller bfill;
char ipadd[16];
char gwadd[16];
char nmadd[16];
float sensor1CalculatedValue=0;
int sensor1SampleRawValue=0;
int sensor2SampleRawValue=0;
float sensor2CalculatedValue=0;
char sensor1ToArrayValue[10];
char sensor2ToArrayValue[10]; //temporarily holds data from vals
int dstart = 0;
int dlen = 0;
int ActionValueByGet=0;
float calibrateValueFromEEPROM=0;
char calibrateOpt=0;
int requestStatus = 1;
int rstPending=0;
const char htmlHeaderGETresponse[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n";

//////START MELODY CONFIG
/*
  Pacman Intro Theme
  More songs available at https://github.com/robsoncouto/arduino-songs
                                              Robson Couto, 2019
*/

int tempo = 105;
int melody[] = {
    // Pacman
    // Score available at https://musescore.com/user/85429/scores/107109
    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
    NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
    NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, NOTE_B5, 32, //2
    NOTE_FS5, -16, NOTE_DS5, 8, NOTE_DS5, 32, NOTE_E5, 32, NOTE_F5, 32,
    NOTE_F5, 32, NOTE_FS5, 32, NOTE_G5, 32, NOTE_G5, 32, NOTE_GS5, 32, NOTE_A5, 16, NOTE_B5, 8};
int notes = sizeof(melody) / sizeof(melody[0]) / 2;
int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;
/////  END OF MELODY CONFIG
// ---------------------JAVASCRIPT------------------------      ///
//m=(y2-y1)/(x2-x1)
// let x1=222.0
// let y1=12.0
// let x2=257.5
// let y2=14.0
// let m=(y2-y1)/(x2-x1)
// //y1=mx1+b
// let b=y1-(m*x1)
// let x=225
// let y=(m*x)+b
// console.log("m= "+m)
// console.log("b= "+b)
// console.log("y= "+y)

// "m= 0.056338028169014086"
// "b= -0.5070422535211279"
// "y= 12.169014084507042"
//y=(0.05405405405405406 * x) + -0.16216216216216317                               /////

void sensor1()
{
  int lectura = 0;
  int sumLectura = 0;
  float x = 0;
  for (int i = 0; i < SAMPLE_QUANTITY; i++)
  {
    digitalWrite(TRIGER_TO_A0, LOW);
    delay(SAMPLING_DELAY_MS);
    lectura = analogRead(VOLTS_IN_A0);
    sumLectura += lectura;
    digitalWrite(TRIGER_TO_A0, LOW);
    delay(SAMPLING_DELAY_MS);
  }
  double m = CALIBRATE_BY_SOFTWARE_M;
  double b = CALIBRATE_BY_SOFTWARE_B;
  x = sumLectura / SAMPLE_QUANTITY;
  sensor1CalculatedValue = (m * x) + b;
  sensor1CalculatedValue=sensor1CalculatedValue<=CALIBRATE_BY_SOFTWARE_VOLT_VALUE?sensor1CalculatedValue-0.15:sensor1CalculatedValue-0.3;
  sensor1SampleRawValue=x;
  Serial.println(F("voltaje 1 :"));
  Serial.print(sensor1CalculatedValue);

  dtostrf(sensor1CalculatedValue+calibrateValueFromEEPROM, 4, 1, sensor1ToArrayValue);
}
void sensor2()
{
  int lectura = 0;
  int sumLectura = 0;
  float x = 0;
  for (int i = 0; i < SAMPLE_QUANTITY; i++)
  {
    digitalWrite(TRIGER_TO_A1, LOW);
    delay(SAMPLING_DELAY_MS);
    lectura = analogRead(VOLTS_IN_A1);
    sumLectura += lectura;
    digitalWrite(TRIGER_TO_A1, LOW);
    delay(SAMPLING_DELAY_MS);
  }
  double m = CALIBRATE_BY_SOFTWARE_M;
  double b = CALIBRATE_BY_SOFTWARE_B;
  x = sumLectura / SAMPLE_QUANTITY;
  sensor2CalculatedValue = (m * x) + b;
  sensor2CalculatedValue=sensor2CalculatedValue<=CALIBRATE_BY_SOFTWARE_VOLT_VALUE?sensor2CalculatedValue-0.15:sensor2CalculatedValue-0.3;
  sensor2SampleRawValue=x;
  Serial.println(F("voltaje 2 :"));
  Serial.print(sensor2CalculatedValue);

  dtostrf(sensor2CalculatedValue+calibrateValueFromEEPROM, 4, 1, sensor2ToArrayValue);
}
static word homePagephone()
{ 
  sensor1();
  sensor2();
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR("$F"
                   "{\"data\":{\"status\":\"$D\",\"rele\":\"$D\",\"sensor1\":\"$S\",\"sensor2\":\"$S\",\"adjustedOptl1m2\":\"$D\",\"adjustedVal\":\"$D\",\"rstPending\":\"$D\",\"sensor1rawValue\":\"$D\",\"sensor2rawValue\":\"$D\"}} \r\n"),
               htmlHeaderGETresponse,requestStatus,digitalRead(PLUGED_DEVICE_CTL), sensor1ToArrayValue, sensor2ToArrayValue, calibrateOpt, ActionValueByGet,rstPending,sensor1SampleRawValue,sensor2SampleRawValue);
  return bfill.position();
}
void restart(int setStatus)
{
  reboot = setStatus;
}
void ethconfig()
{

  sprintf(ipadd, "%u.%u.%u.%u", myip[0], myip[1], myip[2], myip[3]); //char ipadd[16];
  //Serial.println("Ip:");
  Serial.println(ipadd);
  sprintf(nmadd, "%u.%u.%u.%u", netmask[0], netmask[1], netmask[2], netmask[3]); //char ipadd[16];
  //Serial.println("netMask:");
  Serial.println(nmadd);
  sprintf(gwadd, "%u.%u.%u.%u", gwip[0], gwip[1], gwip[2], gwip[3]); //char ipadd[16];
  //Serial.println("Gw:");
  Serial.println(gwadd);
  ether.hisport = TCP_PORT_FOR_HTTP_API;
  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0) //8 => SS=CS, pin  12 on Atmega 328P     Nov 2022 lo paso a 10 q es el valor por defecto.
    Serial.println(F("Failed Ethernet  start"));
  Serial.println(F("probando ether.staticSetup"));
  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) != 0)
  {
    ether.staticSetup(myip, gwip, static_dns, netmask);
    Serial.println(F("probando dns"));
  }
  delay(100); //prueba de desbloqueo
  //dnscheckup();
  Serial.println("DNS Checked.");
  ether.parseIp (ether.hisip, "146.71.79.111");//VPS_SERVER_IP
}
void dnscheckup()
{
  Serial.println(F("..entrando...dnscheckup().probando DNS..."));
  if (ether.dnsLookup(website))

    while (ether.clientWaitingGw())
    {
      ether.packetLoop(ether.packetReceive());
      Serial.println("dns. ok...");
    }

  else
  {
    Serial.println(F("timeout dns.."));
  }
  Serial.println(F(".Saliendo...dnscheckup()"));
}

void(* resetFunc) (void) = 0;  // declare reset fuction at address 0

void setup()
{
  
  tone(BUZZER, 1000); // Send 1KHz sound signal...
  delay(1000);        // ...for 1 sec
  noTone(BUZZER);     // Stop sound...
  time = millis();
  Serial.begin(9600);
  pinMode(TRIGER_TO_A0, OUTPUT);
  pinMode(TRIGER_TO_A1, OUTPUT);
  pinMode(PLUGED_DEVICE_CTL, OUTPUT);
  pinMode(SERIAL_TX, OUTPUT);



  Serial.println(F("EEPROM rest value: "));
  Serial.println(EEPROM.read(7));
  if (EEPROM.read(7) == 1)
  {
    EEPROM.write(8, 55);
    EEPROM.write(9, 249);
    EEPROM.write(10, 1);
    EEPROM.write(7, 0);
    restart(1);
  }
  if (EEPROM.read(7) == 3) //    kevin leandro
  {
    myip[2] = EEPROM.read(8);
    gwip[2] = EEPROM.read(8);
    myip[3] = EEPROM.read(9);
    gwip[3] = EEPROM.read(10);
    Serial.println(F("eepromm 8:"));
    Serial.println(EEPROM.read(8));
    Serial.println(F("eepromm 9:"));
    Serial.println(EEPROM.read(9));
    Serial.println(F("eepromm 10:"));
    Serial.println(EEPROM.read(10));
  }
  /**CALIBRATE EEPROM*/
  if(EEPROM.read(50)!=255){
    Serial.println("m or l?: ");
    if(EEPROM.read(51)>=0 && EEPROM.read(51) < 10){
        Serial.print("Ya tenemos SIGNO:");
        Serial.println((char)EEPROM.read(50));
        Serial.print("Ya tenemos el VALOR a incrementar o decrementar!:");
        Serial.print(EEPROM.read(51));
        Serial.print("");
        switch ((char)EEPROM.read(50)) {
          case 'l':
            calibrateValueFromEEPROM=(EEPROM.read(51)*-1)/10.0;
            ActionValueByGet=EEPROM.read(51);
            calibrateOpt=1;
            break;
          case 'm':
            calibrateValueFromEEPROM=(EEPROM.read(51)*1)/10.0;  
            calibrateOpt=2;
            ActionValueByGet=EEPROM.read(51);
            Serial.println("Estoy en m");          
            break;
          default:
            EEPROM.write(50, 255);
            EEPROM.write(51, 255);
            break;
        }
                
    }else{
        EEPROM.write(50, 255);
        EEPROM.write(51, 255);
      }
  }
  
  delay(5000);
  ethconfig();
  tone(BUZZER, 6000); // Send 1KHz sound signal...
  delay(500);         // ...for 1 sec
  noTone(BUZZER);     // Stop sound...
  delay(500);
  tone(BUZZER, 5000); // Send 1KHz sound signal...
  delay(100);         // ...for 1 sec
  noTone(BUZZER);     // Stop sound...
  delay(100);
  tone(BUZZER, 5000); // Send 1KHz sound signal...
  delay(100);         // ...for 1 sec
  noTone(BUZZER);     // Stop sound...
}
//------------------------------------------------------------------------------------------------------
void loop()
{
  time = millis();
  if (time >= 3600000)
  {
    //restart(1);
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    float minVoltaje = 0;
    float maxVoltaje = 0;
    int pwnChargeVolt = 0;
    float voltajeBatterySource = 0;
    if (BATTERYSYSTEM == SYSTEM1)
    {
      //Serial.println("*****************************");
      if (digitalRead(PLUGED_DEVICE_CTL))
      {
        digitalWrite(PLUGED_DEVICE_CTL, LOW); //turn off the battery charger
        delay(5000);
        sensor1();
        digitalWrite(PLUGED_DEVICE_CTL, HIGH); //turn on the battery charger
        voltajeBatterySource = sensor1CalculatedValue;
      }
      else
      {
        sensor1();
        voltajeBatterySource = sensor1CalculatedValue;
      }
      //Serial.println("*****************************");
      minVoltaje = 12.5;
      maxVoltaje = 13.2;
      pwnChargeVolt = 254; //160
    }

    if (voltajeBatterySource < minVoltaje)
    {
      digitalWrite(PLUGED_DEVICE_CTL, HIGH); //Normally opened Rele 5 volt-> ON
      Serial.print("Voltaje menor a");
      Serial.println(minVoltaje);
      Serial.println("charging battery...");
    }
    if ((voltajeBatterySource >= minVoltaje) && (voltajeBatterySource <= (maxVoltaje)))
    {
      Serial.print("Voltaje Intermedio:");
      Serial.print(voltajeBatterySource, 2);
      Serial.println("...");
      digitalWrite(PLUGED_DEVICE_CTL, HIGH); //Normally opened Rele 5 volt-> ON  < Forcing ON status of rele >
    }
    if (voltajeBatterySource > maxVoltaje)
    {
      digitalWrite(PLUGED_DEVICE_CTL, LOW); //Normaly opened rele 0 volts->OFF
      Serial.print("Voltaje mayor a");
      Serial.println(maxVoltaje, 2);
      Serial.println(" Charging battery stop");
    }

    if (sensor2CalculatedValue >= 4)
    {
      Serial.println("Si hay energia electrica");
    }
    else
    {
      Serial.println("No hay energia electrica");
      for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2)
      {
        divider = melody[thisNote + 1];
        if (divider > 0)
        {
          noteDuration = (wholenote) / divider;
        }
        else if (divider < 0)
        {
          noteDuration = (wholenote) / abs(divider);
          noteDuration *= 1.5; // increases the duration in half for dotted notes
        }
        tone(BUZZER, melody[thisNote], noteDuration * 0.9);
        delay(noteDuration);
        noTone(BUZZER);
      }
      //resetFunc(); //call reset
    }
  }

  word len = ether.packetReceive(); // go receive new packets
  word pos = ether.packetLoop(len); // respond to incoming
  if (pos)
  { // check if valid tcp data is received
    // data received from ethernet
    char *data = (char *)Ethernet::buffer + pos;
    Serial.println(F("----------------"));
    Serial.println("data :");
    Serial.println(data);
    Serial.println(F("----------------"));

    if (strncmp("GET /5", data, 6) == 0)
    { 
      requestStatus=1;
      Serial.println("->");
      Serial.println(data[6]);
      Serial.println("<-");
      Serial.println(data[7]);
      char opt=data[6];
      char optVal=data[7];
      if( (opt=='l') || (opt=='m') ){
        Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Calibrate");
        
        if(opt=='m'){
          char c = optVal;
          ActionValueByGet = c - '0';
          requestStatus=ActionValueByGet>=0&&ActionValueByGet<10?1:0;
          if(requestStatus){
          EEPROM.write(50, 'm');
          EEPROM.write(51, ActionValueByGet);
          rstPending=1;
          }
        }
        if(opt=='l'){
          char c = optVal;
          ActionValueByGet = c - '0';
          requestStatus=ActionValueByGet>=0&&ActionValueByGet<10?1:0;
          if(requestStatus){
          EEPROM.write(50, 'l');
          EEPROM.write(51, ActionValueByGet);
          rstPending=1;
          }
        }
      }
      ether.httpServerReply(homePagephone()); // send web page data
      if(rstPending)restart(1); 
    }
    else if (strncmp("GET /ON", data, 7) == 0)
    {
      digitalWrite(PLUGED_DEVICE_CTL, HIGH); //Normally opened Rele 5 volt-> ON  < Forcing ON status of rele >
      
      ether.httpServerReply(homePagephone()); // send web page data
    }
    else if (strncmp("GET /OFF", data, 8) == 0)
    {
      
      digitalWrite(PLUGED_DEVICE_CTL, LOW); 
      ether.httpServerReply(homePagephone()); // send web page data
    }
    else if (strncmp("GET /RST", data, 8) == 0)
    { //definir si es para version rele de estado solido o rele de bobina
      EEPROM.write(50, 255);//RESET CALIBRATE
      EEPROM.write(51, 255);//RESET CALIBRATE
      EEPROM.write(6, 1);
      rstPending=1;
      ether.httpServerReply(homePagephone()); // send web page data
      restart(1);
    }
    else if (strncmp("GET /RSTNTW", data, 11) == 0)
    {
      // byte myip[] = {192, 168, 55, 249};
      // EEPROM.write(7, 1);
      // ethconfig();
    }
    else{
      requestStatus=0;
      ether.httpServerReply(homePagephone()); // send web page data
    }
    
  }

//POST REQUEST EVERY 15 SECONDS
if (currentMillis - previousMillisForPOSTrequest >= POSTrequestInterval){
    previousMillisForPOSTrequest = currentMillis;
    Stash::prepare(PSTR("POST /$F HTTP/1.1" "\r\n"
      "Host: $F" "\r\n"
      "Content-Length: 27" "\r\n"
      "Content-Type: application/x-www-form-urlencoded" "\r\n"
      "\r\n"
      "field1=value1&field2=value2"),
  PSTR(PATH), websiteTarget);
  ether.tcpSend();
  }
}
