#pragma once

#include "vector2.hpp"

#include <cmath>
#include <compare>
#include <iostream>
#include <numbers>

#if __has_include(<SFML/Graphics.hpp>)
#include <SFML/Graphics.hpp>
#ifndef WITH_SFML
#define WITH_SFML
#endif
#endif

#if __has_include(<imgui.h>)
#include <imgui.h>
#ifndef WITH_IMGUI
#define WITH_IMGUI
#endif
#endif

template <class T>
class Vector2 {
   public:
    using value_type = T;
    using class_type = Vector2<T>;
    value_type x, y;

    Vector2(T value) : x(value), y(value) {}
    Vector2(T x = 0, T y = 0) : x(x), y(y) {}
    Vector2(const class_type& vector) : x(vector.x), y(vector.y) {}

#ifdef WITH_SFML
    Vector2(const sf::Vector2<T>& vector) : x(vector.x), y(vector.y) {}
    operator sf::Vector2<T>() const { return sf::Vector2<T>(x, y); }
#endif

#ifdef WITH_IMGUI
    Vector2(const ImVec2& vector) : x(vector.x), y(vector.y) {}
    operator ImVec2() const { return ImVec2(x, y); }
#endif

    static auto from_angle_degrees(float angle)
    {
        const auto rad = angle * std::numbers::template pi_v<T> / 180;
        return class_type(std::cos(rad), std::sin(rad));
    }

    static auto from_angle(float angle)
    {
        return class_type(std::cos(angle), std::sin(angle));
    }

    // Random vector between min and max
    static auto from_random(T min, T max)
    {
        return class_type(min + randf() * (max - min), min + randf() * (max - min));
    }

    // Random vector between x0 and x1, y0 and y1
    static auto from_random(T x0, T x1, T y0, T y1)
    {
        return class_type(x0 + randf() * (x1 - x0), y0 + randf() * (y1 - y0));
    }

    // Random unit vector
    static auto from_random() { return class_type(randf(), randf()); }

    // Random vector with min and max magnitude, centered around zero
    static auto from_symetric_random(T min, T max)
    {
        return from_random(min, max) * (randf() > 0.5 ? 1 : -1);
    }

    constexpr auto operator+(const class_type& right) const
    {
        return class_type(x + right.x, y + right.y);
    }
    constexpr auto operator-(const class_type& right) const
    {
        return class_type(x - right.x, y - right.y);
    }
    constexpr auto operator+(T n) const { return class_type(x + n, y + n); }
    constexpr auto operator*(T n) { return class_type(x * n, y * n); }
    constexpr auto operator/(T n) { return class_type(x / n, y / n); }

    constexpr auto operator==(const class_type& right) const
    {
        return x == right.x && y == right.y;
    }
    constexpr auto operator!=(const class_type& right) const
    {
        return x != right.x || y != right.y;
    }
    constexpr auto operator=(const class_type& right)
    {
        x = right.x;
        y = right.y;
        return *this;
    }

    auto operator<=>(const class_type& right) const
    {
        return lengthSquared() <=> right.lengthSquared();
    }

    // Dot product
    auto operator*(const class_type& right) const { return x * right.x + y * right.y; }
    // Cross product
    auto operator^(const class_type& right) const { return x * right.y - y * right.x; }
    auto operator*=(const T& n)
    {
        x *= n;
        y *= n;
        return *this;
    }
    auto operator/=(const T& n)
    {
        x /= n;
        y /= n;
        return *this;
    }
    auto operator+=(const T& n)
    {
        x += n;
        y += n;
        return *this;
    }
    auto operator-=(const T& n)
    {
        x -= n;
        y -= n;
        return *this;
    }
    auto operator-=(const class_type& n)
    {
        x -= n.x;
        y -= n.y;
        return *this;
    }
    auto operator+=(const class_type& n)
    {
        x += n.x;
        y += n.y;
        return *this;
    }

    auto distance(const class_type& right) const
    {
        auto dx = x - right.x;
        auto dy = y - right.y;
        return sqrt(dx * dx + dy * dy);
    }

    auto distanceSquared(const class_type& right) const
    {
        auto dx = x - right.x;
        auto dy = y - right.y;
        return dx * dx + dy * dy;
    }

    auto length() const { return std::hypot(x, y); }
    auto lengthSquared() const
    {
        using LargerType = std::conditional_t<std::is_floating_point_v<T>, double, long long>;
        LargerType x_large = x;
        LargerType y_large = y;
        return static_cast<T>(x_large * x_large + y_large * y_large);
    }

    constexpr auto rotate(float rad) const
    {
        const auto cs = std::cos(rad);
        const auto sn = std::sin(rad);
        return class_type(x * cs - y * sn, x * sn + y * cs);
    }

    constexpr auto rotateDegrees(float angle) const
    {
        const auto rad = angle * std::numbers::template pi_v<float> / 180;
        return rotate(rad);
    }

    constexpr auto normalize() const
    {
        const T mag = length();
        return class_type(x / mag, y / mag);
    }
    constexpr auto setMag(T magnitude)
    {
        return class_type(normalize() * magnitude);
    }

    constexpr auto limit(T max)
    {
        if (length() > max) {
            return setMag(max);
        }
        return *this;
    }

    constexpr auto limit(T min, T max)
    {
        if (lengthSquared() < min * min) return setMag(min);
        if (lengthSquared() > max * max) return setMag(max);
        return *this;
    }

    constexpr auto constrain(T left, T top, T right, T bottom)
    {
        if (x < left) x = left;
        if (x > right) x = right;
        if (y < top) y = top;
        if (y > bottom) y = bottom;
        return *this;
    }

    auto heading() const { return atan2(y, x); }

    auto angle(const class_type& right) const { return std::atan2(right.y - y, right.x - x); }
    auto angle() const { return std::atan2(y, x); }

    auto toroidal_difference(const value_type width, const value_type height) const
    {
        return Vector2<T>(std::abs(x) > width / 2 ? -(width - std::abs(x)) * std::signbit(x) : x,
            std::abs(y) > height / 2 ? -(height - std::abs(y)) * std::signbit(y) : y);
    }

    auto toroidal_distance(const class_type& other, const value_type width, const value_type height) const
    {
        return sqrt(toroidal_distance2(other, width, height));
    }

    auto toroidal_distance2(const class_type& other, const value_type width, const value_type height) const
    {
        value_type dx = x - other.x;
        value_type dy = y - other.y;

        dx = dx > width / 2 ? width - dx : dx;
        dy = dy > height / 2 ? height - dy : dy;

        return dx * dx + dy * dy;
    }

    constexpr auto toroidalWrap(const T width, const T height)
    {
        if (x < 0) x = width + x;
        if (x > width) x = x - width;
        if (y < 0) y = height + y;
        if (y > height) y = y - height;
        return *this;
    }

private:
    static constexpr T randf()
    {
        return static_cast<T>(rand()) / static_cast<T>(static_cast<T>(RAND_MAX));
    }
};

// template <typename T>
// std::ostream& operator<<(std::ostream& os, const Vector2<T>& vector)
// {
//     os << "(" << vector.x << ", " << vector.y << ")";
//     return os;
// }

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;
