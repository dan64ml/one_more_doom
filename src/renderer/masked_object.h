#ifndef MASKED_OBJECT_H_
#define MASKED_OBJECT_H_

#include <vector>
#include <string>

//#include "graphics/texture.h"
#include "plane_utils.h"

//namespace mobj {
//  class MapObject;
//}

namespace rend {

//class SegmentRendContext;
//struct DPoint;

struct MaskedObject {
  std::string text;
  // Current texture
  //graph::Texture texture;
  // Visible segment phys lenght. Necessary for texturing
  //double segment_len;
  // Distance from the start of sprite to the leftmost visible point
  //int full_offset;  

  int height;
  int z;

  DPoint p1;
  DPoint p2;
  // Data to calculate screen coordinates (screen x of seg's ends)
  int sx_leftmost;
  int sx_rightmost;

  // For walls and middle part of portals
  //int mid_y_top;
  //int mid_y_bottom;
  //double mid_dy_top;
  //double mid_dy_bottom;

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