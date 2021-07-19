#define LED_red 4  // The pin the LED is connected to

void setup() {
  pinMode(LED_red, OUTPUT); // Declare the LED as an output
}

void loop() {
  digitalWrite(LED_red, HIGH); // Turn the LED on
  delay(500);
  digitalWrite(LED_red, LOW); // Turn the LED on
  delay(500);
  
}
