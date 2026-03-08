# Boat Profile

![Dead in the water](/docs/profiles/boat/dead_in_the_water_boat_profile.png)

## The Situation

A man is floating in the middle of a lake.

The water is calm.  
The sun is shining.  
The boat is silent.

Too silent.

In front of him is a beautiful dashboard with many meters and gauges:

- Voltage
- Current
- Amp-hours
- Battery Temperature
- Motor Temperature
- State of Charge
- Charge cycles
- Internal resistance

Numbers everywhere.

Unfortunately, none of them answers the only question he actually has:

**Can I still get back to the harbor?**

---

## What Happened

The battery is empty.

Not gradually empty.

Not politely empty.

**Completely empty.**

The boat stopped 20 minutes ago.

The dashboard tried to warn him:

- A red LED blinked
- A buzzer beeped
- The display showed `SOC 12%`

He ignored all of it.

Why?

Because he had no idea what `SOC` meant. He assumed it was some maritime authority.

He rented this electric boat this morning. The rental company said:

> "It's very simple. Just press the throttle."

They did not mention that the dashboard required an engineering degree.

---

## The Real Problem

Many battery dashboards are designed for **engineers**, not **users**.

They show technical telemetry instead of actionable information.

| Dashboard Shows | What the User Needs |
|----------------|---------------------|
| 48.7 V | Can I still return to the harbor? |
| -32 A | Am I using too much power? |
| SOC 12% | Should I turn around now? |

A boat renter does not need battery engineering data.

They need **simple decisions**.

---

## The BatteryGauge Solution

ESP32 BatteryGauge v2 solves this problem with a **profile-based architecture**.

Instead of showing the same dashboard everywhere, the system adapts to the **usage scenario**.

Different environments require different information:

- Boat rental
- Camper / Vanlife
- Off-grid solar
- DIY battery banks

Each **profile** decides:

- Which data is relevant
- How it should be displayed
- When warnings should appear

---

## Boat Profile

The **Boat Profile** is designed for inexperienced users.

Instead of technical telemetry, it shows simple, situational information.

Example display:

🟢 Safe
🟡 Turn around soon
🔴 Return to harbor immediately

Because when someone rents a boat, they should be enjoying the lake.
Not reverse-engineering the dashboard.

---

## Design Philosophy

BatteryGauge follows a simple rule:

> A battery monitor should help people make decisions, not interpret telemetry.

Technology should adapt to the user and the situation. Not the other way around.

Because on a rental boat, simplicity is not a luxury:

It is **risk management**.
---

## The Man on the Lake

The man is still floating on the lake.
He has now discovered that his phone battery is also empty.
He is waving politely to a passing paddleboat for help.

If his dashboard had used the **BatteryGauge Boat Profile**, it would have told him one hour earlier:

🔴 Return to harbor immediately

Instead, he learned an important lesson.

Never trust a dashboard that looks like the cockpit of a spacecraft.

Especially when you're just trying to rent a boat.
