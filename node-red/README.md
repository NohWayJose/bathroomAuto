# Bathroom Automation — Node-RED

## Infrastructure

- **Host:** DietPi at `192.168.7.245:1880`
- **Node-RED:** v4.1.0
- **Dashboard:** `@flowfuse/node-red-dashboard` v1.30.2 (FlowFuse Dashboard 2, Vue 3 / Vuetify)
- **MQTT broker:** Mosquitto on the same host (`192.168.7.245:1883`)

## Deployment

The Node-RED REST API (`PUT /flows`) returns 404 on this version. Deployment is done by writing
directly to the flows file and restarting the service:

```bash
scp flows.json root@192.168.7.245:/mnt/dietpi_userdata/node-red/flows.json
ssh root@192.168.7.245 "systemctl restart node-red"
```

Python scripts in `/tmp/fix_*.py` were used throughout development to patch specific nodes
programmatically before copying the result back here.

Context storage is enabled in `settings.js` (`contextStorage: localfilesystem`) so global
variables survive Node-RED restarts.

## Flow Architecture

### Tabs / Pages

| Tab | Page | Purpose |
|-----|------|---------|
| Bathroom | Bathroom | Live sensor display, fan controls, mode selection, clock |
| Settings | Settings | Slider config, theme editor |

### Key Nodes

| ID | Name | Role |
|----|------|------|
| `498e716b9e836af6` | Fan Dango | Main fan logic — 7 outputs |
| `582ca29e5eb191ab` | StoreMode | Stores selected mode, resets timer state on mode change |
| `5b18f792835863a2` | ModeVisCtrl | CSS injection template — shows/hides widget groups by mode |
| `b5a317c9926c2696` | SetDefaults | Inject on startup — pushes default values to all sliders and stores |
| `41b1dcb34a5ead83` | SunsetCheck | Calculates sunset time using `suncalc2` |
| `3db6b54aa070b26c` | ParseThemes | Reads `bathroom-themes.json`, populates theme dropdown |
| `9b492f4f11c3800e` | ApplyTheme | Applies selected theme to all colour outputs, saves `currentTheme` to global |
| `bdaf68dcf46d2db4` | HandlePaste | Validates and saves edited theme JSON, re-applies theme |

### Fan Dango outputs

1. Timestamp → MQTT
2. DeviceSwitch (relay on/off)
3. StateLabel
4. ModeDD (mode dropdown update)
5. StProg → `bathroom/stprog` MQTT (short timer progress)
6. LtProg → `bathroom/ltprog` MQTT (long timer progress)
7. ModeVisCtrl (CSS visibility trigger)

### Modes

- **`humidity`** — fan runs when humidity exceeds threshold; shows temp/humidity gauges
- **`shortTimer`** — fixed short timer; hides gauges, shows short timer progress bar
- **`longTimer`** — fixed long timer; hides gauges, shows long timer progress bar

## Dashboard Visibility (CSS injection)

`ui_update.visible` is unreliable for `ui-gauge`, `ui-chart`, and `ui-progress` in this
version. Instead, `ui-group` nodes are tagged with `className`, and a hidden `ui-template`
(ModeVisCtrl) injects a `<style>` rule into the page head on every mode change.

| Class | Groups hidden |
|-------|--------------|
| `.sensor-group` | Temp, Humidity |
| `.timer-group` | Timer progress container |
| `.st-progress` | Short timer progress bar |
| `.lt-progress` | Long timer progress bar |

ModeVisCtrl lives in a group with class `fan-ctrl-hidden` which is permanently hidden via a
self-injected style on mount.

## Timer Progress (MQTT)

Fan Dango publishes JSON `{ v, max, show }` to `bathroom/stprog` and `bathroom/ltprog` on a
1-minute cadence while the respective timer is active. Two MQTT-in nodes subscribe and feed
parse functions that output percentage + "Time remaining: N min" label to `ui-progress` bars
on the Bathroom page.

## Settings Page

Sliders and their defaults (set by SetDefaults inject on startup):

| Slider | Default | Units | Global key |
|--------|---------|-------|------------|
| Humidity Threshold | 85 | % | `humidityThreshold` |
| Short Timer | 5 | min | `shortTimer` |
| Long Timer | 15 | min | `longTimer` |
| Night Mode Delay | 0 | h | `nightModeDelay` |

Each slider group title updates dynamically to show the current value (e.g. `Humidity - 85%`)
via a small hidden `ui-template` (templateScope: `local`) wired to both the slider output and
SetDefaults.

## Themes

Themes are stored in `bathroom-themes.json` as a JSON object keyed by theme name. Each theme
defines hex colour strings for: `nightLight`, `background`, `fiveMinTick`, `fifteenMinTick`,
`minuteHand`, `hourHand`.

The selected theme name is persisted to global context (`currentTheme`) and restored on
restart via `ParseThemes`. The Clipboard widget on the Settings page lets you copy the current
theme JSON and paste in edits; saving re-applies the theme immediately and writes the file.

## MQTT Topic Reference

All devices publish/subscribe to the Mosquitto broker at `192.168.7.245:1883`.

### Bathroom sensors

| Topic | Direction | Publisher | Valid values | Purpose |
|-------|-----------|-----------|--------------|---------|
| `bathroom/temp` | pub | SHT30 v2 ESP32 | float (°C) | Bathroom air temperature |
| `bathroom/humidity` | pub | SHT30 v2 ESP32 | float (%) | Bathroom relative humidity |
| `esp/dht/b/temp` | pub | ESP8266 (simplified) | float (°C) | Bathroom temp (older sketch) |
| `esp/dht/b/humid` | pub | ESP8266 (simplified) | float (%) | Bathroom humidity (older sketch) |
| `esp/dht/h/temp` | pub | Hall ESP | float (°C) | Hall temperature |
| `esp/dht/h/hum` | pub | Hall ESP | float (%) | Hall humidity |
| `esp/dht/k/temp` | pub | Kitchen ESP | float (°C) | Kitchen temperature |
| `esp/dht/k/hum` | pub | Kitchen ESP | float (%) | Kitchen humidity |
| `esp/dht/k/client` | pub | Kitchen ESP | string | Kitchen client connection status |
| `lounge/temp` | pub | Lounge sensor | float (°C) | Lounge temperature |
| `lounge/humidity` | pub | Lounge sensor | float (%) | Lounge humidity |
| `lounge/client` | pub | Lounge sensor | string | Lounge client connection status |

Topic location codes: `b` = bathroom, `h` = hall, `k` = kitchen.

### Fan control

| Topic | Direction | Publisher → Subscriber | Valid values | Purpose |
|-------|-----------|------------------------|--------------|---------|
| `bathroom/fanSwitch` | pub | Node-RED → FanRelay ESP / Node-RED (echo) | `ON`, `OFF` | Commands the fan relay; also echoed back to NR to detect manual overrides |
| `bathroom/fan/mode` | pub | Node-RED → Node-RED | `humidity`, `shortTimer`, `longTimer` | Sets the active fan control mode |
| `esp/dht/b/fanState` | pub | SHT30 v2 ESP32 → Node-RED | `fanOff`, `fanOn10`, `humidityTrigger` | Fan state reported by the sensor node |
| `esp/dht/b/fanresume` | sub | Node-RED → SHT30 v2 ESP32 | any | Tells ESP to resume normal fan operation |
| `esp/dht/b/fanStateNR` | sub | Node-RED → SHT30 v2 ESP32 | any | Node-RED-driven fan state override |

### Timer progress (internal, NR → NR via MQTT)

| Topic | Direction | Valid values | Purpose |
|-------|-----------|--------------|---------|
| `bathroom/stprog` | pub/sub | JSON `{"v": float, "max": float, "show": bool}` | Short timer elapsed (min) and total; `show:false` hides progress bar |
| `bathroom/ltprog` | pub/sub | JSON `{"v": float, "max": float, "show": bool}` | Long timer elapsed (min) and total; `show:false` hides progress bar |

### Mirror LED colours (Node-RED → Mirror ESP32)

All colour topics carry a 6-character hex RGB string with no `#`, e.g. `0062FF`.

| Topic | Purpose |
|-------|---------|
| `bathroom/mirror/nightLightCol` | Night-light LED colour |
| `bathroom/mirror/BGLightCol` | Background / ambient LED colour |
| `bathroom/mirror/fiveMinTickCol` | Clock 5-minute tick mark colour |
| `bathroom/mirror/fifteenMinTickCol` | Clock 15-minute tick mark colour |
| `bathroom/mirror/minCol` | Minute hand colour |
| `bathroom/mirror/hourCol` | Hour hand colour |

### Mirror control

| Topic | Direction | Publisher → Subscriber | Valid values | Purpose |
|-------|-----------|------------------------|--------------|---------|
| `bathroom/mirror/heater` | pub | Node-RED → Mirror ESP32 | `ON`, `OFF` | Demist heater relay |
| `bathroom/mirror/light` | pub | Node-RED → Mirror ESP32 | `ON`, `OFF` | Mirror light on/off |
| `bathroom/mirror/light/switch` | sub | Physical switch → Node-RED | any | Physical mirror light switch state |
| `bathroom/mirror/SSH/switch` | sub | SSH trigger → Node-RED | any | SSH-triggered mirror switch |
| `bathroom/mirror/reset` | pub | Node-RED → Mirror ESP32 | any | Resets the mirror ESP32 |

### Settings config (Node-RED internal)

| Topic | Valid values | Purpose |
|-------|--------------|---------|
| `bathroom/humidityThreshold` | integer (%) | Humidity level that triggers fan in humidity mode |
| `bathroom/shortTimer` | integer (min) | Short timer duration |
| `bathroom/longTimer` | integer (min) | Long timer duration |

### Other / integrations

| Topic | Broker | Direction | Purpose |
|-------|--------|-----------|---------|
| `esp/soil/a/humid` | local | pub | Soil moisture sensor reading |
| `esp/soil/a/irrigate` | local | sub | Irrigation trigger |
| `glucose/reading` | local | pub/sub | CGM glucose data relay |
| `company/code` | remote | sub | Access code MQTT input (OpenSesame) |
| `cmnd/opensesame/POWER1` | remote | pub | Shelly switch command (OpenSesame) |

## Known Errors on Startup (pre-existing, benign)

Several `ui-gauge` and `ui-chart` nodes report "No group configured" / `getBase` TypeError.
These are orphaned widgets from earlier iterations and don't affect operation.
