#include "flat_wall_animator.h"

namespace world {

FlatWallAnimator::FlatWallAnimator() {
  floor_sectors_.resize(flat_names_.size());
  ceiling_sectors_.resize(flat_names_.size());
  flat_idxs_.resize(flat_names_.size(), 0);
}

void FlatWallAnimator::CreateFlatsList(std::vector<Sector>& sectors) {
  for (auto& sec : sectors) {
    if (int idx = GetFlatIdx(sec.floor_pic); idx != -1) {
      floor_sectors_[idx].push_back(&sec);
    }
    if (int idx = GetFlatIdx(sec.ceiling_pic); idx != -1) {
      ceiling_sectors_[idx].push_back(&sec);
    }
  }
}

int FlatWallAnimator::GetFlatIdx(const std::string& flat_name) const {
  for (size_t i = 0; i < flat_names_.size(); ++i) {
    if (flat_name == flat_names_[i][0]) {
      return i;
    }
  }

  return -1;
}

void FlatWallAnimator::TickTime() {
  ++tick_counter_;
  if (tick_counter_ < kTicksPerStep) {
    return;
  } else {
    tick_counter_ = 0;
  }

  for (size_t i = 0; i < flat_idxs_.size(); ++i) {
    flat_idxs_[i] = (flat_idxs_[i] + 1) % flat_names_[i].size();
  }

  for (size_t i = 0; i < flat_names_.size(); ++i) {
    for (auto sec : floor_sectors_[i]) {
      sec->floor_pic = flat_names_[i][flat_idxs_[i]];
    }
  }
}

} // namespace world
