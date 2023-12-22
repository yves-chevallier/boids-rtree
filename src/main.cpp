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
#include <iomanip>

#include "geometry/spatial.hpp"
#include "geometry/body.hpp"
#include "fps.hpp"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

#define N 10000
#define RADIUS 100 // Radius of the circle around the mouse to query for neighbors

// struct Boid {
//     point_2d position;
//     struct ByPos {
//         using result_type = point_2d;
//         result_type const& operator()(Boid const& boid) const { return boid.position; }
//     };
//     auto toVec2() const { return sf::Vector2f(position.x(), position.y()); }
// };

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Spatial Tree", sf::Style::Close);

    RTree<Body> tree;

    for (int i = 0; i < N; ++i) {
        const auto x = static_cast<float>(rand() % WINDOW_WIDTH);
        const auto y = static_cast<float>(rand() % WINDOW_HEIGHT);
        tree.insert(Body(Vector2f(x, y)));
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

    // Body circle
    sf::CircleShape bodyShape(1.f);
    bodyShape.setFillColor(sf::Color::Cyan);

    // Highlight circle
    sf::CircleShape bodySeen(2.f);
    bodySeen.setFillColor(sf::Color::Yellow);

    FpsCounter fpsCounter(0.5f, [&](float min, float current, float max) {
        (void)current;
        (void)max;
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << min << " FPS";
        text.setString(ss.str());
    });

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) { if (event.type == sf::Event::Closed) window.close(); }

        window.clear();

        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        sf::Vector2f mousePositionFloat = sf::Vector2f(mousePosition.x, mousePosition.y);

        // Draw clear alpha circle around mouse
        spotlight.setPosition(mousePositionFloat - sf::Vector2f(RADIUS, RADIUS));
        window.draw(spotlight);

        for (auto const& body : tree) {
            bodyShape.setPosition(body.position);
            window.draw(bodyShape);
        }

        for (Body const & body : tree.query(Distance<Body>(Vector2f(mousePositionFloat), RADIUS))) {
            bodySeen.setPosition(body.position);
            window.draw(bodySeen);
        }

        window.draw(text);
        window.display();
        fpsCounter.update();
    }
}
