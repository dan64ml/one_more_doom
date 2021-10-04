#ifndef LINE_TEXTURE_SWITCHER_H_
#define LINE_TEXTURE_SWITCHER_H_

#include "structure_object.h"

#include <unordered_map>
#include <string>

#include "world/world_types.h"

namespace sobj {

class LineTextureSwitcher : public StructureObject {
 public:
  LineTextureSwitcher(world::Line* line);

  bool TickTime() override;
  bool Trigger(mobj::MapObject*) override { return true;}

  // Original engine uses hardcoded texture names to define if it's a switch
  // 0 - line isn't a switch, 1 - top texture, 2 - middle, 3 - bottom
  int static IsSwitch(const world::Line* line);

 private:
  const static std::unordered_map<std::string, std::string> texture_pairs_;

  world::Line* line_;
  int on_time_ = 33;  // 1sec
  int texture_number_;
  std::string off_name_;
};

} // namespace sobj

#endif  // LINE_TEXTURE_SWITCHER_H_
