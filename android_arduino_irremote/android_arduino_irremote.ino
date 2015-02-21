/*
 * An IR LED must be connected to Arduino PWM pin 3.
 * http://zhangtory.com
 * by ZhangTory
 */
#include <IRremote.h>
#include <SoftwareSerial.h>

SoftwareSerial wifi(2, 4); // RX, TX
String comdata = ""; //get from wifi rx,tx
String data = ""; //get data from comdata
String op = ""; //operating
String irRaw = ""; //raw code from wifi data
int serverFlag = 0; //if wifi restart then make sure tcp server if working

int RECV_PIN = 11; //recive btn pin
int LED_PIN = 13; //flash led
decode_results results; //ir results
unsigned int rawCode[100];
int bits;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

void setup() {
  Serial.begin(9600);
  Serial.println("Serial Connected!");
  //set ESP8266 data rate
  wifi.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(2000);
  digitalWrite(LED_PIN, LOW);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(2, INPUT_PULLUP);
}

//read wifi rx,tx
void getComdata()
{
  while (wifi.available() > 0) {
    comdata += char(wifi.read());
    delay(2);
  }
}

//start tcp server
void initTcpServer()
{
  wifi.println("AT+CIPMUX=1");
  getComdata();
  Serial.println(comdata);
  comdata = "";
  delay(10);
  wifi.println("AT+CIPSERVER=1,4998");
  getComdata();
  Serial.println(comdata);
  comdata = "";
  delay(10);
  wifi.println("AT+CIPSTO=2880");
  getComdata();
  Serial.println(comdata);
  comdata = "";
  delay(10);
}

//read command from wifi comdata
void command()
{
  int custom = int(comdata[7]) - 48;//recode which client
  int len = int(comdata[9]) - 48;
  if (comdata[10] != ':') {
    len *= 10;
    len += (comdata[10] - 48);
  }
  if (comdata[10] == ':') {
    for (int i = 11; i < len + 11; i++) {
      data += comdata[i];
    }
  }
  if (comdata[11] == ':') {
    for (int i = 12; i < len + 12; i++) {
      data += comdata[i];
    }
  }
  //  Serial.print("comdatalen: ");
  //  Serial.println(comdata.length());
  //  Serial.print("comdata: ");
  //  Serial.println(comdata);
  //  Serial.print("custom: ");
  //  Serial.println(custom);
  //  Serial.print("len: ");
  //  Serial.println(len);

  if (comdata.indexOf('e') + 1 == comdata.indexOf('n') && comdata.indexOf('n') + 1 == comdata.indexOf('d')) {
    for (int i = 0; i < data.indexOf('&'); i++) {
      op += data[i];
    }
    for (int i = data.indexOf('&') + 1 ; i < data.length() - 3 ; i++) {
      irRaw += data[i];
    }

    Serial.print("op:");
    Serial.println(op);
    Serial.print("irRaw:");
    Serial.println(irRaw);
    Serial.print("data: ");
    Serial.println(data);

    if (op == "1") { // 1 == send
      string2int();
      sendCode();
    }
    Serial.print("bits");
    Serial.println(bits);
    data = "";
    op = "";
    irRaw = "";
  } else {
    comdata = "";
    while (comdata == "") {
      getComdata();
    }
    command();
  }
}

void string2int()
{
  //unsigned int  raw ;  string2int
  int temp = 0;
  bits = 0;
  for (int i = 0; i < irRaw.length(); i++) {
    if (irRaw[i] == ',') {
      rawCode[bits++] = temp;
      temp = 0;
    } else {
      temp *= 10;
      temp += int(irRaw[i]) - 48;
    }
  }
  bits--;
}

//get rawCode
//wait ir
void getRawCode(decode_results *results) {
  digitalWrite(LED_PIN, HIGH);
  int count = results->rawlen - 1;
  Serial.print(count);
  Serial.print(" | ");
  for (int i = 1; i <= count; i++) {
    if ((i % 2) == 1) {
      rawCode[i - 1] = results->rawbuf[i] * USECPERTICK;
    }
    else {
      rawCode[i - 1] = results->rawbuf[i] * USECPERTICK;
    }
    Serial.print(rawCode[i - 1]);
    Serial.print(" ");
  }
  Serial.println();
  bits = count;
  postRawCode();//send code to phones
  digitalWrite(LED_PIN, LOW);
}

//send commend
void sendCode() {
  irsend.sendRaw(rawCode, bits + 1, 38);
  Serial.print("sendRaw[");
  Serial.print(++bits);
  Serial.print("]:{");
  for (int i = 0; i <= bits; i++) {
    Serial.print(rawCode[i]);
    if (i != bits - 1) {
      Serial.print(",");
    }
  }
  Serial.println("};");
  ledFlash();
  delay(200);
}

//flash led to show the commend has been done
void ledFlash() {
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);
}



//send rawCode to phones
void postRawCode() {

}

void loop() {
  //if wifi restart then make sure tcp server if working
  if (serverFlag == 0) {
    initTcpServer();
    serverFlag = 1;
  }

  //get data from wifi esp8266
  getComdata();
  if (comdata.indexOf('I') + 1 == comdata.indexOf('P') && comdata.indexOf('P') + 1 == comdata.indexOf('D')) {
    //Serial.println(comdata);
    command();
    comdata = "";
  }

  if (comdata.length() > 0) {
    Serial.print(comdata);
    comdata = "";
  }

  if (Serial.available()) {
    wifi.write(Serial.read());
  }
}
