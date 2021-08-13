#ifndef MASKED_OBJECT_H_
#define MASKED_OBJECT_H_

#include <vector>
#include <string>

#include "plane_utils.h"

namespace rend {

struct MaskedObject {
  std::string texture_name;

  int height;
  int z;

  DPoint p1;
  DPoint p2;
  // Data to calculate screen coordinates (screen x of seg's ends)
  int sx_leftmost;
  int sx_rightmost;

  // Distance from the start of sprite to the leftmost visible point
  // or the same parametr from contex for a portal
  int full_offset;

  // ?? is it necessary?
  int pixel_texture_y_shift;

  // The distance from VP to the center of the object.
  // -1 for portals!!! 
  double distance;

  std::vector<int> top_clip;
  std::vector<int> bottom_clip;
  
  // Returns false if whole the object was clipped
  bool Clip(const std::vector<int>& top, const std::vector<int>& bottom);
};

} // namespace rend

#endif  // MASKED_OBJECT_H_