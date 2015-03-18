The code is scalable for multiple restos, menus, ingredients and dishes; and follows OOP principles.

I have applied a simple prediction strategy of just adding 10 of each ingredient.

I maintain a separate state of inventory with extra field for storing the number of days since the ingredient was bought in a file "inventory_state_with_expiry.txt"


Instructions:

(1) g++ main.cpp inventory.cpp restaurant.cpp ingredient.cpp dish.cpp helper.cpp order.cpp resto_system.cpp && ./a.out

Sample Input:

ingredients.txt, "menus" folder containing menu for each restaurant, dishes.txt, orders.txt
(sample input is present in the folder)

Sample output:

(1) Inventory suggestions - inventory_orders.txt 

(2) State of Inventory pre-opening - inventory_state_pre_opening.txt
