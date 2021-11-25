#ifndef HEAVY_DUDE_H_
#define HEAVY_DUDE_H_

#include "monster.h"

namespace mobj {

class HeavyDude : public Monster {
 public:
  HeavyDude(id::mobjtype_t type);

 protected:
  void A_CPosAttack() override;
  void A_CPosRefire() override;

};

} // namespace mobj

#endif  // HEAVY_DUDE_H_
