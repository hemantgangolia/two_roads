#include "inventory.h"

Inventory::Inventory () {}

Inventory::Inventory (std::string d, std::string i_name, int qty, int b_s) {
	date = d;
	ingredient_name = i_name;
	quantity = qty;
	bought_since = b_s;
}

void Inventory::addToQuantity(int qty) {
	quantity += qty;
}