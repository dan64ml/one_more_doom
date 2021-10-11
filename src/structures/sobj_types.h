#ifndef SOBJ_TYPES_H_
#define SOBJ_TYPES_H_

namespace sobj {

enum class MoveDirection {
  kUp,
  kDown,
  kWait,
  kSuspend  // Sobj can be suspended by using a line
};

enum class MoveResult {
  kOk,
  kCrushed,
  kGotDest
};

} // namespace sobj

#endif  // SOBJ_TYPES_H_
