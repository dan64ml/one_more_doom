#ifndef FLASH_FSM_H_
#define FLASH_FSM_H_

#include <string>

#include "objects/base_fsm.h"

namespace wpn {

class Weapon;

class FlashFSM  : public mobj::BaseFSM<Weapon> {
 public:
  FlashFSM() = default;
  FlashFSM(id::statenum_t state) : BaseFSM(state) {}

  std::string GetSpriteName() const;

 private:
  void CallStateFunction(id::FuncId foo_id, Weapon* obj) override;

};

} // namespace wpn

#endif  // FLASH_FSM_H_
