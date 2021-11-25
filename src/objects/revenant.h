#ifndef REVENANT_H_
#define REVENANT_H_

#include "monster.h"

#include "world/world.h"

namespace mobj {

class Revenant : public Monster {
 public:
  Revenant(id::mobjtype_t type);

 protected:
  void A_SkelWhoosh() override;
  void A_SkelFist() override;
  void A_SkelMissile() override;

};

} // namespace mobj

#endif // REVENANT_H_
