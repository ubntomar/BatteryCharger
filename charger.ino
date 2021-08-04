//las direcciones ip se deben cambia en el setup y en la funcion verifica 
#include <EtherCard.h>
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
int releB = 4;
int releD = 6;
const char website[] PROGMEM = "www.redesagingenieria.com";
#define REQUEST_RATE 900000 //1800000 milliseconds  --30 minutos
int read_sensor_delay = 100;
byte Ethernet::buffer[330]; //copy and pasted in agc.php    225
char myArray[0];
static uint32_t timer;
static uint32_t timer2;
static uint32_t timer3;
unsigned long previousMillis = 0;
const long interval = 130000; // interval at which to blink (milliseconds)
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
// ---------------------------------------------
//  122(x1)-> 7.1v(y1)  y=(0.05563*lectura)+0.31267
//  264(x2)-> 15.v(y2)

void sensor1()
{
  int lectura = 0;
  float sensor1ValueSum = 0;
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(trigerA, HIGH);
    delay(read_sensor_delay);
    lectura = analogRead(sensorPin);
    sensor1ValueSum += (0.05563 * lectura) + 0.31267;
    digitalWrite(trigerA, LOW);
  }
  sensor1Value = sensor1ValueSum / 5;
  Serial.println(F("voltaje 1 "));
  //Serial.println(lectura);
  Serial.println(sensor1Value);

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
  sensor2Value = (sensor2ValueSum / 5) - 0.2;
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
  releb_value = digitalRead(releB);

  reled_value = digitalRead(releD);

  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
                   "$F"
                   "$D-$D-$D-$D-$S-$S-7890 \r\n"),
               htmlHeaderphone, releb_value, relec_value, reled_value, relee_value, charVal, charVal2);
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
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println(F("Failed Ethernet  start"));
  Serial.println(F("probando ether.staticSetup"));
  if (ether.begin(sizeof Ethernet::buffer, mymac) != 0)
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
  pinMode(releD, OUTPUT);
  pinMode(ucclockin, INPUT);
  pinMode(A3, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(acVoltajeLive, INPUT);
  digitalWrite(acVoltajeLive, LOW);
  digitalWrite(releB, EEPROM.read(1));
  digitalWrite(releD, EEPROM.read(3));
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
    { //Battery charger must be 24v
      Serial.println("*****************************");
      if(!digitalRead(MosfetControl)){
        digitalWrite(MosfetControl, HIGH);//turn off the battery charger
        delay(5000);  
        sensor1();
        digitalWrite(MosfetControl, LOW); //turn on the battery charger
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
      digitalWrite(MosfetControl, LOW);//Normally opened Rele 0 volt-> ON
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
      digitalWrite(MosfetControl, HIGH);//Normaly opened rele 5 volts->OFF
      Serial.print("Voltaje mayor a");
      Serial.println(maxVoltaje, 2);
      Serial.println(" Charging battery stop");
    }

    if (digitalRead(acVoltajeLive) == 1)
    {
      Serial.println("Si hay energia electrica");
    }
    else
    {
      Serial.println("No hay energia electrica");
      //digitalWrite(acVoltajeLive,LOW);
    }
  }

  word len = ether.packetReceive(); // go receive new packets
  word pos = ether.packetLoop(len); // respond to incoming
  //memset( &myipok, 0, sizeof(myipok));
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

      digitalWrite(releD, HIGH);

      delay(500);
      digitalWrite(releB, LOW);

      digitalWrite(releD, LOW);

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
