
#define STM32_CHECK_PERIOD 20000      // task loop in ms
#define CHECKTICK_BATTERY 1           // x CHECK_PERIOD = duration in ms

SemaphoreHandle_t stm32_lock;

int battery = 0;
bool stm32_running = false;

void stm32_start() {
  if (stm32_running) return;
  
  Serial.begin(115200, SERIAL_8N1);
  Serial.setTimeout(10);
  
  stm32_lock = xSemaphoreCreateMutex();
  stm32_running = true;

  xTaskCreatePinnedToCore(
                    stm32_task,   
                    "stm32_task",
                    1000,      
                    NULL,       
                    0,      
                    NULL,     
                    0);        
}

void stm32_task( void * parameter ) {

  int tickerBattery = 0;
  int event;

  xSemaphoreTake(stm32_lock, portMAX_DELAY);
  stm32_sendSerialCommand(KXKM_STM32_Energy::SET_LOAD_SWITCH, 1);
  xSemaphoreGive( stm32_lock );
  
  // loop
  while (true) {
    xSemaphoreTake(stm32_lock, portMAX_DELAY);
    if (!stm32_running) {
      xSemaphoreGive( stm32_lock );
      break;
    }
    
    // check Button
    stm32_sendSerialCommand(KXKM_STM32_Energy::GET_BUTTON_EVENT);
    event = stm32_readSerialAnswer();
    if (event == KXKM_STM32_Energy::BUTTON_DOUBLE_CLICK_EVENT) LOG("BTN click");
    else if (event == KXKM_STM32_Energy::BUTTON_CLICK_EVENT) LOG("BTN double-click");

    // check Battery
    tickerBattery -= 1;
    if (tickerBattery <= 0) {
      stm32_sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_PERCENTAGE);
      battery = stm32_readSerialAnswer();
      LOG("Battery: " + String(battery) + "%");
      tickerBattery = CHECKTICK_BATTERY;
    }

    xSemaphoreGive( stm32_lock );
    delay(STM32_CHECK_PERIOD);
  }
  vTaskDelete(NULL);
}

byte stm32_batteryLevel() {
  xSemaphoreTake(stm32_lock, portMAX_DELAY);
  byte batt = battery;
  xSemaphoreGive( stm32_lock );
  return batt;
}

void stm32_reset() {
  xSemaphoreTake(stm32_lock, portMAX_DELAY);
  stm32_sendSerialCommand(KXKM_STM32_Energy::SET_LOAD_SWITCH, 0);
  stm32_sendSerialCommand(KXKM_STM32_Energy::REQUEST_RESET);
  delay(1000);
  Serial.println("STM did not reset, going with soft reset");
  WiFi.disconnect();
  delay(500);
  ESP.restart();
  xSemaphoreGive( stm32_lock );
}

void stm32_shutdown() {
  xSemaphoreTake(stm32_lock, portMAX_DELAY);
  stm32_sendSerialCommand(KXKM_STM32_Energy::SET_LOAD_SWITCH, 0);
  stm32_sendSerialCommand(KXKM_STM32_Energy::SHUTDOWN);
  xSemaphoreGive( stm32_lock );
}


// Send commands / receive answers 

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

void stm32_wait() {
  xSemaphoreTake(stm32_lock, portMAX_DELAY);
  xSemaphoreGive( stm32_lock );
}
