#ifndef DISH_HPP // This is used to avoid including a header more than once.
#define DISH_HPP

#include "ingredient.h"
#include <string>
#include <vector>

class Dish
{
public:
  std::string name;
  std::vector<Ingredient*> ingredients;
  std::vector<int> quantity;
  Dish ();
  Dish ( std::string );
};

#endif