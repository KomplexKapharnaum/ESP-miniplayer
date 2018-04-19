
const unsigned long BATTERY_CHECK_PERIOD_MS = 10000;
const unsigned long BUTTON_CHECK_PERIOD_MS = 200;

int battery = 0;
bool started = false;

void stm32_setup() {
  Serial.begin(115200, SERIAL_8N1);
  Serial.setTimeout(10);
  started = true;
}


void stm32_loop() {
  if (!started) return;
  static unsigned long lastBatteryCheck, lastButtonCheck;
  if (millis() - lastBatteryCheck > BATTERY_CHECK_PERIOD_MS)
  {
    lastBatteryCheck = millis();
    //stm32_sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_VOLTAGE);
    //stm32_readSerialAnswer();

    stm32_sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_PERCENTAGE);
    battery = stm32_readSerialAnswer();
    LOG("Battery: "+String(battery)+"%");
  }

  if (millis() - lastButtonCheck > BUTTON_CHECK_PERIOD_MS)
  {
    lastButtonCheck = millis();
    stm32_sendSerialCommand(KXKM_STM32_Energy::GET_BUTTON_EVENT);
    int event = stm32_readSerialAnswer();
    
    if (event == KXKM_STM32_Energy::BUTTON_DOUBLE_CLICK_EVENT)
    {
      LOG("BTN click");
      
    }
    else if (event == KXKM_STM32_Energy::BUTTON_CLICK_EVENT)
    {
      LOG("BTN double-click");
    }
  }
}

byte stm32_batteryLevel() {
  return battery;
}

void stm32_reset() {
  stm32_sendSerialCommand(KXKM_STM32_Energy::REQUEST_RESET);
  delay(1000);
  ESP.restart();
}


/* Send commands / receive answers */

void stm32_sendSerialCommand(KXKM_STM32_Energy::CommandType cmd, int arg)
{
  stm32_flushSerialIn();

  Serial.write(KXKM_STM32_Energy::PREAMBLE);
  Serial.write(cmd);
  Serial.write(' ');
  Serial.println(arg);
}

void stm32_sendSerialCommand(KXKM_STM32_Energy::CommandType cmd)
{
  stm32_flushSerialIn();

  Serial.write(KXKM_STM32_Energy::PREAMBLE);
  Serial.write(cmd);
  Serial.println("");
}

void stm32_setLeds(uint8_t *values)
{
  int arg = 0;
  for (int i = 0; i < 6; i++)
    arg += values[i] * pow(10, i);

  stm32_sendSerialCommand(KXKM_STM32_Energy::SET_LEDS, arg);
}

long stm32_readSerialAnswer()
{
  if (Serial.find(KXKM_STM32_Energy::PREAMBLE))
  {
    long arg = Serial.parseInt();
    return arg;
  }
  return 0;
}

void stm32_flushSerialIn()
{
  while (Serial.available())
    Serial.read();
}


