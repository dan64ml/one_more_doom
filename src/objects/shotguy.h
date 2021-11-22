#ifndef SHOTGUY_H_
#define SHOTGUY_H_

#include "monster.h"

namespace mobj {

class ShotGuy : public Monster {
 public:
  ShotGuy(id::mobjtype_t type);

 protected:
  void A_SPosAttack() override;
};

} // namespace mobj

#endif // SHOTGUY_H_
