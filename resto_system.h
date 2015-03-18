#ifndef RESTO_SYS_HPP // This is used to avoid including a header more than once.
#define RESTO_SYS_HPP

#include "ingredient.h"
#include "dish.h"
#include "restaurant.h"
#include <string>
#include <map>

class RestoSystem
{
public:
  std::map<std::string, Ingredient*> ingredient_map;
  std::map<std::string, Dish*> dish_map;
  std::map<std::string, Restaurant*> resto_map;
  std::vector<Restaurant*> restos;
  

  int readIngredients ( std::map<std::string, Ingredient*> &);
  int readDishes ( std::map<std::string, Dish*> &, std::map<std::string, Ingredient*> & );
  int readRestaurantMenus ( std::map<std::string, Dish*> &, std::vector<Restaurant*> &, std::map<std::string, Restaurant*>  &);
  void runSys (std::map<std::string, Restaurant*> &, std::map<std::string, Dish*> &, std::map<std::string, Ingredient*> &);

  void run ();
};


#endif