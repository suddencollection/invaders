#pragma once

#include "yx.hpp"

#include <filesystem>
#include <string>

class Sprite
{
public:
  Sprite() = delete;
  Sprite(std::filesystem::path path);
  ~Sprite() = default;

  wchar_t operator[](int index);
  YX<int> size();
  int bufferSize();

private:
  void loadSprite(std::filesystem::path path);
  void storeSize();

private:
  YX<int> m_size{};
  std::wstring m_buffer{};
};
