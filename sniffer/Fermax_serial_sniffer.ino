#include <AltSoftSerial.h>

// Arduino UNO/Nano RX pin D8
AltSoftSerial fermaxSerial;

void setup() {

  // Init USB serial
  Serial.begin(115200);
  Serial.println("fermaxSerial: Start");

  // Init Fermax Serial
  fermaxSerial.begin(1200, SERIAL_8O1);
}

void loop() {
  char buffer[3];
  unsigned long currentMicros = micros();

  if (fermaxSerial.available()) {

    // Get the first byte
    unsigned char reponse = fermaxSerial.read();
    sprintf (buffer, "%02X", reponse);
    Serial.print(buffer);

    // Start timer 10000 microsec
    currentMicros = micros();
    while(micros() - currentMicros < 10000)
    {
        if (fermaxSerial.available()) {
          // Get the second byte
          unsigned char reponse = fermaxSerial.read();
          sprintf (buffer, "%02X", reponse);
          Serial.print(buffer);
          break;
        }
    }

    Serial.print("\r\n");
  }

}
