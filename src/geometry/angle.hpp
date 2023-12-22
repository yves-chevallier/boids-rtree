#pragma once

#include <numbers>
#include <cmath>

class Angle {
public:
    static constexpr double pi = std::numbers::pi;
    Angle(double radians = 0.0) : radians(normalizeRadians(radians)) {}

    void setRadians(double radians) { this->radians = normalizeRadians(radians); }
    double getRadians() const { return radians; }

    operator double() const { return radians; }

    static double shortestDistance(const Angle& a1, const Angle& a2) {
        double delta = std::fmod(a2.radians - a1.radians, 2 * pi);
        if (delta < -pi)
            delta += 2 * pi;
        else if (delta > pi)
            delta -= 2 * pi;
        return delta;
    }
    void setDegrees(double degrees) { this->radians = normalizeRadians(degrees * pi / 180.0); }
    double getDegrees() const { return radians * 180.0 / pi; }
    void rotate(double radians) { this->radians = normalizeRadians(this->radians + radians); }
private:
    double radians;

    // Normalize angle to be within -π to +π
    static double normalizeRadians(double radians) {
        double normalized = std::fmod(radians, 2 * pi);
        if (normalized < -pi)
            normalized += 2 * pi;
        else if (normalized > pi)
            normalized -= 2 * pi;
        return normalized;
    }
};