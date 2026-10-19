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
  pinModeFast(PIN_UPDI, INPUT_PULLUP);

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
const uint8_t UPDI_SYNCH = 0x55;
const uint8_t UPDI_ACK = 0x40;
// UPDI Instructions
const uint8_t UPDI_LDCS = 0x80;
const uint8_t UPDI_STCS = 0xc0;
const uint8_t UPDI_KEY_64 = 0xe0;

// UPDI registers
const uint8_t UPDI_REG_ASI_KEY_STATUS = 0x07;
const uint8_t UPDI_REG_RESET_REQ = 0x08;
const uint8_t UPDI_REG_ASI_SYS_STATUS = 0x0B;

// Bitmasks for UPDI_REG_ASI_KEY_STATUS
const uint8_t UPDI_UROWWR_BIT = 0x20;
const uint8_t UPDI_NVMPROG_BIT = 0x10;
const uint8_t UPDI_CHER_BIT = 0x08;
// Values for UPDI_REG_RESET_REQ
const uint8_t UPDI_RSTREQ_RUN = 0x00;
const uint8_t UPDI_RSTREQ_RESET = 0x59;
// Bitmasks for UPDI_REG_ASI_SYS_STATUS
const uint8_t UPDI_LOCKSTATUS_BIT = 0x01;

// UPDI Keys
const uint8_t chip_erase_key[] = {0x65, 0x73, 0x61, 0x72, 0x45, 0x4d, 0x56, 0x4e}; // 0x4E564D4572617365 'NVMErase'
const uint8_t nvmprog_key[]    = {0x20, 0x67, 0x6f, 0x72, 0x50, 0x4d, 0x56, 0x4e}; // 0x4E564D50726F6720 'NVMProg '
const uint8_t userrow_key[]    = {0x65, 0x74, 0x26, 0x73, 0x55, 0x4d, 0x56, 0x4e}; // 0x4E564D5573267465 'NVMUs&te'

uint8_t updi_send_byte(uint8_t b)
{
  // RXD is connected to TXD; we receive the same byte that we sent.
  serial_updi.write(b);
  return serial_updi.read();
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
  // 1'000'000 [us] / 115'200 [bps] * (128 + 12) = 1215 [us]

  const uint32_t start = micros();
  while ((micros() - start) < timeoutUs) {
    if (serial_updi.available()) {
      uint8_t b = serial_updi.read();
      return {true, b};
      delayMicroseconds(50);
    }
  }
  return {false, 0x00};
}

void reset_target()
{
  // This is required for ATtiny devices to satisfy the POR timing window.
  // It is also acceptable for AVR Dx/Ex devices, where it simply ensures a clean reset
  // before the HV pulse is applied.

  // Keep the target reset while the START button is pressed.
  openDrainFast(PIN_PWR, FLOATING); // Off
  do {
    delay(50);
  } while (digitalReadFast(PIN_INPUT) == LOW);
  openDrainFast(PIN_PWR, LOW);  // On

  delay(1);   // < 8.8 ms
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
constexpr uint32_t updi_key_timeout_ms = 100;
constexpr uint32_t updi_erase_timeout_ms = 1000;

void erase_chip()
{
  // Send Chip Erase key
  updi_send_key_64(chip_erase_key);
  delayMicroseconds(updi_inter_byte_delay_us);

  // Send NVMPROG key
  updi_send_key_64(nvmprog_key);
  delayMicroseconds(updi_inter_byte_delay_us);

  // Check key status
  bool done = false;
  const uint32_t start_keys = millis();
  while ((millis() - start_keys) < updi_key_timeout_ms) {
    auto [status, value] = updi_read_reg(UPDI_REG_ASI_KEY_STATUS);
    if (!status) {
      return;
    }
    const uint8_t mask = UPDI_NVMPROG_BIT | UPDI_CHER_BIT;
    if ((value & mask) == mask) {
      done = true;
      break;
    }
  }
  if (!done) {
    return;
  }
  delayMicroseconds(updi_inter_byte_delay_us);

  updi_write_reg(UPDI_REG_RESET_REQ, UPDI_RSTREQ_RESET);
  delayMicroseconds(updi_inter_byte_delay_us);

  done = false;
  const uint32_t start_erase = millis();
  while ((millis() - start_erase) < updi_erase_timeout_ms) {
    updi_write_reg(UPDI_REG_RESET_REQ, UPDI_RSTREQ_RUN);
    delayMicroseconds(updi_inter_byte_delay_us);

    auto [status, value] = updi_read_reg(UPDI_REG_ASI_SYS_STATUS);
    if (!status) {
      return;
    }
    if ((value & UPDI_LOCKSTATUS_BIT) == 0x00) {
      done = true;
      break;
    }
    delayMicroseconds(updi_inter_byte_delay_us);
  }
}

void start_prog()
{
  // Send NVMPROG key
  updi_send_key_64(nvmprog_key);
}

constexpr uint32_t updi_wait_timeout_us = 2000;

void enable_updi(bool erase)
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
    return;
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

  if (erase) {
    erase_chip();
  }
  else {
    start_prog();
  }

  serial_updi.end();
}

constexpr int chip_erase_threshold = 5000; // [ms]

void loop()
{
  if (checkButtonStatus()) {
    reset_target();
    bool erase = false;
    if (millis() - pushed_time > chip_erase_threshold) {
      // The START button was held down for over the threshold.
      erase = true;
    }
    send_hv_pulse();
    enable_updi(erase);
  }
  delay(10);
}
