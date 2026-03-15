## Test Overview

The following test scenarios validate the normal functionality and edge-case handling of the **Pro Mini LTC2944 bridge**.
The bridge converts raw measurements from the **LTC2944 battery monitor IC** into normalized battery telemetry for the **ESP32 BatteryGauge application**.

The following capabilities are validated:

- battery voltage measurement
- battery current measurement
- battery temperature measurement
- battery state-of-charge estimation (SOC)
- battery profile awareness
- calibration capability
- safe-state handling

The tests focus on two main aspects:

1. **Normal operation**
2. **Edge cases and fault handling**

Tests are performed using a **bench power supply** to simulate battery behavior.

---

## Test Setup

![Bench test setup](/images/ltc2944_bridge_testing.png)

---

## Test Matrix

| ID | Category | Scenario | Validates |
|----|---------|---------|-----------|
| TC-01 | Measurement | Nominal voltage measurement | Correct LTC2944 voltage telemetry |
| TC-02 | Measurement | Current measurement under load | Correct current telemetry |
| TC-03 | Measurement | Temperature reporting | Temperature measurement path |
| TC-04 | SOC | SOC estimation during discharge | SOC calculation stability |
| TC-05 | SOC | SOC recovery during charge | SOC correction behavior |
| TC-06 | Calibration | Calibration applied | Calibration accuracy |
| TC-07 | Profile | Correct battery profile recognition | Chemistry + class configuration |
| TC-08 | Profile | Chemistry switch validation | Li-ion / LiFePO4 / AGM / GEL |
| TC-09 | Profile | Cell count configuration | Lithium S-count mapping |
| TC-10 | Profile | Lead battery voltage class | Lead-acid system voltage mapping |
| TC-11 | Fault | Invalid chemistry/class combination | Configuration error handling |
| TC-12 | Fault | Startup voltage plausibility failure | Fault detection logic |
| TC-13 | Fault | Wrong battery type connected | Plausibility validation |
| TC-14 | Fault | Deeply discharged battery | Edge case handling |
| TC-15 | Fault | Incorrect installation | Safe-state behavior |
| TC-16 | Safety | Telemetry disabled during fault | Safe-state enforcement |
| TC-17 | Safety | LED SOS error indication | Hardware fault signaling |
| TC-18 | Safety | Serial diagnostic output | Debug capability |

---

# Hardware Configuration

Two input pins define the battery chemistry.

| Code | Chemistry |
|-----|-----------|
| 00 | Li-ion |
| 01 | LiFePO4 |
| 10 | AGM |
| 11 | GEL |

---

# Lithium Battery Configuration

Lithium batteries are configured using the number of cells in series.

| Class | Battery |
|------|---------|
| 0000 | 1S |
| 0001 | 2S |
| 0010 | 3S |
| 0011 | 4S |
| 0100 | 6S |
| 0101 | 8S |
| 0110 | 12S |
| 0111 | 14S |

---

# Lead Battery Configuration

Lead batteries are configured using nominal system voltage.

| Class | Battery |
|------|---------|
| 0000 | 12V |
| 0001 | 24V |
| 0010 | 48V |

---

# Fault Handling Validation

A valid hardware configuration does not guarantee that the correct battery is connected.

Examples of incorrect situations include:

- a **1S battery connected while 3S Li-ion is configured**
- a **severely discharged battery outside the plausible voltage range**
- an **incorrect installation**

To detect these situations, the system performs a **startup voltage plausibility check**.

The measured pack voltage is compared against the expected voltage range of the selected battery profile.

---

## Fault Mode Behaviour

If the measured voltage falls outside the plausible range:

- A diagnostic message is printed to the **serial port**
- The system enters **fault mode**
- The **status LED emits an SOS pattern**
- **Normal telemetry output is disabled**

This prevents the **ESP32 BatteryGauge application** from processing invalid battery data.

---

## Safe State Validation

The following safe-state mechanisms must be validated:

| ID | Scenario | Expected Behaviour |
|----|----------|-------------------|
| SS-01 | Invalid chemistry/class | SOS LED + telemetry disabled |
| SS-02 | Implausible startup voltage | Fault mode triggered |
| SS-03 | Battery mismatch | Telemetry blocked |
| SS-04 | Recovery after fault | Reset needed |

# Detailed Test Cases

This section contains detailed functional and fault-handling test cases for the **Pro Mini LTC2944 bridge**.

The purpose of these tests is to validate that the bridge:

- converts raw LTC2944 measurements into normalized telemetry
- applies the selected battery profile correctly
- detects invalid or implausible configurations
- protects the ESP32 BatteryGauge application from invalid battery data

---

## TC-01 Nominal Voltage Telemetry

### Objective
Verify that the bridge correctly reads the pack voltage from the LTC2944 and publishes valid normalized telemetry during normal operation.

### Category
Normal functionality

### Preconditions
- Pro Mini bridge firmware is flashed and running
- LTC2944 is connected and responding
- Battery chemistry input pins are set to **Li-ion**
- Battery class input pins are set to **1S**
- Bench power supply is connected to simulate the battery
- ESP32 or serial monitor is available to observe telemetry output
- No fault state is active

### Test Setup
- Chemistry: `00` = Li-ion
- Class: `0000` = 1S
- Bench power supply set to a plausible nominal 1S Li-ion voltage, for example **3.90 V**

### Test Steps
1. Power on the system.
2. Verify that the bridge completes startup without entering fault mode.
3. Observe the startup messages on the serial port.
4. Observe the voltage telemetry sent by the bridge.
5. Compare the reported voltage with the configured power supply voltage.
6. Keep the voltage stable for at least 60 seconds.
7. Observe whether telemetry remains stable over time.

### Expected Result
- The startup plausibility check passes.
- No diagnostic fault message is printed.
- The status LED indicates normal operation.
- Voltage telemetry is enabled.
- Reported battery voltage is within acceptable tolerance of the power supply setting.
- Telemetry remains stable without unexpected spikes, resets, or oscillations.

### Pass Criteria
- Measured voltage is reported correctly within the defined tolerance.
- No fault mode is entered.
- Normal telemetry output remains active for the full test duration.

### Notes
This testcase serves as a baseline for all other measurement and profile validation tests.

---

## TC-02 Current Measurement Under Simulated Load

### Objective
Verify that the bridge correctly reads current via the LTC2944 and exposes valid current telemetry while a controlled load is applied.

### Category
Normal functionality

### Preconditions
- System is operational in a valid profile
- LTC2944 current sensing path is connected correctly
- Shunt and wiring are installed according to design
- Bench power supply is connected
- A known resistive load is available
- No fault state is active

### Test Setup
- Chemistry: `00` = Li-ion
- Class: `0000` = 1S
- Bench power supply set to **4.00 V**
- Known load connected to draw measurable current

### Test Steps
1. Power on the system with no additional load connected.
2. Record the baseline reported current.
3. Connect the test load.
4. Observe the reported current value on serial output and/or ESP32 UI.
5. Compare the measured current against the current shown on the bench power supply.
6. Disconnect the load.
7. Observe whether the reported current returns to baseline.

### Expected Result
- Current telemetry is enabled during normal operation.
- Reported current changes when the load is connected.
- Reported current approximately matches the value observed on the bench power supply, within expected tolerance.
- After load removal, the current returns toward baseline.
- No fault state is triggered by a normal load change.

### Pass Criteria
- Current telemetry responds correctly to load application and removal.
- Current values are plausible and directionally correct.
- System remains in normal operating mode throughout the test.

### Notes
This test is important because the current measurement directly influences SOC estimation and user-facing battery behavior.

---

## TC-03 Invalid Hardware Configuration

### Objective
Verify that an unsupported chemistry/class combination is detected as invalid and that the bridge enters safe-state behavior.

### Category
Fault handling

### Preconditions
- Pro Mini bridge firmware is flashed and running
- Input pins for chemistry and class are configurable
- Serial monitor is connected
- Status LED is visible

### Test Setup
Configure the hardware inputs to a combination that is unsupported by the firmware.

Example:
- Chemistry set to a valid code
- Class set to a value that is not supported for that chemistry

### Test Steps
1. Apply the unsupported chemistry/class configuration.
2. Power on the system.
3. Observe the serial output during startup.
4. Observe the status LED behavior.
5. Check whether telemetry is sent to the ESP32 side.

### Expected Result
- A clear diagnostic message is printed to the serial port indicating an invalid configuration.
- The status LED enters the **SOS blink pattern**.
- The system enters fault mode.
- Normal telemetry output is disabled.
- The ESP32 receives no normal battery telemetry from the bridge.

### Pass Criteria
- Invalid configuration is detected immediately at startup.
- Safe-state behavior is activated consistently.
- No normal telemetry is emitted while the configuration remains invalid.

### Notes
This test case validates a key architectural safety rule: a syntactically incorrect hardware configuration must not produce misleading battery data.

---

## TC-04 Startup Voltage Plausibility Failure

### Objective
Verify that the bridge detects a mismatch between the configured battery profile and the measured startup voltage, and blocks invalid telemetry output.

### Category
Fault handling/edge case

### Preconditions
- Pro Mini bridge firmware is flashed and running
- LTC2944 is connected and measuring voltage correctly
- Bench power supply is connected
- Serial monitor is connected
- Status LED is visible

### Test Setup
Configure a valid battery profile, but apply a voltage that is implausible for that profile.

Example:
- Chemistry: `00` = Li-ion
- Class: `0010` = 3S
- Bench power supply output: **3.80 V**

This is a plausible voltage for a 1S battery, but implausible for a 3S Li-ion battery.

### Test Steps
1. Set the chemistry and class pins to **Li-ion 3S**.
2. Set the bench power supply to **3.80 V**.
3. Power on the system.
4. Observe startup behavior on the serial port.
5. Observe LED behavior.
6. Verify whether telemetry output is enabled or blocked.
7. Repeat the test with a voltage inside the plausible range for 3S and compare the behavior.

### Expected Result
- The startup voltage plausibility check fails.
- A diagnostic message is printed to the serial port indicating implausible battery voltage for the selected profile.
- The system enters fault mode.
- The status LED emits the **SOS pattern**.
- Normal telemetry output is disabled.
- When repeated with a plausible 3S voltage, the system starts normally, and telemetry is enabled.

### Pass Criteria
- Implausible startup voltage is consistently detected.
- Telemetry is blocked when the battery/profile combination is not credible.
- Normal operation resumes only when a plausible profile/voltage combination is used and after reset of the system.

### Notes
This is one of the most important safety tests in the design. A valid hardware pin configuration does not prove that the connected battery is actually the configured battery.

---

## TC-05 Recovery After Fault Condition

### Objective
Verify that the bridge can recover from a fault condition once the invalid voltage or configuration is corrected.

### Category
Safe-state handling

### Preconditions
- A previous testcase has successfully forced the bridge into fault mode
- Serial monitor is connected
- Status LED is visible

### Test Setup
Start from a known fault condition, for example:
- Li-ion 3S selected
- Bench supply set to an implausible low voltage

### Test Steps
1. Start the system in a known fault condition.
2. Confirm fault mode through serial output and SOS LED pattern.
3. Correct the fault condition:
   - either set the correct battery profile
   - or adjust the power supply voltage into the plausible range
4. Restart the system.
5. Check whether normal telemetry output resumes.

### Expected Result
- Fault behavior remains active while the invalid condition persists.
- After correction and restart, the startup plausibility check passes.
- The SOS LED pattern stops.
- Normal telemetry is enabled again.
- The ESP32 receives valid normalized battery telemetry.

### Pass Criteria
- Recovery path is deterministic and reproducible.
- Safe-state does not remain latched unintentionally after correction.
- System returns to normal operation only when the fault is genuinely resolved.

### Notes
This test case is useful to demonstrate that the safe-state mechanism is not only protective but also operationally usable during bench testing and maintenance.
