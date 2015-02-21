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
unsigned int rawCode[500];
int bits;
int lastButtonState; //recode btn status

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
  int custom = int(comdata[7]) - 48;
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
  Serial.print("data: ");
  Serial.println(data);
  for (int i = 0; i < data.indexOf('&'); i++) {
    op += data[i];
  }
  for (int i = data.indexOf('&') + 1 ; i < len ; i++) {
    irRaw += data[i];
  }
  data = "";
  Serial.print("op:");
  Serial.println(op);
  Serial.print("irRaw:");
  Serial.println(irRaw);
  op = "";
  irRaw = "";
}

//get rawCode
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
  irsend.sendRaw(rawCode, bits, 38);
  Serial.print("sendRaw[");
  Serial.print(bits);
  Serial.print("]:{");
  for (int i = 0; i < bits; i++) {
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

  int buttonState = digitalRead(2);
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Released");
    irrecv.enableIRIn(); // Re-enable receiver
  }

  if (buttonState) {
    Serial.println("btn");
    delay(1000);
  } else {
    Serial.println("else");
   // if (irrecv.decode(&results)) {
   //   getRawCode(&results);
   //   irrecv.resume(); // Receive the next value
  //  }
  }
  lastButtonState = buttonState;
}
