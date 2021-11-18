#ifndef TROOPER_H_
#define TROOPER_H_

#include "map_object.h"
#include "monster.h"

namespace mobj {

class Trooper : public Monster {
 public:
  Trooper(id::mobjtype_t type);

 protected:
  bool InfluenceObject(MapObject*) override { return false; }
  void A_PosAttack() override;
  
 private:
};

} // namespace mobj

#endif  // TROOPER_H_
