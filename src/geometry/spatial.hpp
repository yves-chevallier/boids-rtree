#pragma once

#include <concepts>
#include <functional>
#include <iterator>
#include <vector>
#include <algorithm>
#include <random>

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
struct PassThrough {
    PassThrough() {}
    bool operator()(T const& el) const { return true;  }
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

template <typename T, typename RTreeIterator>
class RTreeIteratorAdapter : public std::iterator<std::input_iterator_tag, T> {
public:
    RTreeIteratorAdapter(RTreeIterator it) : it_(it) {}

    T& operator*() const { return *it_; }
    RTreeIteratorAdapter& operator++() {
        ++it_;
        return *this;
    }

    bool operator==(const RTreeIteratorAdapter& other) const {
        return it_ == other.it_;
    }
    bool operator!=(const RTreeIteratorAdapter& other) const {
        return it_ != other.it_;
    }

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

// ---------------------------------------------------------------------------

template <typename T>
class SpatialHashing{
public:
    using value_type = T;
    using bounds_type = Boxf;
    using const_iterator = typename std::vector<T>::const_iterator;
    using const_query_iterator = typename std::vector<T>::const_iterator;

    using size_type = typename std::vector<T>::size_type;
    using max_size_type = typename std::numeric_limits<size_type>;
    using query_type = std::vector<std::reference_wrapper<T const>>;

    SpatialHashing(uint width, uint height) : box(0, 0, width, height) {
        // cellSize = width / 20;
        // pivots.resize(width * height / cellSize);
        // hashtable.resize(width * height);
    }

    void insert(const T& data) {
        elements.push_back(data);

        updateBounds(data);
    }

    void clear() { elements.clear(); }

    void update() {
        // Get usage, store on pivot the cell usage
        pivots.clear();
        for (int i = 0; i < pivotTableSize; ++i) pivots.push_back(0);

        for (const auto &element : elements) {
            int2 coord = getCellCoordinates(element.position);
            // for (auto &nh : neighbourhood) {
            //     if (coord.x + nh.x < 0 || coord.x + nh.x >= box.width / cellSize ||
            //         coord.y + nh.y < 0 || coord.y + nh.y >= box.height / cellSize)
            //         continue;
            auto nh = int2(0, 0);
                pivots[getCellIndex(coord + nh)]++;
                //std::cout << "Cell " << getCellIndex(coord + nh) << " has " << pivots[getCellIndex(coord + nh)] << " elements\n";
            // }
        }

        // Get start index in hashtable for each cell
        size_t start = 0;
        for (auto &pivot : pivots) {
            size_type usage = pivot;
            pivot = start;
            start += usage;
        }

        // Optionnaly shuffle the elements to get neighbour in random order
        shuffler.resize(elements.size());
        for (size_type i = 0; i < elements.size(); ++i) shuffler[i] = i;
        std::random_device rd;
        std::default_random_engine gen(rd()); // Utilisation du générateur par défaut

        std::shuffle(shuffler.begin(), shuffler.end(), gen);

        // Fill hashtable
        size_t i = 0;
        hashtable.clear();
        for (int i = 0; i < elements.size(); ++i) hashtable.push_back(0);

        for (auto &element : elements) {
            auto coord = getCellCoordinates(element.position);
            // for (auto const &nh : neighbourhood) { // Diffuse elements to surrounding cells
            //     if (coord.x + nh.x < 0 || coord.x + nh.x >= box.width / cellSize ||
            //         coord.y + nh.y < 0 || coord.y + nh.y >= box.height / cellSize)
            //         continue;
            auto nh = int2(0, 0);
                auto index = pivots[getCellIndex(coord + nh)]++;
                hashtable[index] = i;
            // }
            ++i;
        }

        // Reposition start index in the correct place
        pivots.push_back(pivots.back());
        for (int i = pivots.size() - 1; i > 0; --i) {
            pivots[i] = pivots[i - 1];
        }
        pivots[0] = 0;
    }

    size_type size() const { return elements.size(); }
    const_iterator begin() const { return elements.begin(); }
    const_iterator end() const { return elements.end(); }
    // const_iterator cbegin() const { return elements.cbegin(); }
    // const_iterator cend() const { return elements.cend(); }

    bounds_type bounds() const { return box; }
    query_type query(const T &element, const std::function<bool(const T&)>& pred,
                     size_t maxClosest = max_size_type::max()) const {
        query_type result;
        size_type count = 0;
        auto pivot_index = getCellIndex(getCellCoordinates(element.position));
        auto start = pivots[pivot_index];
        auto end = pivots[pivot_index + 1];
        for (size_type i = start; i < end; i++) {
            if (count++ >= maxClosest) break;
            if (pred(elements[hashtable[i]]))
                result.push_back(elements[hashtable[i]]);
        }
        return result;
    }

    auto begin() { return elements.begin(); }
    auto end() { return elements.end(); }
    // auto cbegin() const { return elements.cbegin(); }
    // auto cend() const { return elements.cend(); }

protected:


    void updateBounds(T el) { box = box.merge(el.position); }

    size_type keyFromHash(size_type hash) const { return hash % cellSize; }

    int2 getCellCoordinates(float2 const pos) const {
        return int2(std::floor(pos.x / cellSize), std::floor(pos.y / cellSize));
    }

#if 0
    size_type getCellIndex(int2 coord) const {
        return coord.x * box.width / cellSize + coord.y;
    }
#else
    size_type getCellIndex(int2 coord) const {
        return hash(coord) % pivotTableSize;
    }
#endif

    size_type hash(int2 coord) const {
        //const int2 prime = float2(73856093, 19349663);
        const int2 prime(15823, 9737333);
        return static_cast<uint>(coord.x * prime.x) ^ static_cast<uint>(coord.y * prime.y);
    }

    std::array<int2, 9> neighbourhood = {
        int2(-1, -1), int2(0, -1), int2(1, -1),
        int2(-1, 0), int2(0, 0), int2(1, 0),
        int2(-1, 1), int2(0, 1), int2(1, 1),
    };

    uint cellSize = 50;
    uint width = 1000;
    uint pivotTableSize = 1000;
    bounds_type box;
    std::vector<T> elements;
    std::vector<size_type> shuffler;
    std::vector<size_type> hashtable;
    std::vector<size_type> pivots;
};