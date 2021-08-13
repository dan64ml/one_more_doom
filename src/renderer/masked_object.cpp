#include "masked_object.h"

//#include "objects/map_object.h"
//#include "plane_utils.h"

namespace rend {

bool MaskedObject::Clip(const std::vector<int>& top, const std::vector<int>& bottom) {
  int count = 0;
  for (int i = sx_leftmost; i <= sx_rightmost; ++i) {
    if (top[i] == -1 || top[i] <= (bottom[i] + 1)) {
      top_clip[i - sx_leftmost] = -1;
      ++count;
      continue;
    }

    top_clip[i - sx_leftmost] = std::min(top[i], top_clip[i - sx_leftmost]);
    bottom_clip[i - sx_leftmost] = std::max(bottom[i], bottom_clip[i - sx_leftmost]);

    if (top_clip[i - sx_leftmost] <= (bottom_clip[i - sx_leftmost] + 1)) {
      //bottom_clip[i - sx_leftmost] = -1;
      top_clip[i - sx_leftmost] = -1;
      ++count;
    }
  }

  return (count != (sx_rightmost - sx_leftmost + 1));
}

} // namespace rend