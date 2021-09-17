#include "bam.h"

#include <cmath>

namespace rend {

double BamCos(BamAngle a) {
  double angle = (2.0 * kPi / kBamAngle360) * a;
  return cos(angle);
}

double BamSin(BamAngle a) {
  double angle = (2.0 * kPi / kBamAngle360) * a;
  return sin(angle);
}

double BamTan(BamAngle a) {
  double angle = (2.0 * kPi / kBamAngle360) * a;
  return tan(angle);
}

BamAngle DegreesToBam(int angle) {
  angle %= 360;
  return (kBamAngle360 / 360.) * angle;
}

BamAngle BamArcSin(double v) {
  double an = asin(v);
  return kBamAngle180 * an / kPi;
}

} // namespace rend
