#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <EEPROM.h>

#define EEPROM_ADDRESS 0
#define BUTTON_PIN 2
#define LED_PIN 3
#define HALL_SENSOR_PIN 4
#define BLINK_INTERVAL 250

// hand position
#define BUTTON_TRIGGER 5
#define LED_UP 7
#define LED_CENTER 8
#define LED_DOWN 12
#define RIBBON_SENSOR A0
#define RIBBON_CENTRAL_VALUE 700
#define RIBBON_TOLERANCE 0.1

// hand vibration
#define VIBRATOR 6
#define HAPTIC_FEEDBACK true

// hand orientation
#define LED_LEFT 10
#define LED_STRAIGHT 11
#define LED_RIGHT 9

// constants
#define BNO055_SAMPLERATE_DELAY_MS 100
#define DEBUG_BUTTON_DEBOUNCE_DELAY 500

// BNO055 sensor

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

// debugButton
unsigned long debugButtonLastDebounceTime = 0;

// IMU calibration status
bool isIMUCalibrated = false;

// blinkLedFastNonBlocking variables
static unsigned long blinkLedFastNonBlockingPreviousMillis = 0;

// clubMovement thresholds
const float baseYThreshold = 9.0; // Base position (club down)
const float upZThreshold = -9.0;  // Club raised (up)
const float tolerance = 2.0;      // Allow ±2 variation

// Initial orientation reference
float initialYaw = 0.0; // Initial yaw (heading) reference
float initialPitch = 0.0;
float initialRoll = 0.0;
bool isYawReferenceSet = false; // Flag to ensure it's set only once

bool hallInterrupt = false;

void hitDetected()
{
  if(digitalRead(BUTTON_TRIGGER) == LOW)
    hallInterrupt = true;
}

void detectClubMovement(float accelY, float accelZ)
{
  // State variables
  static bool isUp = false; // Tracks if the club is currently up

  // Check if club is in base position
  bool atBase = (accelY >= (baseYThreshold - tolerance) && accelY <= (baseYThreshold + tolerance));
  // Check if club is in up position
  bool atUp = (accelZ >= (upZThreshold - tolerance) && accelZ <= (upZThreshold + tolerance));

  // Determine motion
  if (!isUp && atUp)
  {
    // Club is being raised
    Serial.println("UP 1.0");
    isUp = true; // Update state
  }
  else if (isUp && atBase)
  {
    // Club is going down
    Serial.println("DOWN 1.0");
    isUp = false; // Update state
  }
}

void detectBallHit(Adafruit_BNO055 bno)
{

  Serial.println("HIT 1.0");
  sendData(bno);

  hallInterrupt = false;

  // feedback about roll X
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  // here vibration motor (or managed by message via UART?)
  if (HAPTIC_FEEDBACK)
  {
    analogWrite(VIBRATOR, 200);
    delay(200);
    analogWrite(VIBRATOR, 0);
  }

  float roll = euler.x() - initialRoll;
  roll = roll > 180 ? roll - 360 : (roll < -180 ? roll + 360 : roll);
  Serial.print("ROLL: ");
  Serial.println(roll);
  if (roll > 7.0) // Right threshold
  {
    digitalWrite(LED_RIGHT, HIGH);
    delay(200);
    digitalWrite(LED_RIGHT, LOW);
    delay(200);
    digitalWrite(LED_RIGHT, HIGH);
    delay(200);
    digitalWrite(LED_RIGHT, LOW);
    delay(200);
    digitalWrite(LED_RIGHT, HIGH);
    delay(200);
    digitalWrite(LED_RIGHT, LOW);
  }
  else if (roll < -7.0) // Left threshold
  {
    digitalWrite(LED_LEFT, HIGH);
    delay(1000);
    digitalWrite(LED_LEFT, LOW);
    delay(200);
    digitalWrite(LED_LEFT, HIGH);
    delay(200);
    digitalWrite(LED_LEFT, LOW);
    delay(200);
    digitalWrite(LED_LEFT, HIGH);
    delay(200);
    digitalWrite(LED_LEFT, LOW);
  }
  else
  {
    digitalWrite(LED_STRAIGHT, HIGH);
    delay(200);
    digitalWrite(LED_STRAIGHT, LOW);
    delay(200);
    digitalWrite(LED_STRAIGHT, HIGH);
    delay(200);
    digitalWrite(LED_STRAIGHT, LOW);
    delay(200);
    digitalWrite(LED_STRAIGHT, HIGH);
    delay(200);
    digitalWrite(LED_STRAIGHT, LOW);
  }

  delay(2000);
}

void recalibrateYawReference()
{
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  initialRoll = euler.x();
  initialPitch = euler.y();
  initialYaw = euler.z(); // Reset initial yaw
  Serial.print("Reference Recalibrated: ");
  Serial.print(initialRoll);
  Serial.print(initialPitch);
  Serial.println(initialYaw);
}

// Function to save calibration data to EEPROM
void saveCalibration()
{
  uint8_t calibrationData[22];           // Calibration data size is 22 bytes
  bno.getSensorOffsets(calibrationData); // Get offsets
  for (int i = 0; i < 22; i++)
  {
    EEPROM.write(EEPROM_ADDRESS + i, calibrationData[i]); // Write each byte
  }
  EEPROM.write(EEPROM_ADDRESS + 22, 1); // Write a flag to indicate data saved
  Serial.println("Calibration Saved");
}

// Function to load calibration data from EEPROM
void loadCalibration()
{
  uint8_t calibrationData[22];
  for (int i = 0; i < 22; i++)
  {
    calibrationData[i] = EEPROM.read(EEPROM_ADDRESS + i); // Read each byte
  }
  bno.setSensorOffsets(calibrationData); // Apply offsets
  Serial.println("Calibration Loaded");
}

// Function to check if calibration is saved in EEPROM
bool checkCalibrationSaved()
{
  return EEPROM.read(EEPROM_ADDRESS + 22) == 1; // Check flag byte
}

// Function to calibrate IMU
void IMUcalibration(Adafruit_BNO055 bno)
{
  while (!isIMUCalibrated) // Calibrate IMU
  {
    uint8_t system, gyro, accel, mag = 0;
    bno.getCalibration(&system, &gyro, &accel, &mag);

    printCalibration(system, gyro, accel, mag);

    if (system == 3 && gyro == 3 && accel == 3 && mag == 3)
    {
      saveCalibration();

      // Capture initial yaw orientation using magnetometer
      imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
      initialRoll = euler.x();
      initialPitch = euler.y();
      initialYaw = euler.z(); // 'x' corresponds to yaw in VECTOR_EULER mode
      isYawReferenceSet = true;

      Serial.print("Initial Yaw: ");
      Serial.println(initialYaw);

      isIMUCalibrated = true;
      digitalWrite(LED_PIN, HIGH); // Set LED stable
    }
  }
}

// Function to clear EEPROM
void clearEEPROM()
{
  // Loop through all EEPROM addresses
  for (int i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, 0); // Write 0 to each address
  }
  Serial.println("EEPROM cleared!");
}

void printCalibration(uint8_t system, uint8_t gyro, uint8_t accel, uint8_t mag)
{
  Serial.print("Calibration: Sys=");
  Serial.print(system);
  Serial.print(" Gyro=");
  Serial.print(gyro);
  Serial.print(" Accel=");
  Serial.print(accel);
  Serial.print(" Mag=");
  Serial.println(mag);
}

void printEvent(sensors_event_t *event)
{
  double x = -1000000, y = -1000000, z = -1000000; // dumb values, easy to spot problem
  if (event->type == SENSOR_TYPE_ACCELEROMETER)
  {
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_ORIENTATION)
  {
    Serial.print("Orient:");
    x = event->orientation.x;
    y = event->orientation.y;
    z = event->orientation.z;
  }
  else if (event->type == SENSOR_TYPE_MAGNETIC_FIELD)
  {
    Serial.print("Mag:");
    x = event->magnetic.x;
    y = event->magnetic.y;
    z = event->magnetic.z;
  }
  else if (event->type == SENSOR_TYPE_GYROSCOPE)
  {
    Serial.print("Gyro:");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_ROTATION_VECTOR)
  {
    Serial.print("Rot:");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_LINEAR_ACCELERATION)
  {
    Serial.print("Linear:");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_GRAVITY)
  {
    Serial.print("Gravity:");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else
  {
    Serial.print("Unk:");
  }

  Serial.print("\tx= ");
  Serial.print(x);
  Serial.print(" |\ty= ");
  Serial.print(y);
  Serial.print(" |\tz= ");
  Serial.println(z);
}

void sendData(Adafruit_BNO055 bno)
{
  // Fetch orientation data
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  float roll = euler.x() - initialRoll;   // Roll (angle of rotation about x-axis)
  float pitch = euler.y() - initialPitch; // Pitch (angle of rotation about y-axis)
  float yaw = euler.z() - initialYaw;     // Yaw (angle of rotation about z-axis)

  // Fetch acceleration data
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  float ax = accel.x(); // Acceleration in x-axis
  float ay = accel.y(); // Acceleration in y-axis
  float az = accel.z(); // Acceleration in z-axis

  // Output results
  Serial.print("Roll ");
  Serial.println(roll);
  Serial.print("Pitch ");
  Serial.println(pitch);
  Serial.print("Yaw ");
  Serial.println(yaw);
  Serial.print("Accel ");
  Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.println(az);
}

void setup(void)
{
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Button
  pinMode(LED_PIN, OUTPUT);          // Led
  pinMode(HALL_SENSOR_PIN, INPUT);   // Hall sensor

  // high club part
  pinMode(BUTTON_TRIGGER, INPUT_PULLUP);
  pinMode(RIBBON_SENSOR, INPUT);
  pinMode(LED_UP, OUTPUT);
  pinMode(LED_CENTER, OUTPUT);
  pinMode(LED_DOWN, OUTPUT);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_STRAIGHT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
  pinMode(VIBRATOR, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(HALL_SENSOR_PIN), hitDetected, LOW);

  Serial.begin(115200);

  while (!Serial)
    delay(10); // wait for serial port to open!

  // Initialize BNO055 sensor
  if (!bno.begin())
  {
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1)
      ;
  }

  bno.setExtCrystalUse(true);       // Use external crystal for better accuracy
  bno.setMode(OPERATION_MODE_NDOF); // Set sensor mode to NDOF

  // To clear EEPROM uncomment the following line
  // clearEEPROM();

  // Load calibration from EEPROM
  if (checkCalibrationSaved())
  {
    loadCalibration();
    isIMUCalibrated = true;
  }
  else
  {
    isIMUCalibrated = false;
    Serial.println("No saved calibration found.");
  }

  delay(1000);
  Serial.println("Ready!");
}

void loop(void)
{
  // Working Status LED
  unsigned long currentMillis = millis();
  if (currentMillis - blinkLedFastNonBlockingPreviousMillis >= BLINK_INTERVAL)
  {
    blinkLedFastNonBlockingPreviousMillis = currentMillis;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  if (!isIMUCalibrated)
  {
    IMUcalibration(bno);
  }

  sendData(bno);

  // Button to recalibrate reference
  if (digitalRead(BUTTON_PIN) == LOW && (millis() - debugButtonLastDebounceTime > 500))
  {
    debugButtonLastDebounceTime = millis();
    recalibrateYawReference();
  }

  // Process sensor events
  sensors_event_t accelerometerData;
  bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);

  // Extract Y and Z axes from accelerometer
  float accelY = accelerometerData.acceleration.y;
  float accelZ = accelerometerData.acceleration.z;

  // Detect movement
  detectClubMovement(accelY, accelZ);

  // Hit the ball
  if (hallInterrupt)
  {
    detectBallHit(bno);
  }
  Serial.println("HIT 0");

  // high club part
  if (digitalRead(BUTTON_TRIGGER) == HIGH)
  {
    float ribbon_v = analogRead(RIBBON_SENSOR) - RIBBON_CENTRAL_VALUE;
    if (ribbon_v > RIBBON_CENTRAL_VALUE * RIBBON_TOLERANCE)
    {
      digitalWrite(LED_UP, HIGH);
      digitalWrite(LED_CENTER, LOW);
      digitalWrite(LED_DOWN, LOW);
    }
    else if (ribbon_v < -RIBBON_CENTRAL_VALUE * RIBBON_TOLERANCE)
    {

      digitalWrite(LED_UP, LOW);
      digitalWrite(LED_CENTER, LOW);
      digitalWrite(LED_DOWN, HIGH);
    }
    else
    {
      digitalWrite(LED_UP, LOW);
      digitalWrite(LED_CENTER, HIGH);
      digitalWrite(LED_DOWN, LOW);
    }
  }
  else
  {
    digitalWrite(LED_UP, LOW);
    digitalWrite(LED_CENTER, LOW);
    digitalWrite(LED_DOWN, LOW);
  }

  if (HAPTIC_FEEDBACK)
  {
    imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    float yaw = euler.z() - initialYaw; // Yaw (angle of rotation about z-axis)

    if (yaw < -100)
    {
      analogWrite(VIBRATOR, 200);
      delay(100);
      analogWrite(VIBRATOR, 0);
      delay(100);
    }
    else
    {
      analogWrite(VIBRATOR, 0);
    }
  }

  delay(BNO055_SAMPLERATE_DELAY_MS);
}
