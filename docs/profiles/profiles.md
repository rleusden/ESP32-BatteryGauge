# Profiles

ESP32 BatteryGauge v2 uses a **profile-based architecture**.

Instead of presenting the same dashboard for every installation, the system adapts the information it shows to the **actual usage scenario**.

A battery system used in a boat, camper, or off-grid solar installation may use the same hardware, but the **user questions are completely different**.

Profiles allow the system to present the **right information for the situation**.

---

# Why Profiles Exist

<table>
<tr>
<td width="50%" valign="top">

- Voltage
- Current
- Amp-hours
- Battery Temperature
- State of charge
- Charge cycles
- Cell voltages
- Internal resistance

</td>
<td width="50%" align="center">

![Monitor Madness](/images/monitor_madness.png)

</td>
</tr>
</table>

This information can be useful for engineers.

However, most users are not trying to analyze battery chemistry.  
They simply want to make decisions.

Examples of real user questions:

| Scenario | Real Question |
|--------|---------------|
| Boat rental | Can I still return to the harbor? |
| Camper | How long can we stay without charging? |
| Solar installation | Will the battery last through the night? |
| DIY battery bank | What is the system doing right now? |

Showing the same dashboard for all these scenarios leads to confusion.

Profiles solve this problem by adapting the **interface and warnings** to the context.

---

# Profile Architecture

Profiles define how BatteryGauge behaves in a specific environment.

A profile may control:

- Display layout
- Which metrics are shown
- Warning thresholds
- Status messages
- Color indicators
- Estimated runtime calculations

The **measurement hardware remains identical**, but the **interpretation layer changes**.

Sensors → Measurement Layer → Battery Model → Profile → User Interface


This separation allows BatteryGauge to stay **modular and extensible**.

---

# Current Profiles

## Boat Profile

Designed for rental boats and inexperienced users.

Focus:

- remaining range
- return-to-harbor warning
- simple status indicators

Example outputs:

Battery: 34%
Status: Return to the harbor soon
Estimated range: 25 minutes

or

🟢 Safe
🟡 Turn around soon
🔴 Return immediately


More details can be found in:

[Boat Profile](/docs/profiles/boat/boat_profile.md)

---

## Camper Profile (planned)

Designed for camper vans and mobile living setups.

Focus:

- hours remaining
- daily consumption
- charging status

Typical questions:

- How long can we stay off-grid?
- Are solar panels charging enough?
- Do we need to start the engine?

---

## Solar Profile (planned)

Designed for stationary battery systems connected to solar panels.

Focus:

- overnight survival
- charge prediction
- energy flow visualization

Typical questions:

- Will the battery survive the night?
- Are we charging faster than we consume?

---

## Generic Profile

The generic profile exposes more technical information.

Useful for:

- debugging
- system tuning
- DIY battery experiments

Metrics may include:

- voltage
- current
- SOC
- amp-hours
- power

---

# Design Philosophy

BatteryGauge follows a simple principle:

> Show the information people need, not the data the system happens to measure.

The hardware measures electrical parameters.

Profiles translate those parameters into **human decisions**.

---

# Benefits

The profile architecture provides several advantages:

### Usability

Users see information relevant to their situation.

### Flexibility

New environments can be supported without changing the measurement hardware.

### Simplicity

The same firmware can support many different installations.

### Extensibility

New profiles can be added as the system evolves.

---

# Future Ideas

Possible future profiles:

- **Electric boat rental fleets**
- **Tiny house energy systems**
- **Sailing boats**
- **E-bike charging stations**
- **Home battery backup systems**

The profile system makes BatteryGauge adaptable far beyond its original design.

---

# Summary

BatteryGauge is not just a battery monitor.

It is a **battery interpretation system**.

Profiles translate electrical measurements into **clear decisions for real users**.

Because the most important question is rarely:

What is the voltage?

It is usually:

Am I going to make it back?
