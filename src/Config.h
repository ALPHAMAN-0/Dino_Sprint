#ifndef CONFIG_H
#define CONFIG_H

// Shared tuning constants. Game code works in a fixed logical coordinate
// system (glOrtho 0..LOGICAL_W x 0..LOGICAL_H) so it is resolution-independent;
// the reshape handler letterboxes this space into the real window.
namespace cfg {

constexpr float LOGICAL_W         = 1000.0f;
constexpr float LOGICAL_H         = 400.0f;              // 2.5:1, same as the background art
constexpr float TARGET_ASPECT     = LOGICAL_W / LOGICAL_H;

constexpr float BASE_SCROLL_SPEED = 120.0f;              // logical units per second at 1.0x
constexpr float NEAR_LAYER_FACTOR = 1.5f;                // foreground strip scrolls faster (parallax)
constexpr float FOREGROUND_SPLIT  = 0.20f;               // bottom fraction of image = near layer

constexpr int   FRAME_MS          = 16;                  // ~60 FPS timer period
constexpr float MAX_DT            = 0.1f;                // clamp dt after window-drag stalls

constexpr float SPEED_MIN  = 0.25f;
constexpr float SPEED_MAX  = 3.0f;
constexpr float SPEED_STEP = 0.25f;

// Gameplay ground plane (inside the near road strip): where the dino runs
// and obstacles stand.
constexpr float GROUND_Y = 30.0f;

// Day/night cycle: the sun sinks smoothly for the whole day (visibly lower
// every 10 s, fully set at 60 s), night brings moon + stars, then sunrise.
constexpr float DAY_SECONDS     = 60.0f;
constexpr float NIGHT_SECONDS   = 30.0f;
constexpr float SUNRISE_SECONDS = 10.0f;
constexpr float CYCLE_SECONDS   = DAY_SECONDS + NIGHT_SECONDS + SUNRISE_SECONDS;
constexpr float DUSK_START_ALT  = 0.35f;   // darkness ramps in below this sun altitude

} // namespace cfg

#endif // CONFIG_H
