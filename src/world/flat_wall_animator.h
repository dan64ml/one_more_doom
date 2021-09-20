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
  void CreateWallsList(std::vector<SideDef>& sides);

  void TickTime();

 private:
  const std::vector<std::vector<std::string>> flat_names_ = {
    {"NUKAGE1", "NUKAGE2", "NUKAGE3"},
    {"FWATER1", "FWATER2", "FWATER3", "FWATER4"},
    {"SWATER1", "SWATER2", "SWATER3", "SWATER4"},
    {"LAVA1", "LAVA2", "LAVA3", "LAVA4"},
    {"BLOOD1", "BLOOD2", "BLOOD3"},
    {"RROCK05", "RROCK06", "RROCK07", "RROCK08"},
    {"SLIME01", "SLIME02", "SLIME03", "SLIME04"},
    {"SLIME05", "SLIME06", "SLIME07", "SLIME08"},
    {"SLIME09", "SLIME10", "SLIME11", "SLIME12"},
  };

  const std::vector<std::vector<std::string>> texture_names_ = {
    //{"BLODGR4", "BLODGR1"},
    //{"SLADRIP3", "SLADRIP1"},
    {"BLODRIP1", "BLODRIP2", "BLODRIP3", "BLODRIP4"},
    {"FIREWALA", "FIREWALB", "FIREWALL"},
    {"GSTFONT1", "GSTFONT2", "GSTFONT3"},
    {"FIRELAV3", "FIRELAVA"},
    {"FIREMAG3", "FIREMAG2", "FIREMAG1"},
    {"FIREBLU1", "FIREBLU2"},
    {"ROCKRED1", "ROCKRED2", "ROCKRED3"},
    {"BFALL1", "BFALL2", "BFALL3", "BFALL4"},
    {"SFALL1", "SFALL2", "SFALL3", "SFALL4"},
    //{"WFALL4", "WFALL1"},
    {"DBRAIN1", "DBRAIN2", "DBRAIN3", "DBRAIN4"},
  };

  int GetFlatIdx(const std::string& flat_name) const;
  int GetTextureIdx(const std::string& texture_name) const;

  std::vector<std::vector<Sector*>> floor_sectors_;
  std::vector<std::vector<Sector*>> ceiling_sectors_;
  std::vector<int> flat_idxs_;

  std::vector<std::vector<SideDef*>> mid_textures_;
  std::vector<std::vector<SideDef*>> top_textures_;
  std::vector<std::vector<SideDef*>> bottom_textures_;
  std::vector<int> texture_idxs_;

  const int kTicksPerStep = 8;
  int tick_counter_ = 0;
};

} // namespace world

#endif  // FLAT_WALL_ANIMATOR_H_
