/*
KXKM - ESP32 audio & battery module
STM32 Energy API test from ESP32

Battery voltage and percentage are queried every 2s.

The following features are tested :
  * setting LEDs independently
  * using the LED gauge to display a percentage
  * enabling / disabling the load switch (if the user push button is pressed at startup the load switch is not enabled automatically)
  * setting custom batt characteristics (12-14V)
  * Starting a critical section with a 8s timeout
    * Leaving the critical section (main push button)
    * requesting a shutdown (user push button)
  * Request a self reset

The main push button is used to cycle through the tests on double clicks (via STM32 serial).
The user push button is used to select between some tests (shutdown / self reset for example)
*/

#include <AceButton.h>
#include "KXKM_STM32_energy_API.h"

const unsigned long BATTERY_CHECK_PERIOD_MS = 2000;
const unsigned long BUTTON_CHECK_PERIOD_MS = 200;

enum test_type_t {
  INIT,
  TEST_LED_1,
  TEST_LED_2,
  TEST_LOAD_SW,
  TEST_CUSTOM_BATT,
  TEST_ENTER_CRITICAL_SECTION,
  TEST_LEAVE_CRITICAL_SECTION,
  TEST_SELF_RESET
} currentTestType;

ace_button::AceButton button(34);

void setup() {
  ace_button::ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleButtonEvent);
  buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureClick);
  pinMode(34, INPUT);

  Serial.begin(115200, SERIAL_8N1);
  Serial.setTimeout(10);

  Serial.println("Beginning Energy API test sketch.");

  currentTestType = INIT;

  sendSerialCommand(KXKM_STM32_Energy::GET_API_VERSION);
  Serial.print("STM32 API version : ");
  Serial.println(readSerialAnswer());
  Serial.print("Local API version : ");
  Serial.println(KXKM_STM32_Energy::API_VERSION);

  sendSerialCommand(KXKM_STM32_Energy::GET_FW_VERSION);
  Serial.print("STM32 firmware version : ");
  Serial.println(readSerialAnswer());

  sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_TYPE);
  Serial.print("Battery type : ");
  switch (readSerialAnswer())
  {
    case KXKM_STM32_Energy::BATTERY_LIPO: Serial.println("LiPo"); break;
    case KXKM_STM32_Energy::BATTERY_LIFE: Serial.println("LiFe"); break;
    case KXKM_STM32_Energy::BATTERY_CUSTOM: Serial.println("custom"); break;
  }
}

void loop() {
  static unsigned long lastBatteryCheck, lastButtonCheck;
  if (millis() - lastBatteryCheck > BATTERY_CHECK_PERIOD_MS)
  {
    lastBatteryCheck = millis();
    sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_VOLTAGE);

    Serial.print("Batt voltage : ");
    Serial.print(readSerialAnswer());
    Serial.println("mV");

    sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_PERCENTAGE);
    Serial.print("Batt percentage : ");
    Serial.print(readSerialAnswer());
    Serial.println("%");
  }

  if (millis() - lastButtonCheck > BUTTON_CHECK_PERIOD_MS)
  {
    lastButtonCheck = millis();
    sendSerialCommand(KXKM_STM32_Energy::GET_BUTTON_EVENT);

    if (readSerialAnswer() == KXKM_STM32_Energy::BUTTON_DOUBLE_CLICK_EVENT)
    {
      Serial.println("Main button double clicked.");
      endTest(currentTestType);
      currentTestType = (test_type_t)((int)currentTestType + 1);
      beginTest(currentTestType);
    }
  }

  switch (currentTestType)
  {
    case TEST_LED_1:
    {
      static unsigned long ledUpdateTime = millis();
      if (millis() - ledUpdateTime > 100)
      {
        ledUpdateTime = millis();
        uint8_t leds[] = {0, 0, 0, 0, 0, 0};
        static uint8_t idx = 0;
        leds[idx] = 4;
        if (idx < 5)
          leds[idx + 1] = 2;
        if (idx > 0)
          leds[idx - 1] = 1;

        setLeds(leds);
        idx = (idx + 1) % 6;
      }
      break;
    }

    case TEST_LED_2:
    {
      static unsigned long ledUpdateTime = millis();
      if (millis() - ledUpdateTime > 20)
      {
        ledUpdateTime = millis();
        static uint8_t percentage = 0;
        sendSerialCommand(KXKM_STM32_Energy::SET_LED_GAUGE, percentage++);

        if (percentage >= 100)
          percentage = 0;
      }
      break;
    }

    default:
      break;
  };

  button.check();
}

void handleButtonEvent(ace_button::AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case ace_button::AceButton::kEventPressed:
      break;
    case ace_button::AceButton::kEventClicked:
      switch (currentTestType)
      {
        case INIT:
          Serial.println("Disabling load switch autostart.");
          sendSerialCommand(KXKM_STM32_Energy::SET_LOAD_SWITCH, 0);
          break;

        case TEST_ENTER_CRITICAL_SECTION:
        case TEST_LEAVE_CRITICAL_SECTION:
          Serial.println("Shutting down...");
          sendSerialCommand(KXKM_STM32_Energy::SHUTDOWN);
          break;

        default:
          break;
      }
      break;
  }
}

void beginTest(test_type_t test)
{
  switch (test)
  {
    case TEST_LOAD_SW:
      Serial.println("Enabling load switch.");
      sendSerialCommand(KXKM_STM32_Energy::SET_LOAD_SWITCH, 1);
      break;

    case TEST_CUSTOM_BATT:
      Serial.println("Setting new battery characteristics.");
      sendSerialCommand(KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_LOW, 12000);
      sendSerialCommand(KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_3, 12500);
      sendSerialCommand(KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_6, 14000);
      break;

    case TEST_ENTER_CRITICAL_SECTION:
      Serial.println("Entering critical section for 8s.");
      sendSerialCommand(KXKM_STM32_Energy::ENTER_CRITICAL_SECTION, 8000);
      break;

    case TEST_LEAVE_CRITICAL_SECTION:
      Serial.println("Leaving critical section");
      sendSerialCommand(KXKM_STM32_Energy::LEAVE_CRITICAL_SECTION);
      break;

    case TEST_SELF_RESET:
      Serial.println("Self reset");
      sendSerialCommand(KXKM_STM32_Energy::REQUEST_RESET);

    default:
      break;
  };
}

void endTest(test_type_t test)
{
  switch (test)
  {
    case TEST_LOAD_SW:
      Serial.println("Disabling load switch.");
      sendSerialCommand(KXKM_STM32_Energy::SET_LOAD_SWITCH, 0);
      break;

    default:
      break;
  };
}
