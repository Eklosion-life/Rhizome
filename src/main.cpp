// =============================================================
// Rhizome v2 - Diagnostic Firmware V3 - Complet
// 2 plantes + LEDs de visualisation + EMA
// =============================================================
#include <stdint.h>
#include <Arduino.h>

// Brochage
#define PIN_555_PLANT1   4
#define PIN_555_PLANT2   5
#define PIN_LED_PLANT1   6
#define PIN_LED_PLANT2   7

// Lissage
const float ALPHA = 0.3;

// Variables ISR plante 1
volatile uint32_t last_capture_us_1 = 0;
volatile uint32_t period_us_1 = 0;
volatile uint32_t pulse_count_1 = 0;
volatile bool led_state_1 = false;

// Variables ISR plante 2
volatile uint32_t last_capture_us_2 = 0;
volatile uint32_t period_us_2 = 0;
volatile uint32_t pulse_count_2 = 0;
volatile bool led_state_2 = false;

// Variables principales
float smoothed_1 = 0.0;
float smoothed_2 = 0.0;
bool first_1 = true;
bool first_2 = true;

void IRAM_ATTR plant1_isr() {
  uint32_t now = micros();
  period_us_1 = now - last_capture_us_1;
  last_capture_us_1 = now;
  pulse_count_1++;
  
  // Toggle LED visualisation
  led_state_1 = !led_state_1;
  digitalWrite(PIN_LED_PLANT1, led_state_1);
}

void IRAM_ATTR plant2_isr() {
  uint32_t now = micros();
  period_us_2 = now - last_capture_us_2;
  last_capture_us_2 = now;
  pulse_count_2++;
  
  led_state_2 = !led_state_2;
  digitalWrite(PIN_LED_PLANT2, led_state_2);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("Rhizome v2 Diagnostic V3 - Complet");
  Serial.println("=================================");
  
  // Configurer les broches d'entrée 555
  pinMode(PIN_555_PLANT1, INPUT);
  pinMode(PIN_555_PLANT2, INPUT);
  
  // Configurer les LEDs
  pinMode(PIN_LED_PLANT1, OUTPUT);
  pinMode(PIN_LED_PLANT2, OUTPUT);
  digitalWrite(PIN_LED_PLANT1, LOW);
  digitalWrite(PIN_LED_PLANT2, LOW);
  
  // Attacher les interruptions
  attachInterrupt(digitalPinToInterrupt(PIN_555_PLANT1), plant1_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_555_PLANT2), plant2_isr, RISING);
  
  Serial.println("Plante 1 : GPIO 4 (LED GPIO 6)");
  Serial.println("Plante 2 : GPIO 5 (LED GPIO 7)");
  Serial.println();
  Serial.println("Demarrage des mesures...");
  Serial.println();
}

void loop() {
  // Lire valeurs partagées
  uint32_t p1 = period_us_1;
  uint32_t p2 = period_us_2;
  uint32_t c1 = pulse_count_1;
  uint32_t c2 = pulse_count_2;
  
  // Traiter plante 1
  if (p1 > 0) {
    if (first_1) {
      smoothed_1 = p1;
      first_1 = false;
    } else {
      smoothed_1 = (1.0 - ALPHA) * smoothed_1 + ALPHA * (float)p1;
    }
  }
  
  // Traiter plante 2
  if (p2 > 0) {
    if (first_2) {
      smoothed_2 = p2;
      first_2 = false;
    } else {
      smoothed_2 = (1.0 - ALPHA) * smoothed_2 + ALPHA * (float)p2;
    }
  }
  
  // Calculer les fréquences
  float freq_1 = (smoothed_1 > 0) ? 1000000.0 / smoothed_1 : 0;
  float freq_2 = (smoothed_2 > 0) ? 1000000.0 / smoothed_2 : 0;
  
  // Affichage
  Serial.print("P1: ");
  Serial.print(freq_1, 1);
  Serial.print(" Hz (");
  Serial.print(c1);
  Serial.print(" pulses)  |  P2: ");
  Serial.print(freq_2, 1);
  Serial.print(" Hz (");
  Serial.print(c2);
  Serial.println(" pulses)");
  
  delay(200);
}