#ifndef CACODEMON_H_
#define CACODEMON_H_

#include "monster.h"

namespace mobj {

class Cacodemon : public Monster {
 public:
  Cacodemon(id::mobjtype_t type);

 protected:
  void A_HeadAttack() override;

  void ZMove() override;
};

} // namespace mobj

#endif  // CACODEMON_H_
