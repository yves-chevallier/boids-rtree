#pragma once

#include "geometry/box.hpp"
#include "geometry/vector2.hpp"

#include <vector>
#include <concepts>

template<typename T>
concept Predicate = requires(T t, const Vector2f& v) {
    { t(v) } -> std::convertible_to<bool>;
};

template <typename T>
struct Contains {
    Contains(Boxf const& box) : box(box) {}
    Contains(Vector2f const& point, auto radius) : box(point, radius+radius) {}
    bool operator()(T const& el) const {
        return box.contains(el.position);
    }
    Boxf box;
};

template <typename T>
struct Distance {
    Distance(Vector2f const& search, float radius) : search(search), radius(radius) {}
    bool operator()(T const& el) const {
        return (el.position - search).lengthSquared() < radius * radius;
    }
    Vector2f search;
    float radius;
};

template <typename T>
class Collection {
   public:
    using value_type = T;
    using bounds_type = Boxf;
    using const_iterator = typename std::vector<T>::const_iterator;
    using const_query_iterator = typename std::vector<T>::const_iterator;
    using size_type = typename std::vector<T>::size_type;
    using max_size_type = typename std::numeric_limits<size_type>;
    using query_type = std::vector<std::reference_wrapper<T const>>;

    void insert(const T &data) {
        elements.push_back(data);
    }

    void clear() {
        elements.clear();
    }

    void rebuild() {
        // Do nothing
    }

    size_type size() const {
        return elements.size();
    }

    const_iterator begin() const {
        return elements.begin();
    }

    const_iterator end() const {
        return elements.end();
    }

    const_iterator cbegin() const {
        return elements.cbegin();
    }

    const_iterator cend() const {
        return elements.cend();
    }

    bounds_type bounds() const {
        return elements.bounds();
    }

    query_type query(Boxf range, size_t maxClosest=max_size_type::max()) const {
        query_type result;
        size_type count = 0;
        for (auto &element : elements) {
            if (range.contains(element.position)) {
                if (count++ >= maxClosest) break;
                result.push_back(element);
            }
        }
        return result;
    }

    template <Predicate Pred>
    query_type query(Pred pred, size_t maxClosest=max_size_type::max()) const {
        query_type result;
        size_type count = 0;
        for (auto &element : elements) {
            if (pred(element)) {
                if (count++ >= maxClosest) break;
                result.push_back(element);
            }
        }
        return result;
    }

    protected:
    std::vector<T> elements;
};

#if 0

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using point_2d = bg::model::d2::point_xy<float>;
using box = bg::model::box<point_2d>;

template <typename T>
class RTree {
public:

    using value_type = T;
    using bounds_type = Boxf;
    using const_iterator = typename std::vector<T>::const_iterator;
    using const_query_iterator = typename std::vector<T>::const_iterator;
    using size_type = typename std::vector<T>::size_type;

    bgi::rtree<value_type, bgi::quadratic<32>, typename T::ByPos> rtree;

    void insert(const T& data) { rtree.insert(data); }

    void clear() { rtree.clear(); }

    void rebuild() {}

    size_type size() const {
        return rtree.size();
    }

    // const_iterator begin() const { return rtree.begin(); }

    // const_iterator end() const { return rtree.end(); }

    // const_iterator cbegin() const { return rtree.cbegin(); }

    // const_iterator cend() const { return rtree.cend(); }

    // bounds_type bounds() const { return rtree.bounds(); }

protected:
    // struct withinRange {
    //     withinRange(point_2d const& search, float radius)
    //         : search(search), radius(radius) {}
    //     bool operator()(value_type const& el) const {
    //         return bg::distance(el.position, search) < radius;
    //     }
    //     point_2d search;
    //     float radius;
    // };

    // Sight in 2D is a circular sector with a radius and an angle. Span is the
    // width of the sector. span min = angle - span / 2 span max = angle + span
    // / 2
    // struct withinSight {
    //     withinSight(point_2d const& search, float radius, float angle,
    //                 float span)
    //         : search(search), radius(radius) {}
    //     bool operator()(value_type const& el) const {
    //         if (bg::distance(el.position, search) > radius) return false;
    //     }
    //     point_2d search;
    //     float radius;
    // };
    // auto intersecting(auto const& search, auto const& tree, float radius) {
    //     std::vector<std::reference_wrapper<T const>> result;
    //     rtree.query(bgi::satisfies([&](auto const& el) {
    //                     return bg::distance(el.position, search) < radius;
    //                 }),
    //                 std::back_inserter(result));
    //     return result;
    // }

    // auto intersecting(auto const& search, auto const& tree, float radius,
    //                   unsigned int maxNearest) {
    //     std::vector<std::reference_wrapper<T const>> result;
    //     rtree.query(bgi::nearest(search, maxNearest), std::back_inserter(result));
    //     return result;
    // }
};

#endif