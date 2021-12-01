#ifndef ARCHVILE_H_
#define ARCHVILE_H_

#include "monster.h"

#include "world/world.h"

namespace mobj {

class ArchVile : public Monster {
 public:
  ArchVile(id::mobjtype_t type);

 protected:
  void A_VileChase() override;
  void A_VileStart() override;
  void A_VileTarget() override;
  void A_VileAttack() override;

};

} // namespace mobj

#endif // ARCHVILE_H_
