#include "power.hpp"

Power::Power(PowerType type) : type(type) {}

PowerType Power::getType() const {
  return type;
}

void Power::setType(PowerType newType) {
  type = newType;
}

std::string Power::getSymbol() const {
  switch (type) {
    case POWER_DEFENSE:    return "[defense]";  // Defense power
    case POWER_EXTRA_MOVE: return "[extra]";  // Extra move power
    default:               return "[ ]";   // Empty square
  }
}
