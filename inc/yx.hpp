#pragma once

#include <functional>
#include <utility>

template<typename T>
struct YX
{
  T y{};
  T x{};
};

template<typename T>
bool operator<(YX<T> const& l, YX<T> const& r)
{
  return std::make_pair(l.y, l.x) < std::make_pair(r.y, r.x);
}

template<typename T>
bool operator==(YX<T> const& l, YX<T> const& r)
{
  return std::make_pair(l.y, l.x) == std::make_pair(r.y, r.x);
}

template<typename T>
struct std::hash<YX<T>>
{
  std::size_t operator()(const YX<T>& s) const noexcept
  {
    return s.y ^ (s.x << 1);
  }
};
