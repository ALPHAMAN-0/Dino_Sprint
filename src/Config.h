#ifndef CONFIG_H
#define CONFIG_H

namespace cfg {

constexpr float LOGICAL_W         = 1000.0f;
constexpr float LOGICAL_H         = 400.0f;
constexpr float TARGET_ASPECT     = LOGICAL_W / LOGICAL_H;

constexpr float BASE_SCROLL_SPEED = 120.0f;
constexpr float NEAR_LAYER_FACTOR = 1.5f;
constexpr float FOREGROUND_SPLIT  = 0.20f;
constexpr float SCENE_PERIOD      = 2000.0f;

constexpr int   FRAME_MS          = 16;
constexpr float MAX_DT            = 0.1f;

constexpr float SPEED_MIN  = 0.25f;
constexpr float SPEED_MAX  = 3.0f;
constexpr float SPEED_STEP = 0.25f;

constexpr float GROUND_Y = 30.0f;

constexpr float DAY_SECONDS     = 60.0f;
constexpr float NIGHT_SECONDS   = 30.0f;
constexpr float SUNRISE_SECONDS = 10.0f;
constexpr float CYCLE_SECONDS   = DAY_SECONDS + NIGHT_SECONDS + SUNRISE_SECONDS;
constexpr float DUSK_START_ALT  = 0.35f;

}

#endif
