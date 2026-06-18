#define BLYNK_TEMPLATE_ID "TMPL6jO6Ave0W"
#define BLYNK_TEMPLATE_NAME "Plant Health Monitoring System"
#define BLYNK_AUTH_TOKEN "MvlqVb6aOrwyb_hS953siXylRkW6uYLm"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <DHT.h>
char ssid[] = "Riyad";
char pass[] = "12345678";
#define DHTPIN 4
#define DHTTYPE DHT11
#define SOIL_PIN 34
#define RELAY_PIN 5
#define SERVO1_PIN 18
#define SERVO2_PIN 19
#define S0 25
#define S1 27
#define S2 26
#define S3 14
#define COLOR_OUT 33
DHT dht(DHTPIN, DHTTYPE);
Servo servo1;
Servo servo2;
const int wetValue = 1200;
const int dryValue = 3100;
#define IR1 15
#define IR2 16
#define IR3 17
#define IR4 21
#define IR5 22
#define IN1 12
#define IN2 13
#define IN3 32
#define IN4 23
#define ENA 2
#define ENB 0
int speedValue = 80;
bool atStation = false;
#define V_SOIL V0
#define V_TEMP V1
#define V_HUM  V2
#define V_LEAF V3
#define V_PUMP V4
void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);
  pinMode(IR5, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  dht.begin();
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(COLOR_OUT, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  setupPWM();
  stopMotors();
  Serial.println("SYSTEM STARTED");
}
void loop() {
  Blynk.run();
  int s1 = digitalRead(IR1);
  int s2 = digitalRead(IR2);
  int s3 = digitalRead(IR3);
  int s4 = digitalRead(IR4);
  int s5 = digitalRead(IR5);
  bool allBlack = (s1==0 && s2==0 && s3==0 && s4==0 && s5==0);
  if (allBlack) {
    if (!atStation) {
      atStation = true;
      stopMotors();
      Serial.println("STATION REACHED");
      runSoilCheck();
      delay(60000);
    }
    return;
  } else { atStation = false; }
  if (s3 == 0) forward();
  else if (s1==0 || s2==0) left();
  else if (s4==0 || s5==0) right();
  else forward();
  delay(5);
}
void runSoilCheck() {
  Serial.println("PLANT CHECK START");
  moveServo(servo1, 120, 70);
  delay(2000);
  moveServo(servo2, 70, 120);
  delay(2000);
  int moisturePercent = readSoilMoisture();
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  String leafStatus = readLeafHealth();
  Blynk.virtualWrite(V_SOIL, moisturePercent);
  Blynk.virtualWrite(V_TEMP, temperature);
  Blynk.virtualWrite(V_HUM, humidity);
  Blynk.virtualWrite(V_LEAF, leafStatus);
  if (moisturePercent < 25) {
    Serial.println("WATERING");
    digitalWrite(RELAY_PIN, LOW);
    Blynk.virtualWrite(V_PUMP, 1);
    delay(5000);
    digitalWrite(RELAY_PIN, HIGH);
    Blynk.virtualWrite(V_PUMP, 0);
  } else { Blynk.virtualWrite(V_PUMP, 0); }
  moveServo(servo2, 120, 70);
  moveServo(servo1, 70, 120);
}
void setupPWM() {
  ledcAttach(ENA, 1000, 8);
  ledcAttach(ENB, 1000, 8);
  ledcWrite(ENA, speedValue);
  ledcWrite(ENB, speedValue);
}
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
void left() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
void right() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
int readSoilMoisture() {
  int value = analogRead(SOIL_PIN);
  int moisture = map(value, dryValue, wetValue, 0, 100);
  return constrain(moisture, 0, 100);
}
String readLeafHealth() {
  int r, g, b;
  digitalWrite(S2, LOW); digitalWrite(S3, LOW);
  r = pulseIn(COLOR_OUT, LOW);
  digitalWrite(S2, HIGH); digitalWrite(S3, HIGH);
  g = pulseIn(COLOR_OUT, LOW);
  digitalWrite(S2, LOW); digitalWrite(S3, HIGH);
  b = pulseIn(COLOR_OUT, LOW);
  if (r > 500 && g > 500 && b > 500) return "NO_LEAF";
  if (g < r && g < b) return "HEALTHY";
  if (abs(r-g) < 40 && b > r && b > g) return "UNHEALTHY";
  return "UNKNOWN";
}
void moveServo(Servo &servo, int startAngle, int endAngle) {
  if (startAngle < endAngle)
    for (int i=startAngle; i<=endAngle; i++) { servo.write(i); delay(20); }
  else
    for (int i=startAngle; i>=endAngle; i--) { servo.write(i); delay(20); }
}