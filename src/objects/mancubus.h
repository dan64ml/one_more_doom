#ifndef MANCUBUS_H_
#define MANCUBUS_H_

#include "monster.h"

namespace mobj {

class Mancubus : public Monster {
 public:
  Mancubus(id::mobjtype_t type);

 protected:
  void A_FatRaise() override;
  void A_FatAttack1() override;
  void A_FatAttack2() override;
  void A_FatAttack3() override;

 private:
  const rend::BamAngle kFatSpread = rend::kBamAngle90 / 8;

};

} // namespace mobj

#endif // MANCUBUS_H_
