#ifndef FLATS_ANIMATOR_H_
#define FLATS_ANIMATOR_H_

#include <vector>
#include <string>
#include <unordered_map>

#include "world_types.h"

namespace world {

class FlatsAnimator {
 public:
  FlatsAnimator();

  void CreateFlatsList(std::vector<Sector>& sectors);

  void TickTime();

 private:
  struct AnimatedFlat {
    std::string flat_name;
    int first_idx;
    int last_idx;
    int current_idx;

    bool two_digit_idx;
  };

  int GetFlatIdx(const std::string& flat_name) const;

//  std::unordered_map<std::string, AnimatedFlat> names_;
  std::vector<AnimatedFlat> flat_types_;

  std::vector<std::vector<Sector*>> floor_sectors_;
  std::vector<std::vector<Sector*>> ceiling_sectors_;

  const int kTicksPerStep = 11;
  int tick_counter_ = 0;
};

} // namespace world

#endif  // FLATS_ANIMATOR_H_
