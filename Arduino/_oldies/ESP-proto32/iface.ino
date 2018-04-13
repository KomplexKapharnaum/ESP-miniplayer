const int buttonPin = 4;     // the number of the pushbutton pin UTTON on D2 / GPIO4
const int ledPin =  5;      // the number of the LED pin

int buttonState;
bool ledState = false;

void iface_setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);

  iface_led(true);
}

void iface_loop() {

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
}

void iface_led(bool onOff) {
  if (onOff == ledState) return;
  ledState = onOff;
  if (onOff) digitalWrite(ledPin, HIGH);
  else digitalWrite(ledPin, LOW);
}

bool  iface_btn() {
  if (buttonState == HIGH) return true;
  return false;
}

