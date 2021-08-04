#ifndef WORLD_H_
#define WORLD_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "world_types.h"

namespace world {

class World {
 public:
  World();

  void OpenWad(std::string wad_file);
  void LoadLevel(size_t level);

  void TickTime();

 private:
  std::string wad_file_name_;
  // Level names and lumps
  std::vector<std::pair<std::string, std::unordered_map<std::string, LumpPos>>> levels_;
  size_t current_level_;

  std::vector<Sector> sectors_;
  std::vector<SideDef> sides_;
  std::vector<Line> lines_;

private:
 void ClearLevel();

 void LoadSectors(std::ifstream& fin);
 void LoadSideDefs(std::ifstream& fin);
 void LoadLines(std::ifstream& fin, RawVertex* vertexex);
};

} // namespace world

#endif  // WORLD_H_
