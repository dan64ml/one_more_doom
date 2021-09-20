#include "flats_animator.h"

#include <iostream>

namespace world {

FlatsAnimator::FlatsAnimator() {
  flat_types_.push_back({"NUKAGE1", 1, 3, 1, false});
  flat_types_.push_back({"FWATER1", 1, 4, 1, false});
  flat_types_.push_back({"SWATER1", 1, 4, 1, false});
  flat_types_.push_back({"LAVA1", 1, 4, 1, false});
  flat_types_.push_back({"BLOOD1", 1, 3, 1, false});
  flat_types_.push_back({"RROCK05", 5, 8, 5, false});
  flat_types_.push_back({"SLIME01", 1, 4, 1, false});
  flat_types_.push_back({"SLIME05", 5, 8, 5, false});
  flat_types_.push_back({"SLIME09", 9, 12, 9, true});

  floor_sectors_.resize(flat_types_.size());
  ceiling_sectors_.resize(flat_types_.size());
}

void FlatsAnimator::CreateFlatsList(std::vector<Sector>& sectors) {
  for (auto& sec : sectors) {
    std::cout << sec.floor_pic << " " << sec.ceiling_pic << std::endl;
    if (int idx = GetFlatIdx(sec.floor_pic); idx != -1) {
      floor_sectors_[idx].push_back(&sec);
    }
    if (int idx = GetFlatIdx(sec.ceiling_pic); idx != -1) {
      ceiling_sectors_[idx].push_back(&sec);
    }
  }
}

void FlatsAnimator::TickTime() {
  ++tick_counter_;
  if (tick_counter_ < kTicksPerStep) {
    return;
  } else {
    tick_counter_ = 0;
  }

  for (auto& name : flat_types_) {
    ++name.current_idx;
    if (name.current_idx > name.last_idx) {
      name.current_idx = name.first_idx;
    }
    if (name.two_digit_idx) {
      int digit = name.current_idx / 10;
      name.flat_name[name.flat_name.size() - 2] = digit + '0';
      digit = name.current_idx % 10;
      name.flat_name[name.flat_name.size() - 1] = digit + '0';
    } else {
      name.flat_name[name.flat_name.size() - 1] = name.current_idx + '0';
    }
  }

  for (size_t i = 0; i < flat_types_.size(); ++i) {
    for (auto sec : floor_sectors_[i]) {
      sec->floor_pic = flat_types_[i].flat_name;
    }
    for (auto sec : ceiling_sectors_[i]) {
      sec->ceiling_pic = flat_types_[i].flat_name;
    }
  }
}

int FlatsAnimator::GetFlatIdx(const std::string& flat_name) const {
  if (flat_name.size() < 5 || flat_name.size() > 7) {
    return -1;
  }

  if (flat_name == "NUKAGE1" || flat_name == "NUKAGE2" || flat_name == "NUKAGE3") {
    return 0;
  } else if (flat_name == "FWATER1" || flat_name == "FWATER2" || 
             flat_name == "FWATER3" || flat_name == "FWATER4") {
    return 1;
  } else if (flat_name == "SWATER1" || flat_name == "SWATER2" || 
             flat_name == "SWATER3" || flat_name == "SWATER4") {
    return 2;
  } else if (flat_name == "LAVA1" || flat_name == "LAVA2" || 
             flat_name == "LAVA3" || flat_name == "LAVA4") {
    return 3;
  } else if (flat_name == "BLOOD1" || flat_name == "BLOOD2" || flat_name == "BLOOD3") {
    return 4;
  } else if (flat_name == "RROCK05" || flat_name == "RROCK06" || 
             flat_name == "RROCK07" || flat_name == "RROCK08") {
    return 5;
  } else if (flat_name == "SLIME01" || flat_name == "SLIME02" || 
             flat_name == "SLIME03" || flat_name == "SLIME04") {
    return 6;
  } else if (flat_name == "SLIME05" || flat_name == "SLIME06" || 
             flat_name == "SLIME07" || flat_name == "SLIME08") {
    return 7;
  } else if (flat_name == "SLIME09" || flat_name == "SLIME10" || 
             flat_name == "SLIME11" || flat_name == "SLIME12") {
    return 8;
  }

  return -1;
}

} // namespace world
