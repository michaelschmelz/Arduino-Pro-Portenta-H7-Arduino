int myLED;

void setup() {

  randomSeed(analogRead(0));

  #ifdef CORE_CM7  
    bootM4();  
    myLED = LEDB; // built-in blue LED
  #endif

  #ifdef CORE_CM4  
    myLED = LEDR; // built-in greeen LED
  #endif   

  pinMode(myLED, OUTPUT);
}

void loop() {
  digitalWrite(myLED, LOW); // turn the LED on 
  delay(200); // wait for 200 ms
  digitalWrite(myLED, HIGH); // turn the LED off 
  delay(random(1000, 3000)); // wait for a random amount of time between 1 and 3 seconds.
} 