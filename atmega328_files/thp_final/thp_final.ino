#include <rtttl.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

const int ledBeat = 10; // Led connected to pin 10
const int heartSensor = A0; // Piezo sensor connected to pin A0
const int tempPin = A1; // LM35 connected to pin A1
float tempRead; // Read the LM35 values

int threshold = 5; // Threshold for the piezo sensor 
int oldvalue = 0; // Old value for piezo sensor
int newvalue = 0; // New value for piezo sensor
int cnt = 0; // Used for timing 
int timings[16]; // Timing values for the piezo sensor

int firstRun = 1; // Tells it to run the splashscreen

//int ledStatus = 0;
//int speakerStatus = 0;
int buttonCounter;

// Wait time for the millis timers
unsigned long oldmillis = 0;
unsigned long newmillis = 0;
unsigned long waitUntil = 0;
unsigned long waitUntilTemp = 0;

int fullBattery = 880; // Battery max value
int emptyBattery = 300; // Battery min value
int batteryPercent; // Batttery percent

// Button stuff
const int button = 5; // Button set to pin 5
unsigned long time_button = 0; // the last time the output pin was toggled
unsigned long debounce = 200; // the debounce time, increase if the output flickers
int reading_button; // the current reading from the input pin
int previous_button = HIGH; // the previous reading from the input pin

// Speaker stuff
int pinSpeaker = 9;  // Speaker pin 9 
const int octave = 0;  // The octave set for the player
const char heartBeat[] PROGMEM ="heartBeat:16e"; // Makes the heart beat sound
Rtttl player;  // Song player

uint8_t heart[8] = {0x0, 0xa, 0x1f, 0x1f, 0xe, 0x4, 0x0, 0x0}; // Custom char heart
uint8_t temp[8] = {0x4, 0xa, 0xa, 0xa, 0x11, 0x1f, 0x1f, 0xe}; // Custom char temp 
uint8_t temp_c[8] = {0x8, 0xf4, 0x8, 0x43, 0x4, 0x4, 0x43, 0x0}; // Custom char degrees c
uint8_t battery[8] = {0xe, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1f}; // Custom char battery

void setup() {
  Serial.begin(9600); 

  player.begin(pinSpeaker);  // Starts the player

  lcd.init(); // Start up the lcd
  lcd.backlight(); // Turn on the lcd backlight
  lcd.createChar(0, heart); // Custom Char heart
  lcd.createChar(1, temp); // Custom Char temp
  lcd.createChar(2, temp_c); // Custom Char degrees C
  lcd.createChar(3, battery); // Custom Char degrees C
  lcd.begin(16, 2); // Set up the lcd to have 16 char on 2 lines

  pinMode(ledBeat, OUTPUT); 

  pinMode(button, INPUT); // Set the button as input
  digitalWrite(button, HIGH); // initiate the internal pull up resistor
  }

void splashScreen() {
  lcd.clear(); // Clears the lcd
  lcd.setCursor(0,0); // Set cursor to start of the second screen
  lcd.print("#T_H_S"); // Prints to the LCD
  delay(1000);
  firstRun = 0; // Sets firstRun to 0 so it wouldnt run the warmup code again
  lcd.clear(); // Clears the lcd
  lcd.print((char)0); // Print custom heart sign to LCD
  lcd.print(" "); // Prints to the LCD
  lcd.print(" bpm "); // Prints to the LCD
}

void playSong(const char * track)  // Takes the song_# pasted from the if statements below
{
  player.play_P(track, octave);  // Passes the song_# and octave to the player
}

void loop() {

  // Runs splashscreen
  if (firstRun == 1) {
    splashScreen();
  }

 // ------------- Debound code button green start code -------------
  reading_button = digitalRead(button);

  if (reading_button == HIGH && previous_button == LOW && millis() - time_button > debounce) { 
    time_button = millis(); 
    // Do something here
    buttonCounter ++;
    //ledStatus = 1;
    Serial.println(buttonCounter);
    if (buttonCounter == 3) {
      buttonCounter = 0;
      }
  }
  previous_button = reading_button;
   
 // -------------- Debound code button green end code -------------
if (millis() >= waitUntilTemp) {
    
    //lcd.clear();
    tempRead = analogRead(tempPin); // read analog pin to get temp
    tempRead = tempRead * 0.48828125; // converts f to c
    lcd.setCursor(0,1); // Set cursor to start of the second screen
    lcd.print((char)1); // Print custom temp sign to LCD
    lcd.print(" "); // Prints to the LCD
    lcd.print(tempRead); // Prints to the LCD
    lcd.print(" "); // Prints to the LCD
    lcd.print((char)2); // Print custom temp_c sign to LCD
    int batteryRead = analogRead(A2);
    batteryPercent = map(batteryRead, emptyBattery, fullBattery,0 ,100);
    lcd.setCursor(11,1); // Set cursor
    lcd.print((char)3); // Print custom battery icon to LCD
    lcd.print(batteryPercent); // Print to the LCD
    lcd.setCursor(16,1); // Set cursor
    lcd.print("%"); // Print to the LCD

  waitUntilTemp =+ 100;
  }

  if (millis() >= waitUntil) {
  oldvalue = newvalue;
  newvalue = 0;
  for(int i=0; i<64; i++){ // Average over 16 measurements
    //newvalue += analogRead(A2);
    newvalue += analogRead(heartSensor);
  }
  newvalue = newvalue/64;
  // find triggering edge
  if(oldvalue<threshold && newvalue>=threshold){ 
    oldmillis = newmillis;
    newmillis = millis();

    // fill in the current time difference in ringbuffer
    timings[cnt%16]= (int)(newmillis-oldmillis); 
    int totalmillis = 0;
    // calculate average of the last 16 time differences
    for(int i=0;i<16;i++){
      totalmillis += timings[i];
    }

    // calculate heart rate
    int heartrate = 60000/(totalmillis/16);
    //Serial.println(heartrate,DEC);
    cnt++;
    if (buttonCounter == 0) { 
      digitalWrite(ledBeat, HIGH);
      }

    if (buttonCounter <= 1) {
      playSong(heartBeat); // Play button sound
    }

    lcd.clear(); // Clears the lcd
    lcd.setCursor(0,0); // Set cursor to start of the second screen
    lcd.print((char)0); // Print custom heart sign to LCD
    lcd.print(" "); // Prints to the LCD
    lcd.print(heartrate,DEC); // Prints to the LCD
    lcd.print(" bpm "); // Prints to the LCD
  }
  waitUntil += 5;
  }

  digitalWrite(ledBeat, LOW);

}


