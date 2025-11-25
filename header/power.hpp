#ifndef POWER_HPP
#define POWER_HPP

#include <string>

// Enumeration of all possible powers in the game
enum PowerType {
  POWER_NONE = 0,
  POWER_DEFENSE,
  POWER_EXTRA_MOVE
};

// Class representing a power-up object on the board
class Power {
private:
  PowerType type;

public:
  Power(PowerType type = POWER_NONE);

  PowerType getType() const;
  void setType(PowerType newType);

  // Returns a text/emoji symbol for displaying on the board or GUI
  std::string getSymbol() const;
};

#endif // POWER_HPP
