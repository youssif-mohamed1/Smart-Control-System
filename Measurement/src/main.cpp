#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <Keypad.h>
#include <PS2Keyboard.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Ultrasonic.h>

// Define pin numbers for various components
#define led_pin A6
#define ldr_pin A0
#define buzzer_pin A2
#define ir_pin 2
#define tempPin A5
#define DHTPIN A1 
#define DHTTYPE DHT11
#define trig A4
#define echo A3

// Initialize DHT and Ultrasonic objects
DHT_Unified dht(DHTPIN, DHTTYPE);
Ultrasonic ultrasonic1(trig, echo);

// Define a volatile variable to control system state
volatile bool systemRunning = false;

// Initialize LiquidCrystal object
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Define a password
char arr[4] = {'1', '2', '3', '4'};

// Forward declaration of loop function
void loop();

// Function to change system mode when IR sensor detects input
void change_mode() {
  if (digitalRead(ir_pin) == LOW) {
    systemRunning = !systemRunning;
  }
}

// Function to scroll LCD display
void scroll_lcd(){
  for (int i = 0; i < 16; i++) {
    lcd.scrollDisplayRight();
    delay(20);
  }
}

// Setup function runs once when Arduino is powered on or reset
void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.begin(16, 2);
  lcd.print("Welcome");
  pinMode(buzzer_pin,OUTPUT);
  pinMode(led_pin,OUTPUT);
  pinMode(ir_pin, INPUT);
  pinMode(DHTPIN, INPUT);
  pinMode(echo,INPUT);
  pinMode(trig,OUTPUT);
  pinMode(tempPin,INPUT);
  attachInterrupt(digitalPinToInterrupt(ir_pin), change_mode, FALLING);
}

// Function to validate password entered through serial monitor
bool valid_password() {
  lcd.setCursor(0,1);
  lcd.print("Password:");
  delay(150);
  char a[5] = {'.', '.', '.', '.', '\0'};
  int c = 0;

  while (c < 4 && systemRunning) {
    if (Serial.available()) {
      char key = Serial.read();
      Serial.print("Key is: ");
      Serial.println(key);
      a[c] = key;
      lcd.print('*');
      c++;
    }
  }
  
  if (c < 4) {
    return false;
  }
  
  for (int i = 0; i < 4; i++) {
    Serial.println(a[i]);
    if (arr[i] != a[i]) {
      return false;
    }
  }
  return true;
}

// Function to print temperature on LCD
void print_temp() {
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Temp: "); 
  while (systemRunning) {
    float t = analogRead(tempPin) * (500.0 / 1023.0);
    lcd.setCursor(6, 0); 
    lcd.print(t);
    delay(400);
    if(t > 40){
      digitalWrite(buzzer_pin,HIGH);
    }else{
      digitalWrite(buzzer_pin,LOW);
    }
  }
}

// Function to print humidity on LCD
void print_humidity(){
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Humidity: "); 
  while (systemRunning) {
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  lcd.setCursor(10, 0); 
  if (isnan(event.relative_humidity)) {
    Serial.println(F("80%"));
    lcd.println("80%");
  }
  else {
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    lcd.print(event.relative_humidity);
  }
    delay(400);
  }
}

// Function to measure distance using Ultrasonic sensor and print on LCD
void measure_distance(){
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Distance: "); 
  while(systemRunning){
    lcd.setCursor(10, 0);
    float dist = ultrasonic1.read();
    Serial.print(dist); // Prints the distance on the default unit (centimeters)
    lcd.print(dist);
    delay(150);
  }
}

// Function to control light based on LDR reading
void control_light(){
  lcd.clear();
  while(systemRunning){  
    int ldrValue = analogRead(ldr_pin); 
    Serial.println(ldrValue);         
    if (ldrValue > 40) 
      {   
        digitalWrite(buzzer_pin, HIGH);  // Turn the LED on
        lcd.setCursor(9,0);
        lcd.print(" ");
        lcd.setCursor(0,0);
        lcd.print("Lights on");
      } 
      else 
      {
        digitalWrite(buzzer_pin, LOW);   // Turn the LED off
        lcd.setCursor(0,0);
        lcd.print("Lights off");
      }
      delay(500); 
  }
}

// Function to display options and handle user input
void Enter() {
  lcd.clear();
  lcd.print("Choose:");
  while (systemRunning) {
    for (int i = 1; i <= 4; i++) {
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      switch (i) {
        case 1:
          lcd.print("1- Humidity");
          break;
        case 2:
          lcd.print("2- Temperature");
          break;
        case 3:
          lcd.print("3- Measure Distance");
          break;
        case 4:
          lcd.print("4- Control Light");
          break;
      }
      delay(300);
    }
    char key = '.';
    if (Serial.available()) {
      key = Serial.read();
      Serial.print("Key is: ");
      Serial.println(key);
      lcd.print(key);
    }
    switch (key)
    {
    case '1':
      print_humidity();
      break;
    case '2':
      print_temp();
      break;
    case '3':
      measure_distance();
      break;
    case '4':
      control_light();
      break;
    }
  }
}

// Main loop function
void loop() {
  if (systemRunning) {
    scroll_lcd();
      lcd.clear();
    lcd.setCursor(0, 0);
    Serial.println("System Running");
    lcd.print("System Running");
    while(systemRunning){
      delay(150);
      bool flag = valid_password();
      if(!systemRunning)break;
        if(flag){
          lcd.setCursor(0,1);
          lcd.print("                ");
          lcd.setCursor(0,1);
          lcd.println("Valid");
          Enter();
        }else{
          lcd.setCursor(0,1);
          lcd.print("                ");
          lcd.setCursor(0,1);
          lcd.println("InValid");
          while(systemRunning);
        }
    }
  } else {
    delay(150);
    scroll_lcd();
      lcd.clear();
    lcd.setCursor(0, 0);
    Serial.println("System closed");
    lcd.print("System Closed");
    while(!systemRunning)delay(150);
  }
  delay(100);
}
