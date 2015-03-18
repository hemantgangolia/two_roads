#include "ingredient.h"

Ingredient::Ingredient () {}

Ingredient::Ingredient (std::string n, int ex) {
  name = n;
  expiry = ex;
}