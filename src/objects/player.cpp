#include "player.h"

namespace mobj {

void Player::Move(rend::BamAngle delta_angle, double forward_move, double side_move) {
  angle += delta_angle;

  if (z > floor_z) {
    // can't move when fly
    return;
  }

  double dx = forward_move * rend::BamCos(angle);
  double dy = forward_move * rend::BamSin(angle);
  dx += side_move * rend::BamCos(angle - rend::kBamAngle90);
  dy += side_move * rend::BamSin(angle - rend::kBamAngle90);

  mom_x += dx;
  mom_y += dy;
}

bool Player::RunIntoAction() {
  return true;
}

} // namespace mobj