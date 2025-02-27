### PCB Layout Description for Electrical Engineer

This board layout is designed to interface an ESP32-WROOM-32D with a float sensor (240-30O), thermistor (10kO NTC, B3950), and PWM-controlled gauge, while also controlling a 110V AC power relay. The system operates on a 12V DC power source.

* * *

## 1\. Power System

*   Input: 12V DC power supply.
    
*   Voltage Regulation:
    

*   Buck Converter (12V ? 3.3V): Used to power the ESP32.
    
*   Step-Down Regulator (12V ? 5V): Powers the relay module.
    

*   Capacitors:
    

*   100µF electrolytic (12V rail) for stability.
    
*   10µF + 0.1µF ceramic (3.3V ESP32 supply) for noise filtering.
    

* * *

## 2\. Float Sensor Interface (240-30O)

*   Voltage Divider Circuit:
    

*   10kO Pull-up Resistor (3.3V ? Sensor ? ADC).
    
*   Connects to GPIO 34 (ESP32 ADC Input).
    

*   Purpose: Converts sensor resistance to a voltage readable by ESP32.
    

* * *

## 3\. Thermistor Interface (10kO NTC, B3950)

*   Voltage Divider Circuit:
    

*   10kO Pull-up Resistor (3.3V ? Thermistor ? ADC).
    
*   Connects to GPIO 35 (ESP32 ADC Input).
    

*   Purpose: Monitors temperature to trigger power cutoff if it exceeds 60°C.
    

* * *

## 4\. Gauge Control (PWM Output)

*   MOSFET Driver:
    

*   MOSFET: IRLB8721 (N-channel, logic-level).
    
*   10kO Pull-down Resistor (MOSFET Gate to GND).
    
*   1kO Series Resistor (ESP32 GPIO 25 ? MOSFET Gate).
    
*   Gauge Ground (-) connects to MOSFET Drain.
    

*   Purpose: Converts ESP32 PWM signal to variable resistance for smooth gauge control.
    

* * *

## 5\. 110V AC Power Control

*   Solid-State Relay (SSR) or Mechanical Relay (5V Coil, 110V AC Load)
    

*   Control: ESP32 GPIO 27 (KILL\_PIN) switches relay ON/OFF.
    
*   Flyback Diode (1N4007): Across relay coil for protection.
    
*   Optocoupler (PC817): Isolates ESP32 from 110V relay circuit.
    
*   Transistor (2N2222): Drives the relay coil.
    
*   1kO Base Resistor: (ESP32 GPIO ? Transistor Base).
    

*   Purpose: Turns off 110V AC power when conditions are met (low fill level or high temperature).
    

* * *

## 6\. Indicator Backlight (Low-Level Warning)

*   LED Backlight Control (GPIO 26)
    

*   MOSFET or NPN Transistor (2N2222).
    
*   330O Series Resistor (LED protection).
    
*   Blinking controlled by ESP32 logic.
    

*   Purpose: Flashes when the tank level is low.
    

* * *

  
  
  
  
  
  

### ESP32-WROOM-32D Pin Assignments for PCB Design

This table outlines the exact pin connections for your ESP32-WROOM-32D, including power, sensor inputs, and control outputs.

* * *

### 1\. Power Connections

| ESP32 Pin | Function | Connected To |
| --- | --- | --- |
| 3.3V | ESP32 Power | 3.3V from Buck Converter |
| GND | Ground | Common GND |
| VIN | Not used (ESP32 powered from 3.3V) |  |

* * *

### 2\. Float Sensor (240-30O)

| ESP32 Pin | Function | Connected To |
| --- | --- | --- |
| GPIO 34 | ADC Input (Float Sensor) | Voltage divider with 10kO resistor |

Voltage Divider Circuit:  
(3.3V) --- (10kO Resistor) ---( ADC Pin GPIO 34 ) --- (Float Sensor) --- (GND)

*     
    
*   Purpose: Reads resistance changes from float sensor and converts to gauge movement.
    

* * *

### 3\. Thermistor (10kO NTC, B3950)

| ESP32 Pin | Function | Connected To |
| --- | --- | --- |
| GPIO 35 | ADC Input (Thermistor) | Voltage divider with 10kO resistor |

Voltage Divider Circuit:  
(3.3V) --- (10kO Resistor) ---( ADC Pin GPIO 35 ) --- (Thermistor) --- (GND)

*     
    
*   Purpose: Reads temperature and cuts power if it exceeds 60°C.
    

* * *

### 4\. Gauge Control (PWM Output)

| ESP32 Pin | Function | Connected To |
| --- | --- | --- |
| GPIO 25 | PWM Output (Gauge) | MOSFET Gate (IRLB8721) via 1kO resistor |

MOSFET Circuit:  
ESP32 GPIO 25 --- (1kO Resistor) --- (MOSFET Gate)

MOSFET Source --- GND

MOSFET Drain --- Gauge Ground (-)

*     
    
*   Purpose: Provides PWM control for smooth gauge movement.
    

* * *

### 5\. 110V AC Power Control

| ESP32 Pin | Function | Connected To |
| --- | --- | --- |
| GPIO 27 | KILL_PIN (Power Cutoff) | Relay Module or Solid-State Relay |

Relay Control Circuit (With Optocoupler & Transistor):  
ESP32 GPIO 27 --- (1kO Resistor) --- PC817 Optocoupler Input

PC817 Output --- (2N2222 Transistor Base)

Transistor Collector --- Relay Coil

Relay Coil --- 5V Power

*     
    
*   Purpose: Turns off 110V power when level is too low or temperature is too high.
    

* * *

### 6\. Indicator Backlight (Low-Level Warning)

| ESP32 Pin | Function | Connected To |
| --- | --- | --- |
| GPIO 26 | Backlight Control | LED via 330O Resistor |

Backlight LED Circuit:  
ESP32 GPIO 26 --- (330O Resistor) --- LED (+)

LED (-) --- GND

*     
    
*   Purpose: Blinks LED when fill level is too low.
    

* * *

## ESP32 Pin Summary

| ESP32 Pin | Function | Description |
| --- | --- | --- |
| 3.3V | Power | 3.3V Supply from Buck Converter |
| GND | Ground | Common Ground |
| GPIO 34 | Float Sensor Input | Reads resistance (240-30O) via ADC |
| GPIO 35 | Thermistor Input | Reads temperature (10kO NTC) |
| GPIO 25 | PWM Output (Gauge) | Controls gauge via MOSFET (IRLB8721) |
| GPIO 27 | KILL_PIN (Relay) | Controls 110V relay via transistor |
| GPIO 26 | Backlight Control | Blinks LED at low fill levels |

* * *

## Final Design Considerations

## ? ESP32-WROOM-32D centrally placed.  
? 12V, 5V, and 3.3V power rails routed cleanly.  
? High-current traces (for MOSFET, relay) should be at least 1.5mm wide.  
? Keep ADC traces short to minimize noise.

? Route ADC traces separately to avoid noise.  
? Use optocoupler (PC817) for 110V relay isolation.  
? Use high-current traces (=1.5mm) for relay and MOSFET.  
? Use a ground plane to reduce EMI.  
? Keep ESP32 power (3.3V) separate from relay power (5V).
