#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <conio.h>
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

// Forward declaration
class Maze;

// Monster class for enemies that chase the player
class Monster {
public:
    int x, y;
    int id;

    Monster(int startX, int startY, int monsterId) : x(startX), y(startY), id(monsterId) {}

    // Simple AI: Move towards the player using basic pathfinding
    void moveTowardsPlayer(int playerX, int playerY, const Maze& maze);
};

// Maze class handles generation, rendering, and movement validation
class Maze {
private:
    int width, height;
    std::vector<std::vector<char>> grid;
    std::mt19937 rng;

public:
    Maze(int w, int h) : width(w), height(h), grid(h, std::vector<char>(w, '#')), rng(std::random_device{}()) {}

    // ===== NEW: BFS-based distance map computation =====
    // Returns a vector of distances from startX, startY to all reachable cells (-1 if unreachable)
    std::vector<std::vector<int>> computeDistanceMap(int startX, int startY) const {
        std::vector<std::vector<int>> distances(height, std::vector<int>(width, -1));
        std::vector<std::pair<int, int>> queue;
        
        if (!isValidMove(startX, startY)) return distances;
        
        distances[startY][startX] = 0;
        queue.push_back({startX, startY});
        
        int dx[] = {0, 1, 0, -1};
        int dy[] = {-1, 0, 1, 0};
        
        for (size_t i = 0; i < queue.size(); i++) {
            auto [x, y] = queue[i];
            
            for (int dir = 0; dir < 4; dir++) {
                int nx = x + dx[dir];
                int ny = y + dy[dir];
                
                if (isValidMove(nx, ny) && distances[ny][nx] == -1) {
                    distances[ny][nx] = distances[y][x] + 1;
                    queue.push_back({nx, ny});
                }
            }
        }
        
        return distances;
    }
    
    // ===== NEW: Solvability check using BFS =====
    // Verifies that a path exists between start and end positions
    bool isSolvable(int startX, int startY, int endX, int endY) const {
        if (!isValidMove(startX, startY) || !isValidMove(endX, endY)) return false;
        if (startX == endX && startY == endY) return true;
        
        std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
        std::vector<std::pair<int, int>> queue;
        
        visited[startY][startX] = true;
        queue.push_back({startX, startY});
        
        int dx[] = {0, 1, 0, -1};
        int dy[] = {-1, 0, 1, 0};
        
        for (size_t i = 0; i < queue.size(); i++) {
            auto [x, y] = queue[i];
            
            if (x == endX && y == endY) return true;
            
            for (int dir = 0; dir < 4; dir++) {
                int nx = x + dx[dir];
                int ny = y + dy[dir];
                
                if (isValidMove(nx, ny) && !visited[ny][nx]) {
                    if (nx == endX && ny == endY) return true;
                    visited[ny][nx] = true;
                    queue.push_back({nx, ny});
                }
            }
        }
        
        return false;
    }

    // Generate maze using Recursive Backtracking algorithm with validation
    void generate() {
        bool mazeValid = false;
        int attempts = 0;
        const int MAX_ATTEMPTS = 10;
        
        while (!mazeValid && attempts < MAX_ATTEMPTS) {
            attempts++;
            
            // Reset grid
            grid.assign(height, std::vector<char>(width, '#'));
            
            // Ensure odd dimensions for proper maze structure
            int w = width;
            int h = height;
            if (w % 2 == 0) w--;
            if (h % 2 == 0) h--;

            std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
            std::vector<std::pair<int, int>> stack;

            int startX = 1, startY = 1;
            visited[startY][startX] = true;
            grid[startY][startX] = '.';
            stack.push_back({startX, startY});

            std::vector<std::pair<int, int>> directions = {{0, -2}, {2, 0}, {0, 2}, {-2, 0}}; // up, right, down, left

            // Generate main structure using Recursive Backtracking
            while (!stack.empty()) {
                auto [x, y] = stack.back();
                std::vector<std::pair<int, int>> neighbors;

                for (auto [dx, dy] : directions) {
                    int nx = x + dx, ny = y + dy;
                    if (nx > 0 && nx < w - 1 && ny > 0 && ny < h - 1 && !visited[ny][nx]) {
                        neighbors.push_back({nx, ny});
                    }
                }

                if (!neighbors.empty()) {
                    auto [nx, ny] = neighbors[rng() % neighbors.size()];
                    int wx = (x + nx) / 2, wy = (y + ny) / 2;
                    grid[wy][wx] = '.';
                    grid[ny][nx] = '.';
                    visited[ny][nx] = true;
                    stack.push_back({nx, ny});
                } else {
                    stack.pop_back();
                }
            }
            
            // Add controlled extra loops for more interesting paths (fewer than before)
            carveLoops(w / 8);
            
            // Validate maze solvability
            mazeValid = isSolvable(1, 1, w - 2, h - 2);
        }
    }
    
    // ===== IMPROVED: Add controlled loops without breaking maze structure =====
    void carveLoops(int numLoops) {
        for (int loop = 0; loop < numLoops; loop++) {
            int x = 2 + (rng() % (width - 4));
            int y = 2 + (rng() % (height - 4));
            
            // Shorter, controlled random walk
            int steps = 8 + (rng() % 8);
            for (int step = 0; step < steps; step++) {
                if (x > 1 && x < width - 2 && y > 1 && y < height - 2) {
                    grid[y][x] = '.';
                    int dir = rng() % 4;
                    if (dir == 0 && y > 1) y--;
                    else if (dir == 1 && y < height - 2) y++;
                    else if (dir == 2 && x > 1) x--;
                    else if (dir == 3 && x < width - 2) x++;
                }
            }
        }
    }

    // Render the maze with ANSI colors for visual appeal
    void render(const std::vector<Monster>& monsters) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                char c = grid[y][x];
                
                // Check if there's a monster at this position
                bool hasMonster = false;
                for (const auto& monster : monsters) {
                    if (monster.x == x && monster.y == y) {
                        std::cout << "\033[41mM\033[0m"; // Red for monster
                        hasMonster = true;
                        break;
                    }
                }
                
                if (hasMonster) continue;
                
                if (c == '#') {
                    std::cout << "\033[40m \033[0m"; // Black background for walls
                } else if (c == '.') {
                    std::cout << "\033[47m \033[0m"; // White background for paths
                } else if (c == 'P') {
                    std::cout << "\033[42m@\033[0m"; // Green for player
                } else if (c == 'E') {
                    std::cout << "\033[41mX\033[0m"; // Red for exit
                } else {
                    std::cout << c;
                }
            }
            std::cout << std::endl;
        }
    }

    // Check if a move to (x, y) is valid
    bool isValidMove(int x, int y) const {
        return x >= 0 && x < width && y >= 0 && y < height && grid[y][x] != '#';
    }

    // Set a cell to a specific character
    void setCell(int x, int y, char c) {
        grid[y][x] = c;
    }

    // Get the character at a cell
    char getCell(int x, int y) {
        return grid[y][x];
    }

    // Getters for dimensions
    int getWidth() { return width; }
    int getHeight() { return height; }
};

// Implementation of Monster's moveTowardsPlayer method
void Monster::moveTowardsPlayer(int playerX, int playerY, const Maze& maze) {
    std::vector<std::pair<int, int>> possibleMoves;

    // Try moving in 4 directions
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    for (int i = 0; i < 4; i++) {
        int newX = x + dx[i];
        int newY = y + dy[i];
        if (maze.isValidMove(newX, newY)) {
            possibleMoves.push_back({newX, newY});
        }
    }

    if (possibleMoves.empty()) return;

    // Pick the move that gets closest to player
    auto bestMove = possibleMoves[0];
    int minDist = abs(bestMove.first - playerX) + abs(bestMove.second - playerY);

    for (auto& move : possibleMoves) {
        int dist = abs(move.first - playerX) + abs(move.second - playerY);
        if (dist < minDist) {
            minDist = dist;
            bestMove = move;
        }
    }

    x = bestMove.first;
    y = bestMove.second;
}

int main() {
    const int MAZE_WIDTH = 21;
    const int MAZE_HEIGHT = 21;

    Maze maze(MAZE_WIDTH, MAZE_HEIGHT);
    maze.generate();

    // Set player start position
    int playerX = 1, playerY = 1;
    maze.setCell(playerX, playerY, 'P');
    
    // ===== NEW: Compute distance map and find farthest reachable cell for exit =====
    auto distanceMap = maze.computeDistanceMap(playerX, playerY);
    
    int exitX = playerX, exitY = playerY;
    int maxDistance = -1;
    const int MIN_EXIT_DISTANCE = 8; // Exit must be at least this far from start
    
    // Find the farthest reachable cell that's far enough from the player start
    for (int y = 1; y < MAZE_HEIGHT - 1; y++) {
        for (int x = 1; x < MAZE_WIDTH - 1; x++) {
            if (distanceMap[y][x] > maxDistance && distanceMap[y][x] >= MIN_EXIT_DISTANCE) {
                maxDistance = distanceMap[y][x];
                exitX = x;
                exitY = y;
            }
        }
    }
    
    // Fallback: if no cell is far enough, just use the farthest reachable
    if (maxDistance < MIN_EXIT_DISTANCE) {
        maxDistance = -1;
        for (int y = 1; y < MAZE_HEIGHT - 1; y++) {
            for (int x = 1; x < MAZE_WIDTH - 1; x++) {
                if (distanceMap[y][x] > maxDistance) {
                    maxDistance = distanceMap[y][x];
                    exitX = x;
                    exitY = y;
                }
            }
        }
    }
    
    maze.setCell(exitX, exitY, 'E');

    // ===== NEW: Create monsters at strategic reachable positions =====
    std::vector<Monster> monsters;
    
    // Collect reachable cells at reasonable distance from start (for monster placement)
    std::vector<std::pair<int, int>> monsterCandidates;
    const int MIN_MONSTER_DISTANCE = 4;
    // Place monster close to exit to avoid blocking the path: at least 75% of distance to exit
    int monsterMinDistance = std::max(MIN_MONSTER_DISTANCE, (maxDistance * 3) / 4);
    
    for (int y = 1; y < MAZE_HEIGHT - 1; y++) {
        for (int x = 1; x < MAZE_WIDTH - 1; x++) {
            // Only place monsters on reachable cells, far from start (closer to exit, not blocking path)
            if (distanceMap[y][x] >= monsterMinDistance && 
                distanceMap[y][x] < maxDistance && 
                !(x == exitX && y == exitY)) {
                monsterCandidates.push_back({x, y});
            }
        }
    }
    
    // Place only 1 monster, randomly selected from candidates
    int numMonsters = std::min(1, (int)monsterCandidates.size());
    std::mt19937 rng(std::random_device{}());
    std::shuffle(monsterCandidates.begin(), monsterCandidates.end(), rng);
    
    for (int i = 0; i < numMonsters; i++) {
        monsters.push_back(Monster(monsterCandidates[i].first, monsterCandidates[i].second, i));
    }

    int steps = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    // Game loop
    while (true) {
        system(CLEAR);
        maze.render(monsters);
        std::cout << "Steps: " << steps << " | Monsters: " << monsters.size() << std::endl;
        std::cout << "Use WASD to move, Q to quit" << std::endl;
        std::cout << "\033[41mRed M\033[0m = Monster | \033[42mGreen @\033[0m = You | \033[41mRed X\033[0m = Exit" << std::endl;

        char input;
#ifdef _WIN32
        input = _getch();
        input = tolower(input);
#else
        std::cin >> input;
        input = tolower(input);
#endif

        int dx = 0, dy = 0;
        if (input == 'w') dy = -1;
        else if (input == 's') dy = 1;
        else if (input == 'a') dx = -1;
        else if (input == 'd') dx = 1;
        else if (input == 'q') break;

        int newX = playerX + dx, newY = playerY + dy;
        if (maze.isValidMove(newX, newY)) {
            maze.setCell(playerX, playerY, '.');
            playerX = newX;
            playerY = newY;
            maze.setCell(playerX, playerY, 'P');
            steps++;

            // Check collision with monsters
            bool hitByMonster = false;
            for (const auto& monster : monsters) {
                if (playerX == monster.x && playerY == monster.y) {
                    hitByMonster = true;
                    break;
                }
            }

            if (hitByMonster) {
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
                system(CLEAR);
                maze.render(monsters);
                std::cout << "\033[41m=== GAME OVER ===\033[0m" << std::endl;
                std::cout << "You were caught by a monster!" << std::endl;
                std::cout << "Steps taken: " << steps << std::endl;
                std::cout << "Time survived: " << duration.count() << " seconds" << std::endl;
                break;
            }

            // Move monsters once per turn for slower, strategic chasing
            for (int movePhase = 0; movePhase < 1; movePhase++) {
                for (auto& monster : monsters) {
                    monster.moveTowardsPlayer(playerX, playerY, maze);
                    
                    // Check if monster caught player
                    if (playerX == monster.x && playerY == monster.y) {
                        hitByMonster = true;
                    }
                }
                if (hitByMonster) break;
            }

            if (hitByMonster) {
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
                system(CLEAR);
                maze.render(monsters);
                std::cout << "\033[41m=== GAME OVER ===\033[0m" << std::endl;
                std::cout << "You were caught by a monster!" << std::endl;
                std::cout << "Steps taken: " << steps << std::endl;
                std::cout << "Time survived: " << duration.count() << " seconds" << std::endl;
                break;
            }

            // Check win condition
            if (playerX == exitX && playerY == exitY) {
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
                system(CLEAR);
                maze.render(monsters);
                std::cout << "\033[42m=== CONGRATULATIONS ===\033[0m" << std::endl;
                std::cout << "You reached the exit and escaped!" << std::endl;
                std::cout << "Steps taken: " << steps << std::endl;
                std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
                break;
            }
        }
    }

    return 0;
}