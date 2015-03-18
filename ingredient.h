#ifndef INGREDIENT_HPP // This is used to avoid including a header more than once.
#define INGREDIENT_HPP

#include <string>

class Ingredient
{
public:
  std::string name;
  int expiry;
  Ingredient ();
  Ingredient(std::string, int);
};

#endif