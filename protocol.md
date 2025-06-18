## Summary of analyzed frames and commands

### 1. Pressed keys (keypad command)
- **Format:**  
  ```hex
  d1 00 XX
  ```
- **XX = key code:**
  - `0e`: Arm stay
  - `10`: P (Program)
  - `0f`: Bypass
  - `0b`: Memory
  - `11`: E (Enter)
  - `0X`: Numbers (0-9)
- **Function:** Indicates which key was pressed on the keypad.

---

### 2. Status lights (lights command)
- **Format:**  
  ```hex
  10 00 01 c3 XX 00
  ```
- **XX:**
  - `10`: Turn on Prog light
  - `00`: Turn off Prog or Ready light
- **c3/c2:** Changes the light state (`c3` = on, `c2` = off)
- **Function:** Turn on/off status lights (Prog, Ready).

---

### 3. Beeps (sound command)
- **Format:**  
  ```hex
  14 00 YY 00 00 00 8Z
  ```
- **YY:**
  - `01`: Single beep
  - `15`: Triple beep
- **8Z:** `80/81` (Beep type or mode?)
- **Function:** Indicates that the panel emits a single or triple beep.

---

### 4. Operation modes
- **Format:**  
  ```hex
  15 00 XX 00 00 03
  ```
- **XX:**
  - `00`: Exit mode (bypass, memory, programming, etc.)
  - `01`: Enter master programming mode
  - `02`: Enter installer programming mode
- **Function:** Enter/exit special modes.

---

### 5. Zone activity
- **Format:**  
  ```hex
  12 00 XX YY 00 00 00
  ```
- **XX:** bitmap of active zones (e.g.: `10` = zone 1, `12` = zone 2+5, `02` = zone 2, etc.)
- **YY:** bitmap of triggered zones.
- **Function:** Indicates which zones are active/triggered.

---

### 6. Zone bypass
- **Format:**  
  ```hex
  1b 00 00 00 00 00 00 00 00 00
  ```
- **Function:** Shows the state of bypassed zones.

---

### 7. Event memory
- **Format:**  
  ```hex
  20 00 0a 70 00 01 00 00 01 01
  ```
- **Function:** Shows event memory information.

---

### 8. Real Time Clock
- **Format:**  
  ```hex
  54 DW MM_H MM_L SS DD MM YY
  ```
- **DW:** Day of the week (1=Sunday, 2=Monday, 3=Tuesday, ...).
- **MM_H:** Minutes since midnight (high byte).
- **MM_L:** Minutes since midnight (low byte).
- **SS:** Seconds.
- **DD:** Day.
- **MM:** Month.
- **YY:** Year.
- **Function:** Unknown frame, possibly synchronization or heartbeat.

---

### 9. System states (frames 11xxxxx)
- **Format:**  
  ```hex
  11 XX XX XX XX
  ```
- **Examples:**
  - `11 00 00 00 00` : Disarmed (system disarmed)
  - `11 01 00 00 00` : Armed (system armed)
  - `11 00 01 00 00` : Arming (arming countdown started)
  - `11 00 00 01 00` : Stay mode active (partial arm)
- **Function:** Indicates the global system state (armed, disarmed, stay armed, etc.).
