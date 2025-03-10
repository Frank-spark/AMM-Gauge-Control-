### PCB Layout Description for Electrical Engineer

This board layout is designed to interface an ESP32-WROOM-32D with a float sensor (240-30Ω) and PWM-controlled gauge, while providing soft-start control (0-5V) for H2 power supplies. The system operates on 12V DC power from the H2 board's gauge light circuit.

* * *

## 1\. Power System

*   Input: 12V DC from H2 board gauge light
    
*   Voltage Regulation:
    *   Buck Converter (12V → 3.3V): Powers the ESP32
    *   Filtering Capacitors:
        *   100µF electrolytic (12V rail)
        *   10µF + 0.1µF ceramic (3.3V ESP32 supply)

* * *

## 2\. Float Sensor Interface (240-30Ω)

*   Voltage Divider Circuit:
    *   10kΩ Pull-up Resistor (3.3V → Sensor → ADC)
    *   Connects to GPIO 34 (ESP32 ADC Input)
    *   0.1µF ceramic capacitor for noise filtering
    *   Purpose: Converts sensor resistance to voltage readable by ESP32

* * *

## 3\. H2 Signal Input (12V)

*   Level Shifter Circuit:
    *   Input: 12V from H2 board coil signal
    *   Output: 3.3V to ESP32 GPIO 35
    *   Components:
        *   Voltage divider or dedicated level shifter IC
        *   10kΩ pull-down resistor on ESP32 input
        *   Protection diode for reverse voltage
    *   Purpose: Safely converts 12V H2 signal to 3.3V logic level

* * *

## 4\. Power Supply Control (0-5V Output)

*   PWM to Analog Circuit:
    *   Input: ESP32 GPIO 27 (PWM)
    *   Output: 0-5V DC to power supply CC input
    *   Components:
        *   RC low-pass filter (10kΩ + 10µF)
        *   Op-amp buffer (e.g., MCP6002)
        *   Precision voltage reference (optional)
    *   Purpose: Converts PWM to clean 0-5V control signal

* * *

## 5\. Gauge Control (PWM Output)

*   MOSFET Driver:
    *   MOSFET: IRLB8721 (N-channel, logic-level)
    *   10kΩ Pull-down Resistor (Gate to GND)
    *   1kΩ Series Resistor (ESP32 GPIO 25 → Gate)
    *   Gauge Ground (-) connects to MOSFET Drain
    *   Purpose: PWM control for smooth gauge movement

* * *

## 6\. Warning Indicator

*   LED Circuit:
    *   330Ω current-limiting resistor
    *   LED connected to GPIO 26
    *   Purpose: Visual warning for low tank level

* * *

## 7\. Future Expansion - I2C Interface

*   I2C Connector:
    *   4-pin header (3.3V, GND, SDA, SCL)
    *   Pin Assignment:
        *   GPIO 21: SDA
        *   GPIO 22: SCL
    *   Pull-up Resistors:
        *   2x 4.7kΩ resistors (one each for SDA and SCL)
    *   Protection:
        *   Optional TVS diodes for signal protection
    *   Connector Type:
        *   JST-XH or similar 2.54mm pitch header
        *   Keyed to prevent incorrect insertion

### Updated ESP32-WROOM-32D Pin Assignments

| ESP32 Pin | Function | Connected To |
|-----------|----------|--------------|
| VIN | Power Input | 12V (via protection diode) |
| 3.3V | Regulated Power | 3.3V from Buck Converter |
| GND | Ground | Common Ground |
| GPIO 21 | I2C SDA | I2C Connector (with 4.7kΩ pullup) |
| GPIO 22 | I2C SCL | I2C Connector (with 4.7kΩ pullup) |
| GPIO 34 | ADC Input | Float Sensor Voltage Divider |
| GPIO 35 | Digital Input | H2 Signal (via level shifter) |
| GPIO 25 | PWM Output | Gauge MOSFET Gate |
| GPIO 27 | PWM Output | Power Supply Control Circuit |
| GPIO 26 | Digital Output | Warning LED |

### I2C Connector Pinout

```
1 - 3.3V
2 - SDA (GPIO 21)
3 - SCL (GPIO 22)
4 - GND
```

### PCB Layout Considerations for I2C

1. **Signal Routing:**
    * Keep I2C traces short and equal length
    * Route SDA and SCL close together
    * Avoid crossing noisy signals

2. **Pull-up Resistors:**
    * Place 4.7kΩ pull-ups near the ESP32
    * Use 0603 or 0805 package size

3. **Connector Placement:**
    * Edge-mounted for easy access
    * Away from power components
    * Clear labeling for pin 1

* * *

### Power Supply Control Circuit Detail

```
ESP32 GPIO 27 --[10kΩ]--+--[10µF]--GND
                        |
                        +--[Buffer Op-amp]---> To Power Supply CC
```

*   PWM Frequency: 10kHz
*   Filter cutoff: ~1.6Hz (for smooth ramping)
*   Output voltage range: 0-5V DC
*   Ramp time: 5 seconds (0 to full scale)

* * *

### PCB Layout Guidelines

1. **Power Distribution:**
    *   Separate ground planes for analog and digital
    *   Wide traces for 12V input
    *   Star ground configuration

2. **Signal Routing:**
    *   Keep analog signals away from PWM
    *   Short traces for ADC inputs
    *   Ground plane under level shifter

3. **Component Placement:**
    *   ESP32 module centrally located
    *   Power components near input
    *   Analog section isolated from digital

4. **Thermal Considerations:**
    *   Copper pour for MOSFET cooling
    *   Adequate ventilation for buck converter

5. **Protection Features:**
    *   Reverse polarity protection on 12V input
    *   TVS diodes on signal inputs
    *   Fuse for overcurrent protection

* * *

### Board Stack-up

*   2-layer board minimum
*   1oz copper recommended
*   FR4 material
*   Board size: Approximately 2" x 3"

* * *

### Testing Points

Include test points for:
*   12V input
*   3.3V regulated
*   H2 signal (both 12V and 3.3V sides)
*   Power supply control output
*   Float sensor input
*   Ground reference
