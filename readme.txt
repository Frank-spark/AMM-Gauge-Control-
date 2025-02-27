### **README for `gaugecontrol.cpp`**
---
## **Project: ESP32 Gauge Control with Float Sensor and Safety Cutoff**

### **Overview**
This project controls an **analog gauge** using an **ESP32-WROOM-32D**. The gauge reflects the **fill level of a tank**, based on readings from a **resistance-based float sensor (240Ω - 30Ω)**. It also features **a safety cutoff relay** and **a low-level warning backlight**.

### **Features**
✔ Reads **float sensor resistance** and converts it into a gauge display using **PWM control**.  
✔ **Smooths the gauge movement** to prevent needle shaking.  
✔ **Blinks a backlight** when the fill level is low.  
✔ **Cuts power (110V AC)** when the fill level is critically low.  
✔ Uses **noise-filtered ADC readings** for reliable measurements.

---

## **1. Hardware Setup**
### **🛠 Components Needed**
- **ESP32-WROOM-32D**
- **Float Sensor (240Ω - 30Ω)**
- **10kΩ Pull-up Resistor** (for voltage divider)
- **Gauge (0-100% Analog Display)**
- **IRLB8721 MOSFET** (for PWM gauge control)
- **110V Relay (Solid-State or Mechanical)**
- **PC817 Optocoupler** (for isolation)
- **2N2222 Transistor** (for relay control)
- **330Ω Resistor + LED** (for backlight warning)

### **🔌 Wiring Guide**
#### **1️⃣ Power Supply**
| Component | Connection |
|-----------|------------|
| ESP32 3.3V | **3.3V Buck Converter** |
| ESP32 GND | **Common Ground** |

#### **2️⃣ Float Sensor (Resistance Measurement)**
| Pin | Connection |
|-----|-----------|
| **ESP32 GPIO 34** | **Voltage divider output** |
| **3.3V** | **10kΩ pull-up resistor** → Float Sensor |
| **GND** | **Float Sensor Ground** |

#### **3️⃣ Gauge Control (PWM Output)**
| Pin | Connection |
|-----|------------|
| **ESP32 GPIO 25** | **MOSFET Gate (IRLB8721)** |
| **Gauge (-)** | **MOSFET Drain** |
| **GND** | **MOSFET Source** |

#### **4️⃣ 110V AC Power Cutoff**
| Pin | Connection |
|-----|------------|
| **ESP32 GPIO 27** | **Relay Module Input** |
| **Relay NO Terminal** | **110V Load Line** |
| **Relay COM Terminal** | **110V Power** |

#### **5️⃣ Backlight Warning**
| Pin | Connection |
|-----|------------|
| **ESP32 GPIO 26** | **LED via 330Ω Resistor** |
| **GND** | **LED Cathode (-)** |

---

## **2. Code Explanation**
### **⚙ Main Functionalities**
#### **1️⃣ Setup Function**
- Initializes **PWM control for the gauge**.
- Sets up **GPIOs for the backlight and power relay**.

#### **2️⃣ `loop()` Function**
- Reads **float sensor resistance** via **`readFloatSensorResistance()`**.
- Converts resistance into a **PWM signal** for the gauge.
- **Smooths gauge movement** to avoid sudden jumps.
- Calls **`manageSafety()`** to:
  - **Blink backlight** if level is low.
  - **Turn OFF power relay** if level is critically low.

#### **3️⃣ Read Float Sensor Resistance**
```cpp
float readFloatSensorResistance(int pin) {
    int adcValue = readSmoothADC(pin);
    float voltage = (adcValue / 4095.0) * 3.3;  

    if (voltage <= 0.1) return MAX_RESISTANCE;  

    float resistance = (voltage / (3.3 - voltage)) * KNOWN_RESISTOR;
    return resistance;
}
```
- Uses **Ohm’s Law** to convert voltage readings to **resistance (240Ω - 30Ω)**.
- **Failsafe:** If voltage is **too low**, assumes an **empty tank (240Ω).**

#### **4️⃣ Smooth ADC Readings**
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

#### **5️⃣ Safety Handling**
```cpp
void manageSafety(int pwmDuty) {
    unsigned long currentMillis = millis();

    if (pwmDuty < BLINK_THRESHOLD) {
        if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
            lastBlinkTime = currentMillis;
            blinkState = !blinkState;
            digitalWrite(BACKLIGHT_PIN, blinkState);
        }
    } else {
        digitalWrite(BACKLIGHT_PIN, LOW);
    }

    if (pwmDuty < LOW_LEVEL_THRESHOLD) {
        Serial.println("KILL PIN ACTIVATED! Power Off!");
        digitalWrite(KILL_PIN, LOW);
    } else {
        digitalWrite(KILL_PIN, HIGH);
    }
}
```
- **Flashes backlight** when fill level is low.
- **Turns OFF 110V relay** if level is critically low.

---

## **3. Expected Behavior**
| **Tank Level (%)** | **Float Sensor Resistance (Ω)** | **PWM Duty (Gauge)** | **Backlight (GPIO 26)** | **Power Relay (GPIO 27)** |
|--------------------|--------------------------------|----------------------|------------------------|------------------------|
| **100% (Full)**   | 30Ω                            | 225                  | OFF                    | ON (Power OK)         |
| **50% (Half)**    | 135Ω                           | 180                  | OFF                    | ON (Power OK)         |
| **20% (Low)**     | 200Ω                           | 150                  | **BLINKING**           | ON (Power OK)         |
| **5% (Critical)** | 240Ω                           | 140                  | **BLINKING**           | **OFF (Power Cut)**   |

---

## **4. Installation & Usage**
### **📥 Uploading Code**
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
| **Issue**                    | **Possible Fix** |
|------------------------------|------------------|
| Gauge is **not moving** | Check if **PWM is set properly** in Serial Monitor (`PWM: X`). |
| Gauge **jumps suddenly** | Increase **smoothing factor** in `smoothedDuty = (0.05 * target) + (0.95 * smoothedDuty);` |
| Backlight **never blinks** | Verify **pwmDuty < 150** in `manageSafety()`. |
| Power **does not cut off** | Ensure **KILL_PIN (GPIO 27)** is correctly wired to the relay. |

---

## **6. Future Improvements**
🔹 Add **thermistor-based power cutoff** for temperature safety.  


---

### **Final Notes**
- **This firmware controls an analog gauge, blinks a backlight for warnings, and cuts power when necessary.**  
- **It operates on an ESP32 with ADC readings from a float sensor (240-30Ω) using a MOSFET for PWM control.**  
- **Ensure correct wiring for relay and MOSFET circuits to prevent damage.**