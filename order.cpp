#include "order.h"

Order::Order () {}
Order::Order(std::string ts, std::string rs, std::string d, int qty) {
  date = ts;
  resto_id = rs;
  dish = d;
  quantity = qty;
}