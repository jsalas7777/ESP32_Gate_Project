const int LED_PIN = 25;
const int GATE_1 = 26;
const int GATE_2 = 27;
const int GATE_3 = 14;
const int SENSOR_PIN = 19;  // sensor ON when pulled to GND
const int GATES[] = {GATE_1, GATE_2, GATE_3};
const int GATE_COUNT = sizeof(GATES) / sizeof(GATES[0]);

const unsigned long GATE1_PULSE_MS = 1000;  // each pulse: 1s ON, 1s OFF
const int GATE1_PULSES = 3;
const unsigned long GATE2_ON_MS = 20000;
const unsigned long GATE3_ON_MS = 20000;
const unsigned long DEBOUNCE_MS = 50;
const unsigned long LOG_INTERVAL_MS = 2000;
const int BOOT_BLINKS = 3;
const int BLINK_MS = 200;

// Sensor state: true = grounded (ON), false = open (OFF)
bool sensorOn = false;
bool lastSensorReading = false;
unsigned long lastSensorChange = 0;

// Gate sequence, runs while the sensor is OFF
enum SequenceStep { IDLE, GATE1_PULSING, GATE2_ON, GATE3_ON };
SequenceStep step = IDLE;
unsigned long stepStart = 0;

unsigned long lastLog = 0;

const char *resetReasonName(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:  return "power-on";
    case ESP_RST_EXT:      return "external reset (EN button)";
    case ESP_RST_SW:       return "software reset";
    case ESP_RST_PANIC:    return "crash/panic";
    case ESP_RST_INT_WDT:  return "interrupt watchdog";
    case ESP_RST_TASK_WDT: return "task watchdog";
    case ESP_RST_WDT:      return "watchdog";
    case ESP_RST_DEEPSLEEP: return "deep sleep wake";
    case ESP_RST_BROWNOUT: return "brownout";
    default:               return "unknown";
  }
}

// Set the output level LOW before switching each pin to OUTPUT so it never glitches HIGH
void allPinsOff() {
  digitalWrite(LED_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  for (int i = 0; i < GATE_COUNT; i++) {
    digitalWrite(GATES[i], LOW);
    pinMode(GATES[i], OUTPUT);
  }
}

bool readSensor() {
  return digitalRead(SENSOR_PIN) == LOW;  // grounded = ON
}

void startSequence() {
  Serial.println("[SEQ] start: gate_1 pulsing");
  step = GATE1_PULSING;
  stepStart = millis();
}

// Sensor ON: abort the sequence and turn every gate off
void stopSequence() {
  for (int i = 0; i < GATE_COUNT; i++) {
    digitalWrite(GATES[i], LOW);
  }
  if (step != IDLE) {
    Serial.println("[SEQ] stopped by sensor, all gates OFF");
  }
  step = IDLE;
}

void nextStep(SequenceStep next) {
  step = next;
  stepStart = millis();
}

// gate_1 3 pulses -> gate_2 ON 20s -> gate_3 ON 20s
void updateSequence() {
  unsigned long elapsed = millis() - stepStart;

  switch (step) {
    case IDLE:
      break;

    case GATE1_PULSING: {
      int phase = elapsed / GATE1_PULSE_MS;
      if (phase >= GATE1_PULSES * 2) {
        digitalWrite(GATE_1, LOW);
        digitalWrite(GATE_2, HIGH);
        Serial.println("[SEQ] gate_1 done, gate_2 ON");
        nextStep(GATE2_ON);
      } else {
        digitalWrite(GATE_1, phase % 2 == 0 ? HIGH : LOW);
      }
      break;
    }

    case GATE2_ON:
      if (elapsed >= GATE2_ON_MS) {
        digitalWrite(GATE_2, LOW);
        digitalWrite(GATE_3, HIGH);
        Serial.println("[SEQ] gate_2 OFF, gate_3 ON");
        nextStep(GATE3_ON);
      }
      break;

    case GATE3_ON:
      if (elapsed >= GATE3_ON_MS) {
        digitalWrite(GATE_3, LOW);
        Serial.println("[SEQ] gate_3 OFF, done");
        nextStep(IDLE);
      }
      break;
  }
}

void setup() {
  allPinsOff();
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  Serial.begin(115200);

  Serial.printf("[BOOT] ESP32 started, reset reason: %s\n", resetReasonName(esp_reset_reason()));
  Serial.println("[BOOT] all pins OFF (GPIO25, GPIO26, GPIO27, GPIO14)");

  for (int i = 0; i < BOOT_BLINKS; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(BLINK_MS);
    digitalWrite(LED_PIN, LOW);
    delay(BLINK_MS);
  }

  digitalWrite(LED_PIN, HIGH);
  Serial.println("[BOOT] blink done, GPIO25 ON");

  sensorOn = readSensor();
  lastSensorReading = sensorOn;
  Serial.printf("[SENSOR] GPIO19 initial state: %s\n", sensorOn ? "ON" : "OFF");
}

void loop() {
  // Debounced sensor read
  bool reading = readSensor();
  if (reading != lastSensorReading) {
    lastSensorReading = reading;
    lastSensorChange = millis();
  }
  if (reading != sensorOn && millis() - lastSensorChange >= DEBOUNCE_MS) {
    sensorOn = reading;
    Serial.printf("[SENSOR] GPIO19 %s\n", sensorOn ? "ON" : "OFF");
    if (sensorOn) {
      stopSequence();
    }
  }

  // Run the sequence whenever the sensor is OFF (repeats while it stays OFF)
  if (!sensorOn && step == IDLE) {
    startSequence();
  }

  updateSequence();

  if (millis() - lastLog >= LOG_INTERVAL_MS) {
    lastLog = millis();
    Serial.printf("[OK] uptime=%lus, sensor=%s, step=%d\n",
                  millis() / 1000, sensorOn ? "ON" : "OFF", step);
  }
}
