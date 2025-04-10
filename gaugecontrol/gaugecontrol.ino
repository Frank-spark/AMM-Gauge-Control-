#include <Arduino.h>
#include "driver/ledc.h"  // Required for ESP32 Core v3.0.0+
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <DNSServer.h>
#include "webui.h"

// Pin Definitions
#define MOSFET_PWM_PIN 25   // Controls gauge needle
#define BACKLIGHT_PIN 26    // Blinks when level is low
#define PS_CONTROL_PIN 27    // 0-5V control for power supply
#define FLOAT_SENSOR_PIN 34  // Reads float sensor voltage
#define H2_SIGNAL_PIN 35     // Input from H2 board coil plug (12V)
#define I2C_SDA_PIN 21        // I2C Data (reserved for future use)
#define I2C_SCL_PIN 22        // I2C Clock (reserved for future use)
#define watch_dog 33          // 10ms 

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
#define KNOWN_RESISTOR 100  // Changed from 10000 to 100 ohms

// PWM Range for Gauge
#define MIN_DUTY 140         // Empty
#define MAX_DUTY 255         // Full
#define BLINK_THRESHOLD 10  // If PWM < 150, blink BACKLIGHT_PIN
#define LOW_LEVEL_THRESHOLD 145  // Cut power if below this

#define BLINK_INTERVAL 250   // Blink time in milliseconds
#define NUM_SAMPLES 10       // Number of ADC samples for noise filtering
#define STEP_DELAY_MS 20     // Delay between power ramp steps (ms)

// Add these variables for watchdog timing
unsigned long lastWatchdogTime = 0;
#define WATCHDOG_INTERVAL 10   // 10ms interval

unsigned long lastBlinkTime = 0;
bool blinkState = false;
float smoothedDuty = MIN_DUTY; // Start at lowest gauge position

bool pem_running = false;
uint8_t current_ramp_value = 0;

// Increased smoothing factor for faster response
#define NORMAL_SMOOTHING 0.05  // Regular smoothing (5% of new value)
#define RECOVERY_SMOOTHING 0.25 // Faster smoothing when recovering from low level (25% of new value)
bool wasInLowLevel = false;    // Track if we were previously in low level

// WiFi Configuration
#define AP_SSID "GaugeControl"
#define AP_PASSWORD "gaugeconfig"
#define DNS_PORT 53

// Web server and DNS server
AsyncWebServer server(80);
DNSServer dnsServer;
Preferences preferences;

// WiFi mode flags
bool apMode = false;

// Configuration structure
struct Config {
  int maxResistance = 240;
  int minResistance = 30;
  int knownResistor = 100;
  int minDuty = 140;
  int maxDuty = 255;
  int blinkThreshold = 10;
  int lowLevelThreshold = 145;
  int blinkInterval = 250;
  int numSamples = 10;
  int stepDelay = 20;
  char wifiSSID[32] = "";
  char wifiPassword[64] = "";
  int pemMaxValue = 255;
  int pemStepSize = 1;
};

Config config;

// Variables to store current values for the web interface
float currentResistance = 0;
int currentTargetDuty = 0;

// Replace fixed #define values with variables
// Keep the #define for default values
#define DEFAULT_MAX_RESISTANCE 240
#define DEFAULT_MIN_RESISTANCE 30
#define DEFAULT_KNOWN_RESISTOR 100
#define DEFAULT_MIN_DUTY 140
#define DEFAULT_MAX_DUTY 255
#define DEFAULT_BLINK_THRESHOLD 10
#define DEFAULT_LOW_LEVEL_THRESHOLD 145
#define DEFAULT_BLINK_INTERVAL 250
#define DEFAULT_NUM_SAMPLES 10
#define DEFAULT_STEP_DELAY_MS 20

// Add default values
#define DEFAULT_PEM_MAX_VALUE 255
#define DEFAULT_PEM_STEP_SIZE 1

// Variables that will be modified at runtime
int maxResistance = DEFAULT_MAX_RESISTANCE;
int minResistance = DEFAULT_MIN_RESISTANCE;
int knownResistor = DEFAULT_KNOWN_RESISTOR;
int minDuty = DEFAULT_MIN_DUTY;
int maxDuty = DEFAULT_MAX_DUTY;
int blinkThreshold = DEFAULT_BLINK_THRESHOLD;
int lowLevelThreshold = DEFAULT_LOW_LEVEL_THRESHOLD;
int blinkInterval = DEFAULT_BLINK_INTERVAL;
int numSamples = DEFAULT_NUM_SAMPLES;
int stepDelayMs = DEFAULT_STEP_DELAY_MS;

// Add runtime variables
int pemMaxValue = DEFAULT_PEM_MAX_VALUE;
int pemStepSize = DEFAULT_PEM_STEP_SIZE;

// Add this near the other global variables
bool watchdogEnabled = false;
unsigned long bootTime = 0;
#define WATCHDOG_ENABLE_DELAY 5000 // 5 seconds delay before enabling watchdog

// Add this to your global variables
#define WIFI_CONNECT_TIMEOUT_MS 10000 // 10 second timeout for WiFi connection
hw_timer_t * watchdogTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Add this function to handle the hardware timer interrupt
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  digitalWrite(watch_dog, !digitalRead(watch_dog)); // Toggle the watchdog pin
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Gauge Control System Starting...");
    
    // Record boot time
    bootTime = millis();
    
    // Configure watchdog pin as output and set it HIGH immediately to prevent reset
    pinMode(watch_dog, OUTPUT);
    digitalWrite(watch_dog, HIGH);
    
    // Reserve I2C pins for future use
    pinMode(I2C_SDA_PIN, INPUT);
    pinMode(I2C_SCL_PIN, INPUT);
    
    // Initialize WiFi and web server
    initWifi();

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
    // Handle DNS if in AP mode
    if (apMode) {
        dnsServer.processNextRequest();
    }
    
    // Handle watchdog signal
    updateWatchdog();

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
    currentResistance = resistance; // Store for web interface
    
    // Convert resistance to PWM duty cycle
    int targetDuty = map(resistance, maxResistance, minResistance, minDuty, maxDuty);
    currentTargetDuty = targetDuty; // Store for web interface
    
    // Use faster recovery when coming back from low level
    float smoothingFactor = (wasInLowLevel && targetDuty > BLINK_THRESHOLD) ? 
                            RECOVERY_SMOOTHING : NORMAL_SMOOTHING;
    
    // Smooth gauge movement with adaptive smoothing
    smoothedDuty = (smoothingFactor * targetDuty) + ((1.0 - smoothingFactor) * smoothedDuty);

    // Apply smoothed PWM duty cycle to gauge
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, (int)smoothedDuty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL);

    // Track if we are/were in low level state
    bool isInLowLevel = ((int)smoothedDuty < BLINK_THRESHOLD);
    wasInLowLevel = isInLowLevel;

    // Handle low fill level conditions
    manageSafety((int)smoothedDuty);

    // Debug Output (shortened for simplicity)
    Serial.print("R: ");
    Serial.print(resistance);
    Serial.print(" | T: ");
    Serial.print(targetDuty);
    Serial.print(" | S: ");
    Serial.println((int)smoothedDuty);

    delay(50);
}

void startPEM() {
    Serial.println("Starting PEM - Ramping power supply");
    pem_running = true;
    
    // Ramp from 0 to max value using step size
    for (current_ramp_value = 0; current_ramp_value <= pemMaxValue; current_ramp_value += pemStepSize) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL, current_ramp_value);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL);
        delay(stepDelayMs);
    }
    
    // Ensure we reach exactly the max value
    if (current_ramp_value > pemMaxValue) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL, pemMaxValue);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL);
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
    float resistance = (voltage / (3.3 - voltage)) * knownResistor;
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
    if (pwmDuty < blinkThreshold) {
        if (currentMillis - lastBlinkTime >= blinkInterval) {
            lastBlinkTime = currentMillis;
            blinkState = !blinkState;
            digitalWrite(BACKLIGHT_PIN, blinkState);
        }
    } else {
        // Reset blinking state and set backlight HIGH when above threshold
        blinkState = false;
        digitalWrite(BACKLIGHT_PIN, HIGH);
    }

    // If level is too low, shut down power supply by setting control voltage to 0
    if (pwmDuty < lowLevelThreshold) {
        Serial.println("Low level detected! Shutting down power supply.");
        ledc_set_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL);
        pem_running = false;
    }
}

void blinkBacklight() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkTime >= blinkInterval) {
        lastBlinkTime = currentMillis;
        blinkState = !blinkState;
        digitalWrite(BACKLIGHT_PIN, blinkState);
    }
}

// Update the watchdog function
void updateWatchdog() {
    unsigned long currentMillis = millis();
    
    // Only enable watchdog after a safe boot period
    if (!watchdogEnabled && (currentMillis - bootTime > WATCHDOG_ENABLE_DELAY)) {
        watchdogEnabled = true;
        Serial.println("Watchdog activated");
    }
    
    // Simply toggle the watchdog pin at the required interval
    if (watchdogEnabled && (currentMillis - lastWatchdogTime >= WATCHDOG_INTERVAL)) {
        lastWatchdogTime = currentMillis;
        digitalWrite(watch_dog, !digitalRead(watch_dog)); // Toggle
    }
}

void initWifi() {
  preferences.begin("gauge-config", false);
  
  // Start a hardware timer to handle watchdog during WiFi connection
  watchdogTimer = timerBegin(0, 80, true); // Timer 0, divider 80 (1MHz)
  timerAttachInterrupt(watchdogTimer, &onTimer, true);
  timerAlarmWrite(watchdogTimer, 5000, true); // 5ms toggle (10ms period)
  timerAlarmEnable(watchdogTimer);

  // Load stored WiFi credentials
  if (preferences.getString("wifiSSID", "").length() > 0) {
    String ssid = preferences.getString("wifiSSID", "");
    String password = preferences.getString("wifiPassword", "");
    
    ssid.toCharArray(config.wifiSSID, sizeof(config.wifiSSID));
    password.toCharArray(config.wifiPassword, sizeof(config.wifiPassword));
    
    // Try to connect to stored WiFi with timeout
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifiSSID, config.wifiPassword);
    
    Serial.print("Connecting to WiFi");
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_CONNECT_TIMEOUT_MS) {
      delay(100); // Shorter delay
      Serial.print(".");
      
      // Keep watchdog happy manually in addition to the timer
      digitalWrite(watch_dog, HIGH);
      delayMicroseconds(100);
      digitalWrite(watch_dog, LOW);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.print("Connected to WiFi. IP: ");
      Serial.println(WiFi.localIP());
      apMode = false;
    } else {
      WiFi.disconnect();
      startAPMode();
    }
  } else {
    startAPMode();
  }
  
  // Disable the hardware timer once WiFi is connected
  timerAlarmDisable(watchdogTimer);
  timerDetachInterrupt(watchdogTimer);
  timerEnd(watchdogTimer);
  
  // Load other configuration values
  loadConfig();
  
  // Initialize web server
  initWebServer();
}

void startAPMode() {
  Serial.println("Starting AP mode");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // Start DNS server to redirect all domains to the captive portal
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  
  apMode = true;
}

void loadConfig() {
  config.maxResistance = preferences.getInt("maxResistance", DEFAULT_MAX_RESISTANCE);
  config.minResistance = preferences.getInt("minResistance", DEFAULT_MIN_RESISTANCE);
  config.knownResistor = preferences.getInt("knownResistor", DEFAULT_KNOWN_RESISTOR);
  config.minDuty = preferences.getInt("minDuty", DEFAULT_MIN_DUTY);
  config.maxDuty = preferences.getInt("maxDuty", DEFAULT_MAX_DUTY);
  config.blinkThreshold = preferences.getInt("blinkThreshold", DEFAULT_BLINK_THRESHOLD);
  config.lowLevelThreshold = preferences.getInt("lowLevelThreshold", DEFAULT_LOW_LEVEL_THRESHOLD);
  config.blinkInterval = preferences.getInt("blinkInterval", DEFAULT_BLINK_INTERVAL);
  config.numSamples = preferences.getInt("numSamples", DEFAULT_NUM_SAMPLES);
  config.stepDelay = preferences.getInt("stepDelay", DEFAULT_STEP_DELAY_MS);
  config.pemMaxValue = preferences.getInt("pemMaxValue", DEFAULT_PEM_MAX_VALUE);
  config.pemStepSize = preferences.getInt("pemStepSize", DEFAULT_PEM_STEP_SIZE);
  
  // Update the runtime variables with loaded configuration
  maxResistance = config.maxResistance;
  minResistance = config.minResistance;
  knownResistor = config.knownResistor;
  minDuty = config.minDuty;
  maxDuty = config.maxDuty;
  blinkThreshold = config.blinkThreshold;
  lowLevelThreshold = config.lowLevelThreshold;
  blinkInterval = config.blinkInterval;
  numSamples = config.numSamples;
  stepDelayMs = config.stepDelay;
  pemMaxValue = config.pemMaxValue;
  pemStepSize = config.pemStepSize;
}

void saveConfig() {
  preferences.putInt("maxResistance", config.maxResistance);
  preferences.putInt("minResistance", config.minResistance);
  preferences.putInt("knownResistor", config.knownResistor);
  preferences.putInt("minDuty", config.minDuty);
  preferences.putInt("maxDuty", config.maxDuty);
  preferences.putInt("blinkThreshold", config.blinkThreshold);
  preferences.putInt("lowLevelThreshold", config.lowLevelThreshold);
  preferences.putInt("blinkInterval", config.blinkInterval);
  preferences.putInt("numSamples", config.numSamples);
  preferences.putInt("stepDelay", config.stepDelay);
  preferences.putInt("pemMaxValue", config.pemMaxValue);
  preferences.putInt("pemStepSize", config.pemStepSize);
  preferences.putString("wifiSSID", config.wifiSSID);
  preferences.putString("wifiPassword", config.wifiPassword);
  
  // Update the runtime variables with saved configuration
  maxResistance = config.maxResistance;
  minResistance = config.minResistance;
  knownResistor = config.knownResistor;
  minDuty = config.minDuty;
  maxDuty = config.maxDuty;
  blinkThreshold = config.blinkThreshold;
  lowLevelThreshold = config.lowLevelThreshold;
  blinkInterval = config.blinkInterval;
  numSamples = config.numSamples;
  stepDelayMs = config.stepDelay;
  pemMaxValue = config.pemMaxValue;
  pemStepSize = config.pemStepSize;
}

void initWebServer() {
  // Serve the main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });
  
  // Get current configuration
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = getConfigJSON();
    request->send(200, "application/json", jsonResponse);
  });
  
  // Update configuration
  server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Handle empty POST
    request->send(200, "application/json", "{\"status\":\"error\",\"message\":\"No data provided\"}");
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, data, len);
    
    if (error) {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
      return;
    }
    
    // Update configuration
    config.maxResistance = doc["maxResistance"] | 240;
    config.minResistance = doc["minResistance"] | 30;
    config.knownResistor = doc["knownResistor"] | 100;
    config.minDuty = doc["minDuty"] | 140;
    config.maxDuty = doc["maxDuty"] | 255;
    config.blinkThreshold = doc["blinkThreshold"] | 10;
    config.lowLevelThreshold = doc["lowLevelThreshold"] | 145;
    config.blinkInterval = doc["blinkInterval"] | 250;
    config.numSamples = doc["numSamples"] | 10;
    config.stepDelay = doc["stepDelay"] | 20;
    config.pemMaxValue = doc["pemMaxValue"] | 255;
    config.pemStepSize = doc["pemStepSize"] | 1;
    
    // Handle WiFi credentials
    if (doc.containsKey("wifiSSID") && doc["wifiSSID"].as<String>().length() > 0) {
      strncpy(config.wifiSSID, doc["wifiSSID"].as<String>().c_str(), sizeof(config.wifiSSID) - 1);
      config.wifiSSID[sizeof(config.wifiSSID) - 1] = '\0';
    }
    
    if (doc.containsKey("wifiPassword")) {
      strncpy(config.wifiPassword, doc["wifiPassword"].as<String>().c_str(), sizeof(config.wifiPassword) - 1);
      config.wifiPassword[sizeof(config.wifiPassword) - 1] = '\0';
    }
    
    // Save configuration
    saveConfig();
    
    request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Configuration saved\"}");
  });
  
  // Reset to defaults
  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Reset configuration to defaults
    config.maxResistance = 240;
    config.minResistance = 30;
    config.knownResistor = 100;
    config.minDuty = 140;
    config.maxDuty = 255;
    config.blinkThreshold = 10;
    config.lowLevelThreshold = 145;
    config.blinkInterval = 250;
    config.numSamples = 10;
    config.stepDelay = 20;
    config.pemMaxValue = 255;
    config.pemStepSize = 1;
    // Don't reset WiFi credentials
    
    saveConfig();
    
    String jsonResponse = getConfigJSON();
    request->send(200, "application/json", jsonResponse);
  });
  
  // Get live data
  server.on("/livedata", HTTP_GET, [](AsyncWebServerRequest *request) {
    char jsonBuffer[128];
    snprintf(jsonBuffer, sizeof(jsonBuffer), 
        "{\"resistance\":%.1f,\"targetDuty\":%d,\"smoothedDuty\":%d,\"pemRunning\":%s}", 
        currentResistance, currentTargetDuty, (int)smoothedDuty, pem_running ? "true" : "false");
    request->send(200, "application/json", jsonBuffer);
  });
  
  // Handle not found - redirect to root
  server.onNotFound([](AsyncWebServerRequest *request){
    request->redirect("/");
  });
  
  // Start server
  server.begin();
}

String getConfigJSON() {
  DynamicJsonDocument doc(1024);
  
  doc["maxResistance"] = config.maxResistance;
  doc["minResistance"] = config.minResistance;
  doc["knownResistor"] = config.knownResistor;
  doc["minDuty"] = config.minDuty;
  doc["maxDuty"] = config.maxDuty;
  doc["blinkThreshold"] = config.blinkThreshold;
  doc["lowLevelThreshold"] = config.lowLevelThreshold;
  doc["blinkInterval"] = config.blinkInterval;
  doc["numSamples"] = config.numSamples;
  doc["stepDelay"] = config.stepDelay;
  doc["pemMaxValue"] = config.pemMaxValue;
  doc["pemStepSize"] = config.pemStepSize;
  doc["wifiSSID"] = config.wifiSSID;
  doc["wifiPassword"] = ""; // Don't send the password to the client
  
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}
