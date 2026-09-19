const int LED_PIN = 25;
const unsigned long LOG_INTERVAL_MS = 2000;

unsigned long lastLog = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  Serial.println("[BOOT] ESP32 gate started, GPIO25 set as OUTPUT (LED ON)");
}

void loop() {
  if (millis() - lastLog >= LOG_INTERVAL_MS) {
    lastLog = millis();
    Serial.printf("[OK] running, uptime=%lus, GPIO25=%d\n", millis() / 1000, digitalRead(LED_PIN));
  }
}
