#include "world_utils.h"
#include "renderer/plane_utils.h"

namespace math {

ObjPosition LinePointPosition(const world::Line* line, double x, double y) {
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

std::tuple<bool, double, double> GetMobjIntersection(double x1, double y1, double x2, double y2, const mobj::MapObject* mobj) {
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


std::tuple<bool, double, double> GetLinesIntersection(double x1, double y1, double x2, double y2, const world::Line* line) {
  return GetLinesIntersection(x1, y1, x2, y2, line->x1, line->y1, line->x2, line->y2);
}

std::tuple<bool, double, double> GetLinesIntersection(double l1_x1, double l1_y1, double l1_x2, double l1_y2, 
                                                      double l2_x1, double l2_y1, double l2_x2, double l2_y2) {
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

std::tuple<bool, double, double> GetSegmentsIntersection(double s1_x1, double s1_y1, double s1_x2, double s1_y2,
                                                         double s2_x1, double s2_y1, double s2_x2, double s2_y2) {
  const double kEps = 0.0001;

  auto [cross, cx, cy] = GetLinesIntersection(s1_x1, s1_y1, s1_x2, s1_y2, s2_x1, s2_y1, s2_x2, s2_y2);
  if (!cross) {
    return {false, 0, 0};
  }

  double x_left = std::max(std::min(s1_x1, s1_x2), std::min(s2_x1, s2_x2)) - kEps;
  double x_right = std::min(std::max(s1_x1, s1_x2), std::max(s2_x1, s2_x2)) + kEps;
  double y_top = std::min(std::max(s1_y1, s1_y2), std::max(s2_y1, s2_y2)) + kEps;
  double y_bottom = std::max(std::min(s1_y1, s1_y2), std::min(s2_y1, s2_y2)) - kEps;
  cross = x_left <= cx && cx <= x_right && y_bottom <= cy && cy <= y_top;

  return {cross, cx, cy};

}

std::tuple<bool, double, double> GetSegmentsIntersection(double x1, double y1, double x2, double y2, const world::Line* line) {
  return GetSegmentsIntersection(x1, y1, x2, y2, line->x1, line->y1, line->x2, line->y2);
}

std::pair<int, int> ShiftToCenter(int cx, int cy, int x, int y, int shift) {
  x += (x > cx) ? -shift : shift;
  y += (y > cy) ? -shift : shift;
  return {x, y};
}

bool CorrectOpening(world::Opening& op, const world::Line* line, double distance) {
  const double kEps = 0.001;
  if (std::abs(distance) < kEps) {
    return true;
  }

  // Current opening
  int high_z = op.view_line_z + distance * op.coef_high_opening;
  int low_z = op.view_line_z - distance * op.coef_low_opening;
  // Line opening
  int line_low = std::max(line->sides[0]->sector->floor_height, line->sides[1]->sector->floor_height);
  int line_high = std::min(line->sides[0]->sector->ceiling_height, line->sides[1]->sector->ceiling_height);

  if (line_low > low_z) {
    low_z = line_low;
    op.coef_low_opening = (op.view_line_z - low_z) / distance;
  }
  if (line_high < high_z) {
    high_z = line_high;
    op.coef_high_opening = (high_z - op.view_line_z) / distance;
  }

  op.high_z = high_z;
  op.low_z = low_z;
  
  return high_z > low_z;
}

int GetOpenRange(const world::Line* line) {
  if (line->sides[1] == nullptr) {
    return 0;
  }

  return std::min(line->sides[0]->sector->ceiling_height - line->sides[0]->sector->floor_height,
                  line->sides[1]->sector->ceiling_height - line->sides[1]->sector->floor_height);
}

double GetLowestCeilingHeight(const world::Sector* sec) {
  double height = std::numeric_limits<int>::max();

  for (const auto line : sec->lines) {
    auto opp = GetOppositeSector(sec, line);
    if (!opp) {
      continue;
    }

    height = std::min(height, opp->ceiling_height);
  }

  return height;
}

double GetLowestFloorHeight(const world::Sector* sec) {
  double height = std::numeric_limits<int>::max();

  for (const auto line : sec->lines) {
    auto opp = GetOppositeSector(sec, line);
    if (!opp) {
      continue;
    }

    height = std::min(height, opp->floor_height);
  }

  return height;
}

double GetHighestFloorHeight(const world::Sector* sec) {
  double height = std::numeric_limits<int>::min();

  for (const auto line : sec->lines) {
    auto opp = GetOppositeSector(sec, line);
    if (!opp) {
      continue;
    }

    height = std::max(height, opp->floor_height);
  }

  return height;
}

double GetNextHighestFloorHeight(const world::Sector* sec, double current_height) {
  double height = std::numeric_limits<int>::max();

  for (const auto line : sec->lines) {
    auto opp = GetOppositeSector(sec, line);
    if (!opp) {
      continue;
    }
    if (opp->floor_height <= current_height) {
      continue;
    }

    height = std::min(height, opp->floor_height);
  }

  return (height == std::numeric_limits<int>::max()) ? current_height : height;
}

double GetHighestCeilingHeight(const world::Sector* sec) {
  double height = std::numeric_limits<int>::min();

  for (auto line : sec->lines) {
    auto opp = GetOppositeSector(sec, line);
    if (!opp) {
      continue;
    }

    height = std::max(height, opp->ceiling_height);
  }

  return height;
}

world::Sector* GetOppositeSector(const world::Sector* sec, const world::Line* line) {
  if (!(line->flags & world::kLDFTwoSided)) {
    return nullptr;
  }

  return (sec == line->sides[0]->sector) ? line->sides[1]->sector : line->sides[0]->sector;
}

std::pair<double, double> GetOppositeFloorCeilingHeight(const world::Line* line, double x, double y) {
  if (!(line->flags & world::kLDFTwoSided)) {
    return {0, 0};
  }

  ObjPosition pos = LinePointPosition(line, x, y);
  if (pos == kCross) {
    return {0, 0};
  }

  int side = pos ^ 1;
  auto sec = line->sides[side]->sector;

  return {sec->floor_height, sec->ceiling_height};
}

} // namespace math
