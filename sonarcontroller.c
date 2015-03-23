//SONAR distance displayed on a 4x7 segement multiplexed screen.
// Code By: Steven Lawler (Steven.Lawler777@gmail.com)
// Date: 2014/01/02
// Version: 1

// Segment labelling:
//        A
//       ----
//     F |  | B
//       ---- G
//     E |  | C
//       ----   .H
//        D

// 4x7 Segment Pin Layout:
//             12        7
//         -----------------------
//         |   ~~  ~~  ~~  ~~    |
//         |   ~   ~   ~   ~     |
//         |   ~~. ~~. ~~. ~~.   |
//         -----------------------
//              1       6

// HC-SR04 SONAR Module Pin layout
//         -----------------------
//         |    _  HC-SR04  _    |
//         |   / \         / \   |
//         |   \ /  FRONT  \ /   |
//         |                     |
//         -----------------------
//                1  2  3  4

//Pins for the HC-SR04 SONAR Module
// Pin 1: VCC (+5v)
// Pin 2: Trig (SONAR Trigger, TX)
// Pin 3: Echo (SONAR Echo, RX)
// Pin 4: GND

//Remember to use a 220ohm resistor on all cathodes if using 5v.
//Pins on 4x7 Segment Display:
// Pin 1:  E
// Pin 2:  D
// Pin 3:  H
// Pin 4:  C
// Pin 5:  G
// Pin 6:  4th 7 Segment
// Pin 7:  B
// Pin 8:  3rd 7 Segment
// Pin 9:  2nd 7 Segment
// Pin 10: F
// Pin 11: A
// Pin 12: 1st 7 Segment

byte segmentPinCount=8; //Includes the . (Decimal place)
byte segmentDisplayCount=4; //How many 7 segment displays do we have?
byte sevenSegmentPins[8] = {2,3,4,5,6,7,8,9}; // A, B, C, D, E, F, G, H
byte sevenSegmentCathodePins[4]={10,11,14,16}; //1st, 2nd, 3rd, 4th

byte triggerPin = 13; //SONAR trigger pin.
byte echoPin = 12; //SONAR echo pin.
byte sonarTimeTransmit = 2; //Time to transmit sound.
byte sonarTimeDelay = 5; //How long to wait for echo.
float calibrationRatio = 29.1;
int displayThreshhold = 150; //The max number we want to display before sending out an error.

int delayTime = 4; // Time in ms to allow for pin state change.
boolean bigEndian = true; //If the display can't fit the number, which end should we show?
boolean serialCom = true; //Allow serial communication.

long serialData=0;
long displayData=0;

byte sevenSegmentDisplay[40][8] = {
  //A B C D E F G H
  { 1,1,1,1,1,1,0,0 },  // = 0
  { 0,1,1,0,0,0,0,0 },  // = 1
  { 1,1,0,1,1,0,1,0 },  // = 2
  { 1,1,1,1,0,0,1,0 },  // = 3
  { 0,1,1,0,0,1,1,0 },  // = 4
  { 1,0,1,1,0,1,1,0 },  // = 5
  { 1,0,1,1,1,1,1,0 },  // = 6
  { 1,1,1,0,0,0,0,0 },  // = 7
  { 1,1,1,1,1,1,1,0 },  // = 8
  { 1,1,1,0,0,1,1,0 },  // = 9
  { 1,1,1,0,1,1,1,0 },  // = A
  { 0,0,1,1,1,1,1,0 },  // = B
  { 1,0,0,1,1,1,0,0 },  // = C
  { 0,1,1,1,1,0,1,0 },  // = D
  { 1,0,0,1,1,1,1,0 },  // = E
  { 1,0,0,0,1,1,1,0 },  // = F
  { 0,0,0,0,0,0,0,0 },  // = Clear
  { 0,0,0,0,0,0,0,1 },  // = Clear. (With dot)
  { 0,0,0,0,0,0,1,0 },  // = -
  { 0,0,0,0,0,0,1,1 },  // = - . (With dot)
  { 1,1,1,1,1,1,0,1 },  // = 0.
  { 0,1,1,0,0,0,0,1 },  // = 1.
  { 1,1,0,1,1,0,1,1 },  // = 2.
  { 1,1,1,1,0,0,1,1 },  // = 3.
  { 0,1,1,0,0,1,1,1 },  // = 4.
  { 1,0,1,1,0,1,1,1 },  // = 5.
  { 1,0,1,1,1,1,1,1 },  // = 6.
  { 1,1,1,0,0,0,0,1 },  // = 7.
  { 1,1,1,1,1,1,1,1 },  // = 8.
  { 1,1,1,0,0,1,1,1 },  // = 9.
  { 1,1,1,0,1,1,1,1 },  // = A.
  { 0,0,1,1,1,1,1,1 },  // = B.
  { 1,0,0,1,1,1,0,1 },  // = C.
  { 0,1,1,1,1,0,1,1 },  // = D.
  { 1,0,0,1,1,1,1,1 },  // = E.
  { 1,0,0,0,1,1,1,1 },  // = F.
  { 1,0,0,0,1,1,0,0 },  // = R
  { 1,0,0,0,1,1,0,1 },  // = R.
  { 0,0,0,0,1,0,1,0 },  // = r
  { 0,0,0,0,1,0,1,1 }   // = r.
};

void setup()
{
  for (byte pinSetup = 0; pinSetup < segmentPinCount; ++pinSetup) {
    pinMode(sevenSegmentPins[pinSetup], OUTPUT); //Setup our control pins for output.
  }
  
  for (byte segSetup = 0; segSetup < segmentDisplayCount; ++segSetup) {
    pinMode(sevenSegmentCathodePins[segSetup], OUTPUT); //Setup our cathode power pins for each 7 segment display.
  }
  
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  if (serialCom) { Serial.begin(9600); } //Allow serial communication.
}

void loop()
{
  
  for (byte segSetup = 0; segSetup < segmentDisplayCount; ++segSetup) {
    digitalWrite(sevenSegmentCathodePins[segSetup], HIGH); //Reset cathode control pins.
  }
  delayF(1);
  if (Serial.available() > 0)
  {
    serialData = Serial.parseInt();
    Serial.print("Recieved serial input: ");
    Serial.println(serialData);
    displayData = serialData;
  }
  
  displayData=getSONARDistance();
  
  if (serialCom) {
    Serial.print("Distance: ");
    Serial.print(displayData);
    Serial.println("cm");
  }
  
  updateDisplay(displayData);
  
}

void updateDisplay(long theData)
{
  if (theData>displayThreshhold) { // Display ER0R.
    setNumber(0, 14);
    setNumber(1, 38);
    setNumber(2, 0);
    setNumber(3, 39);
  } else {
    displayNumber(displayData);
  }
}

void delayF(long period)
{
  for (long i=0;i<period;i++) {
    updateDisplay(displayData); //Update screen to avoid flickering.
    delay(1);
  }
}

long getSONARDistance()
{
  long timeDuration=0;
  long distance=0;
  digitalWrite(triggerPin, LOW);
  delayF(sonarTimeTransmit);
  digitalWrite(triggerPin, HIGH);
  delayF(sonarTimeDelay);
  digitalWrite(triggerPin, LOW);
  timeDuration = pulseIn(echoPin, HIGH);
  distance = (timeDuration/2) / calibrationRatio; // Divide by 2 to get half the full distance of the return trip.
  
  return distance;
}

void displayNumber(long originalNumber)
{
  long newNumber=originalNumber;
  byte digit=0;
      
  if (bigEndian) {
    for (byte segSetup = segmentDisplayCount; segSetup > 0; --segSetup) {
      digit = newNumber%10;
      
      if (newNumber > 0) {
        setNumber(segSetup-1, digit);
      }
      else {
        setNumber(segSetup-1, 0);
      }
      newNumber /= 10;
    }
  } else {
    long tmpNum=0;
    while (newNumber > 0) { //Reverse the integer.
      tmpNum = tmpNum * 10 + (newNumber % 10);
      newNumber = newNumber / 10;
    }
    newNumber=tmpNum;

    for (byte segSetup = 0; segSetup < segmentDisplayCount; ++segSetup) {
      digit = newNumber%10;
      
      if (newNumber > 0) {
        setNumber(segSetup, digit);
      }
      else {
        setNumber(segSetup, 0);
      }
      newNumber /= 10;
    }
  }
  
}

void setNumber(byte pin, int x)
{
  digitalWrite(sevenSegmentCathodePins[pin], LOW);
   switch(x){
     case 1:
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[1][segIndex]);
       }
       break;
     case 2:
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[2][segIndex]);
       }
       break;
     case 3:
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[3][segIndex]);
       }
       break;
     case 4: 
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[4][segIndex]);
       }
       break;
     case 5:
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[5][segIndex]);
       }
       break;
     case 6:
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[6][segIndex]);
       }
       break;
     case 7:
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[7][segIndex]);
       }
       break;
     case 8:
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[8][segIndex]);
       }
       break;
     case 9:
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[9][segIndex]);
       }
       break;
     case 0:
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[0][segIndex]);
       }
       break;
     case 10: //A
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[10][segIndex]);
       }
       break;
     case 11: //B
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[11][segIndex]);
       }
       break;
     case 12: //C
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[12][segIndex]);
       }
       break;
     case 13: //D
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[13][segIndex]);
       }
       break;
     case 14: //E
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[14][segIndex]);
       }
       break;
     case 15: //F
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[15][segIndex]);
       }
       break;
     case 16: // Clear
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[16][segIndex]);
       }
       break;
     case 17: // Clear . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[17][segIndex]);
       }
       break;
     case 18: // - (Dash)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[18][segIndex]);
       }
       break;
     case 19: // - . (Dash with dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[19][segIndex]);
       }
       break;
     case 20: //0 . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[20][segIndex]);
       }
       break;
     case 21: //1 . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[21][segIndex]);
       }
       break;
     case 22: //2 . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[22][segIndex]);
       }
       break;
     case 23: //3 . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[23][segIndex]);
       }
       break;
     case 24: //4 . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[24][segIndex]);
       }
       break;
     case 25: //5 . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[25][segIndex]);
       }
       break;
     case 26: //6 . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[26][segIndex]);
       }
       break;
     case 27: //7 . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[27][segIndex]);
       }
       break;
     case 28: //8 . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[28][segIndex]);
       }
       break;
     case 29: //9 . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[29][segIndex]);
       }
       break;
     case 30: //A . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[30][segIndex]);
       }
       break;
     case 31: //B . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[31][segIndex]);
       }
       break;
     case 32: //C . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[32][segIndex]);
       }
       break;
     case 33: //D . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[33][segIndex]);
       }
       break;
     case 34: //E . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[34][segIndex]);
       }
       break;
     case 35: //F . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[35][segIndex]);
       }
       break;
     case 36: //R
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[36][segIndex]);
       }
       break;
     case 37: //R . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[37][segIndex]);
       }
       break;
     case 38: //r
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[38][segIndex]);
       }
       break;
     case 39: //r . (With dot)
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[39][segIndex]);
       }
       break;
 
     default: //Clear
       for (byte segIndex = 0; segIndex < segmentPinCount; ++segIndex) {
         digitalWrite(sevenSegmentPins[segIndex], sevenSegmentDisplay[16][segIndex]);
       }
       break;
   }
   delay (delayTime);
   digitalWrite(sevenSegmentCathodePins[pin], HIGH);
}


