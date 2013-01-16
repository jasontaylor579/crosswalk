/*

As of 3:00AM on 20 October 2012 this is the most working version with everything in it!
This version can be uploaded to 2 devices configured with a wireless receiver on pin 4
and a wireless transmitter on pin 3 (and as of this time is currently on both my test devices)

 Turns on and off a powered light output system connected to digital  
 pin 9 when pressing a pushbutton attached to pin 2.
 
 Sends an RF signal (a message of the letter "A" via VirtualWire) to other device(s) running same program,
 causing them to behave as if their pushbutton was pressed.
 
 Polls for RF messages (using the VirtualWire library); receipt of any message on the channel causes unit
 to behave as if pushbutton was pressed.
 
 TO DO: Poll for a specific message 
 
 The circuit:
 * LED attached from pin 13 to ground (for debugging)
 * External lights circuit attached to pin 9
 * pushbutton attached to pin 2 from +5V
 * 10K resistor attached to pin 2 from ground
 
 See JT's sketchy OmniGraffle diagram for a mashup of the above with the lighting circuit (sans RF devices)
     http://bildr.org/2011/03/high-power-control-with-arduino-and-tip120/
 
 created 2005
 by DojoDave <http://www.0j0.org>
 modified 30 Aug 2011
 by Tom Igoe
 modified 20 Oct 2012
 by Jason Taylor
 
 Original example code is in the public domain, including button example code from
 http://www.arduino.cc/en/Tutorial/Button
 and transmit/receive examples from VirtualWire project site
 
 */

#include <VirtualWire.h>

// constants won't change. They're used here to 
// set pin numbers:
const int buttonPin = 2;     // the number of the pushbutton pin
const int ledPin =  9;      // the number of the light output pin - remember "ledPin" here means "external lights"
const int RF_TX_PIN = 3;    // the number of the pin for the transmitter
const int RF_RX_PIN = 4;    // the number of the pin for the receiver
const int debugLED = 13;    // the number of the pin for the (default) debugging LED on the shield

// variables will change:
boolean buttonState = false;         // variable for reading the pushbutton status
boolean oldButtonState = buttonState;
boolean ledState = false;
boolean started = false; // starts a sequence
//long waitUntil=0;
long startTime=0;
long flashStartTime=0;
int holdTime=2000;  // duration of hold sequence
int flashTime=15000;  // duration of full flash sequence
int strobe=500;  // duration of one flash
//int runtime=0;

void setup() {
  Serial.begin(9600);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);      
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  vw_setup(2000);	 // Bits per sec
  vw_set_tx_pin(RF_TX_PIN); // Setup transmit pin
  vw_set_rx_pin(RF_RX_PIN);  // Setup receive pin.
  vw_rx_start();       // Start the receiver PLL running
}

void loop() {
  
  // Each time through loop(), set Pin 9 to the same value as the state variable
  digitalWrite(ledPin, ledState);
  
  // read button value at start of loop, before doing anything
  buttonState = digitalRead(buttonPin);
  
  // triggers more light time every time button is pressed -- add debounce if time permits
  if (buttonState != oldButtonState) startSeq(); // any time button state changes we add time to lights on
  
  // stuff to handle receipt of message from another device
  // right now also extends time even if button was pressed locally and across street at same time!
  
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen)) { // Non-blocking
	int i;
	Serial.print("Got: ");
	for (i = 0; i < buflen; i++) {
           digitalWrite(debugLED, HIGH); //test LED
	   Serial.println(buf[i], HEX);
	}
        digitalWrite(debugLED, LOW);
         startSeq(); // any time we get a message we add time to lights on
  }
  
  
  if (started) { // the button was pushed, so 
   
   // previous working flasher code
    if (millis() >= (flashStartTime)) {  // if LED has been on more than the hold time...
      if (millis() <= (flashStartTime+flashTime)) {  // enter flash sequence if it hasn't entered yet
        if ( ( (millis() - flashStartTime) % strobe ) == 0){ // toggle every strobe duration. PROBLEM LINE! (says Jake)
          ledState = !ledState;
          Serial.println(millis() - flashStartTime, DEC);
        }
      }
      else { (ledState = false); started=false; } // turn off after flash sequence
    }
  }
  
  // read button value at end of loop to see if it changes
  oldButtonState = buttonState;
}

void startSeq () { // so currently if button state changes in either direction we get another seq start...?

   // code for sending a message off to another light kit
   const char *msg = "A";
   digitalWrite(debugLED, true); // turn on the debug LED
   Serial.println("ss");
   vw_send((uint8_t *)msg, strlen(msg));
   vw_wait_tx(); // Wait until the whole message is gone
   Serial.println("es");
   digitalWrite(debugLED, false); // turn off the debug LED
   // end code for sending message off to another light kit
 
  //now let's get the lights on this pole blinking!  
  started=true;
  startTime=millis(); //records when in the program the button was pressed
  flashStartTime=startTime+holdTime; // flashing sequence will start after holding sequence
  //waitUntil=startMillis;
  ledState = true; //turn LED on and start sequence every time button is pressed
  
}
