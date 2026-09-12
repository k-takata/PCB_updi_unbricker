//
// UPDI Unbricker
//
// Unbricks AVR Dx/Ex and ATtiny series
//

#if !(defined(MEGATINYCORE) && _AVR_PINCOUNT == 8)
# error Unsupported device
#endif

#define PIN_HV_75 PIN_PA3
#define PIN_HV_12 PIN_PA1
#define PIN_INPUT PIN_PA2
#define PIN_UPDI  PIN_PA6
#define PIN_PWR   PIN_PA7

// Use UART0 for UPDI
// Use only TX0 pin
auto& serial_updi = Serial;

void setup()
{
  pinModeFast(PIN_INPUT, INPUT_PULLUP);

  pinModeFast(PIN_HV_75, OUTPUT);
  digitalWriteFast(PIN_HV_75, LOW);
  pinModeFast(PIN_HV_12, OUTPUT);
  digitalWriteFast(PIN_HV_12, LOW);

  openDrainFast(PIN_PWR, LOW);  // On

  // UPDI - 8 bit, even parity, 2 stop bit; one-wire
  serial_updi.begin(115200, SERIAL_8E2 | SERIAL_HALF_DUPLEX);

  attachInterrupt(digitalPinToInterrupt(PIN_INPUT), buttonPushed, FALLING);
}

unsigned long pushed_time = 0;
bool button_pushed = false;

void buttonPushed()
{
  if (!button_pushed) {
    pushed_time = millis();
    button_pushed = true;
  }
}

constexpr int debounceDelay = 50; // [ms]

bool checkButtonStatus()
{
  bool ret = false;

  if (button_pushed) {
    if (millis() - pushed_time >= debounceDelay) {
      if (digitalReadFast(PIN_INPUT) == LOW) {
        ret = true;
      }
      button_pushed = false;
    }
  }
  return ret;
}

const uint8_t UPDI_SYNCH = 0x55;
const uint8_t UPDI_KEY_64 = 0xe0;
const uint8_t nvmprog_key[] = {0x20, 0x67, 0x6f, 0x72, 0x50, 0x4d, 0x56, 0x4e}; // 0x4E564D50726F6720

void reset_target()
{
  // This is required only for ATtiny.
  // However, it should be acceptable for AVR Dx/Ex series.

  // Keep reset while the START button is pressed.
  openDrainFast(PIN_PWR, FLOATING); // Off
  do {
    delay(50);
  } while (digitalReadFast(PIN_INPUT) == LOW);
  openDrainFast(PIN_PWR, LOW);  // On

  delay(1);   // < 8.8 ms
}

void send_hv_pulse()
{
  // Recommended pulse widths (t_HV) are:
  //   ATtiny: > 100 us, < 1 ms
  //   AVR Dx/Ex: > 10 us

  // Can we use the same pulse width?

  digitalWriteFast(PIN_HV_12, HIGH);
  delayMicroseconds(100);
  digitalWriteFast(PIN_HV_75, HIGH);
  delayMicroseconds(50);
  digitalWriteFast(PIN_HV_75, LOW);
  digitalWriteFast(PIN_HV_12, LOW);
  delayMicroseconds(10);  // needed?
}

void enable_updi()
{
  // This is required only for AVR Dx/Ex series.
  // However, it should be harmless for ATtiny.

  openDrainFast(PIN_UPDI, LOW);
  // Wait t_Deb0 (0.2 - 1 us)
  __builtin_avr_delay_cycles(F_CPU * 5 / 10'000'000);  // 0.5 us (e.g. 10 cycles @ 20 MHz)
  openDrainFast(PIN_UPDI, FLOATING);

  // Wait t_UPDI (10 - 200 us)
  for (int i = 0; i < 21; ++i) {
    delayMicroseconds(10);
    if (digitalReadFast(PIN_UPDI) == HIGH)
      break;
  }

  // Wait t_DebZ (200 us - 14 ms)
  delay(1);

#if 0
  // long break
  openDrainFast(PIN_UPDI, LOW);
  delay(25);
  openDrainFast(PIN_UPDI, FLOATING);
#endif

  // Send NVMPROG key
  serial_updi.begin(115200, SERIAL_8E2 | SERIAL_HALF_DUPLEX);
  serial_updi.write(UPDI_SYNCH);
  serial_updi.read();
  serial_updi.write(UPDI_KEY_64);
  serial_updi.read();
  for (const auto& e : nvmprog_key) {
    serial_updi.write(e);
    serial_updi.read();
  }
}

void loop()
{
  if (checkButtonStatus()) {
    reset_target();
    send_hv_pulse();
    enable_updi();
  }
  delay(10);
}
