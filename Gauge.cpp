#include <Arduino.h>
#include "driver/ledc.h"  // Required for ESP32 Core v3.0.0+

// Pin Definitions
#define MOSFET_PWM_PIN 25   // Controls gauge needle
#define BACKLIGHT_PIN 26    // Blinks when level is low
#define KILL_PIN 27         // Cuts power when level is low or temp is high
#define LEVEL_SENSOR_PIN 34 // Analog input for tank fill level (Now using GPIO 34)
#define THERMISTOR_PIN 35   // Analog input for thermistor

// PWM Configuration
#define PWM_FREQUENCY 5000  // PWM frequency in Hz
#define PWM_RESOLUTION LEDC_TIMER_10_BIT  // 10-bit for smooth gauge control
#define PWM_CHANNEL LEDC_CHANNEL_0

// Fill Level Ranges
#define MIN_DUTY 140         // Minimum PWM duty cycle (Empty)
#define MAX_DUTY 225         // Maximum PWM duty cycle (Full)
#define BLINK_THRESHOLD 150  // If PWM < 150, blink BACKLIGHT_PIN
#define LOW_LEVEL_THRESHOLD 145  // If below this, cut KILL_PIN power

// Temperature Settings
#define THERMISTOR_BETA 3950  // Beta coefficient (NTC 10kΩ B3950)
#define SERIES_RESISTOR 10000 // 10kΩ pull-up resistor
#define TEMP_THRESHOLD 60.0   // Cut off power if temp > 60°C

#define BLINK_INTERVAL 250   // Blink time in milliseconds
#define NUM_SAMPLES 10       // Number of ADC samples for noise filtering

unsigned long lastBlinkTime = 0;
bool blinkState = false;
float smoothedDuty = MIN_DUTY; // Start at the lowest PWM value

void setup() {
    Serial.begin(115200);

    // Configure PWM for Gauge Control
    ledc_timer_config_t timerConfig = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timerConfig);

    ledc_channel_config_t channelConfig = {
        .gpio_num = MOSFET_PWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0, // Initial duty cycle
        .hpoint = 0
    };
    ledc_channel_config(&channelConfig);

    // Setup Pins
    pinMode(BACKLIGHT_PIN, OUTPUT);
    pinMode(KILL_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, LOW);
    digitalWrite(KILL_PIN, HIGH);  // Start with power ON

    Serial.println("System Initialized");
}

void loop() {
    // Read analog inputs
    int levelADC = readSmoothADC(LEVEL_SENSOR_PIN);
    float inputVoltage = (levelADC / 4095.0) * 3.3;
    int targetDuty = map(levelADC, 0, 4095, MIN_DUTY, MAX_DUTY);

    // Smooth gauge movement
    smoothedDuty = (0.05 * targetDuty) + (0.95 * smoothedDuty);

    // Apply smoothed PWM duty cycle to gauge
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, (int)smoothedDuty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL);

    // Read thermistor temperature
    float temperature = readThermistor(THERMISTOR_PIN);

    // Handle low fill level and over-temp conditions
    manageSafety((int)smoothedDuty, temperature);

    // Debug Output
    Serial.print("Fill Level Voltage: ");
    Serial.print(inputVoltage, 3);
    Serial.print("V | PWM: ");
    Serial.print((int)smoothedDuty);
    Serial.print(" | Temp: ");
    Serial.print(temperature, 1);
    Serial.print("°C | Kill Pin: ");
    Serial.println(digitalRead(KILL_PIN) ? "ON" : "OFF");

    delay(50);
}

// Function to filter ADC noise (Moving Average)
int readSmoothADC(int pin) {
    int total = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        total += analogRead(pin);
        delayMicroseconds(500);
    }
    return total / NUM_SAMPLES;
}

// Function to read thermistor temperature
float readThermistor(int pin) {
    int adcValue = readSmoothADC(pin);
    float voltage = adcValue * (3.3 / 4095.0);
    float resistance = (SERIES_RESISTOR * voltage) / (3.3 - voltage);
    float temperature = 1.0 / ((log(resistance / SERIES_RESISTOR) / THERMISTOR_BETA) + (1.0 / 298.15)) - 273.15;
    return temperature;
}

// Function to manage low-level and over-temp conditions
void manageSafety(int pwmDuty, float temperature) {
    unsigned long currentMillis = millis();

    // Blink Backlight if below 150 PWM
    if (pwmDuty < BLINK_THRESHOLD) {
        if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
            lastBlinkTime = currentMillis;
            blinkState = !blinkState;
            digitalWrite(BACKLIGHT_PIN, blinkState);
        }
    } else {
        digitalWrite(BACKLIGHT_PIN, LOW);
    }

    // Cut Power if level is too low OR temperature too high
    if (pwmDuty < LOW_LEVEL_THRESHOLD || temperature > TEMP_THRESHOLD) {
        digitalWrite(KILL_PIN, LOW);
    } else {
        digitalWrite(KILL_PIN, HIGH);
    }
}
