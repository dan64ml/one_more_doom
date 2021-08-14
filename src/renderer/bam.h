#ifndef BAM_H_
#define BAM_H_

#include <limits>
#include <cstdint>

namespace rend {
// Pi
const double kPi = 3.1415926;

// 0 == East (OX+), 16384 == North (OY+).
using BamAngle = uint16_t;

const BamAngle kBamAngle0 = 0;
const BamAngle kBamAngle45 = std::numeric_limits<BamAngle>::max() / 8;
const BamAngle kBamAngle90 = std::numeric_limits<BamAngle>::max() / 4;
const BamAngle kBamAngle135 = (std::numeric_limits<BamAngle>::max() / 8) * 3;
const BamAngle kBamAngle180 = std::numeric_limits<BamAngle>::max() / 2;
const BamAngle kBamAngle360 = std::numeric_limits<BamAngle>::max();

// Convert degrees to BamAngle
BamAngle DegreesToBam(int angle);

// Calculate angle between line segment and OX axis
BamAngle CalcAngle(int from_x, int from_y, int to_x, int to_y);

// Triginometric functions. May be realized like a lookup tables.
double BamCos(BamAngle a);
double BamSin(BamAngle a);
double BamTan(BamAngle a);


} // namespace rend

#endif  // BAM_H_