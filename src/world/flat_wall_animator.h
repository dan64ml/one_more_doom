#ifndef FLAT_WALL_ANIMATOR_H_
#define FLAT_WALL_ANIMATOR_H_

#include <vector>
#include <string>
#include <unordered_map>

#include "world_types.h"

namespace world {

class FlatWallAnimator {
 public:
  FlatWallAnimator();

  void CreateFlatsList(std::vector<Sector>& sectors);

  void TickTime();

 private:
//  struct AnimatedFlat {
//    std::string flat_name;
//    int first_idx;
//    int last_idx;
//    int current_idx;
//
//    bool two_digit_idx;
//  };
//
//  struct AnimatedSequence {
//    std::string first_name;
//    std::string last_name;
//  };

  const std::vector<std::vector<std::string>> flat_names_ = {
    {"NUKAGE3", "NUKAGE2", "NUKAGE1"},
    {"FWATER4", "FWATER3", "FWATER2", "FWATER1"},
    {"SWATER4", "SWATER3", "SWATER2", "SWATER1"},
    {"LAVA4", "LAVA3", "LAVA2", "LAVA1"},
    {"BLOOD3", "BLOOD2", "BLOOD1"},
    {"RROCK08", "RROCK07", "RROCK06", "RROCK05"},
    {"SLIME04", "SLIME03", "SLIME02", "SLIME01"},
    {"SLIME08", "SLIME07", "SLIME06", "SLIME05"},
    {"SLIME12", "SLIME11", "SLIME10", "SLIME09"},
};

  int GetFlatIdx(const std::string& flat_name) const;

  //std::vector<AnimatedFlat> flat_types_;

  std::vector<std::vector<Sector*>> floor_sectors_;
  std::vector<std::vector<Sector*>> ceiling_sectors_;
  std::vector<int> flat_idxs_;

  const int kTicksPerStep = 8;
  int tick_counter_ = 0;
};

} // namespace world

#endif  // FLAT_WALL_ANIMATOR_H_
