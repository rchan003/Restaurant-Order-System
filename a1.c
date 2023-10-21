#include "a1.h"

/**
	Add your functions to this file.
	Make sure to review a1.h.
	Do NOT include a main() function in this file
	when you submit.
*/

Restaurant* initialize_restaurant(char* name){ 
	// allocating memory 
	struct Restaurant* new_node = (struct Restaurant*)malloc(sizeof(struct Restaurant));
	new_node->name = malloc(sizeof(char) * (strlen(name) + 1));  
	//new_node->menu = (struct Menu*)malloc(sizeof(struct Menu));
	new_node->pending_orders = (struct Queue*)malloc(sizeof(struct Queue)); 
	
	// setting values (except menu) 
	strcpy(new_node->name, name); 
	new_node->num_completed_orders = 0;
	new_node->num_pending_orders = 0; 
	new_node->pending_orders->head = NULL;
	new_node->pending_orders->tail = NULL;
	
	// initalizing menu
	new_node->menu = load_menu(MENU_FNAME);

	return new_node; 
}


Menu* load_menu(char* fname){ 
	// opening & reading the file 
	FILE *file_1 = fopen(fname, "r");
	char line[256]; // set max line length to arbitarily large number 
	int number_items = 0; 
	
	// finding number of lines
		// to create arrays of correct size
	while (fgets(line, sizeof(line), file_1)){
		if (line[0] >= 'A' && line[0] <= 'Z' || line[0] >= 'a' && line[0] <= 'z'){ // only reads line if it starts with alphanumeric 
			number_items++;
		}
	}
	
	fclose(file_1);
	
	FILE *file = fopen(fname, "r");
	
	// allocating memory
	struct Menu* menu_node = (struct Menu*)malloc(sizeof(struct Menu));
	menu_node->num_items = number_items; 
	
	// creating double array of proper length 
	double item_costs[number_items];
	
	// setting remaining struct values 
	menu_node->item_codes = (char **)malloc(sizeof(char *) * number_items);
	menu_node->item_names = (char **)malloc(sizeof(char *) * number_items);
	menu_node->item_cost_per_unit = (double *)malloc(sizeof(double) * number_items);
 
	// reading each line & splitting at MENU_DELIM
	int line_count = 0; 
	while (fgets(line, sizeof(line), file)){
		if(line[0] >= 'A' && line[0] <= 'Z' || line[0] >= 'a' && line[0] <= 'z'){
			// looping through the items in the line 
			int token_count = 0; 
			char *token = strtok(line, MENU_DELIM);
			while (token != NULL){
				token_count++;
				if (token_count == 1){ // item code 
					char* code = (char*)malloc(sizeof(char) * strlen(token) + 1);
					strcpy(code, token);
					menu_node->item_codes[line_count] = code; 
				}
				if (token_count == 2){ // item name 
					char* name = malloc(sizeof(char) * strlen(token) + 1);
					strcpy(name, token);
					menu_node->item_names[line_count] = name;
				}
				if (token_count == 3){ // item cost 
					// remove $ sign
					char no_sign[strlen(token)];
					for (int i = 0; i < strlen(token); i++){
						strcpy(&no_sign[i], &(token[i + 1]));
					}
					// convert str to double 
					double val = atof(no_sign);
					menu_node->item_cost_per_unit[line_count] = val;
					
				}
				token = strtok(NULL, MENU_DELIM);
			}
			line_count++; 
		}
	}
	fclose(file);
	return menu_node; 
}


Order* build_order(char* items, char* quantities){ 
	// initializing variables
	int number_items;
	struct Order* order_node = (struct Order*)malloc(sizeof(struct Order)); 
	int length = strlen(items);
	int increment = ITEM_CODE_LENGTH - 1;
	
	// finding num items 
	number_items = length/increment; 
	order_node->num_items = number_items; 
	
	// item_codes & quantities 
	order_node->item_codes = (char **)malloc(sizeof(char*) * number_items);
	order_node->item_quantities = (int *)malloc(sizeof(int) * number_items);
	
	// filling in item_codes 
	for(int i = 0; i < number_items; i++){
		char s[ITEM_CODE_LENGTH]; // string of item code length 
		strncpy(s, &items[i*increment], increment);
		s[ITEM_CODE_LENGTH-1] = '\0';
		char* k = malloc(sizeof(char)*ITEM_CODE_LENGTH);
		strcpy(k, s);
		order_node->item_codes[i] = k;
	}
	
	// ITEM QUANTITIES 
	// mutable copy of quantity string 
	char mut_quan[strlen(quantities)];
	strncpy(mut_quan, &quantities[0], strlen(quantities)); 
	mut_quan[strlen(quantities)] = '\0';
	// splitting at delim 
	int count = 0;
	char * token = strtok(mut_quan, MENU_DELIM);
	while(token != NULL){
		int x = atoi(token);
		order_node->item_quantities[count] = x;
		token = strtok(NULL, MENU_DELIM);
		count++;
	}
	
	// returning filled order 
	return order_node; 
}


void enqueue_order(Order* order, Restaurant* restaurant){ 
	struct QueueNode* new_node = (struct QueueNode*)malloc(sizeof(struct QueueNode));
	int num_pending = restaurant->num_pending_orders; 
	
	// coping contents of parameter order into malloc'ed new_order 
	new_node->order = order; 
	new_node->next = NULL; 
	
	// if there are no pending orders 
	if(restaurant->pending_orders->tail == NULL){
		restaurant->pending_orders->head = new_node; 
	} else{
		restaurant->pending_orders->tail->next = new_node;
	}
	
	restaurant->pending_orders->tail = new_node; 
	
	restaurant->num_pending_orders = num_pending + 1; 
}


Order* dequeue_order(Restaurant* restaurant){ 
	struct QueueNode* old_head = restaurant->pending_orders->head;

	Order* return_order = old_head->order; 
	
	// updating number of pending & completed orders 
	int number_pending = restaurant->num_pending_orders - 1; 
	int number_completed = restaurant->num_completed_orders + 1; 
	restaurant->num_completed_orders = number_completed;
	restaurant->num_pending_orders = number_pending; 
	
	// moving head to next order in list 
	restaurant->pending_orders->head = restaurant->pending_orders->head->next; 
	
	// if there was only one order in pending orders 
	if(number_pending == 0){
		restaurant->pending_orders->tail = NULL; 
	}
	
	free(old_head);
	return return_order; 
}

double get_item_cost(char* item_code, Menu* menu){ 
	// creating mutable string of item_code
	char mut_item_code[ITEM_CODE_LENGTH];
	strcpy(mut_item_code, item_code);
	
	// loop 
	int i = 0;
	int number_items = menu->num_items;
	
	while(i < number_items){
		// creating mutable string filled with code[i] from menu 
		char* test_code = menu->item_codes[i]; 
		char mut_test_code[ITEM_CODE_LENGTH];
		strcpy(mut_test_code, test_code);
		
		// checking if test_code == item_code 
		int match_count = 0; 
		for(int z = 0; z < ITEM_CODE_LENGTH; z++){
			if(mut_item_code[z] != mut_test_code[z]){
				z = ITEM_CODE_LENGTH + 10; // stopping for loop at first not equal char 
			}
			if(mut_item_code[z] == mut_test_code[z]){
				match_count++;
			}
		}
		
		// checking for codes matching 
		if(match_count == ITEM_CODE_LENGTH){
			return menu->item_cost_per_unit[i]; // this indexing could be wrong 
		}
		
		i++; 
	}
}

double get_order_subtotal(Order* order, Menu* menu){
	double order_subtotal = 0; 
	int number_items = order->num_items; 
	
	// loop 
	for(int i = 0; i < number_items; i++){
		order_subtotal = order_subtotal + get_item_cost(order->item_codes[i], menu) * (order->item_quantities[i]); 
	}
	return order_subtotal; 
}

double get_order_total(Order* order, Menu* menu){
	double subtotal = get_order_subtotal(order, menu); 
	double order_total = subtotal * (100 + TAX_RATE) / 100; 
	return order_total; 
}

int get_num_completed_orders(Restaurant* restaurant){
	return restaurant->num_completed_orders; 
}

int get_num_pending_orders(Restaurant* restaurant){
	return restaurant->num_pending_orders; 
}


void clear_order(Order** order){
	struct Order* deref_order = *order; 
	int number_items = deref_order->num_items; 
	// freeing item codes 
	for(int i = 0; i < number_items; i++){
		free(deref_order->item_codes[i]);
	}
	free(deref_order->item_codes);
	free(deref_order->item_quantities);
	free(deref_order); 
	*order = NULL; 
}

void clear_menu(Menu** menu){
	struct Menu* deref_menu = *menu; 
	int number_items = deref_menu->num_items; 
	for(int i = 0; i < number_items; i++){
		// freeing codes 
		free(deref_menu->item_codes[i]);
		free(deref_menu->item_names[i]);
	}
	free(deref_menu->item_codes);
	free(deref_menu->item_names);
	free(deref_menu->item_cost_per_unit); 
	free(deref_menu);
	*menu = NULL; 
}

void close_restaurant(Restaurant** restaurant){
	struct Restaurant* deref_rest = *restaurant; 

	while(deref_rest->pending_orders->head != NULL){
		struct QueueNode* delete_node = deref_rest->pending_orders->head; 
		struct Order* temp_order = delete_node->order; 
		
		deref_rest->pending_orders->head = deref_rest->pending_orders->head->next; 
		
		clear_order(&temp_order); 
		free(delete_node); 
	}
	free(deref_rest->pending_orders); 
	
	clear_menu(&(deref_rest->menu)); 
	free(deref_rest->name);
	free(deref_rest);
	*restaurant = NULL; 
}


void print_menu(Menu* menu){
	fprintf(stdout, "--- Menu ---\n");
	for (int i = 0; i < menu->num_items; i++){
		fprintf(stdout, "(%s) %s: %.2f\n", 
			menu->item_codes[i], 
			menu->item_names[i], 
			menu->item_cost_per_unit[i]	
		);
	}
}


void print_order(Order* order){
	for (int i = 0; i < order->num_items; i++){
		fprintf(
			stdout, 
			"%d x (%s)\n", 
			order->item_quantities[i], 
			order->item_codes[i]
		);
	}
}


void print_receipt(Order* order, Menu* menu){
	for (int i = 0; i < order->num_items; i++){
		double item_cost = get_item_cost(order->item_codes[i], menu);
		fprintf(
			stdout, 
			"%d x (%s)\n @$%.2f ea \t %.2f\n", 
			order->item_quantities[i],
			order->item_codes[i], 
			item_cost,
			item_cost * order->item_quantities[i]
		);
	}
	double order_subtotal = get_order_subtotal(order, menu);
	double order_total = get_order_total(order, menu);
	
	fprintf(stdout, "Subtotal: \t %.2f\n", order_subtotal);
	fprintf(stdout, "               -------\n");
	fprintf(stdout, "Tax %d%%: \t$%.2f\n", TAX_RATE, order_total);
	fprintf(stdout, "              ========\n");
}