#ifndef INVENTORY_HPP // This is used to avoid including a header more than once.
#define INVENTORY_HPP

#include <string>

class Inventory
{
public:
  std::string date, ingredient_name;
  int quantity, bought_since;
  Inventory ();
  Inventory( std::string , std::string , int , int );
  void addToQuantity ( int );
};

#endif