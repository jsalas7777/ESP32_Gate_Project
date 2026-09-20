const int LED_PIN = 25;
const unsigned long LOG_INTERVAL_MS = 2000;
const int BOOT_BLINKS = 3;
const int BLINK_MS = 200;

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

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  Serial.printf("[BOOT] ESP32 started, reset reason: %s\n", resetReasonName(esp_reset_reason()));

  for (int i = 0; i < BOOT_BLINKS; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(BLINK_MS);
    digitalWrite(LED_PIN, LOW);
    delay(BLINK_MS);
  }

  digitalWrite(LED_PIN, HIGH);
  Serial.println("[BOOT] blink done, GPIO25 ON");
}

void loop() {
  if (millis() - lastLog >= LOG_INTERVAL_MS) {
    lastLog = millis();
    Serial.printf("[OK] running, uptime=%lus, GPIO25=%d\n", millis() / 1000, digitalRead(LED_PIN));
  }
}
