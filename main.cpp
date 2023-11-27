/**
 * For a boid simulation, I am testing a bin lattice structure to speed up
 * the neighbor search. The idea is to divide the screen into a grid of bins
 * and store the boids in the corresponding bin. When querying for neighbors,
 * only the bins in a 3x3 square around the boid need to be checked.
 *
 * Goal is to have a 60 FPS simulation with 10000 boids.
 */
#include <SFML/Graphics.hpp>
#include <cmath>
#include <array>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <span>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

#define NUMBER_BINS 20 // Number of bins should be proportional to sight radius
#define MAX_BOIDS_PER_BIN 100 // Limit number of boids per bin to avoid large queries

#define BOIDS 1000 // To be increased to 10000
#define RADIUS 100 // Radius of the circle around the mouse to query for neighbors

struct Boid {
    sf::Vector2f position;
};

/**
 * In this implementation I am using a fixed size array for the boids in each bin.
 * std::vector is not used because it is slower. I am seeking to optimize the
 * neighbor search, so I am not concerned with the memory overhead of a fixed size
 * array.
 */
template<size_t WIDTH, size_t HEIGHT, size_t BINS, size_t BIN_SIZE>
class BinLattice {
public:
    static constexpr size_t MaxQuerySize = BIN_SIZE * 9;
    //using Bin = std::span<Boid*, BIN_SIZE>;
    using Bin = std::pair<std::array<Boid*, BIN_SIZE>, size_t>;
    using Lattice = std::array<std::array<Bin, BINS>, BINS>;
    using BoidArray = std::array<Boid*, MaxQuerySize>;
    using QueryResult = std::pair<BoidArray, size_t>;  // Pair of array and count

    BinLattice() {
        for (auto &row : lattice) {
            for (auto &bin : row) {
                bin.first.fill(nullptr);
                bin.second = 0;
            }
        }
    }

    void addBoid(Boid* boid) {
        size_t xIndex = static_cast<size_t>(boid->position.x / (WIDTH / BINS));
        size_t yIndex = static_cast<size_t>(boid->position.y / (HEIGHT / BINS));
        auto& bin = lattice[xIndex][yIndex];
        bin.first[bin.second++] = boid;
    }

    void clear() {
        for (auto &row : lattice) {
            for (auto &bin : row) {
                bin.second = 0;
            }
        }
    }

    void query(sf::Vector2f &position, QueryResult& result, float radius) {
        size_t count = 0;

        size_t xIndex = static_cast<size_t>(position.x / (WIDTH / BINS));
        size_t yIndex = static_cast<size_t>(position.y / (HEIGHT / BINS));

        // Query all bins in a 3x3 square around the boid
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                size_t newX = std::clamp(xIndex + dx, 0ul, BINS - 1);
                size_t newY = std::clamp(yIndex + dy, 0ul, BINS - 1);
                auto& bin = lattice[newX][newY];
                for (size_t i = 0; i < bin.second; ++i) {
                    // Check if boid is within radius
                    if (std::hypot(bin.first[i]->position.x - position.x,
                                   bin.first[i]->position.y - position.y) < radius) {
                        result.first[count++] = bin.first[i];
                    }
                }
            }
        }
        result.second = count;
    }

    Lattice lattice;
};

int main() {
    std::vector<Boid> boids;
    for (int i = 0; i < BOIDS; ++i) {
        boids.push_back({{rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT}});
    }
    BinLattice<WINDOW_WIDTH, WINDOW_HEIGHT, NUMBER_BINS, MAX_BOIDS_PER_BIN> binLattice;
    BinLattice<WINDOW_WIDTH, WINDOW_HEIGHT, NUMBER_BINS, MAX_BOIDS_PER_BIN>::QueryResult result;

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Boids");
    sf::CircleShape shape(1.f);
    shape.setFillColor(sf::Color::Green);

    // Fill bin lattice
    for (auto& boid : boids) binLattice.addBoid(&boid);

    // Print to stdout for debugging the lattice structure
    for (auto& row : binLattice.lattice) {
        for (auto& bin : row) {
            std::cout << bin.second << " ";
        }
        std::cout << std::endl;
    }

    // Load a font
    sf::Font font;
    if (!font.loadFromFile("collegiate.ttf")) {
        std::cout << "Error loading font" << std::endl;
    }

    // Setup text
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition(10.f, 10.f);

    sf::Clock frameClock; // Clock to measure frame time
    sf::Clock updateClock; // Clock to measure update intervals
    float fps = 0.0f;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) { if (event.type == sf::Event::Closed) window.close(); }

        binLattice.clear();
        for (auto& boid : boids) binLattice.addBoid(&boid);

        window.clear();

        // Display lattice grid with clear gray lines
        #ifdef DEBUG
        sf::RectangleShape hline(sf::Vector2f(WINDOW_WIDTH / NUMBER_BINS, 1.f));
        hline.setFillColor(sf::Color(255, 255, 255, 35));
        sf::RectangleShape vline(sf::Vector2f(1.f, WINDOW_HEIGHT / NUMBER_BINS));
        vline.setFillColor(sf::Color(255, 255, 255, 35));
        for (size_t i = 0; i < NUMBER_BINS; ++i) {
            for (size_t j = 0; j < NUMBER_BINS; ++j) {
                hline.setPosition(i * WINDOW_WIDTH / NUMBER_BINS, j * WINDOW_HEIGHT / NUMBER_BINS);
                window.draw(hline);
                vline.setPosition(i * WINDOW_WIDTH / NUMBER_BINS, j * WINDOW_HEIGHT / NUMBER_BINS);
                window.draw(vline);
            }
        }
        #endif

        // Get Mouse Position
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        sf::Vector2f mousePositionFloat = sf::Vector2f(mousePosition.x, mousePosition.y);

        // Draw clear alpha circle around mouse
        shape.setFillColor(sf::Color(255, 255, 255, 35));
        shape.setRadius(RADIUS);
        shape.setPosition(mousePositionFloat.x - RADIUS, mousePositionFloat.y - RADIUS);
        window.draw(shape);

        shape.setFillColor(sf::Color::Cyan);
        shape.setRadius(1.f);
        for (auto& boid : boids) {
            shape.setPosition(boid.position);
            window.draw(shape);
        }

        shape.setFillColor(sf::Color::Yellow);
        shape.setRadius(2.f);

        #if 0 // For each boid (real usecase), very slow < 10 FPS
        for (auto& boid : boids) {
            binLattice.query(mousePositionFloat, result, RADIUS);
            for (size_t i = 0; i < result.second; ++i) {
                shape.setPosition(result.first[i]->position);
                window.draw(shape);
            }
        }
        #else // Only for mouse position (for profiling): very fast > 120 FPS
        binLattice.query(mousePositionFloat, result, RADIUS);
        for (size_t i = 0; i < result.second; ++i) {
            shape.setPosition(result.first[i]->position);
            window.draw(shape);
        }
        #endif

        sf::Time frameTime = frameClock.restart();
        if (updateClock.getElapsedTime().asSeconds() >= 0.5) {
            // Update FPS every 0.5 seconds
            fps = 1.0f / frameTime.asSeconds();
            updateClock.restart();
        }

        // Update text
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << fps << " FPS";
        text.setString(ss.str());
        window.draw(text);

        window.display();
    }
}
