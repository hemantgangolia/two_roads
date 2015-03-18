#ifndef RESTAURANT_HPP // This is used to avoid including a header more than once.
#define RESTAURANT_HPP

#include "menu.h"
#include "inventory.h"
#include <string>
#include <map>


class Restaurant {
public:
  std::string name;
  Menu res_menu;
  std::map<std::string, std::vector<Inventory*> > ing_inv_map; // map of ingredient_name -> inventory for that ingredients
  std::map<std::string, int> ing_net_quantity_map;   // map of ingredient_name -> net quantity available

  Restaurant (std::string, Menu);
  Restaurant ();
  void calculateIngredientNetQuantity ();
  void removeIngredientAfterOrder (std::string, int);
};

#endif