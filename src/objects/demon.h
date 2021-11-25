#ifndef DEMON_H_
#define DEMON_H_

#include "monster.h"

namespace mobj {

class Demon : public Monster {
 public:
  Demon(id::mobjtype_t type);

 protected:
  void A_SargAttack() override;
};

} // namespace mobj

#endif // DEMON_H_
