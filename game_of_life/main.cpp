#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <cstdlib> 
#include <ctime>    
#include <optional>
#include <omp.h>

const int rows = 100;
const int cols = 100;
const int cell_size = 8;

std::vector<std::vector<int>> grid(rows, std::vector<int>(cols));
std::vector<std::vector<int>> next_grid(rows, std::vector<int>(cols));

void initialize_grid() {
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            grid[i][j] = rand() % 2;
}

void update_grid() {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int live_neighbors = 0;

            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (dx == 0 && dy == 0) continue; // Skip self

                    int ni = (i + dx + rows) % rows;
                    int nj = (j + dy + cols) % cols;

                    live_neighbors += grid[ni][nj];
                }
            }

            if (grid[i][j] == 1)
                next_grid[i][j] = (live_neighbors == 2 || live_neighbors == 3);
            else
                next_grid[i][j] = (live_neighbors == 3);
        }
    }

    grid.swap(next_grid);
}

void draw_grid(sf::RenderWindow& window) {
    sf::RectangleShape cell(sf::Vector2f(cell_size - 1.0f, cell_size - 1.0f));
    cell.setFillColor(sf::Color::White);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (grid[i][j] == 1) {
                cell.setPosition(sf::Vector2f(static_cast<float>(j * cell_size), static_cast<float>(i * cell_size)));
                window.draw(cell);
            }
        }
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
    initialize_grid();

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(cols * cell_size, rows * cell_size)), "Game of Life - OpenMP + SFML");
    window.setFramerateLimit(10); // FPS

    while (window.isOpen()) {
        while (std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        update_grid();

        window.clear(sf::Color::Black);
        draw_grid(window);
        window.display();
    }

    return 0;
}
