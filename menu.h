#ifndef MENU_HPP // This is used to avoid including a header more than once.
#define MENU_HPP

#include "dish.h"

class Menu
{
public:
  std::vector<Dish*> dishes;
};

#endif