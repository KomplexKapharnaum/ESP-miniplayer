/* Send commands / receive answers */

void sendSerialCommand(KXKM_STM32_Energy::CommandType cmd, int arg)
{
  flushSerialIn();

  Serial.write(KXKM_STM32_Energy::PREAMBLE);
  Serial.write(cmd);
  Serial.write(' ');
  Serial.println(arg);
}

void sendSerialCommand(KXKM_STM32_Energy::CommandType cmd)
{
  flushSerialIn();

  Serial.write(KXKM_STM32_Energy::PREAMBLE);
  Serial.write(cmd);
  Serial.println("");
}

void setLeds(uint8_t *values)
{
  int arg = 0;
  for (int i = 0; i < 6; i++)
    arg += values[i] * pow(10, i);

  sendSerialCommand(KXKM_STM32_Energy::SET_LEDS, arg);
}

long readSerialAnswer()
{
  if (Serial.find(KXKM_STM32_Energy::PREAMBLE))
  {
    long arg = Serial.parseInt();
    return arg;
  }
  return 0;
}

void flushSerialIn()
{
  while (Serial.available())
    Serial.read();
}
