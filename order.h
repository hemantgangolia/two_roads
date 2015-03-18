#ifndef ORDER_HPP // This is used to avoid including a header more than once.
#define ORDER_HPP

#include <string>

class Order {
public:
  std::string date, resto_id, dish;
  int quantity;
  Order ();
  Order ( std::string , std::string , std::string , int );
};

#endif