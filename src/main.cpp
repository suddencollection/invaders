#include "program.hpp"

#include <iostream>

std::filesystem::path get_sprite_path()
{
  char const* char_path = std::getenv("INVADERS_SPRITE_PATH");
  if(char_path == nullptr) {
    throw std::runtime_error("Sprite path undefined!");
  }

  auto path = std::filesystem::path{char_path};

  if(!std::filesystem::exists(path)) {
    throw std::runtime_error("Sprite path does not exist!");
  }

  return path;
}

int main()
{
  // sprites
  try {
    auto path = get_sprite_path();
    auto program = Program{path};
    // program.run();
  }
  catch(std::exception& e) {
    std::cerr << e.what() << '.' << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
