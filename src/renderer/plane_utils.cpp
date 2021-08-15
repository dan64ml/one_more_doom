#include "plane_utils.h"

#include <cmath>

namespace rend {

BamAngle CalcAngle(int from_x, int from_y, int to_x, int to_y) {
  int dx = to_x - from_x;
  int dy = to_y - from_y;

  if (dx == 0 && dy == 0) {
    return 0;
  }
  if (dx == 0) {
    return (dy > 0) ? kBamAngle90 : -kBamAngle90;
  }
  if (dy == 0) {
    return (dx > 0) ? 0 : kBamAngle180;
  }

  double angle_rad = atan(fabs(static_cast<double>(dy) / dx));
  BamAngle angle = (kBamAngle180 / kPi) * angle_rad;

  if (dx > 0) {
    return (dy > 0) ? angle : kBamAngle360 - angle;
  } else {
    return (dy > 0) ? (kBamAngle180 - angle) : (angle + kBamAngle180);
  }
}

std::tuple<bool, DPoint, DPoint>
GetVisibleSegment(DPoint view_point, const world::Line* line, BamAngle h_fov) {
  const BamAngle a1 = CalcAngle(view_point.x, view_point.y, line->x1, line->y1) + h_fov / 2 - view_point.angle;
  const BamAngle a2 = CalcAngle(view_point.x, view_point.y, line->x2, line->y2) + h_fov / 2 - view_point.angle;

  if (DefineVpSide(line, view_point.x, view_point.y)) {
    // we look at the back side of the segment
    return {false, {}, {}};
  }

  const BamAngle right_border_local = kBamAngle0;
  const BamAngle left_border_local = h_fov;

  // Behind back plane, definitely invisible
  if (a1 >= left_border_local && a1 <= kBamAngle360 &&
      a2 >= left_border_local && a2 <= kBamAngle360 &&
      (a1 > a2 ? static_cast<BamAngle>(a1 - a2) : static_cast<BamAngle>(a2 - a1)) <= kBamAngle180) {
    return {false, {}, {}};
  }

  // Fully visible
  if (a1 <= left_border_local && a2 <= left_border_local) {
    return {true, {line->x1, line->y1, a1}, {line->x2, line->y2, a2}};
  }

  BamAngle right_border = view_point.angle - h_fov / 2;
  BamAngle left_border = view_point.angle + h_fov / 2;

  if (a1 <= left_border_local) {
    // Only p1 is in FOV
    auto [seg_n, seg_d] = CreateLine(line);
    BamAngle border_dir = (static_cast<BamAngle>(a2 - a1) > kBamAngle180) ? right_border : left_border;
    BamAngle bd = (static_cast<BamAngle>(a2 - a1) > kBamAngle180) ? right_border_local : left_border_local;
    auto [border_n, border_d] = CreateLine(view_point.x, view_point.y, border_dir);
    auto [end_x, end_y] = CalcIntersectionPoint(seg_n, seg_d, border_n, border_d);
    return {true, {line->x1, line->y1, a1}, {round(end_x), round(end_y), bd}};
  } else if (a2 <= left_border_local) {
    // Only p2 is in FOV
    auto [seg_n, seg_d] = CreateLine(line);
    BamAngle border_dir = (static_cast<BamAngle>(a1 - a2) > kBamAngle180) ? right_border : left_border;
    BamAngle bd = (static_cast<BamAngle>(a1 - a2) > kBamAngle180) ? right_border_local : left_border_local;
    auto [border_n, border_d] = CreateLine(view_point.x, view_point.y, border_dir);
    auto [end_x, end_y] = CalcIntersectionPoint(seg_n, seg_d, border_n, border_d);
    return {true, {round(end_x), round(end_y), bd}, {line->x2, line->y2, a2}};
  } else {
    // Both points are out of FOV, but segmet crosses it.
      auto [seg_n, seg_d] = CreateLine(line);

      BamAngle p1_angle, p2_angle, p1_angle_local, p2_angle_local;
      if (a1 < a2) {
        p1_angle = p1_angle_local = left_border;
        p2_angle = p2_angle_local = right_border;
      } else {
        p1_angle = p1_angle_local = right_border;
        p2_angle = p2_angle_local = left_border;
      }

      auto [p1_n, p1_d] = CreateLine(view_point.x, view_point.y, p1_angle);
      auto [p2_n, p2_d] = CreateLine(view_point.x, view_point.y, p2_angle);

      auto [p1_x, p1_y] = CalcIntersectionPoint(seg_n, seg_d, p1_n, p1_d);
      auto [p2_x, p2_y] = CalcIntersectionPoint(seg_n, seg_d, p2_n, p2_d);

      return {true, DPoint{round(p1_x), round(p1_y), left_border_local}, DPoint{round(p2_x), round(p2_y), right_border_local}};
  }
}

std::tuple<bool, DPoint, DPoint>
GetVisibleSegment(DPoint view_point, int x1, int y1, int x2, int y2, BamAngle h_fov) {
  const BamAngle a1 = CalcAngle(view_point.x, view_point.y, x1, y1) + h_fov / 2 - view_point.angle;
  const BamAngle a2 = CalcAngle(view_point.x, view_point.y, x2, y2) + h_fov / 2 - view_point.angle;

  // Prevent visibility when view point lies on the line.
  // May be a better way exists???
  if (static_cast<BamAngle>(static_cast<BamAngle>(a1 - a2) - kBamAngle180) < 5) {
    return {false, {}, {}};
  }

  if (DefineVpSide(x1, y1, x2, y2, view_point.x, view_point.y)) {
    // we look at the back side of the segment
    return {false, {}, {}};
  }

  const BamAngle right_border_local = kBamAngle0;
  const BamAngle left_border_local = h_fov;

  // Behind back plane, definitely invisible
  if (a1 >= left_border_local && a1 <= kBamAngle360 &&
      a2 >= left_border_local && a2 <= kBamAngle360 &&
      (a1 > a2 ? static_cast<BamAngle>(a1 - a2) : static_cast<BamAngle>(a2 - a1)) <= kBamAngle180) {
    return {false, {}, {}};
  }

  // Fully visible
  if (a1 <= left_border_local && a2 <= left_border_local) {
    return {true, {x1, y1, a1}, {x2, y2, a2}};
  }

  BamAngle right_border = view_point.angle - h_fov / 2;
  BamAngle left_border = view_point.angle + h_fov / 2;

  if (a1 <= left_border_local) {
    // Only p1 is in FOV
    auto [seg_n, seg_d] = CreateLine(x1, y1, x2, y2);
    BamAngle border_dir = (static_cast<BamAngle>(a2 - a1) > kBamAngle180) ? right_border : left_border;
    BamAngle bd = (static_cast<BamAngle>(a2 - a1) > kBamAngle180) ? right_border_local : left_border_local;
    auto [border_n, border_d] = CreateLine(view_point.x, view_point.y, border_dir);
    auto [end_x, end_y] = CalcIntersectionPoint(seg_n, seg_d, border_n, border_d);
    return {true, {x1, y1, a1}, {round(end_x), round(end_y), bd}};
  } else if (a2 <= left_border_local) {
    // Only p2 is in FOV
    auto [seg_n, seg_d] = CreateLine(x1, y1, x2, y2);
    BamAngle border_dir = (static_cast<BamAngle>(a1 - a2) > kBamAngle180) ? right_border : left_border;
    BamAngle bd = (static_cast<BamAngle>(a1 - a2) > kBamAngle180) ? right_border_local : left_border_local;
    auto [border_n, border_d] = CreateLine(view_point.x, view_point.y, border_dir);
    auto [end_x, end_y] = CalcIntersectionPoint(seg_n, seg_d, border_n, border_d);
    return {true, {round(end_x), round(end_y), bd}, {x2, y2, a2}};
  } else {
    // Both points are out of FOV, but segmet crosses it.
      auto [seg_n, seg_d] = CreateLine(x1, y1, x2, y2);

      BamAngle p1_angle, p2_angle, p1_angle_local, p2_angle_local;
      if (a1 < a2) {
        p1_angle = p1_angle_local = left_border;
        p2_angle = p2_angle_local = right_border;
      } else {
        p1_angle = p1_angle_local = right_border;
        p2_angle = p2_angle_local = left_border;
      }

      auto [p1_n, p1_d] = CreateLine(view_point.x, view_point.y, p1_angle);
      auto [p2_n, p2_d] = CreateLine(view_point.x, view_point.y, p2_angle);

      auto [p1_x, p1_y] = CalcIntersectionPoint(seg_n, seg_d, p1_n, p1_d);
      auto [p2_x, p2_y] = CalcIntersectionPoint(seg_n, seg_d, p2_n, p2_d);

      //return {true, DPoint{round(p1_x), round(p1_y), p1_angle_local}, DPoint{round(p2_x), round(p2_y), p2_angle_local}};
      return {true, DPoint{round(p1_x), round(p1_y), left_border_local}, DPoint{round(p2_x), round(p2_y), right_border_local}};
  }
}

std::pair<math::Vec2d, double> CreateLine(int x1, int y1, int x2, int y2) {
  double dx = x2 - x1;
  double dy = y2 - y1;
  math::Vec2d n = {dy, -dx};
  n = n / n.length();
  double d = -(n * math::Vec2d{x1, y1});

  return {n, d};
}

std::pair<math::Vec2d, double> CreateLine(const world::Line* line) {
  double dx = line->x2 - line->x1;
  double dy = line->y2 - line->y1;
  math::Vec2d n = {dy, -dx};
  n = n / n.length();
  double d = -(n * math::Vec2d{line->x1, line->y1});

  return {n, d};
}

std::pair<math::Vec2d, double> CreateLine(int x, int y, BamAngle direction) {
  double dx = BamCos(direction);
  double dy = BamSin(direction);
  math::Vec2d n = {dy, -dx};
  double d = -(n * math::Vec2d(x, y));

  return {n, d};
}

std::pair<double, double> CalcIntersectionPoint(math::Vec2d n1, double d1, math::Vec2d n2, double d2) {
  double det = n1.x * n2.y - n1.y * n2.x;

  double x = (d2 * n1.y - d1 * n2.y) / det;
  double y = (d1 * n2.x - d2 * n1.x) / det;

  return {x, y};
}


int DefineVpSide(const world::Line* line, int vp_x, int vp_y) {
  int dx = line->x2 - line->x1;
  int dy = line->y2 - line->y1;

  if (!dx) {
    if (vp_x <= line->x1) {
      return dy > 0;
    } else {
      return dy < 0;
    }
  }

  if (!dy) {
    if (vp_y <= line->y1) {
      return dx < 0;
    } else {
      return dx > 0;
    }
  }

  double dvp_x = vp_x - line->x1;
  double dvp_y = vp_y - line->y1;
  double area = dx * dvp_y - dvp_x * dy;

  return (area > 0);
}

// 0 - right
int DefineVpSide(int x1, int y1, int x2, int y2, int vp_x, int vp_y) {
  int dx = x2 - x1;
  int dy = y2 - y1;

  if (!dx) {
    if (vp_x <= x1) {
      return dy > 0;
    } else {
      return dy < 0;
    }
  }

  if (!dy) {
    if (vp_y <= y1) {
      return dx < 0;
    } else {
      return dx > 0;
    }
  }

  double dvp_x = vp_x - x1;
  double dvp_y = vp_y - y1;
  double area = dx * dvp_y - dvp_x * dy;

  return (area > 0);
}

std::pair<int, int> CreateIntersection(int r1_left, int r1_right, int r2_left, int r2_right) {
  if (r1_left > r2_right || r1_right < r2_left) {
    return {0, -1};
  }

  if (r1_left <= r2_left && r1_right >= r2_right) {
    return {r2_left, r2_right};
  } else if (r2_left <= r1_left && r2_right >= r1_right) {
    return {r1_left, r1_right};
  } else if (r1_left >= r2_left && r1_left <= r2_right) {
    return {r1_left, r2_right};
  } else {
    return {r2_left, r1_right};
  }
}

double SegmentLength(int x1, int y1, int x2, int y2) {
  return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

double SegmentLength(DPoint p1, DPoint p2) {
  return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}

} // namespace rend
