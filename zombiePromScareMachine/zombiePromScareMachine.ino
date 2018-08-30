/*
  This is the Zombie Prom Scare Machine, designed by Edwin Fallwell for 
  Monroe County Public Library in 2018. To build this, you need an ultrasonic
  rangefinder, an Adafruit Music Maker shield, and an Arduino. 

  This item will continously play track002.mp3 until something gets within 30cm. 
  When this happens, it plays track001.mp3 at a higher volume, then resumes playing 
  track0002.mp3. For this scare machine, track002 is a loop of zombie moans + track001 is
  screams! 
  
  Based on code written by Limor Fried/Ladyada for Adafruit Industries + the New Ping library by Tim Eckel.  
*/

// include SPI, MP3, NewPing, and SD libraries
#include <NewPing.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)
#define CARDCS 4     // Card chip select pin
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

// And these are the pins used for the ultrasonic sensor. 
#define TRIGGER_PIN  9  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     8  // Arduino pin tied to echo pin on the ultrasonic sensor.

// Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define MAX_DISTANCE 200 

// Our NewPing and VS1053 objects. 
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

void setup() {
  Serial.begin(9600); 
  Serial.println("Zombie Prom Scare Machine");

  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
  
   if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }

  // list files
  printDirectory(SD.open("/"), 0);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20,20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
  
  // Start the sound loop
  Serial.println(F("Zombie Moans"));
  musicPlayer.startPlayingFile("track002.mp3");
}

void loop() {
  delay(50);                     
  int pingDistance = sonar.ping_cm();
  Serial.print("Ping: ");
  Serial.print(pingDistance);
  Serial.println("cm");

  if(pingDistance<30 && pingDistance!=0){
    Serial.println("Scary Sound");
    musicPlayer.setVolume(5,5);
    musicPlayer.playFullFile("track001.mp3");
    Serial.println("Zombie Moans");
    musicPlayer.setVolume(20,20);
    musicPlayer.startPlayingFile("track002.mp3");
  }
  
  if (musicPlayer.stopped()) {
    Serial.println("Looping Zombie Moans");
    musicPlayer.startPlayingFile("track002.mp3");
  }
}


/// File listing helper
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

