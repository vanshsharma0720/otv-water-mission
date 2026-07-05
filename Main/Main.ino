/*
 * ENES100 - Water Mission (MS6)
 * Team: Tidal Terps
 * Board: Arduino Mega 2560
 *
 * Tasks:
 * 1. Detect water type (color sensor)
 * 2. Measure water depth (ultrasonic)
 * 3. Collect sample (servo + pump)
 */

#include <Arduino.h>
#include "Enes100.h"
#include <Servo.h>

// -------------------- Team Config --------------------
const char* TEAM_NAME = "Tidal Terps";
const byte TEAM_TYPE = WATER;
const int MARKER_ID = 7;
const int ROOM_NUMBER = 1116;

// -------------------- Pins --------------------

// WiFi (Mega Serial1)
const int WIFI_TX_PIN = 62;
const int WIFI_RX_PIN = 63;

// Motors
const int M1_A = 11;
const int M1_B = 12;
const int M2_A = 3;
const int M2_B = 4;
const int M3_A = 5;
const int M3_B = 7;
const int M4_A = 8;
const int M4_B = 9;

// Color sensor (TCS3200)
const int S0 = 28;
const int S1 = 29;
const int S2 = 30;
const int S3 = 31;
const int OUT = 32;

// Ultrasonic
const int US_F_TRIG = 24;
const int US_F_ECHO = 25;
const int US_B_TRIG = 26;
const int US_B_ECHO = 27;

// Mechanism
const int SERVO_PIN = 44;
const int PUMP_PIN = 46;

// -------------------- Setup --------------------

void setup() {
  Serial.begin(9600);
  delay(1000);

  Enes100.println("Starting system...");

  setupMotors();

  Enes100.begin(TEAM_NAME, TEAM_TYPE, MARKER_ID, ROOM_NUMBER,
                WIFI_TX_PIN, WIFI_RX_PIN);

  setupColorSensor();
  setupDepthSensor();

  pinMode(US_F_TRIG, OUTPUT);
  pinMode(US_F_ECHO, INPUT);

  setupSampleSystem();

  Enes100.println("Ready");
}

// -------------------- Motors --------------------

void setupMotors() {
  pinMode(M1_A, OUTPUT); pinMode(M1_B, OUTPUT);
  pinMode(M2_A, OUTPUT); pinMode(M2_B, OUTPUT);
  pinMode(M3_A, OUTPUT); pinMode(M3_B, OUTPUT);
  pinMode(M4_A, OUTPUT); pinMode(M4_B, OUTPUT);
}

void setMotors(int m1, int m2, int m3, int m4) {

  analogWrite(M1_A, max(m1, 0));
  analogWrite(M1_B, max(-m1, 0));

  analogWrite(M2_A, max(m2, 0));
  analogWrite(M2_B, max(-m2, 0));

  analogWrite(M3_A, max(m3, 0));
  analogWrite(M3_B, max(-m3, 0));

  analogWrite(M4_A, max(m4, 0));
  analogWrite(M4_B, max(-m4, 0));
}

void stopMotors() {
  setMotors(0, 0, 0, 0);
}

// -------------------- Movement --------------------

void forward(int spd) {
  setMotors(spd, spd, spd, spd);
  delay(200);
}

void backward(int spd) {
  setMotors(-spd, -spd, -spd, -spd);
  delay(200);
}

void left(int spd) {
  setMotors(-spd, spd, -spd, spd);
  delay(200);
}

void right(int spd) {
  setMotors(spd, -spd, spd, -spd);
  delay(200);
}

// -------------------- Color Sensor --------------------

void setupColorSensor() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
}

long readColor(char c) {

  if (c == 'R') { digitalWrite(S2, LOW);  digitalWrite(S3, LOW); }
  if (c == 'G') { digitalWrite(S2, HIGH); digitalWrite(S3, HIGH); }
  if (c == 'B') { digitalWrite(S2, LOW);  digitalWrite(S3, HIGH); }

  delay(80);
  return pulseIn(OUT, LOW, 50000);
}

bool isPolluted() {

  long r = readColor('R');
  long g = readColor('G');
  long b = readColor('B');

  return (g < r * 0.7 && g < b * 0.7);
}

void mission1() {

  stopMotors();
  delay(300);

  bool polluted = isPolluted();

  if (polluted) {
    Enes100.mission(WATER_TYPE, FRESH_POLLUTED);
  } else {
    Enes100.mission(WATER_TYPE, FRESH_UNPOLLUTED);
  }
}

// -------------------- Depth --------------------

void setupDepthSensor() {
  pinMode(US_B_TRIG, OUTPUT);
  pinMode(US_B_ECHO, INPUT);
}

long measure(int trig, int echo) {

  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long t = pulseIn(echo, HIGH, 30000);
  if (!t) return -1;

  return (t * 10) / 58;
}

int mission2() {

  long dist = measure(US_B_TRIG, US_B_ECHO);
  if (dist < 0) return -1;

  const int base = 150;
  int depth = base - dist;
  if (depth < 0) depth = 0;

  int d20 = abs(depth - 20);
  int d30 = abs(depth - 30);
  int d40 = abs(depth - 40);

  return (d20 < d30 && d20 < d40) ? 20 :
         (d30 < d20 && d30 < d40) ? 30 : 40;
}

// -------------------- Sample System --------------------

Servo arm;

void setupSampleSystem() {
  pinMode(PUMP_PIN, OUTPUT);
  arm.attach(SERVO_PIN);
  arm.write(0);
}

void collectSample() {

  arm.write(90);
  delay(500);

  digitalWrite(PUMP_PIN, HIGH);
  delay(3000);

  digitalWrite(PUMP_PIN, LOW);
  arm.write(0);
}

// -------------------- Navigation --------------------

int stage = 0;
bool done = false;

void navigate() {

  if (!Enes100.isVisible()) {
    stopMotors();
    return;
  }

  float x = Enes100.getX();
  float y = Enes100.getY();

  if (stage == 0) {
    stage = (y < 1.0) ? 1 : 2;
    return;
  }

  if (stage == 1 && y < 1.35) {
    forward(120);
    return;
  }

  if (stage == 2 && y > 0.65) {
    forward(120);
    return;
  }

  if (stage == 3 && !done) {

    mission1();

    int d = mission2();
    Enes100.mission(DEPTH, d);

    collectSample();

    done = true;
    stage = 4;
  }

  stopMotors();
}

// -------------------- Loop --------------------

void loop() {

  navigate();
  delay(50);
}
