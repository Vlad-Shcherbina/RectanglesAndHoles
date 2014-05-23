#include <ostream>
#include <vector>
#include <utility>


template<typename T>
std::ostream& operator<<(std::ostream &out, const std::vector<T> &v) {
  out << "[";
  bool first = true;
  for (const auto &e : v) {
    if (!first)
      out << ", ";
    first = false;
    out << e;
  }
  out << "]";
  return out;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream &out, const std::pair<T1, T2> &p) {
  out << "(" << p.first << ", " << p.second << ")";
  return out;
}
