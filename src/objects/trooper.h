#ifndef TROOPER_H_
#define TROOPER_H_

#include "map_object.h"

namespace mobj {

class Trooper : public MapObject {
 public:
  Trooper(id::mobjtype_t type);

 protected:
  void CallStateFunction([[maybe_unused]] id::FuncId foo_id) override;

  bool CheckMeleeAttack() override { return false; }
  bool CheckMissileAttack() override;
  
 private:
  void LookFoo();
  void ChaseFoo();
};

} // namespace mobj

#endif  // TROOPER_H_
