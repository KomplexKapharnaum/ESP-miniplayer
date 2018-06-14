/* KXKM - STM32 coprocessor API

This file defines all recognized commands that the STM32 will answer to.
The protocol is based on UART @ 115200 bauds and uses plain text.

The basic transaction is as follows :
  * main processor > STM32 : ### <cmd type> [optional argument]\n
  * STM32 > main processor : ### [optional answer / ack]\n

Tom Magnier - 04/2018
*/

#ifndef KXKM_STM32_ENERGY_API_H
#define KXKM_STM32_ENERGY_API_H

class KXKM_STM32_Energy {
public:
  static constexpr char* PREAMBLE = "### ";

  /* API Version */
  static constexpr uint8_t API_VERSION = 1;

  /* Command types (main processor > STM32) */
  enum CommandType {
    /* Get API version.
       No argument.
       The STM32 will answer with its own API version. */
    GET_API_VERSION = 'A',

    /* Get firmware version.
       No argument.
       The STM32 will answer with its firmware version. */
    GET_FW_VERSION = 'F',

    /* Get battery voltage.
       No argument.
       The STM32 will answer with the voltage in mV */
    GET_BATTERY_VOLTAGE = 'V',

    /* Get battery percentage.
       No argument.
       The STM32 will answer with the estimated battery level. */
    GET_BATTERY_PERCENTAGE = '%',

    /* Get battery type as set by the 3-way selector.
       No argument.
       See enum BatteryType for the possible answers from the STM32. */
    GET_BATTERY_TYPE = 'T',

    /* Set the LEDs independently.
       Argument : for each LED a number between 0 (Off) and 4 (On). Missing LEDs will be treated as 0.
        Example : 444000 will set the first 3 LEDs to full brightness and the last 3 to Off.
        No answer from the STM32. */
    SET_LEDS = 'D',

    /* Set the LEDs to display a percentage.
       Argument : the percentage to display.
       No answer from the STM32. */
    SET_LED_GAUGE = 'G',

    /* Enable / disable main power switch. Default behavior is to switch it on 2s after startup.
       Send a disable command before to cancel this behavior.
       Argument : 1 to enable outputnamespace, 0 to disable.
       No answer from the STM32. */
    SET_LOAD_SWITCH = 'P',

    /* Shutdown the whole board.
       No argument.
       No answer from the STM32. */
    SHUTDOWN = 'S',

    /* Request a self reset.
       No argument.
       No answer from the STM32. */
    REQUEST_RESET = 'R',

    /* Set battery characteristics.
       7 voltage levels can be specified : 0 for the lowest voltage (cut off), 1-5 for the intermediate
        voltages (remaining capacity : 25%; 40%; 55%; 70%; 85%) and 6 for the fully charged voltage.
       Argument : the voltage in mV
       No answer from the STM32. */
    SET_BATTERY_VOLTAGE_LOW = '0',
    SET_BATTERY_VOLTAGE_1   = '1',
    SET_BATTERY_VOLTAGE_2   = '2',
    SET_BATTERY_VOLTAGE_3   = '3',
    SET_BATTERY_VOLTAGE_4   = '4',
    SET_BATTERY_VOLTAGE_5   = '5',
    SET_BATTERY_VOLTAGE_6   = '6',

    /* Enter critical section.
       The board will stay powered while in a critical section (to avoid memory corruption for example).
       The critical section will end at the end of the timeout or when the "Leave critical section"
       is received.
       Argument : the critical section timeout in ms. Max : 10000
       No answer from the STM32. */
    ENTER_CRITICAL_SECTION = 'E',

    /* Leave critical section.
       No argument.
       No answer from the STM32. */
    LEAVE_CRITICAL_SECTION = 'L',

    /* Get push button event. This command is used to get the last button event (clicked / double clicked / no event)
       since the last call.
       No argument.
       See PushButtonEvents for the possible answers from the STM32. */
    GET_BUTTON_EVENT = 'B'

    /* TODO Ping / Watchdog ??? */

  };

  /* Possible answers to the "Get push button event" command. */
  enum PushButtonEvent {
    /* No event since the last command call */
    NO_EVENT = 0,

    /* Button click */
    BUTTON_CLICK_EVENT = 1,

    /* Button double click */
    BUTTON_DOUBLE_CLICK_EVENT = 2
  };

  /* Possible answers to the "Get battery type" command */
  enum BatteryType {
    BATTERY_LIPO = 0,
    BATTERY_LIFE = 1,
    BATTERY_CUSTOM = 2
  };

  static bool hasArgument(CommandType cmd)
  {
    return (cmd == SET_LEDS ||
            cmd == SET_LED_GAUGE ||
            cmd == SET_LOAD_SWITCH ||
            cmd == ENTER_CRITICAL_SECTION ||
            (cmd >= SET_BATTERY_VOLTAGE_LOW && cmd <= SET_BATTERY_VOLTAGE_6));
  }
};

#endif //KXKM_STM32_ENERGY_API_H
