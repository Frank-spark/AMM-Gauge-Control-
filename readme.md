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
✔ **12V to 3.3V Buck Converter**
✔ **12V to 5V Buck Converter**
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

---
Here is a **README** section for the **power supply selection** in your AMM-Gauge-Control project:  

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
🔹 Add **I2C sensors or displays**  
🔹 Implement **I2C communication with external devices**

---

### **Final Notes**
- **This firmware controls an analog gauge, blinks a backlight for warnings, and cuts power when necessary.**  
- **It operates on an ESP32 with ADC readings from a float sensor (240-30Ω) using a MOSFET for PWM control.**  
- **Ensure correct wiring for relay and MOSFET circuits to prevent damage.**
