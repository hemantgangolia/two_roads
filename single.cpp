#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <cstdlib>
#include <algorithm>

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>


using namespace std;

#define DEBUG 0

class Ingredient
{
public:
  string name;
  int expiry;
  Ingredient () {}
  Ingredient(string n, int ex) {
    name = n;
    expiry = ex;
  }
};

class Dish
{
public:
  string name;
  vector<Ingredient*> ingredients;
  vector<int> quantity;
  Dish () {}
  Dish(string n) {
    name = n;
  }
};

class Menu
{
public:
  vector<Dish*> dishes;
};

class Inventory
{
public:
  string date, ingredient_name;
  int quantity, bought_since;
  Inventory () {}
  Inventory(string d, string i_name, int qty, int b_s) {
    date = d;
    ingredient_name = i_name;
    quantity = qty;
    bought_since = b_s;
  }
  void addToQuantity(int qty) {
    quantity += qty;
  }
};

class Restaurant {
public:
  string name;
  Menu res_menu;
  map<string, vector<Inventory*> > ing_inv_map; // map of ingredient_name -> inventory for that ingredients
  std::map<string, int> ing_net_quantity_map;   // map of ingredient_name -> net quantity available
  Restaurant () {}
  Restaurant (string n, Menu menu) {
    name = n;
    res_menu = menu;
  }

  void calculateIngredientNetQuantity () {
    for (std::map<string, std::vector<Inventory*> >::iterator it=ing_inv_map.begin(); it!=ing_inv_map.end(); ++it) {
      std::vector<Inventory*> vec_inv = it->second;
      ing_net_quantity_map[it->first] = 0;
      for (int i = 0; i < vec_inv.size(); i++) {
        ing_net_quantity_map[it->first] += vec_inv[i]->quantity;
      }
    }
  }

  void removeIngredientAfterOrder (string i_name, int qty) {
    ing_net_quantity_map[i_name] -= qty;
    int index = 0;
    vector<Inventory*> vec_inv = ing_inv_map[i_name];
    while (qty > 0) {
      if (vec_inv[index] == NULL) {
        index++;
        continue;
      }

      qty -= vec_inv[index]->quantity;

      if (qty > 0) {
        vec_inv[index] = NULL;
      }
    }
  }

};

class Order {
public:
  string timestamp, resto_id, dish;
  int quantity;
  Order () {}
  Order(string ts, string rs, string d, int qty) {
    timestamp = ts;
    resto_id = rs;
    dish = d;
    quantity = qty;
  }
};


string trim_comma(string str) {
  return str[str.size()-1] == ',' ? str.substr(0, str.size()-1) : str;
}
// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string trim(std::string s) {
        return ltrim(rtrim(s));
}

int readIngredients(map<string, Ingredient*> &ingredient_map) {
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

  else cout << "Unable to open file"; 

  if (DEBUG) {
    for (std::map<string, Ingredient*>::iterator it=ingredient_map.begin(); it!=ingredient_map.end(); ++it)
      std::cout << it->first << " => " << it->second->expiry << '\n';
  }
  return 0;
}

int readDishes(map<string, Dish*> &dish_map, map<string, Ingredient*> &ingredient_map) {
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

  else cout << "Unable to open file"; 

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

string GetBaseFilename(string fName)
{
    int pos = fName.find_last_of(".");
    if(pos == string::npos)  //No extension.
        return fName;
    if(pos == 0)    //. is at the front. Not an extension.
        return fName;
    return fName.substr(0, pos);
}

int readRestaurantMenus(map<string, Dish*> &dish_map, std::vector<Restaurant*> &restos, std::map<string, Restaurant*> &resto_map) {
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

  else cout << "Unable to open file"; 

  return 0;
}

int readInventoryState (std::map<string, Ingredient*> ingredient_map, std::map<string, Restaurant*> &resto_map) {

  cout << "Reading Inventory State ..." << endl;
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
      pch = strtok (NULL, ",");
      resto_id = trim(string(pch));
      pch = strtok (NULL, ",");
      i_name = trim(string(pch));
      pch = strtok (NULL, ",");
      qty = atoi(trim(string(pch)).c_str());
      pch = strtok (NULL, ",");
      bought_since = atoi(trim(string(pch)).c_str());
      
      // add a day to bought_since to reach expiry
      bought_since += 1;
      if (bought_since >= ingredient_map[i_name]->expiry) {
        // dont add the ingredient for today's state if ingredient expired
        continue;
      }
      
      Restaurant * resto = resto_map.find(resto_id)->second;
      Inventory * inv = new Inventory(date, i_name, qty, bought_since);
      resto->ing_inv_map[i_name].push_back(inv);
    }
    myfile.close();
  }

  else cout << "Unable to open file"; 

  // calculates net quantity available for a ingredient
  for (std::map<string, Restaurant*>::iterator resto_it = resto_map.begin(); resto_it!= resto_map.end(); ++resto_it) {
    resto_it->second->calculateIngredientNetQuantity();
  }

  return 0;
}



int serveOrders(std::map<string, Dish*> &dish_map, std::vector<Order*> &orders, std::map<string, Restaurant*> &resto_map, int start_index, int end_index) {

  for (int i = start_index; i < end_index; i++) {
    Restaurant * resto = resto_map[orders[i]->resto_id];

    Dish * dish = dish_map[orders[i]->dish];

    int flag = 1; // we might
    // check whether we can serve the dish or not
    for (int j = 0; j < dish->ingredients.size(); j++) {
      Ingredient * ing = dish->ingredients[j];
      if (resto->ing_net_quantity_map.find(ing->name) == resto->ing_net_quantity_map.end()) {
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
      // we cannot serve the dish
      continue;
    }

    // decrease the inventory
    for (int j = 0; j < dish->ingredients.size(); j++) {
      Ingredient * ing = dish->ingredients[j];
      int ing_qty = dish->quantity[j];
      resto->removeIngredientAfterOrder(ing->name, ing_qty);
    }

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
    }
    resto->calculateIngredientNetQuantity();
  }

}


int getInventorySuggestion (string date, map<string, Ingredient*> &ingredient_map, std::map<string, Restaurant*> &resto_map, std::vector<Order*> &orders, std::map<string, vector<Inventory*> > &resto_inventory_suggestions) {
  // Simple strategy to add 10 of each ingredient and not consider previous orders
  for (std::map<string, Ingredient*>::iterator ingredient_it=ingredient_map.begin(); ingredient_it!=ingredient_map.end(); ++ingredient_it){
    Ingredient *ing = ingredient_it->second;
    Inventory * inv = new Inventory(date, ing->name, 10, 0);

    for (std::map<string, Restaurant*>::iterator resto_it = resto_map.begin(); resto_it!= resto_map.end(); ++resto_it) {
      Restaurant * resto = resto_it->second;
      resto_inventory_suggestions[resto->name].push_back(inv);
    }
  }
}

void outputInventorySuggestion (string date, std::map<string, vector<Inventory*> > &resto_inventory_suggestions) {
  ofstream myfile;
  myfile.open ("inventory_suggestions_pre_opening.txt");
  cout << "Writing inventory suggestions\n";
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
  myfile.open ("state_inventory_pre_opening.txt");
  cout << "Writing state inventory\n";
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
  myfile.open ("state_inventory_EOD.txt");
  cout << "Writing state inventory EOD\n";
  for (std::map<string, Restaurant*>::iterator resto_it = resto_map.begin(); resto_it!= resto_map.end(); ++resto_it) {
      Restaurant * resto = resto_it->second;
      for (std::map<string, vector<Inventory*> >::iterator it = resto->ing_inv_map.begin(); it != resto->ing_inv_map.end(); ++it) {
        vector<Inventory*> vec_inv = it->second;
        for (int i = 0; i < vec_inv.size(); i++) {
          if (vec_inv[i] != NULL) {
            myfile << date << "," << resto->name << "," << vec_inv[i]->ingredient_name << "," << vec_inv[i]->quantity << "," << vec_inv[i]->bought_since << endl;
          }
        }
      }
  } 
  myfile.close();
}

int main() {

  map<string, Ingredient*> ingredient_map;
  map<string, Dish*> dish_map;
  map<string, Restaurant*> resto_map;
  std::map<string, vector<Inventory*> > resto_inventory_suggestions;
  vector<Restaurant*> restos;
  std::vector<Order*> orders;

  readIngredients ( ingredient_map );
  readDishes ( dish_map, ingredient_map );
  readRestaurantMenus ( dish_map, restos, resto_map );
  readInventoryState ( ingredient_map, resto_map );

  readOrders ( orders );
  // TODO - sort orders on basis of timestamp

  outputInventoryState ("date", resto_map);

  getInventorySuggestion ( "date", ingredient_map, resto_map, orders, resto_inventory_suggestions );
  outputInventorySuggestion ("date", resto_inventory_suggestions);

  serveOrders ( dish_map, orders, resto_map, 0, orders.size());

  outputInventoryStateEOD ("date", resto_map);
  return 0;
}