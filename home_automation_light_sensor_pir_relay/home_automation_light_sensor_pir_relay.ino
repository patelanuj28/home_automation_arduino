#include <IRremote.h>

int RECV_PIN = 2;
int RELAY_PIN = 4;
int PIR_PIN = 7;
int minSecsBetweenPIR = 300; // 5 min
long lastCheck = -minSecsBetweenPIR * 1000l;

int photocellPin = 0;     // the cell and 10K pulldown are connected to a0
int photocellReading;     // the analog reading from the sensor divider
int lightcheck = 1;

IRrecv irrecv(RECV_PIN);
IRsend irsend;


decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  pinMode(PIR_PIN, INPUT);
}

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 1; // The RC5/6 toggle state

long motionEnabled = 0;

// Stores the code for later playback
// Most of this code is just logging
void RemoteControlCode(decode_results *results) {
  codeType = results->decode_type;

  int count = results->rawlen;
  if (codeType == NEC) {
    Serial.print("Received NEC: ");
    if (results->value == REPEAT) {
      // Don't record a NEC repeat value as that's useless.
      Serial.println("repeat; ignoring.");
      return;
    }
    toggle = !toggle;
    if (//results->value == 0x807FC13E || //FAN Remote Power on/off Button
      results->value == 0x807FD12E || //FAN Remote Timer Button
      //results->value == 0x807FE11E || //FAN Remote Speed Button
      results->value == 0x8166F906 //|| //AC Remote Timer Button
      //results->value == 0x816651AE || //AC Remote TEMP Down Button
      //results->value == 0x8166A15E || //AC Remote TEMP UP Button
      //results->value == 0x81669966 || //AC Remote FAN Speed Button
      //results->value == 0x8166D926 //AC Remote Mode Button
      //results->value == 0x8166817E //AC Remote Power on/off Button
    ) {
      digitalWrite(RELAY_PIN, toggle);
    }
  }

  Serial.println(results->value, HEX);
  codeValue = results->value;
  codeLen = results->bits;


}



void loop() {

  if (irrecv.decode(&results)) {

    RemoteControlCode(&results);
    irrecv.resume(); // resume receiver

  } else {
    //Code for PIR Montion Sensor & Light Dectory Sensitivity
    long now = millis();
    photocellReading = analogRead(photocellPin);
    Serial.println(photocellReading);
    if (photocellReading < 400 && toggle == 1) {
      if (digitalRead(PIR_PIN) == HIGH) {
        Serial.println("MOVEMENT");
        if (now > (lastCheck + minSecsBetweenPIR * 1000l)) {
          lastCheck = now;
          digitalWrite(RELAY_PIN, LOW);
          toggle = 0;
        }
        digitalWrite(RELAY_PIN, LOW);
        lightcheck = 0;

      } else {
        if (now > (lastCheck + minSecsBetweenPIR * 1000l)) {
          lastCheck = now;
          digitalWrite(RELAY_PIN, HIGH);
          toggle = 1;
          Serial.println("NO MOVEMENT");
        }
      }
    } else {
      int count = 0;
      if (lightcheck == 0 && digitalRead(PIR_PIN == LOW)) {
        if (now > (lastCheck + minSecsBetweenPIR * 1000l)) {
          lastCheck = now;
          digitalWrite(RELAY_PIN, HIGH);
          toggle = 1;
          Serial.println("Lights Checking");
          lightcheck = 1;
        }
      }
      delay(100);
    }

  }
}
