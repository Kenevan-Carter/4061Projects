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
//
// Wire format matches server.c: msg_enum is 4 bytes; ints and floats
// are native (same endianness as server — localhost).
// ============================================================

int connect_to_server(char *server_addr, int server_port) //DONE and working
{
    // TODO: create a TCP socket, connect to (server_addr, server_port),
    //       return the socket fd.
    int fd = socket(AF_INET, SOCK_STREAM, 0); //create tcp socket
    if (fd < 0) { 
        perror("socket"); //sucket fail msg
        return -1;
    }

    struct sockaddr_in addr ={.sin_family =AF_INET, .sin_port = htons((unsigned short)server_port)};
    //struct sockaddr_in addr = {0}; //zero out the struct
    if (inet_pton(AF_INET, server_addr, &addr.sin_addr) <= 0) {
        perror("inet_pton");//invalid server address msg
        close(fd);
        return -1;
    }
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");//connect fail msg

        close(fd);

        return -1;
    }
    return fd;
}

void get_item_list(int sock_fd, FILE *log_fp) //DONE and working
{


    // TODO: send the request, read the count, then loop reading items.
    //fprintf(log_fp, "=== Item List (%d items) ===\n", count);
    // TODO: for each item received:
    //   struct item it = ...;
    //   fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
    //           it.name, it.stock, it.price);
    //fprintf(log_fp, "\n");
    
    int count = 0;
    msg_enum req = LIST_ITEMS; // specfy the request type for list items
    if( write(sock_fd, &req, sizeof(req)) !=(ssize_t)sizeof(req))  { //send request and kill if fail
        
        return;
    }
    msg_enum rsp;
    if (read(sock_fd, &rsp,sizeof(rsp))!=(ssize_t) sizeof(rsp)){//read response type
        return;
    }
    if (read(sock_fd, &count,sizeof(count))!= (ssize_t)sizeof(count)){// read  count of items
        return;
    }
    fprintf(log_fp, "=== Item List (%d items) ===\n", count);
    


    for (int i=0; i <count;i++) {
        struct item it;
        if (read(sock_fd, &it, sizeof(it)) != (ssize_t)sizeof(it))
            return;
        //if(read(sock_fd, &it, sizeof(it)) != (ssize_t)s

        fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
                it.name, it.stock, it.price);
    }
    fprintf(log_fp, "\n");
}

void search_item(int sock_fd, char *query, FILE *log_fp)//DONE and working
{

    // TODO
    int count = 0;
    msg_enum req = SEARCH_ITEM;
    if (write(sock_fd, &req, sizeof(req)) != (ssize_t)sizeof(req))
        return;

    char buf[MAX_STR] = {0};
    strncpy(buf, query, MAX_STR - 1);
    if (write(sock_fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf)){
        return;
    }

    msg_enum rsp; //response type
    if (read(sock_fd, &rsp, sizeof(rsp)) != (ssize_t)sizeof(rsp)){
        return;
    }
    if (read(sock_fd, &count, sizeof(count)) != (ssize_t)sizeof(count)){
        return;
    }
    fprintf(log_fp, "=== Search results for \"%s\" (%d matches) ===\n", query, count);
    // TODO: for each match:
    //   fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
    //           it.name, it.stock, it.price);
    for (int i=0; i < count; i++) {
        struct item it;
        if (read(sock_fd, &it, sizeof(it)) != (ssize_t)sizeof(it))
            return;
        fprintf(log_fp,"  %s | stock: %d | price: $%.2f\n", it.name,it.stock, it.price);
    }
    fprintf(log_fp, "\n");
}

// BONUS (+10): like search_item, but cipher the query on the wire.
// Use encrypt_str() from utils.h; the server will call decrypt_str().
void enc_search_item(int sock_fd, char *query, FILE *log_fp) //DONE and working
{
    int count = 0;
    // TODO (bonus)
    msg_enum req = ENC_SEARCH_ITEM;
    if (write(sock_fd, &req, sizeof(req)) != (ssize_t)sizeof(req))
        return;

    char buf[MAX_STR] ={0};
    strncpy(buf, query, MAX_STR - 1);
    encrypt_str(buf);
    if (write(sock_fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf))
        return;

    msg_enum rsp;

    if (read(sock_fd, &rsp, sizeof(rsp)) != (ssize_t)sizeof(rsp))
        return;

    if (read(sock_fd, &count, sizeof(count)) != (ssize_t)sizeof(count))
        return;

    fprintf(log_fp, "=== Search results for \"%s\" (%d matches) ===\n", query, count);

    // TODO (bonus): for each match:
    //   fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
    //           it.name, it.stock, it.price);
    for (int i = 0; i < count; i++) {
        struct item it;
        if (read(sock_fd, &it, sizeof(it)) != (ssize_t)sizeof(it))
            return;
        //if (read(sock_fd,sizeof())!=(ssize_t)sizeof(it))
        fprintf(log_fp, "  %s | stock: %d | price: $%.2f\n",
                it.name, it.stock, it.price);
    }

    fprintf(log_fp, "\n");
}

void get_stock(int sock_fd, char *item_name, FILE *log_fp) //DONE and working
{
    int stock =0;
    float price =0;

    char err[MAX_STR]= {0}; // buffer for error message 

    msg_enum req= GET_STOCK;
    if (write(sock_fd, &req, sizeof(req)) != (ssize_t)sizeof(req))
        return;

    char buf[MAX_STR] ={0}; //buffer for item name
    strncpy(buf, item_name, MAX_STR-1);
    if (write(sock_fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf))
        return;

    msg_enum rsp;

    if (read(sock_fd, &rsp, sizeof(rsp)) != (ssize_t)sizeof(rsp))
        return;

    if(rsp==STOCK_INFO) {
        if(read(sock_fd, &stock, sizeof(stock))!=(ssize_t)sizeof(stock))
            return; //read stock from server and kill if fail
        if(read(sock_fd,&price,sizeof(price)) != (ssize_t)sizeof(price))
            return; //read price from server and kill if fail
        fprintf(log_fp, "Stock check: %s | stock: %d | price: $%.2f\n\n",
                item_name, stock, price);
    } 
    // TODO: on error:
    //   fprintf(log_fp, "Stock check error for %s: %s\n\n", item_name, err);
    else {
        if (read(sock_fd, err, sizeof(err)) != (ssize_t)sizeof(err))
            return;
        fprintf(log_fp, "Stock check error for %s: %s\n\n", item_name, err);
    }
}

void buy_item(int sock_fd, char *item_name, int amount, FILE *log_fp) //DONE and working
{
    int new_stock = 0;
    float total_cost = 0;

    char err[MAX_STR] = {0};// buffer for error message

    msg_enum req = BUY_ITEM;    // specify  request type 
    if (write(sock_fd, &req, sizeof(req)) != (ssize_t)sizeof(req))
        return;

    char buf[MAX_STR] = {0};


    strncpy(buf, item_name, MAX_STR - 1);// copy item name to buffer in order to ensure that null termination occurs properly
    if (write(sock_fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf))
        return;

    if (write(sock_fd, &amount, sizeof(amount)) != (ssize_t)sizeof(amount))
        return;

    msg_enum rsp;
    if (read(sock_fd, &rsp, sizeof(rsp)) != (ssize_t)sizeof(rsp))
        return;

    if (rsp == BUY_OK) { //on Success
        if (read( sock_fd, &new_stock, sizeof(new_stock)) !=(ssize_t)sizeof(new_stock)) 
            return;
        if (read(sock_fd, &total_cost,sizeof(total_cost))!=(ssize_t)sizeof(total_cost))
            return;
        fprintf(log_fp, "Bought %d x %s for $%.2f (remaining stock: %d)\n\n",
                amount, item_name, total_cost, new_stock);
    } 
    
    else { //om error of purchase
        if (read(sock_fd,err,sizeof(err)) !=(ssize_t)sizeof(err))
            return;
        fprintf(log_fp,"Buy error for %s: %s\n\n", item_name, err);
    }
}

void sell_item(int sock_fd, char *item_name, int amount, FILE *log_fp) //DONE and working
{
    int new_stock = 0;
    char err[MAX_STR] = {0};

    msg_enum req = SELL_ITEM;

    if (write(sock_fd, &req, sizeof(req)) != (ssize_t)sizeof(req))
        return;

    char buf[MAX_STR] = {0};
    strncpy(buf, item_name, MAX_STR - 1);
    if (write(sock_fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf))
        return;
    if (write(sock_fd, &amount, sizeof(amount)) != (ssize_t)sizeof(amount))
        return;

    msg_enum rsp;
    if (read(sock_fd, &rsp, sizeof(rsp)) != (ssize_t)sizeof(rsp))
        return;

    if (rsp == SELL_OK) {
        if (read(sock_fd, &new_stock, sizeof(new_stock)) != (ssize_t)sizeof(new_stock))
            return;
        fprintf(log_fp, "Sold %d x %s (new stock: %d)\n\n",
                amount, item_name, new_stock);
    } else {
        if (read(sock_fd, err, sizeof(err)) != (ssize_t)sizeof(err))
            return;
        fprintf(log_fp, "Sell error for %s: %s\n\n", item_name, err);
    }
}
// Read commands from stdin, dispatch to the functions above.
// Recognized commands: LIST | SEARCH <q> | ESEARCH <q> (bonus) |
//                      STOCK <name> | BUY <name> <n> | SELL <name> <n> | QUIT
// When isatty(0) is true, print a "> " prompt before each command.
void process_input(int sock_fd) //DONE and working
{
    //TODO
    char line[256];
    char arg1[MAX_STR];
    int n;

    while (1) {//loop unitl quit command is given
        if (isatty(0)) {
            printf("> ");
            fflush(stdout);

        }

        if (!fgets(line, sizeof(line), stdin))
            break;
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "LIST") == 0) {
            get_item_list(sock_fd, stdout);
        } else if (sscanf(line, "SEARCH %63s", arg1) == 1) { //parse search command and argument
            search_item(sock_fd, arg1, stdout);
        } else if (sscanf(line, "ESEARCH %63s", arg1) == 1) {// parse enc_search command and argument
            enc_search_item(sock_fd, arg1, stdout);
        } else if (sscanf(line, "STOCK %63s", arg1) == 1) { //parse stock command and argument
            get_stock(sock_fd, arg1, stdout);
        } else if (sscanf(line, "BUY %63s %d", arg1, &n) == 2) {// parse buy command and arguments
            buy_item(sock_fd, arg1, n, stdout);
        } else if (sscanf(line, "SELL %63s %d", arg1, &n) == 2) {// parse sell command and arguments
            sell_item(sock_fd, arg1, n, stdout);
        } else if (strcmp(line, "QUIT") == 0) {
            break;
        }
    }
}

int main(int argc, char *argv[]) //DONE and working
{
    // TODO:
    //   1. check argc == 3, call printSyntax() on error
    //   2. parse server_addr and server_port
    //   3. sock_fd = connect_to_server(...)
    //   4. process_input(?)
    //   5. close(?)
    if (argc != 3) {
        printSyntax();
        return 1;
    }
    char *server_addr = argv[1];
    int server_port = atoi(argv[2]);
    int sock_fd = connect_to_server(server_addr, server_port);
    if (sock_fd < 0)
        return 1;
    process_input(sock_fd);
    close(sock_fd);
    return 0;
}

