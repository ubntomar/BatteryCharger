#include <Arduino.h>
#line 1 "c:\\Users\\OMAR\\Documents\\BatteryCharger\\charger.ino"
#line 1 "c:\\Users\\OMAR\\Documents\\BatteryCharger\\charger.ino"

//las direcciones ip se deben cambia en el setup y en la funcion verifica 
//
#include <EtherCard.h>//Usa por defecto pin 10 de Atmega328 P, pero el diseño de ese pin est{a} sobre el pin 12 . En este momento se modifica la tarjeta pero se podr{i}a modificar la libreria.
#include <stdlib.h>
#include <EEPROM.h>
#include <YetAnotherPcInt.h>
#define PCINT_PIN_A4 A4
#define DEBUG 1 
#define BATTERYSYSTEM 12
#define SYSTEM1 12
#define SYSTEM2 24
#define MAC {0x74, 0x69, 0x69, 0x2D, 0x30, 0x3B}
/////////////////////////////////////////////////////////////////////////
// PIN Connections (Using Arduino UNO):
//   VCC -   3.3V     ///
//   GND -    GND     ///
//   SCK - Pin 13     ///Ping 17 on chip 
//   SO  - Pin 12     ///Ping 16 on chip 
//   SI  - Pin 11     ///Ping 15 on chip 
//   CS  - Pin  8     ///Ping 12 on chip "!important:" for Atmega328P Cs=8 (My mistake! it should be Cs=10) => pin 12 on chip!   
/////////////////////////////////////////////////////////////////////////
// Default network config                                             //
byte myip[] = {192, 168, 30, 254};                   //
byte mymac[] = MAC; //
byte gwip[] = {192, 168, 30, 1};                     //
byte static_dns[] = {8, 8, 8, 8};                    //
byte netmask[] = {255, 255, 255, 0};                 //
////////////////////////////////////////////////////////////////////////
//                   Pic12f1822
//                  1-|      |-8
//                  2-|      |-7
//RA4(Ar y Encj60)->3-|      |-6->RA1(Input) -->Arduino->A5(Output)
//                  4-|______|-5->RA2(Output)-->Arduino->A4(Input)(Interrupción)
////////////////////////////////////////////////////////////////////////
char serial_temp[] = "9999";
int reboot=0;
int releB = 4;//SRS1
int MosfetControl = 5; //D5 ,pin 9
int acVoltajeLive = 7; //D7 ,pin 11
int sensorPin = A1;  //A1 , pin 24 -> sensor modulo de baterias 1 en A1
int sensorPin2 = A0; //A0 , pin 23 ->sensor modulo de baterias 2 en A0
int ucclockin = A2;  //entrada de pulso de comando de micro pic12f629
int buzzer = 6;
int trigerA = 2; //OUTPUT activador para lectura sensor de voltaje
int trigerB = 3; //OUTPUT activador para lectura sensor de voltaje
int getreply = 0;
int repeatstore = 0;
int flagstore = 0;
int getrequest = 0;
int timeout_dns = 0;
const char website[] PROGMEM = "www.redesagingenieria.com";
#define REQUEST_RATE 900000 //1800000 milliseconds  --30 minutos
const long interval =300000; //(milliseconds)
int read_sensor_delay = 100;
byte Ethernet::buffer[330]; //copy and pasted in agc.php    225
char myArray[0];
static uint32_t timer;
static uint32_t timer2;
static uint32_t timer3;
unsigned long previousMillis = 0;
int segs = 0;
int contsegs = 0;
int pass = 0;
unsigned long time;
BufferFiller bfill;
int releb_value = 1;
int relec_value = 1;
int reled_value = 1;
int relee_value = 1;
char ipadd[16];
char gwadd[16];
char nmadd[16];
char part1[4];
char part2[4];
char part3[3];
float sensor1Value;
float sensor2Value;
char charVal[10];
char charVal2[10]; //temporarily holds data from vals
int dstart = 0;
int dlen = 0;
const char htmlHeaderphone[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n";

//////START MELODY CONFIG
/*
  Pacman Intro Theme
  Connect a piezo buzzer or speaker to pin 11 or select a new pin.
  More songs available at https://github.com/robsoncouto/arduino-songs
                                              Robson Couto, 2019
*/
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0
// change this to make the song slower or faster
int tempo = 105;
// change this to whichever pin you want to use
// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {
  // Pacman
  // Score available at https://musescore.com/user/85429/scores/107109
  NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
  NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
  NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

  NOTE_B4, 16,  NOTE_B5, 16,  NOTE_FS5, 16,   NOTE_DS5, 16,  NOTE_B5, 32,  //2
  NOTE_FS5, -16, NOTE_DS5, 8,  NOTE_DS5, 32, NOTE_E5, 32,  NOTE_F5, 32,
  NOTE_F5, 32,  NOTE_FS5, 32,  NOTE_G5, 32,  NOTE_G5, 32, NOTE_GS5, 32,  NOTE_A5, 16, NOTE_B5, 8
};
// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;
// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;
/////  END OF MELODY CONFIG
// ---------------------JAVASCRIPT------------------------      ///
//m=(y2-y1)/(x2-x1)
// let x1=225.0
// let y1=12.0
// let x2=262.0
// let y2=14.0
// let m=(y2-y1)/(x2-x1) 
// //y1=mx1+b
// let b=y1-(m*x1)
// let x=225
// let y=(m*x)+b
// console.log("m= "+m)
// console.log("b= "+b)
// console.log("y= "+y) 
//y=(0.05405405405405406 * x) + -0.16216216216216317                               /////

void sensor1()
{
  int lectura = 0;
  int sumLectura=0;
  float x=0;
  double m=0.05405405405405406;
  double b=-0.16216216216216317;
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(trigerA, HIGH);
    delay(read_sensor_delay);
    lectura = analogRead(sensorPin);
    sumLectura+=lectura;
    digitalWrite(trigerA, LOW);
  }
  x=sumLectura / 5;
  sensor1Value = (m*x)+b;
  Serial.println(F("voltaje 1 :"));
  Serial.print(sensor1Value); 

  delay(read_sensor_delay);
  dtostrf(sensor1Value, 4, 1, charVal);
}
void sensor2()
{
  int lectura2 = 0;
  float sensor2ValueSum = 0;
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(trigerB, HIGH);
    delay(read_sensor_delay);
    lectura2 = analogRead(sensorPin2);
    sensor2ValueSum += (0.05563 * lectura2) + 0.31267;
    digitalWrite(trigerB, LOW);
  }
  sensor2Value = (sensor2ValueSum / 5) - 0.6;
  Serial.println(F("voltaje 2 "));
  Serial.println(lectura2);
  Serial.println(sensor2Value);

  delay(read_sensor_delay);
  dtostrf(sensor2Value, 4, 1, charVal2);
}
static word homePagephone()
{
  sensor1();
  sensor2();
  // releb_value = digitalRead(releB);
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
                   "$F"
                   "'data':{'rele':'$D','sensor1':'$S','sensor2':'$S'} \r\n"),
               htmlHeaderphone, digitalRead(MosfetControl), charVal, charVal2);
  return bfill.position();
}
void restart(int setStatus)
{
 reboot=setStatus;
}
void ethconfig()
{

  sprintf(ipadd, "%u.%u.%u.%u", myip[0], myip[1], myip[2], myip[3]); //char ipadd[16];
  Serial.println("Ip:");
  Serial.println(ipadd);
  sprintf(nmadd, "%u.%u.%u.%u", netmask[0], netmask[1], netmask[2], netmask[3]); //char ipadd[16];
  Serial.println("netMask:");
  Serial.println(nmadd);
  sprintf(gwadd, "%u.%u.%u.%u", gwip[0], gwip[1], gwip[2], gwip[3]); //char ipadd[16];
  Serial.println("Gw:");
  Serial.println(gwadd);
  ether.hisport = 80;
  if (ether.begin(sizeof Ethernet::buffer, mymac, 8) == 0)//8 => SS=CS, pin  12 on Atmega 328P
    Serial.println(F("Failed Ethernet  start"));
  Serial.println(F("probando ether.staticSetup"));
  if (ether.begin(sizeof Ethernet::buffer, mymac, 8) != 0)
  {
    ether.staticSetup(myip, gwip, static_dns, netmask);
    Serial.println(F("probando dns"));
  }
  delay(1000); //prueba de desbloqueo
  dnscheckup();
  Serial.println("DNS Checked.");
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
    timeout_dns = 1;
    Serial.println(F("timeout dns.."));
  }
  Serial.println(F(".Saliendo...dnscheckup()"));
}
void pinChanged(const char* message, bool pinstate) {
  Serial.print(message);
  Serial.println(pinstate ? "HIGH" : "LOW");
  if(reboot==0){
  digitalWrite(A5,HIGH);//Pic must wait 5 ms before read this value    
  }
  else{
    digitalWrite(A5,LOW);
    delay(7000);
    reboot=0;
  }   
}
void setup()
{
  pinMode(A5, OUTPUT);
  digitalWrite(A5,HIGH);
  tone(buzzer, 1000); // Send 1KHz sound signal...
  delay(1000);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  time = millis();
  timer2 = 0;
  timer3 = 0;
  Serial.begin(9600);
  pinMode(trigerA, OUTPUT);
  pinMode(trigerB, OUTPUT);
  pinMode(releB, OUTPUT);
  pinMode(MosfetControl, OUTPUT);
  pinMode(ucclockin, INPUT);
  pinMode(A3, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(acVoltajeLive, INPUT);
  digitalWrite(acVoltajeLive, LOW);
  digitalWrite(releB, EEPROM.read(1));
  pinMode(PCINT_PIN_A4, INPUT_PULLUP);
  PcInt::attachInterrupt(PCINT_PIN_A4, pinChanged, "Pin has changed to ", CHANGE);
  timer = -REQUEST_RATE; // start timing out right away
  Serial.println(F("eeprommm rest value:."));
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
  delay(5000);
  ethconfig();
  tone(buzzer, 6000); // Send 1KHz sound signal...
  delay(500);         // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(500);
  tone(buzzer, 5000); // Send 1KHz sound signal...
  delay(100);         // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(100);
  tone(buzzer, 5000); // Send 1KHz sound signal...
  delay(100);         // ...for 1 sec
  noTone(buzzer);     // Stop sound...
}
//------------------------------------------------------------------------------------------------------
void loop()
{
  
  //ether.persistTcpConnection(false);
  time = millis();
  // Serial.println(time);
  if (time >= 3600000)
  {
    restart(1);
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
      Serial.println("*****************************");
      if(digitalRead(MosfetControl)){
        digitalWrite(MosfetControl, LOW);//turn off the battery charger
        delay(5000);  
        sensor1();
        digitalWrite(MosfetControl, HIGH); //turn on the battery charger
        voltajeBatterySource = sensor1Value;
      }
      else{
        sensor1();
        voltajeBatterySource = sensor1Value;
      }
      Serial.println("*****************************");
      minVoltaje = 12.5;
      maxVoltaje = 13.2;
      pwnChargeVolt = 254; //160
    }

    if (voltajeBatterySource < minVoltaje)
    {
      digitalWrite(MosfetControl, HIGH);//Normally opened Rele 5 volt-> ON
      Serial.print("Voltaje menor a");
      Serial.println(minVoltaje);
      Serial.println("charging battery...");
    }
    if ((voltajeBatterySource >= minVoltaje) && (voltajeBatterySource <= (maxVoltaje)))
    {
      Serial.print("Voltaje Intermedio:");
      Serial.print(voltajeBatterySource, 2);
      Serial.println("...");
    }
    if (voltajeBatterySource > maxVoltaje)
    {
      digitalWrite(MosfetControl, LOW);//Normaly opened rele 0 volts->OFF
      Serial.print("Voltaje mayor a");
      Serial.println(maxVoltaje, 2);
      Serial.println(" Charging battery stop");
    }

    if (sensor2Value >= 4)
    {
      Serial.println("Si hay energia electrica");
    }
    else
    {
      Serial.println("No hay energia electrica");
      for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
        divider = melody[thisNote + 1];
        if (divider > 0) {
          noteDuration = (wholenote) / divider;
        } else if (divider < 0) {
          noteDuration = (wholenote) / abs(divider);
          noteDuration *= 1.5; // increases the duration in half for dotted notes
        }
        tone(buzzer, melody[thisNote], noteDuration * 0.9);
        delay(noteDuration);
        noTone(buzzer);
      }
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

    if (strncmp("GET /50", data, 7) == 0)
    {
      ether.httpServerReply(homePagephone()); // send web page data
    }
    if (strncmp("GET /RST", data, 8) == 0)
    { //definir si es para version rele de estado solido o rele de bobina
      digitalWrite(releB, HIGH);
      delay(500);
      digitalWrite(releB, LOW);
      EEPROM.write(6, 1);
      ether.httpServerReply(homePagephone()); // send web page data
      restart(1);
    }
    if (strncmp("GET /RSTNTW", data, 11) == 0)
    {
      byte myip[] = {192, 168, 55, 249};
      EEPROM.write(7, 1);
      ethconfig();
    }
    //correcion:/IP=IPBYTE3.IPBYTE4.GATEWAYBYTE4
    if (strncmp("GET /IP=", data, 8) == 0) // 055.250.254    /IP=SUBNETBYTE3.IPBYTE4.GATEWAYBYTE4  {--myip[2]--gwip[2]} {--myip[3]} {---gwip[3]}    192.168.55.249    example   55.249.001
    {
      char myipok[] = "192.168.254";
      Serial.println("begin:");
      Serial.println(data[8]);
      dstart = 8;
      //start of replace
      dlen = 11;
      int j = -1;
      for (int i = dstart; i < dstart + dlen; i++)
      {
        j += 1;
        myipok[j] = data[i];
      }
      Serial.println("myipok:");
      Serial.println(myipok);

      int myipok2[2];
      char *p = myipok;
      char *str;
      int i = -1;
      int number;
      while ((str = strtok_r(p, ".", &p)) != NULL)
      {
        i += 1;
        Serial.println(F("ip recibida:::"));
        Serial.println(str);
        number = atoi(str);
        myipok2[i] = number;
        Serial.println(F("number::"));
        Serial.println(number);
        Serial.println(F("ip en array:::"));
        Serial.println(myipok2[i]);
      }
      EEPROM.write(8, myipok2[0]);
      EEPROM.write(9, myipok2[1]);
      EEPROM.write(10, myipok2[2]);
      EEPROM.write(7, 3);
      ether.httpServerReply(homePagephone()); // send web page data
      Serial.println(F("rest"));
      restart(1);
    }
  }
}

