# ESP32 BatteryGauge v2.0 – Softwarearchitectuur

## Doel

Dit document beschrijft de softwarearchitectuur van **ESP32 BatteryGauge v2.0** op bestandsniveau.  
De focus ligt op:

- de rol van elk broncodebestand
- de belangrijkste functies en classes
- de samenhang tussen de componenten
- de actuele implementatie zoals aanwezig in de meegeleverde broncode

Deze versie beschrijft de **huidige 24 broncodebestanden**. Ongebruikte of nog niet uitgewerkte profielbestanden vallen hier nog buiten.

---

## Architectuuroverzicht

De software is opgezet als een kleine embedded applicatie met duidelijke lagen:

```text
Config / compile-time profile selection
                  │
                  ▼
        Main application (setup + loop)
                  │
     ┌────────────┼──────────────┐
     ▼            ▼              ▼
Data acquisition  Time/RTC     Wi-Fi / Web server
     │                           │
     ▼                           ▼
   State                    HTML dispatcher
     │                           │
     ├──────────────┬────────────┘
     ▼              ▼
BatteryEstimator  SessionManager
     │              │
     └──────┬───────┘
            ▼
      Profile-specific UI
```

Belangrijke ontwerpkeuzes:

- **één actief profiel per firmware-build**
- profielkeuze gebeurt via `ACTIVE_PROFILE` in `Config.h`
- runtime blijft generiek en eenvoudig
- profielspecifiek gedrag zit vooral in configuratie en UI
- de monitor is een **aanvulling op een bestaand BMS**, geen vervanging

---

## Hoofdverantwoordelijkheden per laag

### 1. Configuratie
Bepaalt compile-time welk profiel actief is en welke Wi-Fi/NTP-instellingen worden gebruikt.

### 2. Applicatiekern
`ESP32_BatteryGauge_v2_0.ino` initialiseert alle subsystemen en verbindt ze in de hoofdloop.

### 3. Dataverwerking
Levert ruwe meetdata aan in de gedeelde `State` structuur, vanuit demo/mock of seriële invoer.

### 4. Tijd en klok
Levert epoch-tijd via DS3231 RTC, met optionele synchronisatie via Wi-Fi en NTP.

### 5. Energieschatting en sessielogica
- `BatteryEstimator` levert een stabielere batterij-indicatie voor de gebruiker
- `EnergyIntegrator` telt geladen en ontladen Ah/Wh op
- `SessionManager` detecteert en bewaakt actieve verbruikssessies

### 6. Web en UI
De ESP32 biedt een lokale webinterface. Een dispatcher kiest op basis van het actieve profiel de juiste dashboard-HTML.

---

## Bestandsoverzicht

## 1. Hoofdapplicatie

### `ESP32_BatteryGauge_v2_0.ino`
**Rol**  
Entry point van de firmware. Initialiseert alle subsystemen en voert de hoofdloop uit.

**Belangrijkste globale objecten**
- `WebServer server(80)`
- `State st`
- `SessionManager sessionManager`
- `EnergyIntegrator energyIntegrator`
- `BatteryEstimator batteryEstimator`

**Belangrijkste functies**
- `static void wdt_setup()`  
  Configureert de watchdog voor de loop task.
- `static const char* profileName()`  
  Geeft een leesbare profielnaam terug voor logging.
- `void setup()`  
  Initialiseert serial, RTC, Wi-Fi access point, data source, sessieprofiel, estimator en webserver.
- `void loop()`  
  Roept periodiek `dataTick(st)` aan, verrijkt de ruwe state met afgeleide gegevens en werkt daarna integrator, estimator en session manager bij.

**Samenhang**
- leest compile-time profiel uit `Config.h`
- haalt profielconfig op via `buildActiveSessionProfileConfig()`
- gebruikt `rtcNow()` als gemeenschappelijke tijdbron
- geeft `State` door aan de logische componenten
- start de webserver en route-registratie

---

## 2. Configuratie en gedeelde state

### `Config.h`
**Rol**  
Bevat compile-time configuratie.

**Belangrijkste inhoud**
- profiel-ID's:
  - `PROFILE_NORMAL`
  - `PROFILE_BOAT`
  - `PROFILE_CAMPER`
  - `PROFILE_SOLAR_BUFFER`
  - `PROFILE_RENTAL_VEHICLE`
  - `PROFILE_BACKUP_POWER`
- `ACTIVE_PROFILE`
- Wi-Fi/NTP instellingen voor tijd-synchronisatie
- timeouts voor Wi-Fi connectie en NTP sync

**Architectuurfunctie**
- bepaalt welk profiel actief is in de build
- beïnvloedt zowel sessieconfiguratie als HTML-dispatch

### `State.h`
**Rol**  
Definieert de gedeelde runtime-state van de batterijmeting.

**Struct**
- `struct State`
  - `soc`
  - `vpack`
  - `current`
  - `tempC`
  - `charging`
  - `alarms`
  - `lastGoodMs`

**Hulpfunctie**
- `inline void stateSetDefaults(State& st)`  
  Zet veilige/default waarden zodat de UI niet leeg start.

**Architectuurfunctie**
`State` is het centrale overdrachtsobject tussen dataverwerking, logica en weblaag.

**Tekenconventie**
- `current > 0` = laden
- `current < 0` = ontladen

---

## 3. Data-acquisitie en parsing

### `DataProcessing.h`
**Rol**  
Publieke interface voor data-initialisatie en periodieke data-update.

**Belangrijkste items**
- `#define DEMO_MODE 1`
- UART configuratie (`UART_BAUD`, RX/TX pinnen)
- `void dataInit();`
- `void dataTick(State& st);`

### `DataProcessing.cpp`
**Rol**  
Levert batterijdata aan vanuit twee mogelijke bronnen:
- demo/mock scenario
- seriële invoer met CRC-ondersteuning

**Belangrijkste interne functies**
- `crc8_dallas(...)`  
  CRC-8 Dallas/Maxim controle voor seriële payloads.
- `hexNibble(...)` en `parseHexByte(...)`  
  Hulp voor CRC parsing.
- `trimSpaces(...)`  
  Verwijdert whitespace uit tokens.
- `parseKeyValueCSV(...)`  
  Parseert payloads zoals `SOC=...,Vpack=...,I=...`.
- `acceptLineIfValid(...)`  
  Valideert een seriële regel en schrijft geldige data naar `State`.
- `updateMock(State& st)`  
  Simuleert een bootscenario met fasen als idle, cruising, low activity en charging in harbor.
- `pollSerial(State& st)`  
  Leest regels van `Serial2` en verwerkt ze.
- `dataInit()`  
  Initialiseert de seriële ingang indien nodig.
- `dataTick(State& st)`  
  Kiest op basis van `DEMO_MODE` tussen mock update of UART polling.

**Architectuurfunctie**
Deze module is de bron van ruwe meetdata. Alle hogere lagen vertrouwen op de actuele `State` die hieruit voortkomt.

---

## 4. Tijd en klokbeheer

### `RtcClock.h`
**Rol**  
Publieke interface voor tijdbeheer.

**Functies**
- `bool rtcInit()`
- `bool rtcSyncFromHarborWifi()`
- `uint32_t rtcNow()`
- `bool rtcSetEpoch(uint32_t epoch)`
- `bool rtcHasValidTime()`
- `const char* rtcTimeSource()`

### `RtcClock.cpp`
**Rol**  
Implementeert tijdvoorziening via DS3231 RTC met optionele NTP-synchronisatie.

**Belangrijkste logica**
- initialiseert I2C en DS3231
- controleert of RTC al een geldige tijd bevat
- verbindt optioneel met haven-Wi-Fi
- haalt tijd op via NTP
- schrijft geslaagde NTP-tijd terug naar DS3231
- levert tijdbronstatus: `ntp`, `rtc` of `invalid`

**Architectuurfunctie**
Levert een stabiele epoch-tijd aan alle tijdsafhankelijke modules:
- `BatteryEstimator`
- `EnergyIntegrator`
- `SessionManager`

---

## 5. Sessielogica en profielconfiguratie

### `SessionManager.h`
**Rol**  
Definieert de generieke sessielogica voor verbruiksessies.

**Belangrijkste types**
- `enum class SessionState`
  - `IDLE`
  - `START_PENDING`
  - `ACTIVE`
  - `STOP_PENDING`
  - `ENDED`
- `struct SessionProfileConfig`
  - thresholds
  - delays
  - reserve SOC
- `struct SessionData`
  - sessiestatus
  - start/stop tijd
  - elapsed time
  - start/current SOC
  - gemiddelde stroom en vermogen
  - runtime estimate

**Belangrijkste methods**
- `begin(...)`
- `reset()`
- `update(...)`
- `getState()`
- `isActive()`
- `getData()`

### `SessionManager.cpp`
**Rol**  
Implementeert de state machine voor actieve energiesessies.

**Belangrijkste methods**
- `SessionManager::begin(...)`  
  Laadt de actieve profielconfiguratie.
- `SessionManager::reset()`  
  Reset sessiestatus en accumulators.
- `SessionManager::update(...)`  
  Werkt de sessie bij op basis van tijd, SOC, ontlaadstroom en ontlaadvermogen.
- `updateState(...)`  
  Stuurt de overgang tussen `IDLE`, `START_PENDING`, `ACTIVE`, `STOP_PENDING` en `ENDED`.
- `updateAverages(...)`  
  Berekent gemiddelde ontlaadstroom en gemiddeld vermogen over de sessie.
- `updateEstimate()`  
  Schat resterende runtime op basis van SOC-daling en reservegrens.

**Belangrijke ontwerpkeuze**
De `SessionManager` werkt met **discharge-only input**:
- laden wordt `0`
- ontladen wordt als positieve stroom/vermogen doorgegeven

Daarmee blijft de sessielogica generiek en gericht op gebruiks- of verbruikssessies.

### `SessionProfileFactory.h`
**Rol**  
Publieke factory-interface voor het opbouwen van de actieve sessieconfiguratie.

**Functie**
- `SessionProfileConfig buildActiveSessionProfileConfig();`

### `SessionProfileFactory.cpp`
**Rol**  
Levert profielspecifieke drempels en timing op basis van `ACTIVE_PROFILE`.

**Ondersteunde profielen in huidige implementatie**
- boat
- camper
- solar buffer
- default/normal

**Architectuurfunctie**
Deze module koppelt compile-time profielselectie aan runtime sessiegedrag.

---

## 6. Batterijschatting en energie-integratie

### `BatteryEstimator.h`
**Rol**  
Definieert een gebruiker-friendly batterijschatting bovenop de ruwe state.

**Belangrijkste type**
- `struct BatteryEstimate`
  - `displaySocPercent`
  - `remainingAh`
  - `remainingWh`
  - `estimatedRemainingSeconds`

**Belangrijkste methods**
- `begin(float capacityAh, float reserveSocPercent)`
- `update(uint32_t nowEpoch, float socPercent, float currentA, float vpack)`
- `getEstimate()`

### `BatteryEstimator.cpp`
**Rol**  
Levert een stabielere schatting van batterijniveau en resterende gebruiksduur.

**Belangrijkste logica**
- initialiseert resterende Ah op basis van start-SOC
- gebruikt coulomb counting voor korte-termijn gedrag
- begrenst resterende capaciteit tussen 0 en nominale capaciteit
- past zachte driftcorrectie toe wanneer de stroom laag is
- smootht de getoonde SOC
- schat resterende runtime op basis van ontlaadstroom en reserve-SOC

**Architectuurfunctie**
Deze component is bedoeld om technische meetruis te verbergen en een rustigere, bruikbare indicatie te geven aan de eindgebruiker, met name in boat/rental scenario's.

### `EnergyIntegrator.h`
**Rol**  
Definieert een eenvoudige integraallaag voor geladen en ontladen energie.

**Belangrijkste type**
- `struct EnergyData`
  - `dischargedAh`
  - `dischargedWh`
  - `chargedAh`
  - `chargedWh`
  - `netAh`
  - `netWh`

**Belangrijkste methods**
- `reset()`
- `update(uint32_t nowEpoch, float signedCurrentA, float signedPowerW)`
- `getData()`

### `EnergyIntegrator.cpp`
**Rol**  
Integreert stroom en vermogen over tijd.

**Belangrijkste logica**
- gebruikt signed current en signed power
- telt ontladen Ah/Wh apart op van geladen Ah/Wh
- berekent netto gebruikswaarden
- negeert ongeldige of te grote `dt`-sprongen

**Architectuurfunctie**
Levert aanvullende technische energie-informatie die geschikt is voor uitgebreide dashboards of beheer-/ownerweergaven.

---

## 7. Webserver en route-afhandeling

### `WebHandlers.h`
**Rol**  
Publieke interface voor webroute-registratie.

**Functie**
- `void webRegisterRoutes(WebServer& server, State& st);`

### `WebHandlers.cpp`
**Rol**  
Registreert en implementeert de standaard webroutes.

**Belangrijkste functies**
- `handleRoot(WebServer& server)`  
  Levert het actieve dashboard als HTML.
- `handleJson(WebServer& server, State& st)`  
  Levert ruwe runtime-data als JSON.
- `webRegisterRoutes(...)`  
  Registreert `/`, `/json` en `/ping`.

**Architectuurfunctie**
Verbindt de interne applicatiestatus met de buitenwereld via een eenvoudige lokale HTTP-interface.

---

## 8. HTML-dispatch en gedeelde UI-hulp

### `html.h`
**Rol**  
Publieke interface voor de dashboard dispatcher.

**Functie**
- `String getDashboardHtml();`

### `html.cpp`
**Rol**  
Dispatcher die op basis van `ACTIVE_PROFILE` het juiste HTML-dashboard kiest.

**Belangrijkste functie**
- `getDashboardHtml()`

**Architectuurfunctie**
Maakt compile-time profielselectie zichtbaar in de UI-laag zonder runtime-keuzelogica of dynamische configuratie.

### `html_common.h`
**Rol**  
Publieke interface voor gedeelde HTML-bouwstenen.

**Functies**
- `htmlHeader(...)`
- `htmlFooter()`

### `html_common.cpp`
**Rol**  
Levert een minimale generieke HTML-header en footer.

**Architectuurfunctie**
Voorkomt duplicatie tussen profielspecifieke dashboards.

---

## 9. Profielspecifieke UI

### `html_boat.cpp`
**Rol**  
Bootdashboard voor de actieve `PROFILE_BOAT` build.

**Belangrijkste functies**
- `formatDuration(...)`  
  Formatteert seconden als `hh:mm:ss`.
- `sessionStateText(...)`  
  Zet `SessionState` om naar leesbare tekst.
- `htmlBoat()`  
  Bouwt de HTML-pagina op.

**Gebruikte data**
- `SessionManager` → trip/runtime-informatie
- `EnergyIntegrator` → verbruikte/geladen Ah en Wh
- `BatteryEstimator` → gestabiliseerde SOC en remaining runtime

**Architectuurfunctie**
Toont hoe een profielspecifieke UI bovenop generieke logische bouwblokken kan worden gezet.

---

## 10. Wi-Fi kanaalselectie

### `WifiScan.h`
**Rol**  
Publieke interface voor kanaalselectie.

**Functie**
- `int pickBestChannel_1_6_11_withLogging();`

### `WifiScan.cpp`
**Rol**  
Scant omliggende Wi-Fi netwerken en kiest het minst drukke access point kanaal uit 1, 6 of 11.

**Belangrijkste functies**
- `clampChannel(int ch)`
- `pickBestChannel_1_6_11_withLogging()`

**Architectuurfunctie**
Optimaliseert de lokale AP-modus voor gebruik in drukke draadloze omgevingen.

---

## Samenhang tussen de componenten

## Opstartpad

```text
setup()
  ├─ rtcInit()
  ├─ rtcSyncFromHarborWifi()
  ├─ stateSetDefaults(st)
  ├─ buildActiveSessionProfileConfig()
  ├─ sessionManager.begin(...)
  ├─ batteryEstimator.begin(...)
  ├─ pickBestChannel_1_6_11_withLogging()
  ├─ WiFi.softAP(...)
  ├─ dataInit()
  └─ webRegisterRoutes(server, st)
```

## Runtime pad

```text
loop()
  ├─ server.handleClient()
  ├─ dataTick(st)
  ├─ rtcNow()
  ├─ energyIntegrator.update(...)
  ├─ batteryEstimator.update(...)
  └─ sessionManager.update(...)
```

## UI pad

```text
HTTP GET /
  └─ handleRoot()
      └─ getDashboardHtml()
          └─ htmlBoat()   [bij PROFILE_BOAT]
```

## Datarelaties

### `State`
wordt geschreven door:
- `DataProcessing.cpp`

wordt gelezen door:
- `ESP32_BatteryGauge_v2_0.ino`
- `WebHandlers.cpp`
- `BatteryEstimator`
- `EnergyIntegrator`
- `SessionManager` (indirect via afgeleide waarden)

### `BatteryEstimate`
wordt geschreven door:
- `BatteryEstimator.cpp`

wordt gelezen door:
- `html_boat.cpp`

### `EnergyData`
wordt geschreven door:
- `EnergyIntegrator.cpp`

wordt gelezen door:
- `html_boat.cpp`

### `SessionData`
wordt geschreven door:
- `SessionManager.cpp`

wordt gelezen door:
- `html_boat.cpp`

---

## Actuele profielondersteuning

### Compile-time gedefinieerd in `Config.h`
- normal
- boat
- camper
- solar buffer
- rental vehicle
- backup power

### Reëel uitgewerkt in deze codebasis
- boat UI aanwezig
- sessieprofielen deels uitgewerkt voor boat, camper en solar buffer
- fallback/default gedrag aanwezig voor overige profielen

Dit betekent dat de architectuur al breder is opgezet dan de huidige UI-implementatie.

---

## Waarom deze architectuur past bij BatteryGauge v2.0

Deze architectuur ondersteunt het centrale idee van het project:

- bestaande accu's hebben vaak al een **BMS**, maar niet altijd een bruikbare lokale interface
- een bestaand display toont vaak alleen technische waarden
- cloud- of app-afhankelijke oplossingen maken de gebruiker afhankelijk van de fabrikant
- BatteryGauge voegt daar een **lokale, configureerbare, scenario-afhankelijke monitorklasse** aan toe

De softwarestructuur weerspiegelt dat:

- de ruwe batterijdata blijft gescheiden van de gebruikerspresentatie
- profielen zijn compile-time selecteerbaar
- de logische kern is herbruikbaar
- de UI kan afgestemd worden op gebruiksscenario's zoals boat rental, camper of solar buffer

---

## Sterke punten van de huidige opzet

- duidelijke scheiding tussen data, logica en UI
- compile-time profielkeuze houdt runtime eenvoudig
- `State` als centrale dataoverdracht is helder en efficiënt
- sessielogica is generiek en herbruikbaar
- estimator en integrator vullen elkaar logisch aan
- webinterface is lichtgewicht en lokaal bruikbaar
- architectuur is goed uitbreidbaar met extra dashboards of profielspecifieke logica

---

## Mogelijke vervolgstappen

Voor een volgende evolutie van de softwarearchitectuur zijn logische uitbreidingen:

- extra profielspecifieke HTML-bestanden voor camper, solar, rental en backup
- centralere configuratie van accucapaciteit per profiel
- expliciete scheiding tussen gebruiker UI en eigenaar/diagnostics UI
- JSON-uitbreiding met estimator- en sessiedata
- documentatie van seriële payloadformaten en alarmbetekenissen
- opsplitsing in mappen zoals `/logic`, `/ui`, `/platform`, `/profiles`

---

## Samenvatting

ESP32 BatteryGauge v2.0 is momenteel opgebouwd als een compacte maar doordachte embedded architectuur met:

- een centrale gedeelde `State`
- een flexibele data-invoerlaag
- een tijdbron via RTC/NTP
- een generieke sessiemanager
- een batterijschatter voor stabielere gebruikersinformatie
- een energie-integrator voor uitgebreide inzichten
- een lokale webserver met profielspecifieke HTML-dispatch

Daardoor is de codebasis klein genoeg om begrijpelijk te blijven, maar modulair genoeg om door te groeien naar meerdere gebruiksscenario's.
