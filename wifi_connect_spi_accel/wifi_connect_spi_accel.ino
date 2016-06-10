/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include <SPI.h>

const char* ssid     = "PPI";
const char* password = "2020@Pr3c1s1on!";
IPAddress host(10,20,0,184);
const int port = 1337;
// Use WiFiClient class to create TCP connections
WiFiClient client;

//Assign the Chip Select signal to pin 10.
int CS=15;

//This is a list of some of the registers available on the ADXL345.
//To learn more about these and the rest of the registers on the ADXL345, read the datasheet!
char POWER_CTL = 0x2D;  //Power Control Register
char DATA_FORMAT = 0x31;
char DATAX0 = 0x32; //X-Axis Data 0
char DATAX1 = 0x33; //X-Axis Data 1
char DATAY0 = 0x34; //Y-Axis Data 0
char DATAY1 = 0x35; //Y-Axis Data 1
char DATAZ0 = 0x36; //Z-Axis Data 0
char DATAZ1 = 0x37; //Z-Axis Data 1

//This buffer will hold values read from the ADXL345 registers.
char values[10];
//These variables will be used to hold the x,y and z axis accelerometer values.
int x,y,z;
float xg,yg,zg;
float xg_avg, yg_avg, zg_avg;
float stability_xg, stability_yg, stability_zg;
int cnt;

int value = 0;
unsigned char cnt_f=0;



void setup() {

  spi_accel_init();

  
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("connecting to server");
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }


}


void loop() {

  if(!client.connected()) {
    // connect again
    Serial.print("Disconnected... connecting to server");
    if (!client.connect(host, port)) {
      Serial.println("connection failed");
      return;
    }
  } else {
    spi_accel_update();
  }
  
  //Serial.print((float)xg,2);
  //Serial.print(',');
  //Serial.print((float)yg,2);
  //Serial.print(',');
  //Serial.println((float)zg,2);      
   
 if(++cnt > 40) {
  cnt = 0;

  Serial.print((float)xg_avg,2);
  Serial.print(',');
  Serial.print((float)yg_avg,2);
  Serial.print(',');
  Serial.println((float)zg_avg,2);   

//  Serial.print((float)stability_xg,2);
//  Serial.print(',');
//  Serial.print((float)stability_yg,2);
//  Serial.print(',');
//  Serial.println((float)stability_zg,2);      

  stability_xg = 0;
  stability_yg = 0;
  stability_zg = 0;

  //send pulse to server
   ++value;



  
  cnt_f++;
  
  // This will send the request to the server
  //client.write((const uint8_t*)&cnt_f,1);

  // form a JSON-formatted string:
    String jsonString = "{\"x\":\"";
    jsonString += xg_avg;
    jsonString +="\",\"y\":\"";
    jsonString += yg_avg;
    jsonString +="\",\"z\":\"";
    jsonString += zg_avg;
    jsonString +="\"}";

    // print it:
  Serial.println(jsonString);
  client.print(jsonString);

  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    //Serial.println("Received data");
    String line = client.readStringUntil('\r');
    //Serial.print(line);
  }
  
  
 }
 delay(10);

}


//This function will write a value to a register on the ADXL345.
//Parameters:
//  char registerAddress - The register to write a value to
//  char value - The value to be written to the specified register.
void writeRegister(char registerAddress, char value){
  //Set Chip Select pin low to signal the beginning of an SPI packet.
  digitalWrite(CS, LOW);
  //Transfer the register address over SPI.
  SPI.transfer(registerAddress);
  //Transfer the desired register value over SPI.
  SPI.transfer(value);
  //Set the Chip Select pin high to signal the end of an SPI packet.
  digitalWrite(CS, HIGH);
}

//This function will read a certain number of registers starting from a specified address and store their values in a buffer.
//Parameters:
//  char registerAddress - The register addresse to start the read sequence from.
//  int numBytes - The number of registers that should be read.
//  char * values - A pointer to a buffer where the results of the operation should be stored.
void readRegister(char registerAddress, int numBytes, char * values){
  //Since we're performing a read operation, the most significant bit of the register address should be set.
  char address = 0x80 | registerAddress;
  //If we're doing a multi-byte read, bit 6 needs to be set as well.
  if(numBytes > 1)address = address | 0x40;
  
  //Set the Chip select pin low to start an SPI packet.
  digitalWrite(CS, LOW);
  //Transfer the starting register address that needs to be read.
  SPI.transfer(address);
  //Continue to read registers until we've read the number specified, storing the results to the input buffer.
  for(int i=0; i<numBytes; i++){
    values[i] = SPI.transfer(0x00);
  }
  //Set the Chips Select pin high to end the SPI packet.
  digitalWrite(CS, HIGH);
}

// twos complement processing used for spi accel processing
float twos_comp(int x) {
  float x_float;
  if(x & 0xF000) {
    x = ~x + 1;
    x_float = ((float) (x & 0xFFFF)) * -1;
  } else {
    x_float = (float) x;
  }
  return(x_float);
}

// ADXL345
void spi_accel_init() {
    //Initiate an SPI communication instance.
  SPI.begin();
  //Configure the SPI connection for the ADXL345.
  SPI.setDataMode(SPI_MODE3);
  
  //Set up the Chip Select pin to be an output from the Arduino.
  pinMode(CS, OUTPUT);
  //Before communication starts, the Chip Select pin needs to be set high.
  digitalWrite(CS, HIGH);
  
  //Put the ADXL345 into +/- 4G range by writing the value 0x01 to the DATA_FORMAT register.
  writeRegister(DATA_FORMAT, 0x01);
  //Put the ADXL345 into Measurement Mode by writing 0x08 to the POWER_CTL register.
  writeRegister(POWER_CTL, 0x08);  //Measurement mode  
}

void spi_accel_update() {
    //Reading 6 bytes of data starting at register DATAX0 will retrieve the x,y and z acceleration values from the ADXL345.
  //The results of the read operation will get stored to the values[] buffer.
  readRegister(DATAX0, 6, values);

  //The ADXL345 gives 10-bit acceleration values, but they are stored as bytes (8-bits). To get the full value, two bytes must be combined for each axis.
  //The X value is stored in values[0] and values[1].
  x = (((int)values[1])<<8)|((int)values[0]);
  //The Y value is stored in values[2] and values[3].
  y = (((int)values[3])<<8)|(int)values[2];
  //The Z value is stored in values[4] and values[5].
  z = (((int)values[5])<<8)|(int)values[4];

//  Serial.print(x);
//  Serial.print(',');
//  Serial.print(y);
//  Serial.print(',');
//  Serial.println(z);     

  float x_float, y_float, z_float;
  
  x_float = twos_comp(x);
  y_float = twos_comp(y);
  z_float = twos_comp(z);

  // calculate in floating point g's
  xg = x_float * 0.0078;
  yg = y_float * 0.0078;
  zg = z_float * 0.0078;

  // do some sort of avg
  xg_avg = 0.955 * xg_avg + xg * .05;
  yg_avg = 0.955 * yg_avg + yg * .05;
  zg_avg = 0.955 * zg_avg + zg * .05;

  stability_xg = stability_xg + abs(xg - xg_avg);
  stability_yg = stability_yg + abs(yg - yg_avg);
  stability_zg = stability_zg + abs(zg - zg_avg);

}



