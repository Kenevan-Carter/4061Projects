#include "client.h"

void printSyntax(){
    printf("incorrect usage syntax!\n");
    printf("usage: $ ./client server_addr server_port\n");
    printf("       (commands are read from stdin; use < input.txt to redirect)\n");
}

// ============================================================
// You write everything below except main() and the fprintf format
// strings inside each function. Figure out what each parameter
// means from the protocol in instruction.md; fill in the reads,
// writes, and control flow so the fprintfs print the right values.
// ============================================================

int connect_to_server(char *server_addr, int server_port)
{
    int fd= socket(AF_INET,SOCK_STREAM, 0);
    if (fd <0) { perror("socket"); return -1; }


    struct sockaddr_in addr = {.sin_family = AF_INET,.sin_port = htons(server_port)};
    if (inet_pton(AF_INET, server_addr, &addr.sin_addr) <= 0) {
        perror("inet_pton"); close(fd); return -1;
    }
    if (connect(fd , (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect "); close( fd); return -1;
    }
    return fd;
}

void get_item_list(int sock_fd, FILE *log_fp)
{

    int count = 0;
    // TODO: send the request, read the count, then loop reading items.
    uint8_t op = LIST_ITEMS;
    write(sock_fd, &op, 1);
    read(sock_fd, &op, 1);   // ITEM_LIST
    read(sock_fd, &count, sizeof(count));
    count = ntohl(count);
    fprintf(log_fp, "=== Item List (%d items) ===\n", count);
    // TODO: for each item received:
    //   struct item it = ...;
    //   fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
    //           it.name, it.stock, it.price);
    for (int i = 0; i < count; i++) {
    struct item it;
    read(sock_fd, &it, sizeof(it));
    it.stock = ntohl(it.stock);
    fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
            it.name, it.stock, it.price);
    }
    fprintf(log_fp, "\n");
}

void search_item(int sock_fd, char *query, FILE *log_fp)
{


    int count = 0;
    // TODO
    uint8_t op = SEARCH_ITEM;
    write(sock_fd, &op, 1);
    char buf[MAX_STR] = {0};
    strncpy(buf, query, MAX_STR - 1);
    write(sock_fd, buf, MAX_STR);
    read(sock_fd, &op, 1);   // SEARCH_RESULTS
    read(sock_fd, &count, sizeof(count));
    count = ntohl(count);
    fprintf(log_fp, "=== Search results for \"%s\" (%d matches) ===\n", query, count);
    // TODO: for each match:
    //   fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
    //           it.name, it.stock, it.price);
    for (int i = 0; i < count; i++) {
        struct item it;
        read(sock_fd, &it, sizeof(it));
        it.stock = ntohl(it.stock);
        fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
                it.name, it.stock, it.price);
    }
    fprintf(log_fp, "\n");
}

// BONUS (+10): like search_item, but cipher the query on the wire.
// Use encrypt_str() from utils.h; the server will call decrypt_str().
void enc_search_item(int sock_fd, char *query, FILE *log_fp)
{
    int count = 0;


    uint8_t op = ENC_SEARCH_ITEM;
    write(sock_fd, &op, 1);
    char buf[MAX_STR] = {0};

    strncpy(buf, query, MAX_STR - 1);
    encrypt_str(buf);
    write(sock_fd, buf, MAX_STR);


    read(sock_fd, &op, 1); 
    read(sock_fd, &count, sizeof(count));
    count = ntohl(count);
    fprintf(log_fp, "=== Search results for \"%s\" (%d matches) ===\n", query, count);
    // TODO (bonus): for each match:
    //   fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
    //           it.name, it.stock, it.price);
    for (int i = 0; i < count; i++) {
        struct item it;
        read(sock_fd, &it, sizeof(it));
        it.stock = ntohl(it.stock);
        fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
                it.name, it.stock, it.price);
    }
    fprintf(log_fp, "\n");
}

void get_stock(int sock_fd, char *item_name, FILE *log_fp)
{


    int stock = 0;
    float price = 0;
    char err[MAX_STR] = {0};
    uint8_t op = GET_STOCK;
    write(sock_fd, &op, 1);
    char buf[MAX_STR] = {0};
    strncpy(buf, item_name, MAX_STR - 1);

    write(sock_fd, buf, MAX_STR);
    read(sock_fd, &op, 1);
        // TODO: on success:

    if (op == STOCK_INFO) {
        read(sock_fd, &stock, sizeof(stock)); stock = ntohl(stock);
        read(sock_fd, &price, sizeof(price));
        fprintf(log_fp, "Stock check: %s | stock: %d | price: $%.2f\n\n",
                item_name, stock, price);
    } 
    // TODO: on error:
    //   fprintf(log_fp, "Stock check error for %s: %s\n\n", item_name, err);
    else {
        read(sock_fd, err, MAX_STR);
        fprintf(log_fp, "Stock check error for %s: %s\n\n", item_name, err);
    }
    (void)err;
}

void buy_item(int sock_fd, char *item_name, int amount, FILE *log_fp)
{

    int new_stock = 0;


    float total_cost = 0;
    char err[MAX_STR] = {0};

    uint8_t op = BUY_ITEM;
    write(sock_fd, &op, 1);


    char buf[MAX_STR] = {0};

    strncpy(buf, item_name, MAX_STR - 1);
    write(sock_fd, buf, MAX_STR);
    int net = htonl(amount);
    write(sock_fd, &net, sizeof(net));
    read(sock_fd, &op, 1);
    // TODO: on success:
    if (op == BUY_OK) {
        read(sock_fd, &new_stock,  sizeof(new_stock));  new_stock = ntohl(new_stock);
        read(sock_fd, &total_cost, sizeof(total_cost));
        fprintf(log_fp, "Bought %d x %s for $%.2f (remaining stock: %d)\n\n",
                amount, item_name, total_cost, new_stock);
    } 
    // TODO: on error:
    //   fprintf(log_fp, "Buy error for %s: %s\n\n", item_name, err);
    else {
        read(sock_fd, err, MAX_STR);
        fprintf(log_fp, "Buy error for %s: %s\n\n", item_name, err);
    }
    (void)err;
}

void sell_item(int sock_fd, char *item_name, int amount, FILE *log_fp)
{

    int new_stock = 0;
    char err[MAX_STR] = {0};

    uint8_t op = SELL_ITEM;
    write(sock_fd, &op, 1);
    
    char buf[MAX_STR] = {0};
    strncpy(buf, item_name, MAX_STR - 1);
    write(sock_fd, buf, MAX_STR);
    int net = htonl(amount);
    write(sock_fd, &net, sizeof(net));
    read(sock_fd, &op, 1);
    // TODO: on success:
    if (op == SELL_OK) {
        read(sock_fd, &new_stock, sizeof(new_stock)); new_stock = ntohl(new_stock);
        fprintf(log_fp, "Sold %d x %s (new stock: %d)\n\n",
                amount, item_name, new_stock);
    } 
    // TODO: on error:
    //   fprintf(log_fp, "Sell error for %s: %s\n\n", item_name, err);
    
    else {
        read(sock_fd, err, MAX_STR);
        fprintf(log_fp, "Sell error for %s: %s\n\n", item_name, err);
    }
    (void)err;
}

// Read commands from stdin, dispatch to the functions above.
// Recognized commands: LIST | SEARCH <q> | ESEARCH <q> (bonus) |
//                      STOCK <name> | BUY <name> <n> | SELL <name> <n> | QUIT
// When isatty(0) is true, print a "> " prompt before each command.
void process_input(int sock_fd)
{
    // TODO
    char line[ 256];
    char arg1[ MAX_STR];
    
    int n;


    while (1) {
        if (isatty(0)) { 
            printf("> "); fflush(stdout); 
        }

        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';


        if (strcmp(line,  "LIST") == 0) {
            get_item_list(sock_fd, stdout);
        } else if ( sscanf(line,"SEARCH %63s", arg1) == 1) {
            search_item(sock_fd, arg1, stdout);

        } else if ( sscanf(line, "ESEARCH %63s", arg1) ==1){
            enc_search_item(sock_fd, arg1, stdout);
        } else if (sscanf(line, "STOCK %63s", arg1) ==1) {
            get_stock(sock_fd, arg1, stdout);
        } else if (sscanf(line,"BUY %63s %d", arg1, &n) ==2) {
            buy_item(sock_fd, arg1, n, stdout);
        } else if(sscanf(line,  "SELL %63s %d", arg1, &n) == 2) {
            sell_item(sock_fd, arg1, n, stdout);
        } else if(strcmp(line,"QUIT") == 0) {
            break;
        }

    }
}

int main(int argc, char *argv[]) // DONE
{
    // TODO:
    //   1. check argc == 3, call printSyntax() on error
    //   2. parse server_addr and server_port
    //   3. sock_fd = connect_to_server(...)
    //   4. process_input(?)
    //   5. close(?)
    if (argc != 3) { printSyntax(); return 1; }
    char *server_addr = argv[1];
    int   server_port = atoi(argv[2]);
    int sock_fd = connect_to_server(server_addr, server_port);
    if (sock_fd < 0) return 1;
    process_input(sock_fd);
    close(sock_fd);
    return 0;
}
