---
## **Project: ESP32 Gauge Control with H2 Integration**

### **Overview**
This project controls an **analog gauge** and **H2 power supply** using an **ESP32-WROOM-32D**. The gauge reflects the **fill level of a tank** based on readings from a **resistance-based float sensor (240Ω - 30Ω)**. The system implements a **soft-start feature** for H2 power supplies to prevent inrush current.

### **Features**
✔ Reads **float sensor resistance** and converts it to gauge display using **PWM control**  
✔ **Smooths the gauge movement** to prevent needle shaking  
✔ **Blinks a backlight** when the fill level is low  
✔ **Soft-start control** (0-5V ramping) for H2 power supplies  
✔ **H2 signal detection** from existing H2 board  
✔ Uses **noise-filtered ADC readings** for reliable measurements

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

### ** Wiring Guide**
#### **1️ Power Supply**
| Component | Connection |
|-----------|------------|
| ESP32 VIN | **12V from H2 board gauge light** |
| ESP32 GND | **Common Ground** |

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

---

## **2. Code Explanation**
### **⚙ Main Functionalities**
#### **1️ Setup Function**
- Initializes **PWM control for gauge and power supply**
- Sets up **GPIOs for backlight and H2 signal input**

#### **2️ `loop()` Function**
- Monitors **H2 signal** for power control
- Reads **float sensor resistance**
- Controls **gauge display**
- Manages **power supply ramping**
- Handles **low-level warnings**

#### **3️ Power Supply Control**
```cpp
void startPEM() {
    // Ramps power supply control from 0V to 5V over 5 seconds
    for (current_ramp_value = 0; current_ramp_value <= 255; current_ramp_value++) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL, current_ramp_value);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, PS_PWM_CHANNEL);
        delay(STEP_DELAY_MS);
    }
}
```

#### **4️ Read Float Sensor Resistance**
```cpp
float readFloatSensorResistance(int pin) {
    int adcValue = readSmoothADC(pin);
    float voltage = (adcValue / 4095.0) * 3.3;  

    if (voltage <= 0.1) return MAX_RESISTANCE;  

    float resistance = (voltage / (3.3 - voltage)) * KNOWN_RESISTOR;
    return resistance;
}
```
- Uses **Ohm's Law** to convert voltage readings to **resistance (240Ω - 30Ω)**.
- **Failsafe:** If voltage is **too low**, assumes an **empty tank (240Ω).**

#### **5️ Smooth ADC Readings**
```cpp
int readSmoothADC(int pin) {
    int total = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        total += analogRead(pin);
        delayMicroseconds(500);
    }
    return total / NUM_SAMPLES;
}
```
- Averages **10 ADC samples** to **reduce noise**.

#### **6️ Safety Handling**
- **Flashes backlight** when fill level is low
- **Shuts down power supply** by setting control voltage to 0V if level is critically low

---

## **3. Expected Behavior**
| **Event** | **Action** | **Response** |
|-----------|------------|--------------|
| **H2 Signal ON** | 12V on GPIO 35 | Start 5-second power supply ramp |
| **H2 Signal OFF** | 0V on GPIO 35 | Immediate power supply shutdown |
| **Low Tank Level** | PWM < 150 | Backlight starts blinking |
| **Normal Operation** | PWM > 150 | Normal gauge display |

---

## **4. Installation & Usage**
### ** Uploading Code**
1. Open **Arduino IDE**.
2. Install **ESP32 Board Support** via **Boards Manager**.
3. Connect **ESP32-WROOM-32D** to USB.
4. Upload `gaugecontrol.cpp`.

### **🔧 Adjusting Settings**
- Change **gauge min/max values** in:
  ```cpp
  #define MIN_DUTY 140
  #define MAX_DUTY 225
  ```
- Adjust **low-level threshold**:
  ```cpp
  #define LOW_LEVEL_THRESHOLD 145
  ```
- Modify **safety cutoff temperature** (if thermistor is added later):
  ```cpp
  #define TEMP_THRESHOLD 60.0
  ```

---

## **5. Troubleshooting**
| **Issue** | **Possible Fix** |
|-----------|------------------|
| **Power supply not ramping** | Check PWM filter circuit and 0-5V output |
| **H2 signal not detected** | Verify level shifter operation and 12V input |
| **Gauge not moving** | Check MOSFET and PWM output |
| **Backlight issues** | Verify LED connections and GPIO 26 |

---

## **6. Future Improvements**
🔹 Add **feedback monitoring** from power supply  
🔹 Implement **communication with H2 board**  
🔹 Add **diagnostic LED indicators**

---

### **Final Notes**
- **This firmware controls an analog gauge, blinks a backlight for warnings, and cuts power when necessary.**  
- **It operates on an ESP32 with ADC readings from a float sensor (240-30Ω) using a MOSFET for PWM control.**  
- **Ensure correct wiring for relay and MOSFET circuits to prevent damage.**
