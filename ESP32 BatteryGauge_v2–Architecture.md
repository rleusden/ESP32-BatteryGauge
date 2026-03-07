# ESP32 BatteryGauge v2.0 – Architecture

## Overview

ESP32 BatteryGauge v2.0 is designed as a profile-based embedded battery-monitoring platform.

Rather than building a separate firmware per use case, v2.0 provides a **generic runtime core** with **compile-time profile selection** for scenarios such as:

- boat
- camper
- solar buffer
- backup power
- rental vehicle

The main design goal is to keep runtime logic simple and deterministic while allowing different user experiences per deployment target.

The architecture separates **measurement, estimation, session logic and UI**, allowing the same core firmware to support different use cases without increasing runtime complexity.

---

## Why BatteryGauge?

Modern batteries often include a **Battery Management System (BMS)**.  
However, in many real-world installations this still leaves a gap between the battery electronics and the actual user.

Typical limitations of built-in BMS systems include:

- Some batteries **do not provide a display at all**.
- When a display exists, it often shows **technical values only** (voltage, current, cell data) rather than information relevant to the usage scenario.
- Many systems rely on **vendor-specific apps or cloud services**, which makes the user dependent on the manufacturer.
- Cloud-based systems may stop working when a service is discontinued or when connectivity is unavailable.

BatteryGauge is therefore designed as a **companion monitor**, not as a replacement for the BMS.

The BMS continues to perform its primary safety functions:

- cell balancing  
- over/under voltage protection  
- current limits  
- thermal protection  

BatteryGauge focuses on a different layer:

- translating raw battery information into **usage-oriented feedback**
- providing a **local, hardware-based interface**
- allowing the system to be **configured for specific usage scenarios**

Examples include:

- boat rental fleets
- camper installations
- solar buffer systems
- backup power setups

Because the firmware is **profile-based**, the same core platform can be configured for different contexts without depending on a particular battery brand or cloud service.

In short:

> BatteryGauge does not replace the BMS.  
> It complements it by providing a **scenario-aware user interface and local monitoring layer**.

This design keeps the safety-critical battery logic inside the BMS while giving system builders and operators the flexibility to tailor the user experience to their specific application.

---

## Design Principles

### 1. One active profile per firmware build

Although multiple profiles are supported by the codebase, only **one profile is active in a given firmware build**.

This means:

- profile selection happens at compile time
- runtime logic stays simple
- no dynamic profile switching is needed in the field

This is implemented through:

- `ACTIVE_PROFILE` in `Config.h`
- `SessionProfileFactory`
- a generic `SessionManager`
- profile-specific HTML rendering modules

---

### 2. Separation of concerns

The architecture is split into clear layers:

Sensors / mock
↓
State
↓
BatteryEstimator
↓
SessionManager
↓
Profile-specific HTML UI


Each layer has a focused responsibility.

---

## Core Building Blocks

### State

`State` is the shared runtime data structure holding the latest measured or mocked battery values.

Typical fields include:

- state of charge (`soc`)
- pack voltage (`vpack`)
- current (`current`)
- temperature (`tempC`)
- charging state
- alarm flags
- timestamp of last valid update

Current convention:

- **positive current = charging**
- **negative current = discharging**

This convention is preserved in the raw state layer.

---

### BatteryEstimator

The `BatteryEstimator` is responsible for building a **stable battery estimate** from raw measurements.

It combines:

- coulomb counting
- smoothing of displayed SOC
- soft drift correction using SOC as reference
- derived remaining energy and runtime values

This layer exists because raw voltage-based SOC and pure coulomb counting each have limitations on their own.  
By combining both, v2.0 can provide a more stable and trustworthy battery indication.

For renter-oriented UI modes, the estimator is especially important because it hides technical noise and provides a calmer, more understandable result.

---

### SessionManager

The `SessionManager` tracks an active energy consumption session.

Responsibilities include:

- detecting session start and stop
- tracking elapsed session time
- storing start SOC and current SOC
- calculating average discharge current
- calculating average discharge power
- estimating remaining runtime

The session state machine is generic and profile-independent:

- `IDLE`
- `START_PENDING`
- `ACTIVE`
- `STOP_PENDING`
- `ENDED`

To keep session logic simple, the `SessionManager` does not work with signed current directly.  
Instead, it receives **discharge-only values**:

- charging becomes `0`
- discharge is passed as a positive current and power value

This keeps session logic focused on consumption sessions.

---

### SessionProfileFactory

`SessionProfileFactory` maps the compile-time selected profile to a single `SessionProfileConfig`.

This allows each profile to define its own thresholds and timing, while still using the same generic `SessionManager`.

Examples of profile-specific parameters include:

- start current threshold
- stop current threshold
- start delay
- stop delay
- reserve SOC

---

### Profile-Specific UI

The final presentation layer is profile-specific.

For example, `html_boat.cpp` renders the boat-oriented dashboard using generic runtime data from:

- `SessionManager`
- `BatteryEstimator`

This keeps the UI flexible while the underlying logic stays reusable.

---

## Why Compile-Time Profiles?

Version 2.0 explicitly chooses compile-time profile selection over runtime profile switching.

Reasons include:

- lower runtime complexity
- less memory overhead
- easier testing per target scenario
- better fit for embedded systems

In practice, a boat build, camper build, or solar build may share most of the same core code, but present different behavior and UI through profile-specific configuration and rendering.

---

## Renter-First Boat Mode

The boat profile is currently optimized primarily for **renters**, not technical owners.

That means the UI should answer practical questions such as:

- how much battery is left?
- how long can I continue operating?
- is the current usage light, normal, or heavy?
- do I need to recharge soon?

Detailed owner-oriented diagnostics such as raw Ah/Wh flows, calibration values, or drift information can be added later in a more advanced dashboard.

---

## Development Approach

v2.0 is developed incrementally using both real sensor data and realistic mock scenarios.

For the boat profile, the mock behavior was updated to reflect actual usage phases such as:

- idle
- cruising
- low activity
- cruising again
- charging in harbor

This makes the platform easier to test before all hardware integration is finalized.

---

## Summary

ESP32 BatteryGauge v2.0 is built around a simple idea:

- keep the embedded runtime core generic
- keep profiles compile-time selectable
- separate raw measurement, estimation, session logic, and UI
- adapt presentation to the real end user

This makes the platform flexible enough for multiple battery-monitoring scenarios, while still staying small, understandable, and maintainable.
