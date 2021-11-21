#ifndef FSM_H_
#define FSM_H_

#include <unordered_map>
#include <string>

#include "info.h"
#include "base_fsm.h"

namespace mobj {

class MapObject;

enum class FsmState {
  kSpawn,
  kSee,
  kPain,
  kMelee,
  kMissile,
  kDeath,
  kXDeath,
  kRaise
};

class FSM : public BaseFSM<MapObject> {
 public:
  FSM() = default;
  FSM(const id::mobjinfo_t& info);

  std::string GetSpriteName() const;
  
  void ToSeeState(MapObject* obj);
  void ToSpawnState(MapObject* obj);
  void ToDeathState(MapObject* obj);
  void ToXDeathState(MapObject* obj);
  void ToPainState(MapObject* obj);
  void ToMeleeState(MapObject* obj);
  void ToMissileState(MapObject* obj);

  void ToGibsState(MapObject* obj);

  bool HasState(FsmState state) const;
  bool IsSpawnState() const;
  
 protected:
  virtual void CallStateFunction([[maybe_unused]] id::FuncId foo_id, [[maybe_unused]] MapObject* obj) override;

 private:
  id::statenum_t spawn_state;
  id::statenum_t see_state;
  id::statenum_t pain_state;
  id::statenum_t melee_state;
  id::statenum_t missile_state;
  id::statenum_t death_state;
  id::statenum_t xdeath_state;
  id::statenum_t raise_state;
};

} // namespace mobj

#endif  // FSM_H_
