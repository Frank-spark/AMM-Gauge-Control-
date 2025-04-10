---
## **Project: ESP32 Gauge Control with H2 Integration**

### **Overview**
This project controls an **analog gauge** and **H2 power supply** using an **ESP32-WROOM-32D**. The gauge reflects the **fill level of a tank** based on readings from a **resistance-based float sensor (240Ω - 30Ω)**. The system implements a **soft-start feature** for H2 power supplies to prevent inrush current, and includes a **web configuration interface** for easy parameter adjustment.

### **Features**
✔ Reads **float sensor resistance** and converts it to gauge display using **PWM control**  
✔ **Smooths the gauge movement** to prevent needle shaking  
✔ **Blinks a backlight** when the fill level is low  
✔ **Soft-start control** (0-5V ramping) for H2 power supplies  
✔ **H2 signal detection** from existing H2 board  
✔ Uses **noise-filtered ADC readings** for reliable measurements  
✔ **Web-based configuration interface** for parameter adjustment  
✔ **WiFi connectivity** with fallback AP mode for setup  
✔ **Watchdog signal generation** (10ms interval)  
✔ **12V to 3.3V Buck Converter** power for ESP32  
✔ **12V to 5V Buck Converter** power for 5V components  

---

## **1. Hardware Setup**
### ** Components Needed**
- **ESP32-WROOM-32D**
- **Float Sensor (240Ω - 30Ω)**
- **10kΩ Pull-up Resistor** (for voltage divider)
- **Gauge (0-100% Analog Display)**
- **IRLB8721 MOSFET** (for PWM gauge control)
- **Level Shifter** (for 12V H2 signal input)
- **0-5V DAC/PWM Filter** (for power supply control)
- **330Ω Resistor + LED** (for backlight warning)
- **4-pin I2C Connector** (for future expansion)
- **2x 4.7kΩ Pull-up Resistors** (for I2C)
- **Watchdog Circuit** (connected to Pin 33)

### ** Wiring Guide**
#### **1️ Power Supply**
| Component | Connection |
|-----------|------------|
| ESP32 VIN | **12V from H2 board gauge light** |
| ESP32 GND | **Common Ground** |
 12V to 5V Buck Converters:

#### **2️ Float Sensor (Resistance Measurement)**
| Pin | Connection |
|-----|------------|
| **ESP32 GPIO 34** | **Voltage divider output** |
| **3.3V** | **10kΩ pull-up resistor** → Float Sensor |
| **GND** | **Float Sensor Ground** |

#### **3️ Gauge Control (PWM Output)**
| Pin | Connection |
|-----|------------|
| **ESP32 GPIO 25** | **MOSFET Gate (IRLB8721)** |
| **Gauge (-)** | **MOSFET Drain** |
| **GND** | **MOSFET Source** |

#### **4️ Power Supply Control**
| Pin | Connection |
|-----|------------|
| **ESP32 GPIO 27** | **PWM Filter** → Power Supply Control |
| **Power Supply CC** | **0-5V Control Input** |

#### **5️ H2 Signal Input**
| Pin | Connection |
|-----|------------|
| **ESP32 GPIO 35** | **Level Shifter Output** |
| **Level Shifter Input** | **H2 Board Coil Signal (12V)** |

#### **6️ Backlight Warning**
| Pin | Connection |
|-----|------------|
| **ESP32 GPIO 26** | **LED via 330Ω Resistor** |
| **GND** | **LED Cathode (-)** |

#### **7️ I2C Expansion Port**
| Pin | Connection |
|-----|------------|
| **1** | **3.3V** |
| **2** | **GPIO 21 (SDA) with 4.7kΩ pull-up** |
| **3** | **GPIO 22 (SCL) with 4.7kΩ pull-up** |
| **4** | **GND** |

#### **8️ Watchdog Signal Output**
| Pin | Connection |
|-----|------------|
| **ESP32 GPIO 33** | **Watchdog Circuit Input** |

---

## **Power Supply Selection**

This project requires efficient **DC-DC buck converters** to step down **12V** to the required operating voltages (**5V** and **3.3V**). Below are the recommended surface-mount buck converters:

### **12V to 5V Buck Converters**
1. **Texas Instruments LM2675**  
   - Step-down voltage regulator with **1A** output.  
   - High efficiency, low component count.  
   - Ideal for powering microcontrollers and peripherals.  
   - [Datasheet](https://www.ti.com/lit/ds/symlink/lm2675.pdf)  

2. **Murata Power Solutions OKI-78SR-5/1.5-W36H-C**  
   - Drop-in replacement for traditional **78xx linear regulators**.  
   - Delivers **5V at up to 1.5A** with minimal heat dissipation.  
   - [Datasheet](https://power.murata.com/pub/data/power/oki-78sr.pdf)  

### **12V to 3.3V Buck Converters**
1. **Texas Instruments TPS54331**  
   - Integrated **3A** step-down regulator with **28V input capability**.  
   - Optimized for high efficiency in embedded systems.  
   - [Datasheet](https://www.ti.com/lit/ds/symlink/tps54331.pdf)  

2. **Analog Devices LTC3621**  
   - **1A synchronous buck converter** with **17V input capability**.  
   - Compact and energy-efficient for low-power applications.  
   - [Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/LTC3621.pdf)  

### **Selection Considerations**
- **Current Requirements**: Choose a regulator based on the total current draw of your circuits.  
- **Efficiency**: Higher efficiency reduces heat and improves system stability.  
- **Footprint**: Ensure the selected components fit within the PCB layout.  
- **Availability**: Check supplier stock (e.g., [Digi-Key](https://www.digikey.com/) or [Mouser](https://www.mouser.com/)) before finalizing.  


## **2. Code Explanation**
### **⚙ Main Functionalities**
#### **1️ Setup Function**
- Initializes **PWM control for gauge and power supply**
- Sets up **GPIOs for backlight and H2 signal input**
- Starts **watchdog signal generation**
- Initializes **WiFi and web configuration server**

#### **2️ `loop()` Function**
- Monitors **H2 signal** for power control
- Reads **float sensor resistance**
- Controls **gauge display**
- Manages **power supply ramping**
- Handles **low-level warnings**
- Updates **watchdog signal**
- Processes **DNS requests** (in AP mode)

#### **3️ Power Supply Control**
```cpp
void startPEM() {
    // Ramps power supply control from 0V to max value
    for (current_ramp_value = 0; current_ramp_value <= pemMaxValue; current_ramp_value += pemStepSize) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL, current_ramp_value);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL);
        delay(stepDelayMs);
    }
}
```

#### **4️ Read Float Sensor Resistance**
```cpp
float readFloatSensorResistance(int pin) {
    int adcValue = readSmoothADC(pin);
    float voltage = (adcValue / 4095.0) * 3.3;  

    if (voltage <= 0.1) return MAX_RESISTANCE;  

    float resistance = (voltage / (3.3 - voltage)) * knownResistor;
    return resistance;
}
```
- Uses **Ohm's Law** to convert voltage readings to **resistance (240Ω - 30Ω)**.
- **Failsafe:** If voltage is **too low**, assumes an **empty tank (240Ω).**

#### **5️ Smooth ADC Readings**
```cpp
int readSmoothADC(int pin) {
    int total = 0;
    for (int i = 0; i < numSamples; i++) {
        total += analogRead(pin);
        delayMicroseconds(500);
    }
    return total / numSamples;
}
```
- Averages **multiple ADC samples** to **reduce noise**.

#### **6️ Safety Handling**
- **Flashes backlight** when fill level is low
- **Shuts down power supply** by setting control voltage to 0V if level is critically low

#### **7️ Watchdog Signal Generation**
```cpp
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
```
- Generates a **10ms toggle signal** on Pin 33
- Delayed activation allows for **safe boot sequence**

---

## **3.1 Using the Web Configuration Interface**

### **Interface Layout and Navigation**
The web interface is organized into several sections for easy configuration:



1. **Header Section**: Shows the title "Gauge Control Configuration"
2. **Configuration Panels**: Grouped by function (Resistance, PWM, Timing, PEM, WiFi)
3. **Control Buttons**: Save configuration and reset to defaults
4. **Live Monitor**: Real-time data display from the gauge controller
5. **Status Messages**: Appear at the bottom when actions are performed

### **Step-by-Step Configuration Guide**

#### **Resistance Settings Adjustment**
These settings affect how the float sensor readings are interpreted:

1. **Max Resistance (Empty Tank)**: 
   - Default: 240Ω
   - Increase this value if your tank shows "empty" too early
   - Decrease if empty indication happens too late
   - Typical range: 200-300Ω

2. **Min Resistance (Full Tank)**:
   - Default: 30Ω
   - Increase if your tank shows "full" too soon
   - Decrease if the gauge never reaches full
   - Typical range: 20-50Ω

3. **Known Resistor**:
   - Default: 100Ω
   - This is the physical pull-up resistor value in your circuit
   - Only change if you've used a different resistor than specified

#### **PWM Settings Adjustment**
These settings control the gauge needle position:

1. **Min Duty (Empty)**: 
   - Default: 140
   - Controls gauge position when tank is empty
   - Lower value = gauge reads lower when empty
   - Adjust until gauge shows empty correctly

2. **Max Duty (Full)**:
   - Default: 255
   - Controls gauge position when tank is full
   - Higher value = gauge reads higher when full
   - Adjust until gauge shows full correctly

3. **Blink Threshold**:
   - Default: 10
   - Sets level for backlight blinking
   - Increase to start warning earlier (when tank is less empty)
   - Decrease to delay warning (when tank is more empty)

4. **Low Level Threshold**:
   - Default: 145
   - Below this level, power is cut to protect system
   - Should be higher than Blink Threshold but lower than normal operation

#### **PEM Startup Settings**
These settings control the power supply ramp behavior:

1. **Ramp Max Value**: 
   - Default: 255 (maximum PWM)
   - Controls final voltage sent to power supply (0-5V range)
   - Lower for reduced power output
   - Range: 0-255 (0-5V output)

2. **Ramp Step Size**:
   - Default: 1
   - Controls how quickly the voltage ramps up
   - Higher values = faster ramp (less smooth)
   - Lower values = slower ramp (smoother)
   - Recommended range: 1-5

3. **Step Delay**:
   - Default: 20ms
   - Time between voltage steps
   - Longer delay = slower, gentler startup
   - Shorter delay = faster startup
   - Total ramp time = (Max Value ÷ Step Size) × Step Delay

#### **WiFi Settings**
Configure network connectivity:

1. **WiFi SSID**: 
   - Enter your home/office network name
   - Leave blank to remain in AP mode

2. **WiFi Password**:
   - Enter password for your network
   - Will be securely stored in ESP32 memory

### **Using the Live Monitor**
The live monitor shows real-time system status:

- **Resistance**: Current reading from the float sensor (in ohms)
- **Target Duty**: Raw PWM value calculated from resistance
- **Smoothed Duty**: Actual PWM value sent to gauge (after smoothing)
- **PEM Status**: Shows if the power supply is running (green) or stopped (red)

### **Common Adjustments**

1. **Calibrating the Gauge**:
   - With tank empty: Adjust Min Duty until gauge reads empty
   - With tank full: Adjust Max Duty until gauge reads full
   - Fine-tune Min/Max Resistance if needed

2. **Adjusting Response Time**:
   - For faster gauge response: Decrease NORMAL_SMOOTHING in code
   - For smoother movement: Increase NORMAL_SMOOTHING in code
   - Default smoothing values provide good balance

3. **Customizing Power Ramp**:
   - For slower, gentler startup: Increase Step Delay and/or decrease Step Size
   - For faster startup: Decrease Step Delay and/or increase Step Size
   - For lower maximum power: Reduce Max Value

### **Saving and Resetting**

- **Save Configuration**: Click to store all settings in non-volatile memory
- **Reset to Defaults**: Reverts all settings to factory defaults
- Settings persist across power cycles once saved

### **Troubleshooting the Interface**

- **Changes not taking effect**: Ensure you clicked "Save Configuration"
- **Interface not loading**: Check WiFi connection or AP mode
- **Values resetting**: Possible memory corruption; try resetting to defaults
- **Gauge not responding to changes**: Check physical connections

## **3.2 Connecting to the Access Point**

### **Access Point Mode**
When the ESP32 can't connect to a configured WiFi network or no network is configured, it automatically starts in Access Point (AP) mode:

1. **AP Name**: `GaugeControl`
2. **AP Password**: `gaugeconfig`
3. **AP IP Address**: `192.168.4.1`

### **Connection Process**
1. **Power on the ESP32** controller
2. **Wait for initialization** (approximately 5 seconds)
3. **On your phone or computer**:
   - Open WiFi settings
   - Look for the `GaugeControl` network
   - Connect using password `gaugeconfig`
4. **Open a web browser** and navigate to:
   - `http://192.168.4.1`
   - The configuration interface should load automatically

### **Captive Portal**
The system uses a captive portal feature that will:
- Redirect all web requests to the configuration page
- Work similarly to hotel WiFi login pages
- On some devices, a login prompt may appear automatically

### **Station Mode (After Configuration)**
After configuring WiFi credentials and connecting to your network:
1. The ESP32 will connect to your WiFi network
2. The IP address is assigned by your router (DHCP)
3. Check your router's connected devices list to find the assigned IP
4. The serial monitor also displays the assigned IP address at startup

### **Switching Back to AP Mode**
If you need to reconfigure WiFi settings:
1. Power cycle the device 3 times in quick succession OR
2. If previously configured WiFi is unavailable, AP mode starts automatically

### **Troubleshooting Connection Issues**
- **AP not appearing**: Wait 30 seconds after power-on, then try again
- **Can't connect to AP**: Ensure you're using password `gaugeconfig`
- **Can't access 192.168.4.1**: Try disabling mobile data on your phone
- **Connection drops**: Stay within 15 feet (5 meters) of the ESP32

## **4. Expected Behavior**
| **Event** | **Action** | **Response** |
|-----------|------------|--------------|
| **H2 Signal ON** | 12V on GPIO 35 | Start power supply ramp (configurable) |
| **H2 Signal OFF** | 0V on GPIO 35 | Immediate power supply shutdown |
| **Low Tank Level** | PWM < configurable threshold | Backlight starts blinking |
| **Normal Operation** | PWM > threshold | Normal gauge display |
| **Watchdog** | Continuous | 10ms toggle signal on Pin 33 |
| **Power on** | Boot sequence | AP mode if no WiFi configured |

---

## **5. Installation & Usage**
### ** Uploading Code**
1. Open **Arduino IDE**.
2. Install **ESP32 Board Support** via **Boards Manager**.
3. Install required libraries:
   - **ESPAsyncWebServer**
   - **AsyncTCP**
   - **ArduinoJson**
4. Connect **ESP32-WROOM-32D** to USB.
5. Upload `gaugecontrol.ino` and `webui.h`.

### **🔧 Adjusting Settings**
All settings can be adjusted via the web interface. Default values are:
```cpp
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
#define DEFAULT_PEM_MAX_VALUE 255
#define DEFAULT_PEM_STEP_SIZE 1
```

---

## **6. Troubleshooting**
| **Issue** | **Possible Fix** |
|-----------|------------------|
| **Power supply not ramping** | Check PWM filter circuit and 0-5V output |
| **H2 signal not detected** | Verify level shifter operation and 12V input |
| **Gauge not moving** | Check MOSFET and PWM output |
| **Backlight issues** | Verify LED connections and GPIO 26 |
| **Repeated resets** | Check watchdog circuit and delay settings |
| **Can't access web interface** | Verify WiFi connection or AP mode |

---

## **7. Future Improvements**
🔹 Add **feedback monitoring** from power supply  
🔹 Implement **communication with H2 board**  
🔹 Add **diagnostic LED indicators**  
🔹 Add **I2C sensors or displays**  
🔹 Implement **I2C communication with external devices**  
🔹 Add **OTA firmware updates**  
🔹 Implement **data logging** to monitor system performance  

---

### **Final Notes**
- **This firmware controls an analog gauge, blinks a backlight for warnings, and manages a power supply ramp.**  
- **It provides a web interface for configuration and allows for WiFi connectivity.**
- **The watchdog implementation ensures reliability with external monitoring circuits.**
- **Ensure correct wiring for all circuits to prevent damage.**
