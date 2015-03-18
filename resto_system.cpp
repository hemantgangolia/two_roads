#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <cstdlib>
#include <ctime>

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ingredient.h"
#include "inventory.h"
#include "restaurant.h"
#include "dish.h"
#include "menu.h"
#include "order.h"
#include "helper.h"
#include "resto_system.h"

using namespace std;

#define DEBUG 0

void RestoSystem::run() {
  readIngredients ( ingredient_map );
  readDishes ( dish_map, ingredient_map );
  readRestaurantMenus ( dish_map, restos, resto_map );
  runSys (resto_map, dish_map, ingredient_map);
}

int RestoSystem::readIngredients(map<string, Ingredient*> &ingredient_map) {
  cout << "Reading Ingredients ..." << endl;
  string line;
  ifstream myfile ("ingredients.txt");
  if (myfile.is_open()) {
    while(getline(myfile,line))
    {
        stringstream lineStream(line);
        string i_name;
        int expiry;
        lineStream >> i_name >> expiry;
        i_name = trim_comma(i_name);
        if(DEBUG) cout << i_name << " " << expiry << endl;
        Ingredient *ingredient = new Ingredient(i_name, expiry);
        ingredient_map[i_name] = ingredient;
    }
    myfile.close();
  }

  else cout << "Unable to open file" << endl; 

  if (DEBUG) {
    for (std::map<string, Ingredient*>::iterator it=ingredient_map.begin(); it!=ingredient_map.end(); ++it)
      std::cout << it->first << " => " << it->second->expiry << '\n';
  }
  return 0;
}

int RestoSystem::readDishes(map<string, Dish*> &dish_map, map<string, Ingredient*> &ingredient_map) {
  cout << "Reading Dishes ..." << endl;
  string line;
  ifstream myfile ("dishes.txt");
  if (myfile.is_open()) {
    while(getline(myfile,line))
    {
        stringstream lineStream(line);
        string d_name;
        int qty;
        string i_name;
        lineStream >> d_name;
        d_name = trim_comma(d_name);
        
        Dish *dish = new Dish(d_name);
        dish_map[d_name] = dish;

        while (lineStream >> qty >> i_name) {
          i_name = trim_comma(i_name);
          dish->quantity.push_back(qty);
          if (ingredient_map.find(i_name) == ingredient_map.end()) {
            cerr << "Cannot find the ingredient " << i_name << ". Sorry, cannot make " << d_name << endl;
            return 1;
          }
          dish->ingredients.push_back(ingredient_map.find(i_name)->second);
        }
        d_name = d_name.substr(0, d_name.size()-1);
    }
    myfile.close();
  }

  else cout << "Unable to open file" << endl; 

  if (DEBUG) {
    for (std::map<string, Dish*>::iterator it=dish_map.begin(); it!=dish_map.end(); ++it) {
      std::cout << it->first << " => " << it->second->quantity.size() << "\n";
      for (int i = 0; i < it->second->quantity.size(); i++) {
        cout << it->second->quantity[i] << " \n" << it->second->ingredients[i]->name << endl;
      }
    }
  }
  return 0;
}

int RestoSystem::readRestaurantMenus(map<string, Dish*> &dish_map, std::vector<Restaurant*> &restos, std::map<string, Restaurant*> &resto_map) {
  cout << "Reading Restaurant Menus ..." << endl; 
  ifstream fin;
  string dir = "menus", filepath;
  DIR *dp;
  struct dirent *dirp;
  struct stat filestat;

  dp = opendir( dir.c_str() );
  if (dp == NULL){
    cout << "Error opening " << dir << endl;
    return -1;
  }

  while ((dirp = readdir( dp )))
    {
    filepath = dir + "/" + dirp->d_name;

    // If the file is a directory (or is in some way invalid) we'll skip it 
    if (stat( filepath.c_str(), &filestat )) continue;
    if (S_ISDIR( filestat.st_mode ))         continue;

    string resto_name = GetBaseFilename(dirp->d_name);
    
    // Endeavor to read a single number from the file and display it
    fin.open( filepath.c_str() );
    string line;
    if (fin.is_open()) {
      while (getline(fin, line)) {
        stringstream lineStream(line);
        string dish_name;

        Menu menu;
        while(lineStream >> dish_name){
          if (DEBUG) cout << trim_comma(dish_name) << endl;
          menu.dishes.push_back(dish_map.find(dish_name)->second);
        }
        Restaurant * resto = new Restaurant(resto_name, menu);
        restos.push_back(resto);
        resto_map[resto_name] = resto;
      }
      fin.close();
    }
  }   
  closedir( dp );
  return 0;
}

int readOrders(std::vector<Order*> &orders) {
  cout << "Reading Orders ..." << endl;
  string line;
  ifstream myfile ("orders.txt");
  if (myfile.is_open()) {
    while(getline(myfile,line))
    {
        stringstream lineStream(line);

        char * pch;
        string ts, resto_name, dish_name;
        int qty;

        pch = strtok ((char*)line.c_str(), ",");
        ts = trim(string(pch));
        ts = ts.substr(0, ts.find(' '));

        pch = strtok (NULL, ",");
        resto_name = trim(string(pch));
        pch = strtok (NULL, ",");
        dish_name = trim(string(pch));
        pch = strtok (NULL, ",");
        qty = atoi(trim(string(pch)).c_str());
        
        Order * o = new Order(ts, resto_name, dish_name, qty);
        orders.push_back(o);
    }
    myfile.close();
  }

  else cout << "Unable to open file" << endl; 

  return 0;
}

int readInventoryState (string today, std::map<string, Ingredient*> ingredient_map, std::map<string, Restaurant*> &resto_map) {

  for (std::map<string, Restaurant*>::iterator resto_it = resto_map.begin(); resto_it!= resto_map.end(); ++resto_it) {
    resto_it->second->ing_inv_map.clear();
    resto_it->second->ing_net_quantity_map.clear();
  }

  cout << "Reading Inventory State for " << today << endl;

  string line;
  ifstream myfile ("inventory_state_with_expiry.txt");
  if (myfile.is_open()) {
    while(getline(myfile,line))
    {
      char * pch;
      string date, resto_id, i_name;
      int qty, bought_since;

      pch = strtok ((char*)line.c_str(), ",");
      date = trim(string(pch));

      if (date.compare(today) != 0) {
        continue;
      }

      pch = strtok (NULL, ",");
      resto_id = trim(string(pch));
      pch = strtok (NULL, ",");
      i_name = trim(string(pch));
      pch = strtok (NULL, ",");
      qty = atoi(trim(string(pch)).c_str());
      pch = strtok (NULL, ",");
      bought_since = atoi(trim(string(pch)).c_str());

      if (qty == 0) {
        continue;
      }
      
      if (bought_since >= ingredient_map[i_name]->expiry) {
        // dont add the ingredient for today's state if ingredient expired
        cout << "Item expired! " << i_name << " " << resto_id << " " << qty << endl;
        continue;
      }
      
      Restaurant * resto = resto_map.find(resto_id)->second;
      Inventory * inv = new Inventory(date, i_name, qty, bought_since);
      resto->ing_inv_map[i_name].push_back(inv);
    }
    myfile.close();
  }

  else {
    cout << "Unable to open file" << endl; 
    return 1;
  }

  // calculates net quantity available for a ingredient
  for (std::map<string, Restaurant*>::iterator resto_it = resto_map.begin(); resto_it!= resto_map.end(); ++resto_it) {
    resto_it->second->calculateIngredientNetQuantity();
  }

  return 0;
}



int serveOrders(string date, std::map<string, Dish*> &dish_map, std::vector<Order*> &orders, std::map<string, Restaurant*> &resto_map, int end_index) {

  for (int i = end_index; i < orders.size(); i++) {
    if (orders[i]->date.compare(date) != 0) {
      break;
    }

    Restaurant * resto = resto_map[orders[i]->resto_id];

    Dish * dish = dish_map[orders[i]->dish];

    int flag = 1, k;
    for (k = 0; k < orders[i]->quantity; k++) {

      flag = 1; // we might
      // check whether we can serve the dish or not
      for (int j = 0; j < dish->ingredients.size(); j++) {
        Ingredient * ing = dish->ingredients[j];
        if (resto->ing_net_quantity_map.find(ing->name) == resto->ing_net_quantity_map.end()) {
          // we don't have the ingredient, thus cannot serve the dish
          flag = 0;
          break;
        }
        

        int ing_qty = dish->quantity[j];
        if (resto->ing_net_quantity_map[ing->name] - ing_qty < 0) {
          flag = 0; // we cannot
          break;
        } 
      }

      if (flag == 0) {
        // we cannot serve the dish, go to the next dish
        continue;
      }

      // decrease the inventory
      for (int j = 0; j < dish->ingredients.size(); j++) {
        Ingredient * ing = dish->ingredients[j];
        int ing_qty = dish->quantity[j];
        resto->removeIngredientAfterOrder(ing->name, ing_qty);
      }

      if (flag == 0) {
        // we cannot serve any more dishes 
        break;
      }
    }

    cout << "New Order => " << date << " " << resto->name << " " << dish->name << " " << orders[i]->quantity << endl;
    cout << "----Served Order => " << date << " " << resto->name << " " << dish->name << " " << k << endl;
  }

  return 0;
}

int reStockInventory (std::map<string, Restaurant*> &resto_map, std::map<string, vector<Inventory*> > &resto_inventory_suggestions) {

  for (std::map<string, std::vector<Inventory*> >::iterator it = resto_inventory_suggestions.begin(); it != resto_inventory_suggestions.end(); ++it ) {
    string resto_name = it->first;
    Restaurant * resto = resto_map[resto_name];
    vector<Inventory*> vec_inv = it->second;

    for (int i = 0; i < vec_inv.size(); i++) {
      string ing_name = vec_inv[i]->ingredient_name;
      resto->ing_inv_map[ing_name].push_back(vec_inv[i]);
      cout << "Inventory Restocked! " << resto_name << " " << ing_name << " " << vec_inv[i]->quantity << " " << vec_inv[i]->date << endl;
    }
    resto->calculateIngredientNetQuantity();
  }

}

int getInventorySuggestion (string date, map<string, Ingredient*> &ingredient_map, std::map<string, Restaurant*> &resto_map, std::vector<Order*> &orders, std::map<string, vector<Inventory*> > &resto_inventory_suggestions) {
  // Simple strategy to add 10 of each ingredient and not consider previous orders
  for (std::map<string, Ingredient*>::iterator ingredient_it=ingredient_map.begin(); ingredient_it!=ingredient_map.end(); ++ingredient_it){
    Ingredient *ing = ingredient_it->second;

    for (std::map<string, Restaurant*>::iterator resto_it = resto_map.begin(); resto_it!= resto_map.end(); ++resto_it) {
      Inventory * inv = new Inventory(date, ing->name, 10, 0);
      Restaurant * resto = resto_it->second;
      resto_inventory_suggestions[resto->name].push_back(inv);
    }
  }
}

void outputInventorySuggestion (std::map<string, vector<Inventory*> > &resto_inventory_suggestions) {
  ofstream myfile;
  myfile.open ("inventory_orders.txt", ios::app);
  cout << "Writing inventory suggestions \n";
  for (std::map<string, vector<Inventory*> >::iterator it = resto_inventory_suggestions.begin(); it!= resto_inventory_suggestions.end(); ++it) {
    std::vector<Inventory*> vec_inv = it->second;

    for (int i = 0; i < vec_inv.size(); i++) {
      myfile << vec_inv[i]->date << "," << it->first << "," << vec_inv[i]->ingredient_name << "," << vec_inv[i]->quantity <<"\n";
    }
  }
  myfile.close();
}

void outputInventoryState (string date, std::map<string, Restaurant*> &resto_map) {
  ofstream myfile;
  myfile.open ("inventory_state__pre_opening.txt", ios::app);
  cout << "Writing state inventory for " << date << endl;
  for (std::map<string, Restaurant*>::iterator resto_it = resto_map.begin(); resto_it!= resto_map.end(); ++resto_it) {
      Restaurant * resto = resto_it->second;
      for (std::map<string, int>::iterator it = resto->ing_net_quantity_map.begin(); it != resto->ing_net_quantity_map.end(); ++it) {
        myfile << date << "," << resto->name << "," << it->first << "," << it->second << endl;
      }
  } 
  myfile.close();
}


void outputInventoryStateEOD (string date, std::map<string, Restaurant*> &resto_map) {
  ofstream myfile;
  myfile.open ("inventory_state_with_expiry.txt");
  cout << "Writing state inventory EOD for " << date << endl;
  for (std::map<string, Restaurant*>::iterator resto_it = resto_map.begin(); resto_it!= resto_map.end(); ++resto_it) {
      Restaurant * resto = resto_it->second;
      for (std::map<string, vector<Inventory*> >::iterator it = resto->ing_inv_map.begin(); it != resto->ing_inv_map.end(); ++it) {
        vector<Inventory*> vec_inv = it->second;
        for (int i = 0; i < vec_inv.size(); i++) {
          if (vec_inv[i] != NULL) {
            // add +1 to bought_since at EOD
            myfile << date << "," << resto->name << "," << vec_inv[i]->ingredient_name << "," << vec_inv[i]->quantity << "," << vec_inv[i]->bought_since+1 << endl;
          }
        }
      }
  } 
  myfile.close();
}


void RestoSystem::runSys(std::map<string, Restaurant*> &resto_map, std::map<string, Dish*> &dish_map, std::map<string, Ingredient*> &ingredient_map) {
  cout << "Running System \n";
  std::vector<Order*> orders;
  std::map<string, vector<Inventory*> > resto_inventory_suggestions;
  readOrders ( orders );

  int start_index = 0, end_index;
  while (start_index < orders.size()) {

    // -------------- reading previous day orders ---------------------
    string yesterday = orders[start_index]->date;
    end_index = start_index+1;
    while (end_index < orders.size() && orders[end_index]->date == yesterday) {
      end_index++;
    }
    // ----------------------------------------------------------------

    string today = get_tomorrow(yesterday);

    cout << "\n\n\nToday - " << today << endl;

    readInventoryState ( yesterday, ingredient_map, resto_map );

    outputInventoryState (today, resto_map);

    getInventorySuggestion ( today, ingredient_map, resto_map, orders, resto_inventory_suggestions );
    outputInventorySuggestion (resto_inventory_suggestions);

    reStockInventory (resto_map, resto_inventory_suggestions);

    serveOrders(today, dish_map, orders, resto_map, end_index);

    outputInventoryStateEOD (today, resto_map);

    resto_inventory_suggestions.clear();

    start_index = end_index;
  }
}