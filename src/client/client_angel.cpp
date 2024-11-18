#include "api.h"
#include "utils.h"
#include <string>
#include <iostream>

using namespace cycles;

class BotClient {
    Connection connection;
    std::string name;
    GameState state;

    // Helper function to check if a position is safe
    bool isSafePosition(sf::Vector2i position) {
        return state.isInsideGrid(position) && state.isCellEmpty(position);
    }

    // Decide the best move to avoid edges and other players
    Direction decideMove() {
        sf::Vector2i currentPosition = state.players[0].position; // Assuming bot is the first player

        // Possible directions and their corresponding vectors
        std::vector<std::pair<Direction, sf::Vector2i>> moves = {
            {Direction::north, sf::Vector2i(0, -1)},
            {Direction::east, sf::Vector2i(1, 0)},
            {Direction::south, sf::Vector2i(0, 1)},
            {Direction::west, sf::Vector2i(-1, 0)}
        };

        // Iterate through moves and prioritize safe ones
        for (const auto &move : moves) {
            sf::Vector2i newPosition = currentPosition + move.second;
            if (isSafePosition(newPosition)) {
                return move.first;
            }
        }

        // If no safe move is found (rare), default to staying still
        return Direction::north; // This could be any direction, chosen arbitrarily
    }

    void sendMove() {
        Direction bestMove = decideMove();
        connection.sendMove(bestMove);
    }

    void receiveGameState() {
        state = connection.receiveGameState();
    }

public:
    BotClient(const std::string &botName) : name(botName) {
        connection.connect(name);
        if (!connection.isActive()) {
            exit(1);
        }
    }

    void run() {
    while (connection.isActive()) {
        try {
            receiveGameState();
            sendMove();
        } catch (const std::exception &e) {
            std::cerr << "[Critical] Connection lost: " << e.what() << std::endl;
            break;
        }
    }
    std::cerr << "[Info] Bot has stopped running due to disconnection." << std::endl;
}

};

int main() {
    BotClient bot("Angel");
    bot.run();
    return 0;
}
