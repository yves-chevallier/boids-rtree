#pragma once

#include <concepts>
#include <vector>
#include <iterator>
#include <functional>

#include "geometry/box.hpp"
#include "geometry/vector2.hpp"

template <typename T>
concept Predicate = requires(T t, const Vector2f& v) {
    { t(v) } -> std::convertible_to<bool>;
};

template <typename T>
class SpatialCollection {
public:
    using value_type = T;
    using bounds_type = Boxf;
    using const_iterator = std::iterator<std::input_iterator_tag, T>;
    using size_type = size_t;
    using max_size_type = typename std::numeric_limits<size_type>;
    using query_type = std::vector<std::reference_wrapper<T const>>;

    virtual ~SpatialCollection() = default;

    virtual void insert(const T& data) = 0;
    virtual void clear() = 0;
    virtual void rebuild() = 0;
    virtual size_type size() const = 0;
    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator cend() const = 0;
    virtual bounds_type bounds() const = 0;

    virtual query_type query(
        const std::function<bool(const T&)>& pred,
        size_t maxClosest = max_size_type::max()) const = 0;
};

template <typename T>
struct Contains {
    Contains(Boxf const& box) : box(box) {}
    Contains(Vector2f const& point, auto radius)
        : box(point, radius + radius) {}
    bool operator()(T const& el) const { return box.contains(el.position); }
    Boxf box;
};

template <typename T>
struct Distance {
    Distance(Vector2f const& search, float radius)
        : search(search), radius(radius) {}
    bool operator()(T const& el) const {
        return (el.position - search).lengthSquared() < radius * radius;
    }
    Vector2f search;
    float radius;
};

template <typename T>
class Collection : public SpatialCollection<T> {
public:
    using value_type = T;
    using bounds_type = Boxf;
    using const_iterator = typename std::vector<T>::const_iterator;
    using const_query_iterator = typename std::vector<T>::const_iterator;

    using size_type = typename std::vector<T>::size_type;
    using max_size_type = typename std::numeric_limits<size_type>;
    using query_type = std::vector<std::reference_wrapper<T const>>;

    Collection() : box(0, 0, 0, 0) {}
    void insert(const T& data) {
        elements.push_back(data);
        updateBounds(data);
    }

    void clear() { elements.clear(); }

    void rebuild() {
        // Do nothing
    }

    size_type size() const { return elements.size(); }
    const_iterator begin() const { return elements.begin(); }
    const_iterator end() const { return elements.end(); }
    const_iterator cbegin() const { return elements.cbegin(); }
    const_iterator cend() const { return elements.cend(); }
    bounds_type bounds() const { return box; }
    query_type query(const std::function<bool(const T&)>& pred,
                     size_t maxClosest = max_size_type::max()) const {
        query_type result;
        size_type count = 0;
        for (auto& element : elements) {
            if (pred(element)) {
                if (count++ >= maxClosest) break;
                result.push_back(element);
            }
        }
        return result;
    }

protected:
    bounds_type box;
    std::vector<T> elements;

    void updateBounds(T el) { box = box.merge(el.position); }
};

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using point_2d = bg::model::d2::point_xy<float>;
using box = bg::model::box<point_2d>;

// Spécialisation des traits pour sf::Vector2f
namespace boost {
namespace geometry {
namespace traits {

template <>
struct tag<Vector2f> {
    typedef point_tag type;
};

template <>
struct coordinate_type<Vector2f> {
    typedef float type;
};

template <>
struct coordinate_system<Vector2f> {
    typedef bg::cs::cartesian type;
};

template <>
struct dimension<Vector2f> : boost::mpl::int_<2> {};

template <size_t Dimension>
struct access<Vector2f, Dimension> {
    static float get(Vector2f const& p) { return Dimension == 0 ? p.x : p.y; }

    static void set(Vector2f& p, float value) {
        if (Dimension == 0)
            p.x = value;
        else
            p.y = value;
    }
};

}  // namespace traits
}  // namespace geometry
}  // namespace boost

template<typename T, typename RTreeIterator>
class RTreeIteratorAdapter : public std::iterator<std::input_iterator_tag, T> {
public:
    RTreeIteratorAdapter(RTreeIterator it) : it_(it) {}

    T& operator*() const { return *it_; }
    RTreeIteratorAdapter& operator++() { ++it_; return *this; }

    bool operator==(const RTreeIteratorAdapter& other) const { return it_ == other.it_; }
    bool operator!=(const RTreeIteratorAdapter& other) const { return it_ != other.it_; }

private:
    RTreeIterator it_;
};

template <typename T>
class RTree {
public:
    using value_type = T;
    using bounds_type = Boxf;
    using tree_type =
        bgi::rtree<value_type, bgi::quadratic<32>, typename T::ByPos>;
    using const_iterator = typename tree_type::const_iterator;
    using const_query_iterator = typename tree_type::const_query_iterator;

    using size_type = typename tree_type::size_type;
    using max_size_type = typename std::numeric_limits<size_type>;
    using query_type = std::vector<std::reference_wrapper<T const>>;

    void insert(const T& data) { rtree.insert(data); }

    void clear() { rtree.clear(); }

    void rebuild() {
        // Do nothing
    }

    size_type size() const { return rtree.size(); }
    const_iterator begin() const { return rtree.begin(); }
    const_iterator end() const { return rtree.end(); }
    const_iterator cbegin() const { return rtree.begin(); }
    const_iterator cend() const { return rtree.end(); }
    bounds_type bounds() const { return Boxf(0, 0, 0, 0); }
    query_type query(const std::function<bool(const T&)>& pred,
                     size_t maxClosest = max_size_type::max()) const {
        query_type result;
        size_type count = 0;
        rtree.query(bgi::satisfies([&](auto const& el) {
                        if (count++ >= maxClosest) return false;
                        return pred(el);
                    }),
                    std::back_inserter(result));
        return result;
    }

protected:
    tree_type rtree;
};
