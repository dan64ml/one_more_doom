#ifndef IMP_H_
#define IMP_H_

#include "monster.h"

namespace mobj {

class Imp : public Monster {
 public:
  Imp(id::mobjtype_t type);

 protected:
  void A_TroopAttack() override;
  
};

} // namespace mobj

#endif // IMP_H_
