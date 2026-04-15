# Stage Tracker
### NIDA Digital Electronics Workshop

A single pixel follows a performer across a stage. A side-mounted distance sensor tracks their position left-to-right and maps it to an addressable LED strip in real time.

Built for a hands-on intro workshop — simple, readable code, easy to tweak.

---

## Hardware

| Part | Notes |
|---|---|
| ESP32-S3 Supermini | Microcontroller |
| VL53L0X ToF sensor | I2C distance sensor, side-mounted |
| WS2812B LED strip | Tested at 22 LEDs |
| 2x tactile buttons | Mode + colour toggle |
| Jumper cables | Pre-header-pinned for solderless assembly |

**Power:** LED strip powered from ESP32 5V pin (USB). Fine for short strips at moderate brightness. Use external power injection for longer strips.

---

## Wiring

```
VL53L0X  SDA  →  IO8
VL53L0X  SCL  →  IO9
VL53L0X  VIN  →  3V3
VL53L0X  GND  →  GND

LED strip DATA  →  IO5
LED strip 5V    →  5V
LED strip GND   →  GND

Button A  →  IO1 + GND   (mode toggle)
Button B  →  IO2 + GND   (colour toggle)
```

**Button wiring note:** Tactile buttons have 4 pins — two pairs internally connected. Wire one pin from each side: one to the IO pin, one to GND. Internal pullups handle the rest, no resistor needed.

---

## Arduino IDE Setup

**Board settings:**
- Board: `ESP32S3 Dev Module`
- PSRAM: `Disabled`
- Upload Speed: `921600`

**If upload fails:** Hold BOOT, tap RESET, release BOOT, then upload.

**Libraries** (install via Library Manager):
- `FastLED`
- `Adafruit VL53L0X`

---

## Buttons

| Button | Function |
|---|---|
| A | Toggle follow mode / static (full strip on) |
| B | Toggle fixed colour / rainbow drift |

Static mode is useful as a fallback if the sensor has issues on the day.

---

## Tunable Parameters

All at the top of the file — these are the ones to play with:

```cpp
#define NUM_LEDS        22      // Match to your strip length

#define SENSOR_MIN_MM   100     // Distance at near edge of stage (mm)
#define SENSOR_MAX_MM   2000    // Distance at far edge of stage (mm)

#define SMOOTHING       0.08f   // 0.0 = frozen, 1.0 = instant snap
                                // 0.05–0.15 is the sweet spot

#define PIXEL_COLOR     CRGB::Cyan  // Fixed colour mode
                                    // Try: Red, Green, Blue, White, Purple
```

**Calibrating for your stage:**
1. Place the sensor at one end of the strip, pointing along the stage
2. Measure the distance from sensor to the near edge and far edge of the acting area
3. Update `SENSOR_MIN_MM` and `SENSOR_MAX_MM` to match

**Colour order:** If colours look wrong, change `GRB` to `RGB` or `BGR` in the LED config.

**Drift speed:** In `COLOUR_DRIFT` mode, the rainbow cycles through hue. Change `driftH += 0.3f` — higher = faster.

---

## Serial Monitor

Connect at **115200 baud** to see status messages. Useful for debugging sensor connection and button presses.

```
Stage Tracker — starting up
Sensor ready.
Mode: FOLLOW
Colour mode: DRIFT
```

If you see `VL53L0X not found` the strip will flash red — check your wiring on IO8/IO9.

---

## Firmware Logic

```
1. Read distance from VL53L0X sensor
2. Constrain to SENSOR_MIN_MM → SENSOR_MAX_MM
3. Map to LED index 0 → NUM_LEDS
4. Smooth toward target (exponential moving average)
5. Light that pixel, black out the rest
6. Loop at ~60fps
```

Colour is applied after position is calculated, so both modes work independently.

---

## Going Further

- Swap VL53L0X for a **TF-Luna** lidar for longer stage ranges (up to 8m)
- Add a **trail effect** — fade trailing pixels rather than hard black
- Use **multiple pixels** for a wider pool of light
- Drive from a **PC PSU** via a breakout module for longer strips (sustainability talking point)
- Explore [FastLED docs](https://fastled.io) for more effects
- [Instructables](https://www.instructables.com) and [GitHub](https://github.com) for similar projects and inspiration
