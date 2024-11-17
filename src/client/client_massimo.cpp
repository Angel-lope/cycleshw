#include "api.h"
#include "utils.h"
#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>
using namespace cycles;

// I took the code of the bot that only goes north and added a little bit to it
class BotClient {
  Connection connection;
  std::string name;
  GameState state;
  Player my_player;

// made a vector which i'm going to loop over so that my bot goes in circle
  const std::vector<Direction> directions = {Direction::north, Direction::east, Direction::south, Direction::west};

// defined a variable which indicates the current index in the vector
  size_t current_direction_index = 0;

  // Helper function to check if the current direction is a valid move
  bool is_valid_move(Direction direction) {
    auto new_pos = my_player.position + getDirectionVector(direction);

    // Check if the move is inside the grid
    if (!state.isInsideGrid(new_pos)) {
      return false;
    }

    // Check if the cell is already occupied
    if (state.getGridCell(new_pos) != 0) {
      return false;
    }

    return true;
  }

  void sendMove() {
    // every time the function is called calls will increment
    static int calls = 0;

    // If the current move is invalid, switch to the next direction
    while (!is_valid_move(directions[current_direction_index])) {
      current_direction_index++;
      current_direction_index %= directions.size(); // Wrap around to avoid out-of-bounds

      // If we loop through all directions and find no valid move, stop sending moves
      static int max_attempts = directions.size();
      if (--max_attempts <= 0) {
        std::cerr << "No valid moves available! Stopping the bot." << std::endl;
        exit(1);
      }
    }

    // the bot currently goes in the direction at index of the directions array
    connection.sendMove(directions[current_direction_index]);

    // this if condition avoids the bot circulating in squares but makes it a thin rectangle
    // this makes it harder for other bots to kill mine
    if (directions[current_direction_index] == Direction::east || directions[current_direction_index] == Direction::west) {
      current_direction_index++;

      // this line makes it so that if we are at the last index then it goes back to the first element
      current_direction_index %= directions.size();
    }
    calls++;

    // Every 30 moves we change direction
    // 30 isn't a random number, it's enough for the bot to closely follow its tail and not bump into it
    if (calls % 30 == 0) {
      current_direction_index++; // Increment direction index
      current_direction_index %= directions.size(); // Wrap around to avoid out-of-bounds
    }
  }

  void receiveGameState() {
    try {
      // Receive game state and handle connection issues
      state = connection.receiveGameState();
    } catch (const std::exception &e) {
      std::cerr << "Failed to receive game state: " << e.what() << std::endl;
      exit(1);
    }

    // Find my player data by name
    bool player_found = false;
    for (const auto &player : state.players) {
      if (player.name == name) {
        my_player = player;
        player_found = true;
        break;
      }
    }

    if (!player_found) {
      std::cerr << "Error: Bot name '" << name << "' not found in the game state!" << std::endl;
      exit(1);
    }

    std::cout << "There are " << state.players.size() << " players" << std::endl;
  }

public:
  BotClient(const std::string &botName) : name(botName) {
    // Connect to the server and handle connection issues
    try {
      connection.connect(name);
      if (!connection.isActive()) {
        throw std::runtime_error("Connection is not active after initialization.");
      }
    } catch (const std::exception &e) {
      std::cerr << "Failed to connect: " << e.what() << std::endl;
      exit(1);
    }
  }

  void run() {
    while (connection.isActive()) {
      try {
        receiveGameState();
        sendMove();
      } catch (const std::exception &e) {
        std::cerr << "Error in the main loop: " << e.what() << std::endl;
        exit(1);
      }
    }

    std::cerr << "Connection lost. Exiting." << std::endl;
  }
};

int main() {
  BotClient bot("Polipo_Ridel"); // this was my PlayStation username back as a kid
  bot.run();
  return 0;
}
