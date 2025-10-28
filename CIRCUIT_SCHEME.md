# Photodiode Circuit Scheme - Emitter Follower Configuration

## Overview
This circuit uses a photodiode with an NPN transistor in emitter-follower (common-collector) configuration to amplify and buffer the photodiode signal for measurement on GPIO2 of the ESP32-C3.

## Circuit Diagram

```
Power Supply (+3.3V)
        |
        |
        R1 (10kΩ - 100kΩ)
        |
        |
    [Photodiode]
    Cathode (-)
        |
    Anode (+)
        |
        |------------------------> Base (NPN Transistor)
        |
       GND


NPN Transistor (2N3904, BC547, or similar):

    +3.3V
      |
      |
  Collector
      |
    [NPN]
      |
   Emitter -----------------> GPIO2 (ADC1_CH2) - ESP32-C3
      |
      |
     R2 (10kΩ - 47kΩ)
      |
      |
     GND
```

## Component List

| Component | Value/Type | Purpose |
|-----------|------------|---------|
| Photodiode | BPW34 | Light sensor (reverse-biased) |
| NPN Transistor | ZTX652 or similar | Signal amplifier/buffer |
| R1 | 10kΩ - 100kΩ | Photodiode load resistor |
| R2 | 10kΩ - 47kΩ | Emitter resistor |
| C1 (optional) | 0.1µF ceramic | Noise filtering on GPIO2 to GND |

## Connections

### Photodiode Circuit
1. **Cathode (-)** → +3.3V (through R1)
2. **Anode (+)** → Transistor Base & GND

### NPN Transistor
1. **Collector** → +3.3V
2. **Base** → Photodiode anode connection
3. **Emitter** → GPIO2 (ESP32-C3 ADC input) & R2

### ESP32-C3
1. **GPIO2** → Transistor emitter
2. **3.3V** → Power supply (photodiode cathode via R1 & transistor collector)
3. **GND** → Ground (photodiode anode & R2)

## How It Works

1. **Photodiode Operation**: The photodiode is reverse-biased. When light hits it, it conducts current through R1, creating a voltage at the transistor base.

2. **Transistor Amplification**: The NPN transistor operates in emitter-follower mode (common-collector configuration):
   - Input: Base voltage from photodiode
   - Output: Emitter voltage (approximately 0.6V less than base due to Vbe drop)
   - Provides current amplification and low output impedance

3. **Signal Characteristics**:
   - **Non-inverting**: More light = Higher voltage at GPIO2
   - **Buffered**: Low output impedance for clean ADC reading
   - **Linear**: Voltage proportional to light intensity

4. **ADC Reading**: GPIO2 measures the emitter voltage (0V to ~2.7V range)

## Key Features

✅ **Non-inverting response** - More light produces higher voltage  
✅ **Low output impedance** - Better for ADC measurements  
✅ **Buffer action** - Isolates photodiode from ADC loading  
✅ **Linear response** - Proportional voltage output  
✅ **Simple design** - Only 3 components + photodiode  

## Tuning Guidelines

### Sensitivity Adjustment
- **Increase R1** (e.g., 100kΩ) → Higher sensitivity, slower response
- **Decrease R1** (e.g., 10kΩ) → Lower sensitivity, faster response

### Output Voltage Range
- **Increase R2** (e.g., 47kΩ) → Higher emitter voltage
- **Decrease R2** (e.g., 10kΩ) → Lower emitter voltage

### Noise Reduction
- Add 0.1µF ceramic capacitor between GPIO2 and GND
- Keep wires short (< 10cm if possible)
- Twist photodiode wires together if longer runs needed

## Expected Voltage Range

| Light Condition | Approximate GPIO2 Voltage |
|-----------------|--------------------------|
| Complete darkness | 0V - 0.2V |
| Low ambient light | 0.3V - 0.8V |
| Room lighting | 0.8V - 1.5V |
| Bright light | 1.5V - 2.7V |

*Note: Actual values depend on photodiode type and resistor values chosen*

## PCB Layout Tips

1. Keep photodiode leads as short as possible
2. Place decoupling capacitor close to GPIO2
3. Route ADC trace away from noisy signals (PWM, switching)
4. Use ground plane for better noise immunity
5. Consider shielding if in electrically noisy environment

## Troubleshooting

| Issue | Possible Cause | Solution |
|-------|----------------|----------|
| No voltage change | Photodiode reversed | Check cathode to +3.3V |
| Voltage always high/low | Wrong transistor pinout | Verify C-B-E pins |
| Noisy readings | Long wires or EMI | Add capacitor, shorten wires |
| Saturated readings | R1 too high | Reduce R1 value |
| Low sensitivity | R1 too low | Increase R1 value |

## Related Files
- `src/main.cpp` - Arduino code for reading the sensor
- `README.md` - Project documentation

---
**Project**: RayZ - Laser Tag System  
**Target**: ESP32-C3 Target Device  
**Last Updated**: October 28, 2025
