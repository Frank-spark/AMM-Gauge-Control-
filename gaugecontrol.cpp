#include <Arduino.h>
#include "driver/ledc.h"  // Required for ESP32 Core v3.0.0+

// Pin Definitions
#define MOSFET_PWM_PIN 25   // Controls gauge needle
#define BACKLIGHT_PIN 26    // Blinks when level is low
#define PS_CONTROL_PIN 27    // 0-5V control for power supply
#define FLOAT_SENSOR_PIN 34  // Reads float sensor voltage
#define H2_SIGNAL_PIN 35     // Input from H2 board coil plug (12V)

// PWM Configuration
#define PWM_FREQUENCY 5000  // PWM frequency in Hz
#define PWM_RESOLUTION LEDC_TIMER_8_BIT  // 8-bit for gauge control
#define PWM_CHANNEL LEDC_CHANNEL_0

// PWM Configuration for Power Supply Control
#define PS_PWM_FREQ 10000
#define PS_PWM_RES LEDC_TIMER_8_BIT
#define PS_PWM_CHANNEL LEDC_CHANNEL_1

// Float Sensor Resistance Range
#define MAX_RESISTANCE 240  // Ohms (Empty Tank)
#define MIN_RESISTANCE 30   // Ohms (Full Tank)
#define KNOWN_RESISTOR 10000 // 10kO series resistor

// PWM Range for Gauge
#define MIN_DUTY 140         // Empty
#define MAX_DUTY 225         // Full
#define BLINK_THRESHOLD 150  // If PWM < 150, blink BACKLIGHT_PIN
#define LOW_LEVEL_THRESHOLD 145  // Cut power if below this

#define BLINK_INTERVAL 250   // Blink time in milliseconds
#define NUM_SAMPLES 10       // Number of ADC samples for noise filtering

unsigned long lastBlinkTime = 0;
bool blinkState = false;
float smoothedDuty = MIN_DUTY; // Start at lowest gauge position

bool pem_running = false;
uint8_t current_ramp_value = 0;

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

    // Configure PWM for Power Supply Control
    ledc_timer_config_t psTimerConfig = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PS_PWM_RES,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = PS_PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&psTimerConfig);

    ledc_channel_config_t psChannelConfig = {
        .gpio_num = PS_CONTROL_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = PS_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&psChannelConfig);

    // Setup Pins
    pinMode(BACKLIGHT_PIN, OUTPUT);
    pinMode(H2_SIGNAL_PIN, INPUT_PULLDOWN);
    digitalWrite(BACKLIGHT_PIN, LOW);

    // Initialize power supply control to 0V
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL);

    Serial.println("System Initialized");
}

void loop() {
    // Check H2 signal
    bool h2_signal = digitalRead(H2_SIGNAL_PIN);

    // Handle PEM start/stop based on H2 signal
    if (h2_signal && !pem_running) {
        startPEM();
    } else if (!h2_signal && pem_running) {
        stopPEM();
    }

    // Read float sensor resistance
    float resistance = readFloatSensorResistance(FLOAT_SENSOR_PIN);
    
    // Convert resistance to PWM duty cycle
    int targetDuty = map(resistance, MAX_RESISTANCE, MIN_RESISTANCE, MIN_DUTY, MAX_DUTY);
    
    // Smooth gauge movement
    smoothedDuty = (0.05 * targetDuty) + (0.95 * smoothedDuty);

    // Apply smoothed PWM duty cycle to gauge
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, (int)smoothedDuty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL);

    // Handle low fill level conditions
    manageSafety((int)smoothedDuty);

    // Debug Output
    Serial.print("Resistance: ");
    Serial.print(resistance);
    Serial.print("O | PWM: ");
    Serial.print((int)smoothedDuty);
    Serial.print(" | Kill Pin: ");
    Serial.println(digitalRead(KILL_PIN) ? "ON" : "OFF");

    delay(50);
}

void startPEM() {
    Serial.println("Starting PEM - Ramping power supply");
    pem_running = true;
    
    // Ramp from 0 to full over 5 seconds
    for (current_ramp_value = 0; current_ramp_value <= 255; current_ramp_value++) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL, current_ramp_value);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL);
        delay(STEP_DELAY_MS);
    }
}

void stopPEM() {
    Serial.println("Stopping PEM");
    pem_running = false;
    
    // Immediately set power supply control to 0
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL);
    current_ramp_value = 0;
}

// Function to read float sensor resistance using a voltage divider
float readFloatSensorResistance(int pin) {
    int adcValue = readSmoothADC(pin);
    float voltage = (adcValue / 4095.0) * 3.3;  // Convert ADC to voltage
    
    if (voltage <= 0.1) return MAX_RESISTANCE;  // Default to empty if sensor fails

    // Use Ohm's law: R_sensor = (V / (3.3 - V)) * Known_Resistor
    float resistance = (voltage / (3.3 - voltage)) * KNOWN_RESISTOR;
    return resistance;
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

// Function to manage low-level conditions
void manageSafety(int pwmDuty) {
    unsigned long currentMillis = millis();

    // Blink Backlight if below threshold
    if (pwmDuty < BLINK_THRESHOLD) {
        if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
            lastBlinkTime = currentMillis;
            blinkState = !blinkState;
            digitalWrite(BACKLIGHT_PIN, blinkState);
        }
    } else {
        digitalWrite(BACKLIGHT_PIN, LOW);
    }

    // If level is too low, shut down power supply by setting control voltage to 0
    if (pwmDuty < LOW_LEVEL_THRESHOLD) {
        Serial.println("Low level detected! Shutting down power supply.");
        ledc_set_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL);
        pem_running = false;
    }
}

void blinkBacklight() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
        lastBlinkTime = currentMillis;
        blinkState = !blinkState;
        digitalWrite(BACKLIGHT_PIN, blinkState);
    }
}
