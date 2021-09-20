#include "flat_wall_animator.h"

#include <algorithm>

namespace world {

FlatWallAnimator::FlatWallAnimator() {
  floor_sectors_.resize(flat_names_.size());
  ceiling_sectors_.resize(flat_names_.size());
  flat_idxs_.resize(flat_names_.size(), 0);

  mid_textures_.resize(texture_names_.size());
  top_textures_.resize(texture_names_.size());
  bottom_textures_.resize(texture_names_.size());
  texture_idxs_.resize(texture_names_.size(), 0);
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

void FlatWallAnimator::CreateWallsList(std::vector<SideDef>& sides) {
  for (auto& side : sides) {
    if (int idx = GetTextureIdx(side.mid_texture); idx != -1) {
      mid_textures_[idx].push_back(&side);
    }
    if (int idx = GetTextureIdx(side.top_texture); idx != -1) {
      top_textures_[idx].push_back(&side);
    }
    if (int idx = GetTextureIdx(side.bottom_texture); idx != -1) {
      bottom_textures_[idx].push_back(&side);
    }
  }
}

void FlatWallAnimator::CreateScrollingWallsList(std::vector<Line>& lines) {
  for (auto& line : lines) {
    if (line.specials == 48) {
      scrolling_walls_.push_back(&line);
    }
  }
}

int FlatWallAnimator::GetFlatIdx(const std::string& flat_name) const {
  for (size_t i = 0; i < flat_names_.size(); ++i) {
    const auto& names = flat_names_[i];
    if (std::find(begin(names), end(names), flat_name) != end(names)) {
      return i;
    }
  }

  return -1;
}

int FlatWallAnimator::GetTextureIdx(const std::string& texture_name) const {
  for (size_t i = 0; i < texture_names_.size(); ++i) {
    const auto& names = texture_names_[i];
    if (std::find(begin(names), end(names), texture_name) != end(names)) {
      return i;
    }
  }

  return -1;
}

void FlatWallAnimator::TickTime() {
  // Scrolling walls
  for (auto line : scrolling_walls_) {
    line->sides[0]->texture_offset++;
  }

  ++tick_counter_;
  if (tick_counter_ < kTicksPerStep) {
    return;
  } else {
    tick_counter_ = 0;
  }

  // Flats
  for (size_t i = 0; i < flat_idxs_.size(); ++i) {
    flat_idxs_[i] = (flat_idxs_[i] + 1) % flat_names_[i].size();
  }

  for (size_t i = 0; i < flat_names_.size(); ++i) {
    for (auto sec : floor_sectors_[i]) {
      sec->floor_pic = flat_names_[i][flat_idxs_[i]];
    }
    for (auto sec : ceiling_sectors_[i]) {
      sec->ceiling_pic = flat_names_[i][flat_idxs_[i]];
    }
  }

  // Walls
  for (size_t i = 0; i < texture_idxs_.size(); ++i) {
    texture_idxs_[i] = (texture_idxs_[i] + 1) % texture_names_[i].size();
  }

  for (size_t i = 0; i < texture_names_.size(); ++i) {
    for (auto side : mid_textures_[i]) {
      side->mid_texture = texture_names_[i][texture_idxs_[i]];
    }
    for (auto side : top_textures_[i]) {
      side->top_texture = texture_names_[i][texture_idxs_[i]];
    }
    for (auto side : bottom_textures_[i]) {
      side->bottom_texture = texture_names_[i][texture_idxs_[i]];
    }
  }
}

} // namespace world
