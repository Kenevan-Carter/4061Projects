#include "server.h"

void printSyntax(){
    printf("incorrect usage syntax!\n");
    printf("usage: $ ./server server_addr server_port num_workers\n");
}

// ============================================================
// load_inventory: read items.csv into the global inventory[] array
// CSV format:
//     name,stock,price
//     laptop,50,999.99
//     ...
// Skip the header line. Set num_items to the number of items loaded.
// ============================================================
void load_inventory(char *filename)
{
    // Open the file
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // Read the header line
    char line[256];
    // If the header line is not read, set num_items to 0 and return
    if (fgets(line, sizeof line, fp) == NULL) {
        fclose(fp);
        num_items = 0;
        return;
    }

    // Read the data lines
    num_items = 0;
    // Stops when MAX_ITEMS is reached or the file is read to the end
    while (num_items < MAX_ITEMS && fgets(line, sizeof line, fp) != NULL) {
        struct item *it = &inventory[num_items];
        // If the line is read, increment num_items and set the item
        if (sscanf(line, "%63[^,],%d,%f", it->name, &it->stock, &it->price) == 3)
            num_items++;
    }
    // Close the file
    fclose(fp);
}

// ============================================================
// handle_list_items: send the full inventory to the client.
// Protocol:
//     write ITEM_LIST (msg_enum)
//     write num_items (int)
//     write each struct item (num_items of them)
// Remember to lock the inventory_lock mutex while reading inventory.
// ============================================================
void handle_list_items(int client_fd)
{
    // Send the ITEM_LIST message
    msg_enum rsp = ITEM_LIST;
    if (write(client_fd, &rsp, sizeof(msg_enum)) != (ssize_t)sizeof(msg_enum))
        return;

    // Lock the inventory_lock mutex
    pthread_mutex_lock(&inventory_lock);
    int n = num_items;
    // Send the number of items
    if (write(client_fd, &n, sizeof(int)) != (ssize_t)sizeof(int)) {
        pthread_mutex_unlock(&inventory_lock);
        return;
    }
    // Send the items
    for (int i = 0; i < n; i++) {
        if (write(client_fd, &inventory[i], sizeof(struct item)) != (ssize_t)sizeof(struct item)) {
            pthread_mutex_unlock(&inventory_lock);
            return;
        }
    }
    // Unlock the inventory_lock mutex
    pthread_mutex_unlock(&inventory_lock);
}

// ============================================================
// handle_search: read a query string from the client, then return
// every item whose name contains the query as a substring.
// Protocol (recv): char query[MAX_STR]
// Protocol (send): SEARCH_RESULTS (msg_enum), count (int),
//                  then count x struct item
// ============================================================
void handle_search(int client_fd)
{
    // Read the query string
    char query[MAX_STR];
    memset(query, 0, sizeof(query));
    if (read(client_fd, query, sizeof(query)) != (ssize_t)sizeof(query))
        return;

    // Initialize the results array
    struct item results[MAX_RESULTS];
    int count = 0;

    // Lock the inventory_lock mutex
    pthread_mutex_lock(&inventory_lock);
    // Search for items that contain the query string
    for (int i = 0; i < num_items && count < MAX_RESULTS; i++) {
        if (strstr(inventory[i].name, query) != NULL)
            results[count++] = inventory[i];
    }
    // Unlock the inventory_lock mutex
    pthread_mutex_unlock(&inventory_lock);

    // Send the SEARCH_RESULTS message
    msg_enum rsp = SEARCH_RESULTS;
    // Send the number of results if it is less than MAX_RESULTS
    if (write(client_fd, &rsp, sizeof(msg_enum)) != (ssize_t)sizeof(msg_enum))
        return;
    // Send the number of results if it is less than MAX_RESULTS
    if (write(client_fd, &count, sizeof(int)) != (ssize_t)sizeof(int))
        return;
    // Send the results if they are less than MAX_RESULTS
    for (int j = 0; j < count; j++) {
        if (write(client_fd, &results[j], sizeof(struct item)) != (ssize_t)sizeof(struct item))
            return;
    }
}

// ============================================================
// BONUS (optional, +10 pts):
// handle_enc_search: like handle_search, but the query was ciphered on
// the wire using the Caesar shift from utils.h. Call decrypt_str() on
// it before running the normal substring search. Reply format is the
// standard SEARCH_RESULTS message (unciphered).
// ============================================================
void handle_enc_search(int client_fd)
{
    // Read the query string
    char query[MAX_STR];
    memset(query, 0, sizeof(query));
    if (read(client_fd, query, sizeof(query)) != (ssize_t)sizeof(query))
        return;

    // Decrypt the query string
    decrypt_str(query);

    // Initialize the results array
    struct item results[MAX_RESULTS];
    int count = 0;

    // Lock the inventory_lock mutex
    pthread_mutex_lock(&inventory_lock);
    // Search for items that contain the query string
    for (int i = 0; i < num_items && count < MAX_RESULTS; i++) {
        if (strstr(inventory[i].name, query) != NULL)
            results[count++] = inventory[i];
    }
    // Unlock the inventory_lock mutex
    pthread_mutex_unlock(&inventory_lock);

    // Send the SEARCH_RESULTS message
    msg_enum rsp = SEARCH_RESULTS;
    // Send the SEARCH_RESULTS message
    if (write(client_fd, &rsp, sizeof(msg_enum)) != (ssize_t)sizeof(msg_enum))
        return;
    // Send the number of results if it is less than MAX_RESULTS
    if (write(client_fd, &count, sizeof(int)) != (ssize_t)sizeof(int))
        return;
    // Send the results if they are less than MAX_RESULTS
    for (int j = 0; j < count; j++) {
        if (write(client_fd, &results[j], sizeof(struct item)) != (ssize_t)sizeof(struct item))
            return;
    }
}

// ============================================================
// handle_get_stock: read an item name and respond with its stock/price.
// Protocol (recv): char name[MAX_STR]
// Protocol (send): STOCK_INFO (msg_enum), stock (int), price (float)
//             or:  ERROR_MSG (msg_enum), char err[MAX_STR]
// ============================================================
void handle_get_stock(int client_fd)
{
    // Read the item name
    char name[MAX_STR];
    memset(name, 0, sizeof(name));
    if (read(client_fd, name, sizeof(name)) != (ssize_t)sizeof(name))
        return;

    // Lock the inventory_lock mutex
    pthread_mutex_lock(&inventory_lock);
    // Search for the item
    int idx = -1;
    for (int i = 0; i < num_items; i++) {
        if (strcmp(inventory[i].name, name) == 0) {
            idx = i;
            break;
        }
    }
    // If the item is found, set the stock and price
    int stock = 0;
    float price = 0.0f;
    if (idx >= 0) {
        stock = inventory[idx].stock;
        price = inventory[idx].price;
    }
    pthread_mutex_unlock(&inventory_lock);

    // Send the STOCK_INFO message if the item is found
    if (idx >= 0) {
        msg_enum rsp = STOCK_INFO;
        // Send the STOCK_INFO message if the item is found
        if (write(client_fd, &rsp, sizeof(msg_enum)) != (ssize_t)sizeof(msg_enum))
            return;
        // Send the stock if the item is found
        if (write(client_fd, &stock, sizeof(int)) != (ssize_t)sizeof(int))
            return;
        // Send the price if the item is found
        if (write(client_fd, &price, sizeof(float)) != (ssize_t)sizeof(float))
            return;
    } else { // Send the ERROR_MSG message if the item is not found
        msg_enum rsp = ERROR_MSG;
        // Send the ERROR_MSG message if the item is not found
        char err[MAX_STR];
        memset(err, 0, sizeof(err));
        snprintf(err, sizeof(err), "item not found");
        if (write(client_fd, &rsp, sizeof(msg_enum)) != (ssize_t)sizeof(msg_enum))
            return;
        if (write(client_fd, err, sizeof(err)) != (ssize_t)sizeof(err))
            return;
    }
}

// ============================================================
// handle_buy_item: read item name and amount, decrement stock.
// Protocol (recv): char name[MAX_STR], int amount
// Protocol (send): BUY_OK (msg_enum), new_stock (int), total_cost (float)
//             or:  ERROR_MSG (msg_enum), char err[MAX_STR]
// Send an error if the item doesn't exist OR if stock < amount.
// ============================================================
void handle_buy_item(int client_fd)
{
    // Read the item name
    char name[MAX_STR];
    memset(name, 0, sizeof(name));
    if (read(client_fd, name, sizeof(name)) != (ssize_t)sizeof(name))
        return;

    // Read the amount
    int amount = 0;
    if (read(client_fd, &amount, sizeof(int)) != (ssize_t)sizeof(int))
        return;

    // Lock the inventory_lock mutex
    pthread_mutex_lock(&inventory_lock);
    // Search for the item
    int idx = -1;
    for (int i = 0; i < num_items; i++) {
        if (strcmp(inventory[i].name, name) == 0) {
            idx = i;
            break;
        }
    }
    // If the item is not found, send the ERROR_MSG message
    if (idx < 0) {
        pthread_mutex_unlock(&inventory_lock);
        msg_enum rsp = ERROR_MSG;
        // Send the ERROR_MSG message if the item is not found
        char err[MAX_STR];
        memset(err, 0, sizeof(err));
        snprintf(err, sizeof(err), "item not found");
        // Send the ERROR_MSG message if the item is not found
        if (write(client_fd, &rsp, sizeof(msg_enum)) != (ssize_t)sizeof(msg_enum))
            return;
        // Send the ERROR_MSG message if the item is not found
        write(client_fd, err, sizeof(err));
        return;
    }

    // If the stock is less than the amount, send the ERROR_MSG message
    if (inventory[idx].stock < amount) {
        pthread_mutex_unlock(&inventory_lock);
        msg_enum rsp = ERROR_MSG;
        // Send the ERROR_MSG message if the stock is less than the amount
        char err[MAX_STR];
        memset(err, 0, sizeof(err));
        snprintf(err, sizeof(err), "insufficient stock");
        if (write(client_fd, &rsp, sizeof(msg_enum)) != (ssize_t)sizeof(msg_enum))
            return;
        // Send the ERROR_MSG message if the stock is less than the amount
        write(client_fd, err, sizeof(err));
        return;
    }

    // Decrement the stock
    inventory[idx].stock -= amount;
    // Set the new stock
    int new_stock = inventory[idx].stock;
    // Set the total cost
    float total_cost = (float)amount * inventory[idx].price;
    // Unlock the inventory_lock mutex
    pthread_mutex_unlock(&inventory_lock);

    // Send the BUY_OK message
    msg_enum rsp = BUY_OK;
    // Send the BUY_OK message
    if (write(client_fd, &rsp, sizeof(msg_enum)) != (ssize_t)sizeof(msg_enum))
        return;
    // Send the new stock
    if (write(client_fd, &new_stock, sizeof(int)) != (ssize_t)sizeof(int))
        return;
    // Send the total cost
    write(client_fd, &total_cost, sizeof(float));
}

// ============================================================
// handle_sell_item: read item name and amount, increment stock.
// Protocol (recv): char name[MAX_STR], int amount
// Protocol (send): SELL_OK (msg_enum), new_stock (int)
//             or:  ERROR_MSG (msg_enum), char err[MAX_STR]
// ============================================================
void handle_sell_item(int client_fd)
{
    // Read the item name
    char name[MAX_STR];
    memset(name, 0, sizeof(name));
    if (read(client_fd, name, sizeof(name)) != (ssize_t)sizeof(name))
        return;

    // Read the amount
    int amount = 0;
    if (read(client_fd, &amount, sizeof(int)) != (ssize_t)sizeof(int))
        return;

    // Lock the inventory_lock mutex
    pthread_mutex_lock(&inventory_lock);
    // Search for the item
    int idx = -1;
    for (int i = 0; i < num_items; i++) {
        if (strcmp(inventory[i].name, name) == 0) {
            idx = i;
            break;
        }
    }
    // If the item is not found, send the ERROR_MSG message
    if (idx < 0) {
        pthread_mutex_unlock(&inventory_lock);
        msg_enum rsp = ERROR_MSG;
        // Send the ERROR_MSG message if the item is not found
        char err[MAX_STR];
        memset(err, 0, sizeof(err));
        snprintf(err, sizeof(err), "item not found");
        // Send the ERROR_MSG message if the item is not found
        if (write(client_fd, &rsp, sizeof(msg_enum)) != (ssize_t)sizeof(msg_enum))
            return;
        write(client_fd, err, sizeof(err));
        return;
    }

    // Increment the stock
    inventory[idx].stock += amount;
    // Set the new stock
    int new_stock = inventory[idx].stock;
    // Unlock the inventory_lock mutex
    pthread_mutex_unlock(&inventory_lock);

    // Send the SELL_OK message
    msg_enum rsp = SELL_OK;
    if (write(client_fd, &rsp, sizeof(msg_enum)) != (ssize_t)sizeof(msg_enum))
        return;
    write(client_fd, &new_stock, sizeof(int));
}

// ============================================================
// save_inventory: write the current inventory[] to output/inventory.csv
// Format should match items.csv (header: "name,stock,price").
// ============================================================
void save_inventory()
{
    // Open the file
    FILE *fp = fopen("output/inventory.csv", "w");
    if (fp == NULL) {
        perror("fopen output/inventory.csv");
        return;
    }
    fprintf(fp, "name,stock,price\n");
    // Lock the inventory_lock mutex
    pthread_mutex_lock(&inventory_lock);
    // Write the inventory to the file
    for (int i = 0; i < num_items; i++) {
        fprintf(fp, "%s,%d,%.2f\n",
                inventory[i].name, inventory[i].stock, inventory[i].price);
    }
    // Unlock the inventory_lock mutex and close file
    pthread_mutex_unlock(&inventory_lock);
    fclose(fp);
}

// ============================================================
// handle_client: the worker thread function for one client connection.
// Loop reading a msg_enum from the socket and dispatching to the right
// handler. Break out of the loop and close the socket when the client
// disconnects (read returns <= 0).
// ============================================================
void *handle_client(void *arg)
{
    // Get the client file descriptor
    int client_fd = *(int *)arg;
    free(arg);

    // Read the message from the client
    while (1) {
        msg_enum msg;
        if (read(client_fd, &msg, sizeof(msg)) != (ssize_t)sizeof(msg))
            break;

        // Dispatch the message to the right handler
        switch (msg) {
        case LIST_ITEMS:
            handle_list_items(client_fd);
            break;
        case SEARCH_ITEM:
            handle_search(client_fd);
            break;
        case ENC_SEARCH_ITEM:
            handle_enc_search(client_fd);
            break;
        case GET_STOCK:
            handle_get_stock(client_fd);
            break;
        case BUY_ITEM:
            handle_buy_item(client_fd);
            break;
        case SELL_ITEM:
            handle_sell_item(client_fd);
            break;
        case TERMINATE:
            close(client_fd);
            return NULL;
        default:
            break;
        }
    }
    // Close the client file descriptor
    close(client_fd);
    return NULL;
}

// ============================================================
// sigterm_handler: on SIGTERM, save inventory and exit(0).
// ============================================================
void sigterm_handler(int sig)
{
    // Save the inventory
    (void)sig;
    save_inventory();
    exit(0);
}

int main(int argc, char *argv[])
{
    // Check the number of arguments
    if (argc != 4) {
        printSyntax();
        return 1;
    }

    // Parse the server address and port
    char *server_addr = argv[1];
    int server_port = atoi(argv[2]);
    (void)argv[3];

    // Run the bookeeping code
    bookeepingCode();
    // Load the inventory
    load_inventory("items.csv");
    // Register the signal handler
    signal(SIGTERM, sigterm_handler);

    // Create the socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    // If the socket is not created, return 1
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    // Set the socket option
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // Create the address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)server_port);
    if (inet_pton(AF_INET, server_addr, &addr.sin_addr) <= 0) {
        fprintf(stderr, "invalid server address\n");
        close(listen_fd);
        return 1;
    }

    // Bind the socket to the address
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    // Listen for incoming connections
    if (listen(listen_fd, 32) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    // Accept incoming connections
    while (1) {
        // Accept an incoming connection
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        // Create a new thread for the client
        int *pfd = malloc(sizeof(int));
        if (pfd == NULL) {
            close(client_fd);
            continue;
        }
        *pfd = client_fd;
        // Create a new thread for the client
        pthread_t tid;
        // If the thread is not created, return 1
        if (pthread_create(&tid, NULL, handle_client, pfd) != 0) {
            perror("pthread_create");
            free(pfd);
            close(client_fd);
            continue;
        }
        // Detach the thread
        pthread_detach(tid);
    }
}
