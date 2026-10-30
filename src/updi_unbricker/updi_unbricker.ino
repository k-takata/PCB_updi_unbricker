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

#define UPDI_BAUD_RATE  115200

void setup()
{
  pinModeFast(PIN_INPUT, INPUT_PULLUP);

  pinModeFast(PIN_HV_75, OUTPUT);
  digitalWriteFast(PIN_HV_75, LOW);
  pinModeFast(PIN_HV_12, OUTPUT);
  digitalWriteFast(PIN_HV_12, LOW);

  openDrainFast(PIN_PWR, LOW);  // On

  // UPDI - 8-bit, even parity, 2 stop bits; one-wire
  //serial_updi.begin(UPDI_BAUD_RATE, SERIAL_8E2 | SERIAL_HALF_DUPLEX);
  //serial_updi.end();
  // Keep the UPDI pin Hi-Z. The internal pull-up (5 V) must not pull up
  // the UPDI line of a 3.3 V target.
  pinModeFast(PIN_UPDI, INPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_INPUT), buttonPushed, FALLING);
}

volatile unsigned long pushed_time = 0;
volatile bool button_pushed = false;

void buttonPushed()
{
  if (!button_pushed) {
    pushed_time = millis();
    button_pushed = true;
  }
}

constexpr int debounce_delay_ms = 50;

bool checkButtonStatus()
{
  bool ret = false;

  if (button_pushed) {
    if (millis() - pushed_time >= debounce_delay_ms) {
      if (digitalReadFast(PIN_INPUT) == LOW) {
        ret = true;
      }
      button_pushed = false;
    }
  }
  return ret;
}

static bool wait_for_pin_state(uint8_t pin, int8_t expected, uint32_t timeoutUs)
{
  const uint32_t start = micros();
  while ((micros() - start) < timeoutUs) {
    if (digitalReadFast(pin) == expected) {
      return true;
    }
  }
  return false;
}

// UPDI Control Frames
constexpr uint8_t UPDI_SYNCH = 0x55;
constexpr uint8_t UPDI_ACK = 0x40;
// UPDI Instructions
constexpr uint8_t UPDI_LDCS = 0x80;
constexpr uint8_t UPDI_STCS = 0xc0;
constexpr uint8_t UPDI_KEY_64 = 0xe0;

// UPDI registers
constexpr uint8_t UPDI_REG_CTRLB = 0x03;
constexpr uint8_t UPDI_REG_ASI_KEY_STATUS = 0x07;
constexpr uint8_t UPDI_REG_RESET_REQ = 0x08;
constexpr uint8_t UPDI_REG_ASI_SYS_STATUS = 0x0B;

// Bitmasks for UPDI_REG_CTRLB
constexpr uint8_t UPDI_CCDETDIS_BIT = 0x08;
// Bitmasks for UPDI_REG_ASI_KEY_STATUS
constexpr uint8_t UPDI_UROWWR_BIT = 0x20;
constexpr uint8_t UPDI_NVMPROG_BIT = 0x10;
constexpr uint8_t UPDI_CHER_BIT = 0x08;
// Values for UPDI_REG_RESET_REQ
constexpr uint8_t UPDI_RSTREQ_RUN = 0x00;
constexpr uint8_t UPDI_RSTREQ_RESET = 0x59;
// Bitmasks for UPDI_REG_ASI_SYS_STATUS
constexpr uint8_t UPDI_LOCKSTATUS_BIT = 0x01;

// UPDI Keys
constexpr uint8_t chip_erase_key[] = {0x65, 0x73, 0x61, 0x72, 0x45, 0x4d, 0x56, 0x4e}; // 0x4E564D4572617365 'NVMErase'
constexpr uint8_t nvmprog_key[]    = {0x20, 0x67, 0x6f, 0x72, 0x50, 0x4d, 0x56, 0x4e}; // 0x4E564D50726F6720 'NVMProg '
constexpr uint8_t userrow_key[]    = {0x65, 0x74, 0x26, 0x73, 0x55, 0x4d, 0x56, 0x4e}; // 0x4E564D5573267465 'NVMUs&te'

void updi_send_byte(uint8_t b)
{
  // In half-duplex mode, the core ignores the echo of the transmitted byte
  // (RX is not stored in the RX buffer while transmitting).
  serial_updi.write(b);
}

// Send a UPDI Instruction
void updi_send_inst(uint8_t inst)
{
  // Each instruction must start with a synch byte.
  updi_send_byte(UPDI_SYNCH);
  updi_send_byte(inst);
}

// Send a 64-bit key
void updi_send_key_64(const uint8_t (&arr)[8])
{
  updi_send_inst(UPDI_KEY_64);
  for (const auto& e : arr) {
    updi_send_byte(e);
  }
}

void updi_write_reg(uint8_t reg, uint8_t val)
{
  updi_send_inst(UPDI_STCS | reg);
  updi_send_byte(val);
}

constexpr uint32_t updi_read_reg_timeout_us = 2000; // > (128 + 12) bits

struct updi_reg_status {
  bool status;
  uint8_t value;
};

updi_reg_status updi_read_reg(
    uint8_t reg, uint32_t timeoutUs = updi_read_reg_timeout_us)
{
  updi_send_inst(UPDI_LDCS | reg);

  // Direction Change:
  // The number of Idle bits can be configured through the Guard Time Value
  // (GTVAL) bit field in the Control A (UPDI.CTRLA) register.
  // The default is 128 bits and the minimum is 2 bits.
  // timeoutUs should be set based on it.
  // E.g., if the baud rate is 115200 and the Idle bits are 128 bits, the
  // minimum timeout should be:
  //   1'000'000 [us] / 115'200 [bps] = 8.681 [us / bit]
  //   8.681 * (128 + 12) = 1215 [us] (Idle bits + 1 frame)

  // write() only queues the data. Wait until the instruction is sent
  // completely, otherwise the timeout expires before the response arrives
  // (the queued frames take up to 1 ms, as long as the guard time).
  serial_updi.flush();
  const uint32_t start = micros();
  while ((micros() - start) < timeoutUs) {
    if (serial_updi.available()) {
      uint8_t b = serial_updi.read();
      return {true, b};
    }
    delayMicroseconds(50);
  }
  return {false, 0x00};
}

constexpr uint32_t chip_erase_threshold_ms = 5000;
constexpr uint32_t por_release_delay_ms = 2;  // < 8.8 ms

// Returns true if the START button was held down for over the threshold.
bool reset_target(uint32_t pressed_time)
{
  // This is required for ATtiny devices to satisfy the POR timing window.
  // It is also acceptable for AVR Dx/Ex devices, where it simply ensures a clean reset
  // before the HV pulse is applied.

  // Keep the target reset while the START button is pressed.
  openDrainFast(PIN_PWR, FLOATING); // Off
  do {
    delay(50);
  } while (digitalReadFast(PIN_INPUT) == LOW);

  // Measure the hold time here. Do not use pushed_time after this point,
  // because contact bounce on release may overwrite it in the ISR.
  const bool long_press = (millis() - pressed_time) > chip_erase_threshold_ms;
  openDrainFast(PIN_PWR, LOW);  // On

  // Wait for the target to be released from POR before applying the HV pulse.
  // A too short delay (1 ms) caused HV failures (AVR64DD28 @ 5 V, ATtiny Chip
  // Erase). Keep it shorter than 8.8 ms for ATtiny.
  delay(por_release_delay_ms);

  return long_press;
}

constexpr uint32_t hv_pulse_12_us = 200;
constexpr uint32_t hv_pulse_75_us = 20;
constexpr uint32_t hv_pulse_gap_us = 10;

void send_hv_pulse()
{
  // Recommended pulse widths (t_HV) are:
  //   ATtiny: > 100 us, < 1 ms
  //   AVR Dx/Ex: > 10 us

  // Use a common pulse sequence for both device families and tune if needed.

  digitalWriteFast(PIN_HV_12, HIGH);
  delayMicroseconds(hv_pulse_12_us - hv_pulse_75_us);
  digitalWriteFast(PIN_HV_75, HIGH);
  delayMicroseconds(hv_pulse_75_us);
  digitalWriteFast(PIN_HV_75, LOW);
  digitalWriteFast(PIN_HV_12, LOW);
  delayMicroseconds(hv_pulse_gap_us);
}

constexpr uint32_t updi_inter_byte_delay_us = 20;   // > 2 bits

// Error codes. They are notified by blinking the VDD indicator LEDs.
constexpr uint8_t ERR_NONE = 0;
constexpr uint8_t ERR_NO_UPDI = 1;          // UPDI is not enabled (handshake failed)
constexpr uint8_t ERR_KEY_NO_RESPONSE = 2;  // No response when reading the key status
constexpr uint8_t ERR_KEY_REJECTED = 3;     // The Chip Erase key is not accepted
constexpr uint8_t ERR_LOCK_NO_RESPONSE = 4; // No response when reading the lock status
constexpr uint8_t ERR_ERASE_TIMEOUT = 5;    // Chip Erase doesn't finish in time

// Notify an error by blinking the target power (the VDD indicator LEDs).
// This resets the target, but it is acceptable because the operation failed.
// It doesn't blink if the target is powered externally.
void notify_error(uint8_t code)
{
  constexpr uint32_t blink_ms = 150;

  delay(500);
  for (uint8_t i = 0; i < code; ++i) {
    openDrainFast(PIN_PWR, FLOATING); // Off
    delay(blink_ms);
    openDrainFast(PIN_PWR, LOW);      // On
    delay(blink_ms);
  }
}

// t_P_CE (Chip Erase time)
//   AVR64DDxx: 11 ms
//   AVR128DAxx: 11 ms
//   AVR16LAxx: 10 ms
// t_P_UPDICE (UPDI Chip Erase time)
//   AVR16LAxx: 50 ms
//   AVR32LAxx: 70 ms
constexpr uint32_t updi_erase_timeout_ms = 1000;

uint8_t erase_chip()
{
  // Send Chip Erase key
  updi_send_key_64(chip_erase_key);
  delayMicroseconds(updi_inter_byte_delay_us);

  // Disable the contention detection.
  // Without it, the target aborts its response (the logic analyzer shows a
  // 2 us low pulse at the end of the guard time) and the following register
  // reads fail. The key must be the first instruction after the UPDI is
  // enabled, so write CTRLB after the key.
  updi_write_reg(UPDI_REG_CTRLB, UPDI_CCDETDIS_BIT);
  delayMicroseconds(updi_inter_byte_delay_us);

  // Send NVMPROG key
  updi_send_key_64(nvmprog_key);
  delayMicroseconds(updi_inter_byte_delay_us);

  // Check key status
  {
    auto [status, value] = updi_read_reg(UPDI_REG_ASI_KEY_STATUS);
    if (!status) {
      return ERR_KEY_NO_RESPONSE;
    }
    const uint8_t mask = UPDI_NVMPROG_BIT | UPDI_CHER_BIT;
    if ((value & mask) != mask) {
      return ERR_KEY_REJECTED;
    }
  }
  delayMicroseconds(updi_inter_byte_delay_us);

  updi_write_reg(UPDI_REG_RESET_REQ, UPDI_RSTREQ_RESET);
  delayMicroseconds(updi_inter_byte_delay_us);

  bool responded = false;
  const uint32_t start_erase = millis();
  while ((millis() - start_erase) < updi_erase_timeout_ms) {
    updi_write_reg(UPDI_REG_RESET_REQ, UPDI_RSTREQ_RUN);
    delayMicroseconds(updi_inter_byte_delay_us);

    auto [status, value] = updi_read_reg(UPDI_REG_ASI_SYS_STATUS);
    if (!status) {
      // The target may not respond for a while after the reset. Retry.
      delay(1);
      continue;
    }
    responded = true;
    if ((value & UPDI_LOCKSTATUS_BIT) == 0x00) {
      // Chip Erase finished.
      return ERR_NONE;
    }
    delayMicroseconds(updi_inter_byte_delay_us);
  }
  return responded ? ERR_ERASE_TIMEOUT : ERR_LOCK_NO_RESPONSE;
}

void start_prog()
{
  // Send NVMPROG key
  updi_send_key_64(nvmprog_key);
}

constexpr uint32_t updi_wait_timeout_us = 2000;

uint8_t enable_updi(bool erase)
{
  // This is required only for AVR Dx/Ex series.
  // However, it should be harmless for ATtiny.

  openDrainFast(PIN_UPDI, LOW);
  // Wait t_Deb0 (0.2 - 1 us)
  __builtin_avr_delay_cycles(F_CPU * 5 / 10'000'000);  // 0.5 us (e.g. 10 cycles @ 20 MHz)
  openDrainFast(PIN_UPDI, FLOATING);

  // Wait t_UPDI (10 - 200 us) and allow a generous timeout for real hardware variation.
#if 1
  if (!wait_for_pin_state(PIN_UPDI, HIGH, updi_wait_timeout_us)) {
    return ERR_NO_UPDI;
  }
#else
  for (int i = 0; i < 21; ++i) {
    delayMicroseconds(10);
    if (digitalReadFast(PIN_UPDI) == HIGH)
      break;
  }
#endif

  // Wait t_DebZ (200 us - 14 ms)
  delay(1);

#if 0
  // long break
  openDrainFast(PIN_UPDI, LOW);
  delay(25);
  openDrainFast(PIN_UPDI, FLOATING);
#endif

  serial_updi.begin(UPDI_BAUD_RATE, SERIAL_8E2 | SERIAL_HALF_DUPLEX);

  // begin() sets the pin to INPUT_PULLUP. Keep the pin Hi-Z during the
  // communication too, so that the internal pull-up (5 V) does not pull up
  // the UPDI line of a 3.3 V target. The UPDI line is pulled up by the target
  // (AVR Dx/Ex) or by the external pull-up resistor (ATtiny).
  pinModeFast(PIN_UPDI, INPUT);

  uint8_t err = ERR_NONE;
  if (erase) {
    err = erase_chip();
  }
  else {
    start_prog();
  }

  serial_updi.end();

  // begin() sets the pin to INPUT_PULLUP and end() doesn't restore it.
  pinModeFast(PIN_UPDI, INPUT);

  return err;
}

void loop()
{
  if (checkButtonStatus()) {
    // Copy with interrupts disabled; pushed_time is 32 bits wide.
    noInterrupts();
    const uint32_t pressed_time = pushed_time;
    interrupts();

    // If the START button was held down for over the threshold, erase the chip.
    const bool erase = reset_target(pressed_time);
    send_hv_pulse();
    const uint8_t err = enable_updi(erase);
    if (err != ERR_NONE) {
      notify_error(err);
    }

    // Discard any bounce of the button release detected during the sequence.
    button_pushed = false;
  }
  delay(10);
}
