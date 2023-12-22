#pragma once

#include <SFML/System/Clock.hpp>
#include <functional>
#include <limits>

class FpsCounter {
    sf::Clock clock;
    sf::Clock refreshClock;
    float refresh_interval;
    std::function<void(float min, float current, float max)> callback;
    float fps_actual = 0;
    float fps_min = std::numeric_limits<float>::max();
    float fps_max = 0;
    float fps_min_shadow = 0;
    float fps_max_shadow = 0;

   public:
    FpsCounter(float refresh_interval, decltype(callback) cb = nullptr)
        : refresh_interval{refresh_interval}, callback{cb}
    {
        clock.restart();
    }
    void restart() { clock.restart(); }
    void update()
    {
        fps_actual = 1.f / clock.getElapsedTime().asSeconds();

        if (fps_actual < fps_min) fps_min = fps_actual;
        if (fps_actual > fps_max) fps_max = fps_actual;
        if (refreshClock.getElapsedTime().asSeconds() > refresh_interval) {
            refreshClock.restart();
            fps_min_shadow = fps_min;
            fps_max_shadow = fps_max;
            resetMinMax();
            if (callback) callback(fps_min_shadow, fps_actual, fps_max_shadow);
        }

        clock.restart();
    }

    int getFps() { return fps_actual; }
    int getFpsMin() { return fps_min_shadow; }
    int getFpsMax() { return fps_max_shadow; }

   private:
    void resetMinMax()
    {
        fps_min = std::numeric_limits<float>::max();
        fps_max = 0;
    }
};