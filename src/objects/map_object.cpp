#include "map_object.h"

namespace mobj {

bool MapObject::TimeTick() {
  MoveObject();

  x += mom_x;
  y += mom_y;

  mom_x = mom_y = 0;
}

void MapObject::MoveObject() {
  XYMove();
  ZMove();
}

void MapObject::XYMove() {
  if (!mom_x && ! mom_y) {
    return;
  }

  // Keep original behavior
  if (mom_x > kMaxMove) {
    mom_x = kMaxMove;
  } else if (mom_x < -kMaxMove) {
    mom_x = -kMaxMove;
  }

  if (mom_y > kMaxMove) {
    mom_y = kMaxMove;
  } else if (mom_y < -kMaxMove) {
    mom_y = -kMaxMove;
  }
  
  double dx = mom_x;
  double dy = mom_y;
  int x_dest;
  int y_dest;

  do {
    if (dx > kMaxMove / 2 || dy > kMaxMove / 2) {
      dx /= 2;
      dy /= 2;

      x_dest = x + dx;
      y_dest = y + dy;
    } else {
      x_dest = x + dx;
      y_dest = y + dy;

      dx = dy = 0;
    }

    if (!TryMoveTo(x_dest, y_dest)) {
      if (!RunIntoAction()) {
        return;
      }
    }
  } while (dx || dy);

  SlowDown();
}

void MapObject::ZMove() {

}

// By default - do nothing and continue
bool MapObject::RunIntoAction() {
  return true;
}

// By defaul - stop the object
void MapObject::SlowDown() {
  mom_x = mom_y = 0;
}

bool MapObject::TryMoveTo(int new_x, int new_y) {
  // check if the new position empty
  if (!CheckPosition(new_x, new_y)) {
    return false;
  }

  // Check heights. Keep original behavior
  if (ceiling_z - floor_z < height) {
    // mobj too high
    return false;
  }
  if (!(flags & MF_TELEPORT) && (ceiling_z - z < height)) {
    // hit the ceiling by the head :)))
    return false;
  }
  if (!(flags & MF_TELEPORT) && (floor_z - z > kMaxStepSize)) {
    // The step too high
    return false;
  }
  if (!(flags & (MF_DROPOFF|MF_FLOAT)) && (floor_z - dropoff_z > height)) {
    // prevent falling
    return false;
  }

  // TODO: rebind the mobj and change x and y

  // TODO: ??? Act special lines ???

  return true;
}

bool MapObject::CheckPosition(int new_x, int new_y) {
  // fill here floor_z, ceiling_z and dropoff_z...
  // p_map.c
}

} // namespace mobj