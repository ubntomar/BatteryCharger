//las direcciones ip se deben cambia en el setup y en la funcion verifica
//
#include <EtherCard.h> //Usa por defecto pin 10 de Atmega328 P, pero el diseño de ese pin est{a} sobre el pin 12 . En este momento se modifica la tarjeta pero se podr{i}a modificar la libreria.
#include <stdlib.h>    //Lo de arriba?Creo q ya no aplica por q se hace no por hardware sino por software
#include <EEPROM.h>

#define VOLTS_IN_A0 A0   
#define VOLTS_IN_A1 A1   
#define TRIGER_TO_A0 A2 
#define TRIGER_TO_A1 A3 
#define SERIAL_RX 0
#define SERIAL_TX 1

#define SOFT_RESET 5   
#define BUZZER 6
#define PLUGED_DEVICE_CTL 7
#define BATTERYSYSTEM 12
#define SYSTEM1 12
#define SYSTEM2 24
#define MAC {0x74, 0x69, 0x69, 0x2D, 0x30, 0x3B}
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
byte myip[] = {192, 168, 30, 254};   //
byte mymac[] = MAC;                  //
byte gwip[] = {192, 168, 30, 1};     //
byte static_dns[] = {8, 8, 8, 8};    //
byte netmask[] = {255, 255, 255, 0}; //

int reboot = 0;

const char website[] PROGMEM = "www.google.com";
const long interval = 300000; //(milliseconds)
int read_sensor_delay = 4; 
int sample=30;
byte Ethernet::buffer[300]; //copy and pasted in agc.php    225
unsigned long previousMillis = 0;
unsigned long time;
BufferFiller bfill;
char ipadd[16];
char gwadd[16];
char nmadd[16];
float sensor1Value;
float sensor2Value;
char charVal[10];
char charVal2[10]; //temporarily holds data from vals
int dstart = 0;
int dlen = 0;
int ActionValueByGet=0;
float calibrateValue=0;
char calibrateOpt=0;
int requestStatus = 1;
int rstPending=0;
const char htmlHeaderphone[] PROGMEM =
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
#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978
#define REST 0
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
void(* resetFunc) (void) = 0;  // declare reset fuction at address 0
void sensor1()
{
  int lectura = 0;
  int sumLectura = 0;
  float x = 0;
  double m = 0.056338028169014086;
  double b = -0.5070422535211279;
  for (int i = 0; i < sample; i++)
  {
    digitalWrite(TRIGER_TO_A0, HIGH);
    delay(read_sensor_delay);
    lectura = analogRead(VOLTS_IN_A1);
    sumLectura += lectura;
    digitalWrite(TRIGER_TO_A0, LOW);
  }
  x = sumLectura / sample;
  sensor1Value = (m * x) + b;
  Serial.println(F("voltaje 1 :"));
  Serial.print(sensor1Value);

  delay(read_sensor_delay);
  dtostrf(sensor1Value+calibrateValue, 4, 1, charVal);
}
void sensor2()
{
  int lectura2 = 0;
  float sensor2ValueSum = 0;
  for (int i = 0; i < sample; i++)
  {
    digitalWrite(TRIGER_TO_A1, HIGH);
    delay(read_sensor_delay);
    lectura2 = analogRead(VOLTS_IN_A0);
    sensor2ValueSum += (0.05563 * lectura2) + 0.31267;
    digitalWrite(TRIGER_TO_A1, LOW);
  }
  sensor2Value = (sensor2ValueSum / sample) - 0.6;
  Serial.println(F("voltaje 2 "));
  Serial.println(lectura2);
  Serial.println(sensor2Value);
  if(sensor2Value<0.5){
    sensor2Value=0;
  }
  delay(read_sensor_delay);
  dtostrf(sensor2Value, 4, 1, charVal2);
}
static word homePagephone()
{ 
  sensor1();
  sensor2();
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR("$F"
                   "{\"data\":{\"status\":\"$D\",\"rele\":\"$D\",\"sensor1\":\"$S\",\"sensor2\":\"$S\",\"adj\":\"$D\",\"adjVal\":\"$D\",\"rstPending\":\"$D\"}} \r\n"),
               htmlHeaderphone,requestStatus,digitalRead(PLUGED_DEVICE_CTL), charVal, charVal2, calibrateOpt, ActionValueByGet,rstPending);
  return bfill.position();
}
void restart(int setStatus)
{
  reboot = setStatus;
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
  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0) //8 => SS=CS, pin  12 on Atmega 328P     Nov 2022 lo paso a 10 q es el valor por defecto y equivale a
    Serial.println(F("Failed Ethernet  start"));
  Serial.println(F("probando ether.staticSetup"));
  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) != 0)
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
    Serial.println(F("timeout dns.."));
  }
  Serial.println(F(".Saliendo...dnscheckup()"));
}

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
            calibrateValue=(EEPROM.read(51)*-1)/10.0;
            ActionValueByGet=EEPROM.read(51);
            calibrateOpt=1;
            break;
          case 'm':
            calibrateValue=(EEPROM.read(51)*1)/10.0;  
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
      if (digitalRead(PLUGED_DEVICE_CTL))
      {
        digitalWrite(PLUGED_DEVICE_CTL, LOW); //turn off the battery charger
        delay(5000);
        sensor1();
        digitalWrite(PLUGED_DEVICE_CTL, HIGH); //turn on the battery charger
        voltajeBatterySource = sensor1Value;
      }
      else
      {
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

    if (sensor2Value >= 4)
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
      resetFunc(); //call reset
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
}
