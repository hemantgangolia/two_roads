#include "restaurant.h"
#include "menu.h"
#include "inventory.h"

Restaurant::Restaurant () {}

Restaurant::Restaurant (std::string n, Menu menu) {
  name = n;
  res_menu = menu;
}

void Restaurant::calculateIngredientNetQuantity () {
  for (std::map<std::string, std::vector<Inventory*> >::iterator it=ing_inv_map.begin(); it!=ing_inv_map.end(); ++it) {
    std::vector<Inventory*> vec_inv = it->second;
    ing_net_quantity_map[it->first] = 0;
    for (int i = 0; i < vec_inv.size(); i++) {
      ing_net_quantity_map[it->first] += vec_inv[i]->quantity;
    }
  }
}

void Restaurant::removeIngredientAfterOrder (std::string i_name, int qty) {
  ing_net_quantity_map[i_name] -= qty;
  int index = 0;
  std::vector<Inventory*> vec_inv = ing_inv_map[i_name];
  while (qty > 0) {
    if (vec_inv[index] == NULL) {
      index++;
      continue;
    }
    int we_have = vec_inv[index]->quantity;

    if (qty > we_have) {
      vec_inv[index] = NULL;
      qty -= we_have;
    } else {
      vec_inv[index]->quantity -= qty;
      qty = 0;
    }
  }
}