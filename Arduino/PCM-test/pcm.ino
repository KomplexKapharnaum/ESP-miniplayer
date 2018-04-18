#include <Wire.h>

void pcm_setup() {
  Wire.begin(2, 4);

  pcm_set(37, 0x01);  // disable clock autoset
  pcm_set(13, 0x10);  // set clock source to BCK

  // Configure PLL
  pcm_set(20, 0x00);
  pcm_set(21, 0x20);
  pcm_set(22, 0x00);
  pcm_set(23, 0x00);
  pcm_set(24, 0x01);

  // Clock dividers
  pcm_set(27, 0x01);
  pcm_set(28, 0x0F);
  pcm_set(29, 0x03);
  pcm_set(30, 0x07);
}

byte pcm_get(byte registr) {
  //Serial.println("--");
  
  Wire.beginTransmission(0x4C);
  Wire.write(registr);
  Wire.endTransmission();

  Serial.print("PCM asks: 0x");
  if (registr < 16) Serial.print("0");
  Serial.println(String(registr, HEX));

  int n = Wire.requestFrom( 0x4C, 1);
  byte data = Wire.read();

  Serial.print("PCM says: 0x");
  if (data < 16) Serial.print("0");
  Serial.println(String(data, HEX));

  return data;
}

void pcm_set(byte registr, byte value) {
  //Serial.println("--");

  Wire.beginTransmission(0x4C);
  Wire.write(registr);
  Wire.write(value);
  Wire.endTransmission();

  Serial.print("PCM set: 0x");
  if (registr < 16) Serial.print("0");
  Serial.print(String(registr, HEX));
  Serial.print(" to 0x");
  if (value < 16) Serial.print("0");
  Serial.println(String(value, HEX));
}

