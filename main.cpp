/**
 * For a boid simulation, I am testing a bin lattice structure to speed up
 * the neighbor search. The idea is to divide the screen into a grid of bins
 * and store the boids in the corresponding bin. When querying for neighbors,
 * only the bins in a 3x3 square around the boid need to be checked.
 *
 * Goal is to have a 60 FPS simulation with 10000 boids.
 */
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using point_2d = bg::model::d2::point_xy<float>;
using box = bg::model::box<point_2d>;

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

#define BOIDS 10000
#define RADIUS 50 // Radius of the circle around the mouse to query for neighbors

struct Boid {
    point_2d position;
    struct ByPos {
        using result_type = point_2d;
        result_type const& operator()(Boid const& boid) const { return boid.position; }
    };
    auto toVec2() const { return sf::Vector2f(position.x(), position.y()); }
};

auto intersecting(auto const& search, auto const& tree, float radius) {
    std::vector<std::reference_wrapper<Boid const>> result;
    tree.query(bgi::satisfies([&](auto const& boid) {
                   return bg::distance(boid.position, search) < radius ;
               }),
               std::back_inserter(result));
    return result;
}

int main() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4.0;
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Boids", sf::Style::Close, settings);

    bgi::rtree<Boid, bgi::quadratic<32>, Boid::ByPos> rtree;

    for (int i = 0; i < BOIDS; ++i) {
        const auto x = static_cast<float>(rand() % WINDOW_WIDTH);
        const auto y = static_cast<float>(rand() % WINDOW_HEIGHT);
        rtree.insert({{x, y}});
    }

    // Load a font
    sf::Font font;
    if (!font.loadFromFile("collegiate.ttf")) std::cout << "Error loading font" << std::endl;

    // Setup text
    sf::Text text("", font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition(10.f, 10.f);

    // Mouse circle
    sf::CircleShape spotlight(1.f);
    spotlight.setFillColor(sf::Color(255, 255, 255, 35));
    spotlight.setRadius(RADIUS);

    // Boid circle
    sf::CircleShape boidShape(1.f);
    boidShape.setFillColor(sf::Color::Cyan);

    // Highlight circle
    sf::CircleShape boidSeen(2.f);
    boidSeen.setFillColor(sf::Color::Yellow);

    sf::Clock frameClock;
    sf::Clock updateClock;
    float fps = 0.0f;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) { if (event.type == sf::Event::Closed) window.close(); }

        window.clear();

        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        sf::Vector2f mousePositionFloat = sf::Vector2f(mousePosition.x, mousePosition.y);

        // Draw clear alpha circle around mouse
        spotlight.setPosition(mousePositionFloat - sf::Vector2f(RADIUS, RADIUS));
        window.draw(spotlight);

        for (auto const& boid : rtree) {
            boidShape.setPosition(boid.toVec2());
            window.draw(boidShape);
        }

        for (Boid const & boid : intersecting(point_2d(mousePosition.x, mousePosition.y), rtree, RADIUS)) {
            boidSeen.setPosition(boid.toVec2());
            window.draw(boidSeen);
        }

        // Calculate FPS
        sf::Time frameTime = frameClock.restart();
        if (updateClock.getElapsedTime().asSeconds() >= 0.5) {
            fps = 1.0f / frameTime.asSeconds();
            updateClock.restart();
        }

        // Display FPS
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << fps << " FPS";
            text.setString(ss.str());
            window.draw(text);
        }
        window.display();
    }
}
