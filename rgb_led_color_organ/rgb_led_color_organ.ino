/* Code for getting data from MSGEQ7 from David Wang
 * http://www.instructables.com/id/Blinking-LEDs-to-the-Frequency-of-Musi/
 * Filtering from Nathan Olivares
 * There's stuff to improve but it works
 */
 
int analogPin = 0; // MSGEQ7 OUT
int strobePin = 2; // MSGEQ7 STROBE
int resetPin = 4; // MSGEQ7 RESET
int spectrumValue[7];
 
// MSGEQ7 OUT pin produces values around 50-80
// when there is no input, so use this value to
// filter out a lot of the chaff.
int filterValue = 80;
 
// LED pins connected to the PWM pins on the Arduino
 
int ledPinG = 9;
int ledPinR = 10;
int ledPinB = 11;


// Make this variable later, but the upper limit on 
// the MSGEQ7s value, lower for quieter songs
// 1023 max
// Kinda unnecessary now, but too busy to fix
int maxVal = 800;

// Smoothing
int newR = 0;
int newG = 0;
int newB = 0;
int Rtot = 0;
int Gtot = 0;
int Btot = 0;
const int sLvl = 5; // This is the amount of samples to smooth
// More samples = smoother, but more delay and less "pop" with music
int Rsmooth[sLvl];
int Gsmooth[sLvl];
int Bsmooth[sLvl];
int i = 0;
int j = 0; // Timer for trimmer
int maxNum = 0; // max the trimmer sees
float trimmer = 1; // trimmer


void setup()
{
  Serial.begin(57600); // was no noise at 38400, 57600
  // Read from MSGEQ7 OUT
  pinMode(analogPin, INPUT);
  // Write to MSGEQ7 STROBE and RESET
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
 
  // Set analogPin's reference voltage
  analogReference(DEFAULT); // 5V
 
  // Set startup values for pins
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
  
  // Initialize to 0
    for (int thisReading = 0; thisReading < sLvl; thisReading++) {
      Rsmooth[thisReading] = 0;
      Gsmooth[thisReading] = 0;
      Bsmooth[thisReading] = 0;
    }

}
 
void loop()
{
  // Set reset pin low to enable strobe
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);
 
  // Get all 7 spectrum values from the MSGEQ7
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(strobePin, LOW);
    delayMicroseconds(36); // Allow output to settle
 
    spectrumValue[i] = analogRead(analogPin);
 
    // Constrain any value above 1023 or below filterValue
    spectrumValue[i] = constrain(spectrumValue[i], filterValue, 1023);
 
 
    // Remap the value to a number between 0 and 255
    spectrumValue[i] = map(spectrumValue[i], filterValue, maxVal, 0, 255);
 
    // Remove serial stuff after debugging
    Serial.print(spectrumValue[i]);
    Serial.print(" ");
    digitalWrite(strobePin, HIGH);


   }

   Serial.println();
   
   
   // Write the PWM values to the LEDs
   Serial.print(trimmer);
   Serial.print("   ");
   analogWrite(ledPinR, newR);
   Serial.print(newR);
   analogWrite(ledPinG, newG);
   Serial.print("   ");
   Serial.print(newG);
   analogWrite(ledPinB, newB);
   Serial.print("   ");
   Serial.print(newB);
   Serial.print("   ");


   if (i == sLvl) {
    i = 0;
   }

   // Running averager for sLvl number of values
   // R is bass, so I included the spectrum from ~0-200Hz
   Rtot = Rtot - Rsmooth[i];
   Rsmooth[i] = (spectrumValue[0] + spectrumValue[1])*.5;
   Rtot = Rtot + Rsmooth[i];
   
   Gtot = Gtot - Gsmooth[i];
   Gsmooth[i] = spectrumValue[3];
   Gtot = Gtot + Gsmooth[i];

   Btot = Btot - Bsmooth[i];
   Bsmooth[i] = spectrumValue[6];
   Btot = Btot + Bsmooth[i];
   
   newR = trimmer*Rtot/sLvl;
   newG = trimmer*Gtot/sLvl;
   newB = trimmer*Btot/sLvl;
   i = i + 1;

   // Weird things happen when an analog value is above 255
   // and R likes to do that the most
   if (newR > 254) {
    newR = 255;
   }
   
   // If the hi hat is above a threshold, no filter,
   // the filter would slow it down and ruin the flash
   if (spectrumValue[6] > 50){
    newB = spectrumValue[6];
   }

   // I get a lot of noise with G, and this fixes it
   if (newR < 5 && newG < 5 && newB < 100) {
    newG = 0;
    newB = 0;
   }


    // Trimming the values for different volumes
    if (j > 350) {
      j = 0;
      if (maxNum > 254) {
        trimmer = trimmer - .03;
      }
      if (maxNum < 200 && trimmer < 1.5 ) {
        trimmer = trimmer + .03;
      }
    }
    j = j + 1;
    if (newG > maxNum || newB > maxNum || newR > maxNum || j == 1) {
      maxNum = newG;
    }

}
