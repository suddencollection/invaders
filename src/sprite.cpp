#include "sprite.hpp"

#include <cassert>
#include <fstream>

wchar_t Sprite::operator[](int index)
{
  return m_buffer[index];
}

YX<int> Sprite::size()
{
  assert(m_size.y != -1 && m_size.x != -1);
  return m_size;
}

int Sprite::bufferSize()
{
  // return m_size.y * (m_size.x + 1);
  return m_buffer.size();
}

void Sprite::storeSize()
{
  m_size = {0, 0};

  // count width
  for(size_t i{}; i < m_buffer.length(); ++i) {
    if(m_buffer[i] == '\n') {
      m_size.x = i;
      break;
    }
  }

  // count height
  for(size_t i{}; i < m_buffer.length(); ++i) {
    if(m_buffer[i] == '\n') {
      ++m_size.y;
    }
  }
}

Sprite::Sprite(std::filesystem::path path)
{
  loadSprite(path);
}

void Sprite::loadSprite(std::filesystem::path path)
{
  std::ifstream file{path, std::ios::ate};

  if(!file.is_open())
    throw std::runtime_error("Failed to load " + path.string() + " sprite");

  std::streamoff fileLength{file.tellg()};
  file.seekg(file.beg);

  size_t lineNum{};
  size_t maxWidth{};
  size_t width{1};
  char ch{};

  // count number of lines
  for(std::streamoff i{0}; i < fileLength; ++i) {
    file.read(&ch, 1);
    if(ch == '\n') {
      if(maxWidth < width)
        maxWidth = width;

      width = 0;
      ++lineNum;
    }

    ++width;
  }

  file.seekg(file.beg);

  // read sprites
  m_buffer.reserve(lineNum * maxWidth);
  for(size_t line{}; line < lineNum; ++line) {
    for(size_t column{}; column < maxWidth; ++column) {
      // file.read(&buffer[(line * spriteWidth) + column], 1);
      file.read(&ch, 1);

      // if the sprite is too short, fill the remainder with blank spaces
      if(ch == '\n') {
        for(size_t i{column}; i < maxWidth - 1; ++i) {
          m_buffer.push_back(' ');
        }

        m_buffer.push_back('\n'); // separator
        break;
      }

      m_buffer.push_back(ch);
    }
  }

  // init m_size
  storeSize();
}
