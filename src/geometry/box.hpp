#pragma once

#include <cmath>
#include <vector>

#if __has_include(<SFML/Graphics.hpp>)
#include <SFML/Graphics.hpp>
#ifndef WITH_SFML
#define WITH_SFML
#endif
#endif

#include "geometry/vector2.hpp"

template <class T>
class Box {
   public:
    using value_type = T;
    using vector_type = Vector2<T>;

    Box(vector_type position, T width, T height)
        : left(position.x - width / 2), top(position.y - height / 2), width(width), height(height)
    {
    }

    Box(vector_type position, T size)
        : left(position.x - size / 2), top(position.y - size / 2), width(size), height(size)
    {
    }

    Box(T rectLeft, T rectTop, T rectWidth, T rectHeight)
        : left(rectLeft), top(rectTop), width(rectWidth), height(rectHeight)
    {
    }

    Box(vector_type topLeft, vector_type bottomRight)
        : left(topLeft.x),
          top(topLeft.y),
          width(bottomRight.x - topLeft.x),
          height(bottomRight.y - topLeft.y)
    {
    }

#ifdef WITH_SFML
    Box(const sf::Rect<T>& rect)
        : left(rect.left), top(rect.top), width(rect.width), height(rect.height)
    {
    }
    operator sf::Rect<T>() const { return sf::Rect<T>(left, top, width, height); }
    operator sf::RectangleShape() const
    {
        sf::RectangleShape rect(vector_type(width, height));
        rect.setPosition(left, top);
        return rect;
    }

    void draw(sf::VertexArray& vertices, sf::Color color = sf::Color::White) const
    {
        vertices.append(sf::Vertex(topLeft(), color));
        vertices.append(sf::Vertex(topRight(), color));

        vertices.append(sf::Vertex(topRight(), color));
        vertices.append(sf::Vertex(bottomRight(), color));

        vertices.append(sf::Vertex(bottomRight(), color));
        vertices.append(sf::Vertex(bottomLeft(), color));

        vertices.append(sf::Vertex(bottomLeft(), color));
        vertices.append(sf::Vertex(topLeft(), color));
    }
#endif

    vector_type center() const { return {left + width / 2, top + height / 2}; }
    T area() const { return width * height; }
    T right() const { return left + width; }
    T bottom() const { return top + height; }
    T perimeter() const { return 2 * (width + height); }

    auto scale(T factor) const
    {
        const T newWidth = width * factor;
        const T newHeight = height * factor;
        const T newLeft = left - (newWidth - width) / 2;
        const T newTop = top - (newHeight - height) / 2;
        return Box<T>(newLeft, newTop, newWidth, newHeight);
    }
    auto offset(T offset) const
    {
        const T newWidth = width + offset;
        const T newHeight = height + offset;
        const T newLeft = left - (newWidth - width) / 2;
        const T newTop = top - (newHeight - height) / 2;
        return Box<T>(newLeft, newTop, newWidth, newHeight);
    }

    vector_type topLeft() const { return vector_type(left, top); }
    vector_type topRight() const { return vector_type(right(), top); }
    vector_type bottomLeft() const { return vector_type(left, bottom()); }
    vector_type bottomRight() const { return vector_type(right(), bottom()); }

    bool contains(T x, T y) const
    {
        const T minX = std::min(left, static_cast<T>(left + width));
        const T maxX = std::max(left, static_cast<T>(left + width));
        const T minY = std::min(top, static_cast<T>(top + height));
        const T maxY = std::max(top, static_cast<T>(top + height));
        return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
    }
    bool contains(const vector_type& point) const { return contains(point.x, point.y); }
    bool intersects(const Box<T>& box) const
    {
        const T minX = std::min(left, static_cast<T>(left + width));
        const T maxX = std::max(left, static_cast<T>(left + width));
        const T minY = std::min(top, static_cast<T>(top + height));
        const T maxY = std::max(top, static_cast<T>(top + height));
        const T minX2 = std::min(box.left, static_cast<T>(box.left + box.width));
        const T maxX2 = std::max(box.left, static_cast<T>(box.left + box.width));
        const T minY2 = std::min(box.top, static_cast<T>(box.top + box.height));
        const T maxY2 = std::max(box.top, static_cast<T>(box.top + box.height));
        return (minX < maxX2) && (maxX > minX2) && (minY < maxY2) && (maxY > minY2);
    }

    Box<T> intersection(const Box<T>& box) const
    {
        const T minX = std::min(left, static_cast<T>(left + width));
        const T maxX = std::max(left, static_cast<T>(left + width));
        const T minY = std::min(top, static_cast<T>(top + height));
        const T maxY = std::max(top, static_cast<T>(top + height));
        const T minX2 = std::min(box.left, static_cast<T>(box.left + box.width));
        const T maxX2 = std::max(box.left, static_cast<T>(box.left + box.width));
        const T minY2 = std::min(box.top, static_cast<T>(box.top + box.height));
        const T maxY2 = std::max(box.top, static_cast<T>(box.top + box.height));
        const T newLeft = std::max(minX, minX2);
        const T newTop = std::max(minY, minY2);
        const T newWidth = std::min(maxX, maxX2) - newLeft;
        const T newHeight = std::min(maxY, maxY2) - newTop;
        if (newWidth < 0 || newHeight < 0) {
            return Box<T>(0, 0, 0, 0);
        }
        return Box<T>(newLeft, newTop, newWidth, newHeight);
    }

    bool operator==(const Box<T>& other) const
    {
        return left == other.left && top == other.top && width == other.width &&
               height == other.height;
    }
    bool operator!=(const Box<T>& other) const { return !(*this == other); }

    std::vector<Box<T>> toroidal(T worldWidth, T worldHeight) const
    {
        std::vector<Box<T>> zones;
        zones.push_back(*this);

        // Gérer le dépassement horizontal
        if (left < 0) zones.emplace_back(left + worldWidth, top, width, height);
        if (right() > worldWidth) zones.emplace_back(left - worldWidth, top, width, height);

        // Gérer le dépassement vertical
        if (top < 0) zones.emplace_back(left, top + worldHeight, width, height);
        if (bottom() > worldHeight) zones.emplace_back(left, top - worldHeight, width, height);

        // Gérer le dépassement diagonal
        if (left < 0 && top < 0)
            zones.emplace_back(left + worldWidth, top + worldHeight, width, height);
        if (left < 0 && bottom() > worldHeight)
            zones.emplace_back(left + worldWidth, top - worldHeight, width, height);
        if (right() > worldWidth && top < 0)
            zones.emplace_back(left - worldWidth, top + worldHeight, width, height);
        if (right() > worldWidth && bottom() > worldHeight)
            zones.emplace_back(left - worldWidth, top - worldHeight, width, height);
        return zones;
    }

    std::vector<Box<T>> toroidal(vector_type dim) const {
        return toroidal(dim.x, dim.y);
    }

    std::string toString() const
    {
        return "Box(" + std::to_string(left) + ", " + std::to_string(top) + ", " +
               std::to_string(width) + ", " + std::to_string(height) + ")";
    }

    friend std::ostream& operator<<(std::ostream& os, const Box<T>& box)
    {
        return os << box.toString();
    }

    T left, top, width, height;
};

using Boxf = Box<float>;
using Boxi = Box<int>;
using Boxu = Box<unsigned int>;