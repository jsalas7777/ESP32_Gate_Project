const int LED_PIN = 25;
const int GATE_1 = 26;
const int GATE_2 = 27;
const int GATE_3 = 14;
const int GATES[] = {GATE_1, GATE_2, GATE_3};
const char *GATE_NAMES[] = {"gate_1", "gate_2", "gate_3"};
const int GATE_COUNT = sizeof(GATES) / sizeof(GATES[0]);
const unsigned long GATE_ON_MS = 1000;
const unsigned long GATE_OFF_MS = 1000;
const unsigned long LOG_INTERVAL_MS = 2000;
const int BOOT_BLINKS = 3;
const int BLINK_MS = 200;

unsigned long lastLog = 0;
unsigned long lastStep = 0;
int activeGate = 0;
bool gateOn = false;

void gateOnNow(int index) {
  digitalWrite(GATES[index], HIGH);
  gateOn = true;
  Serial.printf("[GATE] %s ON (GPIO%d)\n", GATE_NAMES[index], GATES[index]);
}

void gateOffNow(int index) {
  digitalWrite(GATES[index], LOW);
  gateOn = false;
  Serial.printf("[GATE] %s OFF (GPIO%d)\n", GATE_NAMES[index], GATES[index]);
}

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

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  for (int i = 0; i < GATE_COUNT; i++) {
    pinMode(GATES[i], OUTPUT);
    digitalWrite(GATES[i], LOW);
  }

  Serial.printf("[BOOT] ESP32 started, reset reason: %s\n", resetReasonName(esp_reset_reason()));

  for (int i = 0; i < BOOT_BLINKS; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(BLINK_MS);
    digitalWrite(LED_PIN, LOW);
    delay(BLINK_MS);
  }

  digitalWrite(LED_PIN, HIGH);
  Serial.println("[BOOT] blink done, GPIO25 ON");

  gateOnNow(activeGate);
  lastStep = millis();
}

void loop() {
  // gate_1 ON -> OFF -> gate_2 ON -> OFF -> gate_3 ON -> OFF -> repeat
  if (gateOn && millis() - lastStep >= GATE_ON_MS) {
    lastStep = millis();
    gateOffNow(activeGate);
  } else if (!gateOn && millis() - lastStep >= GATE_OFF_MS) {
    lastStep = millis();
    activeGate = (activeGate + 1) % GATE_COUNT;
    gateOnNow(activeGate);
  }

  if (millis() - lastLog >= LOG_INTERVAL_MS) {
    lastLog = millis();
    Serial.printf("[OK] running, uptime=%lus, GPIO25=%d\n", millis() / 1000, digitalRead(LED_PIN));
  }
}
