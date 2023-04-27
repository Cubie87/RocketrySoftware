// include all files required 
// todo: move these to src dir bypassing dependencies
#include <MPU9250_WE.h>
#include <Wire.h>
#define MPU9250_ADDR 0x68

#include <SD.h>
#include <SPI.h>
File myFile;

#include <Adafruit_H3LIS331.h>
#include <Adafruit_Sensor.h>
Adafruit_H3LIS331 lis = Adafruit_H3LIS331();

#include <InternalTemperature.h>

#include <Adafruit_LPS2X.h>


// declare led blink functions
void ledBlink(int LED);
void accelError(int LED);
void baroError(int LED);
void sdError(int LED);




// declare variables as needed
Adafruit_LPS22 lps;
Adafruit_Sensor *lps_temp, *lps_pressure;

// LED selection (13 for Teensy)
int LED = 13;

char csvFilename[32] = {0};
File csvLogFile;


MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR);

void setup(){
  // start serial bus (will not halt code if serial is not connected)
  Serial.begin(115200);

  // start I2C bus
  Wire.begin();

  if(!myMPU9250.init()){ // initialise low G accelerometer
    Serial.println("MPU9250 does not respond");
    accelError(LED);
  }
  Serial.println("MPU9250 is connected");
  if(!myMPU9250.initMagnetometer()){
    Serial.println("Magnetometer does not respond");
  }
  Serial.println("Magnetometer is connected");

  Serial.println("Position you MPU9250 flat and don't move it - calibrating...");
  delay(1000);
  // accelerometer setup and calibration
  myMPU9250.autoOffsets();
  myMPU9250.enableGyrDLPF();
  myMPU9250.setGyrDLPF(MPU9250_DLPF_6); // 20hz sampling limit
  myMPU9250.setSampleRateDivider(5);
  myMPU9250.setGyrRange(MPU9250_GYRO_RANGE_2000);
  myMPU9250.setAccRange(MPU9250_ACC_RANGE_16G);
  myMPU9250.enableAccDLPF(true);
  myMPU9250.setAccDLPF(MPU9250_DLPF_6);
  myMPU9250.setMagOpMode(AK8963_CONT_MODE_100HZ);



  if(!lps.begin_I2C()){ // initialise barometer
    Serial.println("Failed to find LPS2X chip");
    baroError(LED);
  }
  lis.setDataRate(LIS331_DATARATE_100_HZ);

  
  if(!lis.begin_I2C()){   // initialise high g accelerometer
    Serial.println("Couldnt start");
    while (1) yield();
  }
  lis.setRange(H3LIS331_RANGE_100_G);   // 100, 200, or 400 G!

  
  if(!SD.begin(BUILTIN_SDCARD)){ // initialise SD card
    Serial.println("ERROR WITH SD CARD");
    sdError(LED);
  }
  // determine a suitable sd card filename.
  // the oldest file is 0, the youngest file is the highest number
  int i = 0;
  sprintf(csvFilename, "%d.csv", i);
  while(SD.exists(csvFilename)){
    i++;
    sprintf(csvFilename, "%d.csv", i);
  }
  // touch file
  csvLogFile = SD.open(csvFilename, FILE_WRITE);
  // write some basic file headers to the file.
  csvLogFile.println("Millis, Amb Temp(C), Pressure (hPa), Hx, Hy, Hz, Lx, Ly, Lz, gyrX, gyrY, gyrZ, Tx, Ty, Tz, CPU Temp(C)");
  csvLogFile.close(); // remember to close the file and flush changes!

  // serial debugging. Possibility to turn this into telemetry in the future
  Serial.println("SD CARD Preamble written.");
  Serial.print("DATALOG FILENAME:");
  Serial.println(csvFilename);
  


  
  Serial.println("Done!");
  delay(200);
}

void loop(){
  // blink LED as required. This is non-blocking
  ledBlink(LED);
  

  // read low G accelerometer values for acceleration, gyration, and magnetometer
  xyzFloat gValue = myMPU9250.getGValues();
  xyzFloat gyr = myMPU9250.getGyrValues();
  xyzFloat magValue = myMPU9250.getMagValues();

  // read high G accelerometer values
  sensors_event_t event;
  lis.getEvent(&event);

  // read barometer for temperature and pressure
  sensors_event_t temp;
  sensors_event_t pressure;
  lps.getEvent(&pressure, &temp);



  // write to SD card
  char dataLine[240] = {0}; //  milli,temp, pres, hx,   hy,   hz    lx,   ly,   lz    gyrX, gyrY, gyrZ, Tx,   Ty,   Tz,  CPU Temp(C)         temp,             pres               hx,                   hy,                   hz,                   lowX             lowY             lowZ             
  sprintf(dataLine,             "%ld, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f X", millis(), temp.temperature, pressure.pressure, event.acceleration.x, event.acceleration.y, event.acceleration.z, gValue.x * -9.81, gValue.y * -9.81, gValue.z * 9.81, gyr.x, gyr.y, gyr.z, magValue.x, magValue.y, magValue.z, InternalTemperature.readTemperatureC());
  Serial.println(dataLine);
  csvLogFile = SD.open(csvFilename, FILE_WRITE);
  csvLogFile.println(dataLine);
  csvLogFile.close();

  delay(50);
}








void ledBlink(int LED){
    // duty cycle % power off time
    if(millis() % 1000 > 800){
        digitalWrite(LED, HIGH);
    } else{
        digitalWrite(LED, LOW);
    }
}



// module error handling

void accelError(int LED){
    while(1){
        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
        delay(100);

        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
        delay(100);

        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
        delay(1000);
    }
}
void baroError(int LED){
    while(1){
        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
        delay(100);

        digitalWrite(LED, HIGH);
        delay(600);
        digitalWrite(LED, LOW);
        delay(100);

        digitalWrite(LED, HIGH);
        delay(600);
        digitalWrite(LED, LOW);
        delay(1000);
    }
}
void sdError(int LED){
    while(1){
        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
        delay(100);

        digitalWrite(LED, HIGH);
        delay(600);
        digitalWrite(LED, LOW);
        delay(100);

        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
        delay(1000);
    }
}
