#include "flash_fsm.h"

#include <cassert>

#include "weapon.h"

namespace wpn {

void FlashFSM::CallStateFunction(id::FuncId foo_id, Weapon* obj) {
  switch (foo_id)
  {
  case id::A_NULL:
    /* code */
    break;
  
  default:
    break;
  }
}

std::string FlashFSM::GetSpriteName() const {
  auto base_name = GetSpriteBaseName();
  if (base_name.empty()) {
    return base_name;
  }

  return base_name + "0";
}

} // namespace wpn
