#include <Adafruit_MotorShield.h>
#include <stdlib.h>
#include <Wire.h>
#include <SparkFun_I2C_Mux_Arduino_Library.h>
#include <Adafruit_LPS35HW.h>

// Interval to print out pressure sensor values
#define PRESSURE_INTERVAL 1000

// Interval to run pump cycle
#define PUMP_INTERVAL 10000

// Max pressure read by sensors 
#define PRESSURE_MAX 1260
// Max pressure read by sensors 
//#define PRESSURE_MAX 1055.892

// Pressure to inflate all pockets to in their cycle
#define UPPER_WORKING_PRESSURE 1200

// Pressure to start inflating again
#define PRESSURE_MIN 1015

// Length of time to flash emergency LED
#define WARNING_FLASH_TIME 5000

// Number of pocket systems
#define NUM_POCKETS 4

// LED pin on MS2
const int ledPin = 9;

// Last time the pressure read was run
unsigned long previousPressureTime = 0;

// Keeps track of if a pressure max is exceeded
bool emergencyStop = false;

int currentPocket = 0;

// Instance of the mux
QWIICMUX mux;

// Instance of the pressure sensor
Adafruit_LPS35HW ps;

// Create the motor shield object
Adafruit_MotorShield MS1 = Adafruit_MotorShield(0x60);
Adafruit_MotorShield MS2 = Adafruit_MotorShield(0x61);

enum PocketState {
  EMPTY,
  FILLING,
  HOLDING,
};

struct Pocket {
  Adafruit_DCMotor *pump;
  Adafruit_DCMotor *valve;
  uint8_t sensor;
  PocketState state;
  unsigned long startTime;
};

// Array of pointers to each air pump object
Pocket pockets[NUM_POCKETS];

/**
  * Initializes communication between mux and motor drivers.
  * Initializes pressure sensors, air pump speed to max, air valve state to open
  */
void setup() {
  Serial.begin(9600);
  Wire.begin();

  delay(1000);

  // Start multiplexer
  while (!mux.begin(0x71)) {
    Serial.println("MUX not found.");
  }

  // Start motor shield
  if (!MS1.begin()) {
    Serial.println("Motor Shield 1 not found");
    while (1);
  }

  if (!MS2.begin()) {
    Serial.println("Motor Shield 2 not found");
    while (1);
  }

  // Initalize pocket data
  pockets[0] = {MS1.getMotor(2), MS1.getMotor(1), 3, EMPTY, 0};
  pockets[1] = {MS1.getMotor(3), MS1.getMotor(4), 7, EMPTY, 0};
  pockets[2] = {MS2.getMotor(2), MS2.getMotor(1), 4, EMPTY, 0};
  pockets[3] = {MS2.getMotor(3), MS2.getMotor(4), 0, EMPTY, 0};

  // Set LED pin as output
  pinMode(ledPin, OUTPUT);
  // Set LED to OFF
  digitalWrite(ledPin, LOW);

  // Initialize all values in pockets
  for (int i = 0; i < NUM_POCKETS; i++) {
    mux.setPort(pockets[i].sensor);

    // give hardware time to wait before reading another address
    delay(100);

    if (!ps.begin_I2C()) {
      Serial.print("LPS35HW not found.");
      Serial.println(pockets[i].sensor);
    } 
    else {
      Serial.print("LPS35HW found.");
      Serial.println(pockets[i].sensor);
    }

    // Set pump and valve speed to max
    pockets[i].pump->setSpeed(100);
    pockets[i].valve->setSpeed(255);
  }

}

/**
  * Hold air valve pressure.
  */
void openValve(Pocket &p) {
  p.valve->run(FORWARD); // Opens valve = air flowing into pocket
}

/**
  * Vents air valve.
  */
void closeValve(Pocket &p) {
  p.valve->run(BACKWARD); // Closes valve = air retained in pocket
}

/**
  * Turns on air pump.
  */
void pumpOn(Pocket &p) {
  p.pump->run(FORWARD);
}

/**
  * Turns off air pump.
  */
void pumpOff(Pocket &p) {
  p.pump->run(RELEASE);
}

float readPocketPressure(Pocket &p) {
  mux.setPort(p.sensor);
  delay(10);
  return ps.readPressure();
}

void flashLED() {
  unsigned long startFlash = millis();
  while (millis() - startFlash < WARNING_FLASH_TIME) {
    digitalWrite(ledPin, HIGH); // Turn on LED
    delay(500);

    digitalWrite(ledPin, LOW); // Turn off LED
    delay(500);
  }
}

/**
 * Turns on one pocket for inflate cycle.
 * Inflates pocket to upper pressure threshld and holds for 5 seconds. 
 * Then turns off pump and opens valve.
 */
void updatePocketState(Pocket &p) {
  float pressure = readPocketPressure(p);

  switch (p.state) {
    case EMPTY:
      Serial.print("EMPTY");
      Serial.println(p.sensor);
      openValve(p);
      pumpOn(p);
      p.state = FILLING;
      p.startTime = millis();
      break;

    case FILLING:
      Serial.print("FILLING");
      Serial.println(p.sensor);
      if (pressure >= UPPER_WORKING_PRESSURE) {
        pumpOff(p);
        openValve(p);
        p.state = HOLDING;
        p.startTime = millis();
      }
      break;

    case HOLDING:
      Serial.print("HOLDING");
      pumpOff(p);
      closeValve(p);
      Serial.println(p.sensor);
      if (millis() - p.startTime >= PUMP_INTERVAL) {
        openValve(p);
        p.state = EMPTY;
        p.startTime = millis();
      }
      break;

  }
}

/**
 * Checks if the pressure sensors exceed the pressure max. 
 * 
 * @return true if any pressure is above max
 */
bool maxPressureCheck() {
  for (int i = 0; i < NUM_POCKETS; i++) {

    float pressure = readPocketPressure(pockets[i]);

    Serial.print("Channel ");
    Serial.print(pockets[i].sensor);
    Serial.print(" | Pressure: ");
    Serial.print(pressure);
    Serial.print(" hPa");
    Serial.println();

    if (pressure >= PRESSURE_MAX) {
      Serial.print("Pressure too high on channel ");
      Serial.print(pockets[i].sensor);
      Serial.print(": ");
      Serial.print(pressure);
      Serial.print(" hPa, threshold = ");
      Serial.println(PRESSURE_MAX);
      return true;
    }
  }
  return false;
}

void shutOffAllPockets() {
  for (int i = 0; i < NUM_POCKETS; i++) {
    pumpOff(pockets[i]);
    openValve(pockets[i]);
    pockets[i].state = EMPTY;
  }
}

void emergencyShutoff() {
  Serial.println("Emergency stop triggered.");
  emergencyStop = true;
  shutOffAllPockets();
  flashLED();
}

void loop() {

  unsigned long currentTime = millis();

  // Reads out pressure sensor values every second
  if ((currentTime - previousPressureTime) >= PRESSURE_INTERVAL) {
    previousPressureTime = currentTime;
    if (maxPressureCheck()) {
      emergencyShutoff();
    }
  }

  if (emergencyStop) {
    return;
  }

  updatePocketState(pockets[currentPocket]);
  if (pockets[currentPocket].state == EMPTY) {
    currentPocket = (currentPocket + 1) % NUM_POCKETS;
  }

}