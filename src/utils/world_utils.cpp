#include "world_utils.h"
#include "renderer/plane_utils.h"

namespace math {

ObjPosition LinePointPosition(const world::Line* line, int x, int y) {
  int dx = line->x2 - line->x1;
  int dy = line->y2 - line->y1;

  if (!dx) {
    if (x <= line->x1) {
      return (dy > 0) ? kLeftSide : kRightSide;
    } else {
      return (dy < 0) ? kLeftSide : kRightSide;
    }
  }

  if (!dy) {
    if (y <= line->y1) {
      return (dx < 0) ? kLeftSide : kRightSide;
    } else {
      return (dx > 0) ? kLeftSide : kRightSide;
    }
  }

  double dvp_x = x - line->x1;
  double dvp_y = y - line->y1;
  double area = dx * dvp_y - dvp_x * dy;

  return (area > 0) ? kLeftSide : kRightSide;
}

// Considers that the line is infinite
ObjPosition LineBBoxPosition(const world::Line* line, const world::BBox* bbox) {
  int dx = line->x2 - line->x1;
  int dy = line->y2 - line->y1;

  int p1p, p2p;

  if (dy == 0) {
    // horizontal line
    p1p = bbox->top > line->y1;
    p2p = bbox->bottom > line->y1;
    if (dx < 0) {
      p1p ^= 1;
      p2p ^= 1;
    }
  } else if (dx == 0) {
    // vertical line
    p1p = bbox->right < line->x1;
    p2p = bbox->left < line->x1;
    if (dy < 0) {
      p1p ^= 1;
      p2p ^= 1;
    }
  } else if (dx*dy > 0 ) {
    p1p = LinePointPosition(line, bbox->left, bbox->top);
    p2p = LinePointPosition(line, bbox->right, bbox->bottom);
  } else {
    p1p = LinePointPosition(line, bbox->right, bbox->top);
    p2p = LinePointPosition(line, bbox->left, bbox->bottom);
  }

  if (p1p == p2p) {
    return p1p ? kLeftSide : kRightSide;
  }

  return kCross;
}

double GetDistanceToIntersection(int x1, int y1, int x2, int y2, const mobj::MapObject* mobj) {
  world::BBox bb {x1, x2, y1, y2};
  if (bb.left > bb.right) {
    std::swap(bb.left, bb.right);
  }
  if (bb.top < bb.bottom) {
    std::swap(bb.top, bb.bottom);
  }

  // Definitely don't cross
  if (mobj->x - mobj->radius > bb.right || mobj->x + mobj->radius < bb.left) {
    return -1;
  }
  if (mobj->y + mobj->radius < bb.bottom || mobj->y - mobj->radius > bb.top) {
    return -1;
  }

  auto center_angle = rend::CalcAngle(x1, y1, mobj->x, mobj->y);
  auto cp_angle = rend::CalcAngle(x1, y1, x2, y2);
  double center_dist = rend::SegmentLength(x1, y1, mobj->x, mobj->y);
  double cp_dist = abs(center_dist * rend::BamTan(center_angle - cp_angle));

  if (cp_dist > mobj->radius) {
    return -1;
  }

  return sqrt(center_dist * center_dist + cp_dist * cp_dist);
}

std::tuple<bool, double, double> GetMobjIntersection(int x1, int y1, int x2, int y2, const mobj::MapObject* mobj) {
  world::BBox bb {x1, x2, y1, y2};
  if (bb.left > bb.right) {
    std::swap(bb.left, bb.right);
  }
  if (bb.top < bb.bottom) {
    std::swap(bb.top, bb.bottom);
  }

  // Definitely don't cross
  if (mobj->x - mobj->radius > bb.right || mobj->x + mobj->radius < bb.left) {
    return {false, 0, 0};
  }
  if (mobj->y + mobj->radius < bb.bottom || mobj->y - mobj->radius > bb.top) {
    return {false, 0, 0};
  }

  auto mobj_angle = rend::CalcAngle(x1, y1, mobj->x, mobj->y);
  double dx = 2 * mobj->radius * rend::BamCos(rend::kBamAngle90 - mobj_angle);
  double dy = 2 * mobj->radius * rend::BamSin(rend::kBamAngle90 - mobj_angle);
    
  double mobj_x1 = mobj->x - dx / 2;
  double mobj_y1 = mobj->y + dy / 2;
  double mobj_x2 = mobj_x1 + dx;
  double mobj_y2 = mobj_y1 - dy;

  return GetSegmentsIntersection(x1, y1, x2, y2, mobj_x1, mobj_y1, mobj_x2, mobj_y2);
}


std::tuple<bool, double, double> GetLinesIntersection(int x1, int y1, int x2, int y2, const world::Line* line) {
  /*const double kEps = 0.000001;

  double n1_x = y2 - y1;
  double n1_y = x1 - x2;
  double len = sqrt(n1_x * n1_x + n1_y * n1_y);
  n1_x /= len;
  n1_y /= len;
  double d1 = -(x1 * n1_x + y1 * n1_y);

  double n2_x = line->y2 - line->y1;
  double n2_y = line->x1 - line->x2;
  len = sqrt(n2_x * n2_x + n2_y * n2_y);
  n2_x /= len;
  n2_y /= len;
  double d2 = -(line->x1 * n2_x + line->y1 * n2_y);

  const double det = n1_x * n2_y - n1_y * n2_x;
  if (std::abs(det) < kEps) {
    return {false, 0, 0};
  }

  double cx = (d2 * n1_y - d1 * n2_y) / det;
  double cy = (d1 * n2_x - d2 * n1_x) / det;

  return {true, cx, cy};*/
  return GetLinesIntersection(x1, y1, x2, y2, line->x1, line->y1, line->x2, line->y2);
}

std::tuple<bool, double, double> GetLinesIntersection(int l1_x1, int l1_y1, int l1_x2, int l1_y2, 
                                                      int l2_x1, int l2_y1, int l2_x2, int l2_y2) {
  const double kEps = 0.000001;

  double n1_x = l1_y2 - l1_y1;
  double n1_y = l1_x1 - l1_x2;
  double len = sqrt(n1_x * n1_x + n1_y * n1_y);
  n1_x /= len;
  n1_y /= len;
  double d1 = -(l1_x1 * n1_x + l1_y1 * n1_y);

  double n2_x = l2_y2 - l2_y1;
  double n2_y = l2_x1 - l2_x2;
  len = sqrt(n2_x * n2_x + n2_y * n2_y);
  n2_x /= len;
  n2_y /= len;
  double d2 = -(l2_x1 * n2_x + l2_y1 * n2_y);

  const double det = n1_x * n2_y - n1_y * n2_x;
  if (std::abs(det) < kEps) {
    return {false, 0, 0};
  }

  double cx = (d2 * n1_y - d1 * n2_y) / det;
  double cy = (d1 * n2_x - d2 * n1_x) / det;

  return {true, cx, cy};
}

std::tuple<bool, double, double> GetSegmentsIntersection(int s1_x1, int s1_y1, int s1_x2, int s1_y2,
                                                         int s2_x1, int s2_y1, int s2_x2, int s2_y2) {
  auto [cross, cx, cy] = GetLinesIntersection(s1_x1, s1_y1, s1_x2, s1_y2, s2_x1, s2_y1, s2_x2, s2_y2);
  if (!cross) {
    return {false, 0, 0};
  }

  double x_left = std::max(std::min(s1_x1, s1_x2), std::min(s2_x1, s2_x2));
  double x_right = std::min(std::max(s1_x1, s1_x2), std::max(s2_x1, s2_x2));
  double y_top = std::min(std::max(s1_y1, s1_y2), std::max(s2_y1, s2_y2));
  double y_bottom = std::max(std::min(s1_y1, s1_y2), std::min(s2_y1, s2_y2));
  cross = x_left <= cx && cx <= x_right && y_bottom <= cy && cy <= y_top;

  return {cross, cx, cy};

}

std::tuple<bool, double, double> GetSegmentsIntersection(int x1, int y1, int x2, int y2, const world::Line* line) {
  const double kEps = 0.5;

  auto [cross, cx, cy] = GetLinesIntersection(x1, y1, x2, y2, line);
  if (!cross) {
    return {false, 0, 0};
  }

  double x_left = std::max(std::min(x1, x2), std::min(line->x1, line->x2)) - kEps;
  double x_right = std::min(std::max(x1, x2), std::max(line->x1, line->x2)) + kEps;
  double y_top = std::min(std::max(y1, y2), std::max(line->y1, line->y2)) + kEps;
  double y_bottom = std::max(std::min(y1, y2), std::min(line->y1, line->y2)) - kEps;
  cross = x_left <= cx && cx <= x_right && y_bottom <= cy && cy <= y_top;

  return {cross, cx, cy};
}

double GetDistanceToIntersection(int x1, int y1, int x2, int y2, const world::Line* line) {
  world::BBox bb {x1, x2, y1, y2};
  if (bb.left > bb.right) {
    std::swap(bb.left, bb.right);
  }
  if (bb.top < bb.bottom) {
    std::swap(bb.top, bb.bottom);
  }

//  if (bb.left > line->bbox.right || bb.right < line->bbox.left) {
//    return -1;
//  }
//  if (bb.bottom > line->bbox.top || bb.top < line->bbox.bottom) {
//    return -1;
//  }

  auto [n1, d1] = rend::CreateLine(x1, y1, x2, y2);
  auto [n2, d2] = rend::CreateLine(line);

  //auto [cx, cy] = rend::CalcIntersectionPoint(n1, d1, n2, d2);
  double cx, cy;
  if (!rend::FindIntersectionPoint(cx, cy, n1, d1, n2, d2)) {
    return -1;
  }

  int bb_eps = 2;

  if (cx + bb_eps < bb.left || cx - bb_eps > bb.right || cy + bb_eps < bb.bottom || cy - bb_eps > bb.top) {
    return -1;
  }
  if (cx + bb_eps < line->bbox.left || cx - bb_eps > line->bbox.right || cy + bb_eps < line->bbox.bottom || cy - bb_eps > line->bbox.top) {
    return -1;
  }

  return rend::SegmentLength(x1, y1, cx, cy);
}

} // namespace math