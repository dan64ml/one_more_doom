#ifndef VEC2D_H_
#define VEC2D_H_

#include <cmath>

namespace math {

template <class T>
struct Vector2D {
  T x;
  T y;

  Vector2D() : x(0), y(0) {}
  Vector2D(int x, int y) : x(x), y(y) {}
  Vector2D(T x, T y) : x(x), y(y) {}

  T operator*(const Vector2D& rhs) const { return x * rhs.x + y * rhs.y; }
  T length() const { return sqrt(x*x + y*y); }

  // TODO: сделать их шаблонными!!! Конфликтуют со скалярным произведением!!!
  //template<class U>
  Vector2D& operator*(T a) { x *= a; y *= a; return *this; }
  template<class U>
  Vector2D& operator/(U a) { x /= a; y /= a; return *this; }

};

using Vec2d = Vector2D<double>;

} // namespace math

#endif  // VEC2D_H_