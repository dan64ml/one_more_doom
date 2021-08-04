#ifndef WAD_UTILS_H_
#define WAD_UTILS_H_

#include <string>
#include <fstream>
#include <memory>

namespace wad {

template<int N>
bool strcmp(const char* str1, const char* str2) {
  for (int i = 0; i < N; ++i) {
    if (str1[i] != str2[i]) {
      return false;
    }
    if (str1[i] == 0 && str2[i] == 0) {
      return true;
    }
  }

  return true;
}

template<int N>
std::string to_string(const char* str) {
  int size = 0;
  for (size = 0; size < N && str[size] != 0; ++size);
  
  std::string ret(str, size);
  for (auto& c : ret) {
    c = toupper(c);
  }

  return ret;
}

template<class T>
std::unique_ptr<T[]> LoadLump(std::ifstream& fin, int offset, int size) {
  int num_elements = size / sizeof(T);
  std::unique_ptr<T[]> ret(new T[num_elements]);

  fin.seekg(offset);
  fin.read(reinterpret_cast<char*>(ret.get()), size);

  return ret;
}

} // namespace wad

#endif  // WAD_UTILS_H_
