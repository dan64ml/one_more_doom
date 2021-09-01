#ifndef AMMUNUTION_H_
#define AMMUNUTION_H_

class Pistol;

namespace wpn {

struct Ammo {
  int bullets = 100;
  int shells = 100;
  int rockets = 100;
  int cells = 100;

  template<class T>
  bool GetAmmo(int num) {
    if constexpr (std::is_same_v<T, Pistol>) {
      if (num <= bullets) {
        bullets -= num;
        return true;
      }
    }

    return false;
  }
};

} // namespace wpn

#endif  // AMMUNUTION_H_
