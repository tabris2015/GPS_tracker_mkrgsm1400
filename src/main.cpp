#include <Arduino.h>
#include <MKRGSM.h>
// SoftwareSerial Serial1(10, 11); // RX, TX (TX not used)
const int sentenceSize = 80;

char sentence[sentenceSize];
String lat_raw;
String lon_raw;
char n_s;
char e_w;
String req;
int data_ready = 0;
void getField(char* buffer, int index)
{
  int sentencePos = 0;
  int fieldPos = 0;
  int commaCount = 0;
  while (sentencePos < sentenceSize)
  {
    if (sentence[sentencePos] == ',')
    {
      commaCount ++;
      sentencePos ++;
    }
    if (commaCount == index)
    {
      buffer[fieldPos] = sentence[sentencePos];
      fieldPos ++;
    }
    sentencePos ++;
  }
  buffer[fieldPos] = '\0';
} 

void displayGPS()
{
  char field[20];
  getField(field, 0);
  if (strcmp(field, "$GPRMC") == 0)
  {
    getField(field, 3);  // number
    lat_raw = field;
    getField(field, 4); // N/S
    n_s = field[0];
    float lat_deg = lat_raw.substring(0,lat_raw.length() - 7).toFloat();
    float lat_min_dec = lat_raw.substring(lat_raw.length() - 7).toFloat()/60.0;
    float lat_dec = (lat_deg + lat_min_dec) * (n_s == 'N' ? 1 : -1);
    
    getField(field, 5);  // number
    lon_raw = field;
    getField(field, 6); // N/S
    e_w = field[0];
    float lon_deg = lon_raw.substring(0,lon_raw.length() - 7).toFloat();
    float lon_min_dec = lon_raw.substring(lon_raw.length() - 7).toFloat()/60.0;
    float lon_dec = (lon_deg + lon_min_dec) * (e_w == 'E' ? 1 : -1);
    String lat_str = String(lat_dec, 6);
    String lon_str = String(lon_dec, 6);

    req = "/mapacentral/api/vehiculoposition?id=34&latitud=";
    req += lat_str;
    req += "&longitud=";
    req += lon_str;

   
    data_ready = 1;
  }
}

void InitGPRS();
void SendGPRS();
void setup()
{
  // reset the ublox module
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);
  Serial.begin(115200);
  Serial1.begin(9600);
  SerialGSM.begin(115200);
  delay(5000);
  InitGPRS();
}

void loop()
{
  static int i = 0;
  if (Serial1.available())
  {
    char ch = Serial1.read();
    if (ch != '\r\n' && i < sentenceSize)
    {
      sentence[i] = ch;
      i++;
    }
    else
    {
     sentence[i] = '\0';
     i = 0;
     displayGPS();
    }
  }
  if(data_ready)
  {
    //  Serial.println(req);
     SendGPRS();
     data_ready = 0;
  }

  if (SerialGSM.available()) {
    Serial.write(SerialGSM.read());
  }
}

void InitGPRS()
{
//   at+cmee=2
// AT+CGATT=1
// at+upsd=0,7,"0.0.0.0"
// at+upsda=0,3
// at+upsnd=0,8
// at+uping="18.221.56.190",1,32,5000,64
  char buffer[100];
  char c;
  int i = 0;
  String response;
  SerialGSM.print("AT\r\n");
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  delay(500);
  SerialGSM.print("AT+CMEE=2\r\n");
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  // delay(200);
  // SerialGSM.flush();
  SerialGSM.print("AT+CGATT=1\r\n");
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  
  // delay(1000);

  SerialGSM.print("at+upsd=0,7,\"0.0.0.0\"\r\n");
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  delay(1000);

  SerialGSM.print("at+upsda=0,3\r\n");
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  delay(1000);

  SerialGSM.print("at+upsnd=0,8\r\n");
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  delay(500);
}
void SendGPRS()
{
  // 
//   AT+UHTTP=0,0,"18.221.56.190"
// AT+UHTTP=0,5,80
// AT+UHTTPC=0,1,"/mapacentral/api/vehiculoposition?id=34&latitud=-16.469349&longitud=-68.150574","gps"
// AT+UHTTPER=0
// AT+ULSTFILE=2,"gps"
// AT+URDFILE="gps"
  String response;
  digitalWrite(LED_BUILTIN, 1);
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  SerialGSM.print("AT+UHTTP=0,0,\"18.221.56.190\"\r\n");
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  delay(1000);

  SerialGSM.print("AT+UHTTP=0,5,80\r\n");
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  delay(200);


  SerialGSM.print("AT+UHTTPC=0,1,\"");
  SerialGSM.print(req);
  SerialGSM.print("\",\"gps\"\r\n");
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  delay(2000);

  SerialGSM.print("AT+URDFILE=\"gps\"\r\n");
  do
  {
    Serial.write(SerialGSM.read());
  } while (SerialGSM.available());
  delay(3000);
  digitalWrite(LED_BUILTIN, 0);
}

// // baud rate used for both Serial ports
// unsigned long baud = 115200;

// void setup() {
//   // reset the ublox module
//   pinMode(GSM_RESETN, OUTPUT);
//   digitalWrite(GSM_RESETN, HIGH);
//   delay(100);
//   digitalWrite(GSM_RESETN, LOW);

//   Serial.begin(baud);
//   SerialGSM.begin(baud);

// }

// void loop() {
//   if (Serial.available()) {
//     SerialGSM.write(Serial.read());
//   }

//   if (SerialGSM.available()) {
//     Serial.write(SerialGSM.read());
//   }
// }


