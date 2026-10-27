
/* ContactHub - Contact Management System in C */

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <stdlib.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define CLOSESOCKET closesocket
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define CLOSESOCKET close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEFAULT_PORT 8080
#define HASH_SIZE 101
#define REQUEST_SIZE 65536
#define RESPONSE_SIZE 131072

typedef struct {
    char id[20];
    char name[50];
    char phone[20];
    char email[60];
    char address[100];
    char category[30];
} Contact;

typedef struct Node {
    Contact data;
    struct Node *next;
} Node;

typedef struct HashEntry {
    char phone[20];
    Node *contact;
    struct HashEntry *next;
} HashEntry;

typedef struct TreeNode {
    Node *contact;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

static Node *head = NULL;
static HashEntry *phone_table[HASH_SIZE];
static TreeNode *name_root = NULL;
static int next_id = 16;

/* Compare text without considering letter case */
static int text_compare(const char *a, const char *b) {
    while (*a && *b) {
        int x = tolower((unsigned char)*a);
        int y = tolower((unsigned char)*b);

        if (x != y)
            return x - y;

        a++;
        b++;
    }

    return tolower((unsigned char)*a) -
           tolower((unsigned char)*b);
}

/* Check whether a required field is empty or spaces */
static int is_blank(const char *text) {
    while (*text) {
        if (!isspace((unsigned char)*text))
            return 0;
        text++;
    }

    return 1;
}

/* Hash function for phone numbers */
static int hash_phone(const char *phone) {
    unsigned long hash = 0;

    while (*phone) {
        hash = (hash * 31 + (unsigned char)*phone++)
               % HASH_SIZE;
    }

    return (int)hash;
}

/* Search contact by ID in the linked list */
static Node *find_id(const char *id) {
    Node *temp = head;

    while (temp) {
        if (strcmp(temp->data.id, id) == 0)
            return temp;

        temp = temp->next;
    }

    return NULL;
}

/* Search contact by phone using hash table */
static Node *find_phone(const char *phone) {
    HashEntry *entry = phone_table[hash_phone(phone)];

    while (entry) {
        if (strcmp(entry->phone, phone) == 0)
            return entry->contact;

        entry = entry->next;
    }

    return NULL;
}

/* Automatically generate a unique contact ID */
static void generate_id(char *out, size_t capacity) {
    do {
        snprintf(out, capacity, "C%03d", next_id++);
    } while (find_id(out) != NULL);
}

/* Free binary search tree nodes */
static void free_tree(TreeNode *node) {
    if (!node)
        return;

    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

/* Rebuild the hash table and binary search tree */
static void rebuild_indexes(void) {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashEntry *entry = phone_table[i];

        while (entry) {
            HashEntry *next = entry->next;
            free(entry);
            entry = next;
        }

        phone_table[i] = NULL;
    }

    free_tree(name_root);
    name_root = NULL;

    Node *n = head;

    while (n) {
        /* Insert into hash table */
        int index = hash_phone(n->data.phone);
        HashEntry *entry = malloc(sizeof(*entry));

        if (entry) {
            strcpy(entry->phone, n->data.phone);
            entry->contact = n;
            entry->next = phone_table[index];
            phone_table[index] = entry;
        }

        /* Insert into binary search tree */
        TreeNode *fresh = malloc(sizeof(*fresh));

        if (fresh) {
            fresh->contact = n;
            fresh->left = NULL;
            fresh->right = NULL;

            if (!name_root) {
                name_root = fresh;
            } else {
                TreeNode *current = name_root;

                while (1) {
                    int cmp = text_compare(
                        n->data.name,
                        current->contact->data.name
                    );

                    if (cmp < 0) {
                        if (!current->left) {
                            current->left = fresh;
                            break;
                        }

                        current = current->left;
                    } else {
                        if (!current->right) {
                            current->right = fresh;
                            break;
                        }

                        current = current->right;
                    }
                }
            }
        }

        n = n->next;
    }
}

/* Load 15 sample contacts automatically */
static void seed_contacts(void) {
    static const Contact samples[] = {
        {"C001", "Rahul Patel", "9000000001",
         "rahul@example.com", "Ahmedabad", "Friend"},

        {"C002", "Priya Shah", "9000000002",
         "priya@example.com", "Gandhinagar", "Family"},

        {"C003", "Amit Kumar", "9000000003",
         "amit@example.com", "Surat", "Work"},

        {"C004", "Neha Joshi", "9000000004",
         "neha@example.com", "Ahmedabad", "Friend"},

        {"C005", "Karan Mehta", "9000000005",
         "karan@example.com", "Vadodara", "Work"},

        {"C006", "Riya Desai", "9000000006",
         "riya@example.com", "Rajkot", "Family"},

        {"C007", "Arjun Singh", "9000000007",
         "arjun@example.com", "Ahmedabad", "Friend"},

        {"C008", "Ananya Rao", "9000000008",
         "ananya@example.com", "Surat", "Other"},

        {"C009", "Dev Shah", "9000000009",
         "dev@example.com", "Gandhinagar", "Work"},

        {"C010", "Meera Joshi", "9000000010",
         "meera@example.com", "Ahmedabad", "Family"},

        {"C011", "Vivek Patel", "9000000011",
         "vivek@example.com", "Rajkot", "Friend"},

        {"C012", "Isha Mehta", "9000000012",
         "isha@example.com", "Vadodara", "Other"},

        {"C013", "Krish Verma", "9000000013",
         "krish@example.com", "Surat", "Work"},

        {"C014", "Pooja Shah", "9000000014",
         "pooja@example.com", "Ahmedabad", "Family"},

        {"C015", "Yash Desai", "9000000015",
         "yash@example.com", "Gandhinagar", "Friend"}
    };

    int total = (int)(sizeof(samples) / sizeof(samples[0]));

    for (int i = 0; i < total; i++) {
        Node *node = calloc(1, sizeof(*node));

        if (!node) {
            fprintf(stderr, "Could not load all sample contacts.\n");
            break;
        }

        node->data = samples[i];

        if (!head) {
            head = node;
        } else {
            Node *temp = head;

            while (temp->next)
                temp = temp->next;

            temp->next = node;
        }
    }

    rebuild_indexes();
}

/* Escape text before putting it into JSON */
static void json_string(
    char *out, size_t capacity, const char *text
) {
    size_t j = 0;

    if (capacity == 0)
        return;

    for (size_t i = 0; text[i] && j + 2 < capacity; i++) {
        unsigned char c = (unsigned char)text[i];

        if (c == '"' || c == '\\') {
            out[j++] = '\\';
            out[j++] = (char)c;
        } else if (c == '\n') {
            out[j++] = '\\';
            out[j++] = 'n';
        } else if (c == '\r') {
            out[j++] = '\\';
            out[j++] = 'r';
        } else if (c == '\t') {
            out[j++] = '\\';
            out[j++] = 't';
        } else if (c >= 32) {
            out[j++] = (char)c;
        }
    }

    out[j] = '\0';
}

/* Add a contact to a JSON response */
static void append_contact(
    char *out, size_t capacity,
    const Contact *c, int *first
) {
    char id[80], name[200], phone[80];
    char email[240], address[400], category[120];

    json_string(id, sizeof id, c->id);
    json_string(name, sizeof name, c->name);
    json_string(phone, sizeof phone, c->phone);
    json_string(email, sizeof email, c->email);
    json_string(address, sizeof address, c->address);
    json_string(category, sizeof category, c->category);

    size_t used = strlen(out);

    if (used >= capacity)
        return;

    snprintf(out + used, capacity - used,
        "%s{\"id\":\"%s\",\"name\":\"%s\","
        "\"phone\":\"%s\",\"email\":\"%s\","
        "\"address\":\"%s\",\"category\":\"%s\"}",
        *first ? "" : ",",
        id, name, phone, email, address, category
    );

    *first = 0;
}

/* Send JSON response */
static void response_json(
    socket_t client, int status, const char *body
) {
    const char *label =
        status == 200 ? "OK" :
        status == 400 ? "Bad Request" :
        status == 404 ? "Not Found" :
        status == 409 ? "Conflict" :
        "Internal Server Error";

    char header[512];

    int length = snprintf(header, sizeof header,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n"
        "Content-Length: %zu\r\n\r\n",
        status, label, strlen(body)
    );

    send(client, header, length, 0);
    send(client, body, (int)strlen(body), 0);
}

/* Serve the frontend files */
static void response_file(
    socket_t client, const char *path, const char *type
) {
    FILE *file = fopen(path, "rb");

    if (!file) {
        response_json(client, 404,
            "{\"success\":false,"
            "\"message\":\"Frontend file not found. "
            "Run the server from the project root.\"}");
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *data = malloc((size_t)length + 1);

    if (!data) {
        fclose(file);
        response_json(client, 500,
            "{\"success\":false,\"message\":\"Out of memory\"}");
        return;
    }

    size_t got = fread(data, 1, (size_t)length, file);
    fclose(file);
    data[got] = '\0';

    char header[512];

    int hlen = snprintf(header, sizeof header,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n"
        "Content-Length: %zu\r\n\r\n",
        type, got
    );

    send(client, header, hlen, 0);
    send(client, data, (int)got, 0);

    free(data);
}

/* Decode URL-encoded form values */
static int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void decode(
    char *out, size_t capacity, const char *in
) {
    size_t j = 0;

    for (size_t i = 0; in[i] && j + 1 < capacity; i++) {
        if (in[i] == '+') {
            out[j++] = ' ';
        } else if (in[i] == '%' && in[i + 1] && in[i + 2]) {
            int a = hex_value(in[i + 1]);
            int b = hex_value(in[i + 2]);

            if (a >= 0 && b >= 0) {
                out[j++] = (char)(a * 16 + b);
                i += 2;
            } else {
                out[j++] = in[i];
            }
        } else {
            out[j++] = in[i];
        }
    }

    out[j] = '\0';
}

static void field(
    const char *data, const char *key,
    char *out, size_t capacity
) {
    out[0] = '\0';

    size_t key_length = strlen(key);
    const char *p = data;

    while (p && *p) {
        const char *end = strchr(p, '&');
        size_t length = end ? (size_t)(end - p) : strlen(p);

        if (length > key_length &&
            p[key_length] == '=' &&
            strncmp(p, key, key_length) == 0) {

            char raw[1024];
            size_t n = length - key_length - 1;

            if (n >= sizeof raw)
                n = sizeof raw - 1;

            memcpy(raw, p + key_length + 1, n);
            raw[n] = '\0';

            decode(out, capacity, raw);
            return;
        }

        p = end ? end + 1 : NULL;
    }
}

static void reply_message(
    socket_t client, int status, int success, const char *message
) {
    char escaped[512];
    char body[700];

    json_string(escaped, sizeof escaped, message);

    snprintf(body, sizeof body,
        "{\"success\":%s,\"message\":\"%s\"}",
        success ? "true" : "false", escaped
    );

    response_json(client, status, body);
}

/* Handle API requests */
static void handle_api(
    socket_t client, const char *method,
    const char *path, const char *body
) {
    if (!strcmp(method, "OPTIONS")) {
        response_json(client, 200, "{}");
        return;
    }

    /* List all contacts */
    if (!strcmp(method, "GET") &&
        !strcmp(path, "/api/contacts")) {

        char out[RESPONSE_SIZE] =
            "{\"success\":true,\"contacts\":[";
        int first = 1;

        for (Node *n = head; n; n = n->next)
            append_contact(out, sizeof out, &n->data, &first);

        strncat(out, "]}", sizeof out - strlen(out) - 1);
        response_json(client, 200, out);
        return;
    }

    /* Search by name, phone, or category */
    if (!strcmp(method, "GET") &&
        (!strncmp(path, "/api/search?", 12) ||
         !strncmp(path, "/api/category?", 14))) {

        char value[120];
        int by_phone = strstr(path, "phone=") != NULL;
        int by_category = strstr(path, "category=") != NULL;

        const char *question = strchr(path, '?');

        field(question ? question + 1 : "",
              by_phone ? "phone" :
              by_category ? "category" : "name",
              value, sizeof value);

        char out[RESPONSE_SIZE] =
            "{\"success\":true,\"contacts\":[";
        int first = 1;

        if (by_phone) {
            Node *n = find_phone(value);

            if (n)
                append_contact(out, sizeof out, &n->data, &first);

        } else if (by_category) {
            for (Node *n = head; n; n = n->next) {
                if (text_compare(n->data.category, value) == 0)
                    append_contact(out, sizeof out, &n->data, &first);
            }

        } else {
            TreeNode *stack[2048];
            int top = 0;
            TreeNode *current = name_root;

            while (current || top) {
                while (current && top < 2048) {
                    stack[top++] = current;
                    current = current->left;
                }

                if (!top)
                    break;

                current = stack[--top];

                if (text_compare(current->contact->data.name, value) == 0)
                    append_contact(out, sizeof out,
                                   &current->contact->data, &first);

                current = current->right;
            }
        }

        strncat(out, "]}", sizeof out - strlen(out) - 1);
        response_json(client, 200, out);
        return;
    }

    /* Add contact: ID is generated by the backend */
    if (!strcmp(method, "POST") && !strcmp(path, "/api/add")) {
        Contact c = {0};

        field(body, "name", c.name, sizeof c.name);
        field(body, "phone", c.phone, sizeof c.phone);
        field(body, "email", c.email, sizeof c.email);
        field(body, "address", c.address, sizeof c.address);
        field(body, "category", c.category, sizeof c.category);

        if (is_blank(c.name) || is_blank(c.phone)) {
            reply_message(client, 400, 0,
                "Full name and phone number are required.");
            return;
        }

        if (find_phone(c.phone)) {
            reply_message(client, 409, 0,
                "This phone number already exists.");
            return;
        }

        generate_id(c.id, sizeof c.id);

        Node *n = calloc(1, sizeof *n);

        if (!n) {
            reply_message(client, 500, 0,
                "Memory allocation failed.");
            return;
        }

        n->data = c;

        if (!head) {
            head = n;
        } else {
            Node *temp = head;

            while (temp->next)
                temp = temp->next;

            temp->next = n;
        }

        rebuild_indexes();

        reply_message(client, 200, 1,
            "Contact added successfully.");
        return;
    }

    /* Update contact: keep the existing ID */
    if (!strcmp(method, "POST") && !strcmp(path, "/api/update")) {
        char id[20];

        field(body, "id", id, sizeof id);

        Node *n = find_id(id);

        if (!n) {
            reply_message(client, 404, 0, "Contact not found.");
            return;
        }

        Contact c = {0};
        strcpy(c.id, id);

        field(body, "name", c.name, sizeof c.name);
        field(body, "phone", c.phone, sizeof c.phone);
        field(body, "email", c.email, sizeof c.email);
        field(body, "address", c.address, sizeof c.address);
        field(body, "category", c.category, sizeof c.category);

        if (is_blank(c.name) || is_blank(c.phone)) {
            reply_message(client, 400, 0,
                "Full name and phone number are required.");
            return;
        }

        Node *other = find_phone(c.phone);

        if (other && other != n) {
            reply_message(client, 409, 0,
                "This phone number already belongs to another contact.");
            return;
        }

        n->data = c;
        rebuild_indexes();

        reply_message(client, 200, 1,
            "Contact updated successfully.");
        return;
    }

    /* Delete contact */
    if (!strcmp(method, "POST") && !strcmp(path, "/api/delete")) {
        char id[20];

        field(body, "id", id, sizeof id);

        Node *n = head;
        Node *previous = NULL;

        while (n && strcmp(n->data.id, id) != 0) {
            previous = n;
            n = n->next;
        }

        if (!n) {
            reply_message(client, 404, 0, "Contact not found.");
            return;
        }

        if (previous)
            previous->next = n->next;
        else
            head = n->next;

        free(n);
        rebuild_indexes();

        reply_message(client, 200, 1,
            "Contact deleted successfully.");
        return;
    }

    /* Sort contacts alphabetically using Bubble Sort */
    if (!strcmp(method, "GET") && !strcmp(path, "/api/sort")) {
        Node *list[4096];
        int count = 0;

        for (Node *n = head; n && count < 4096; n = n->next)
            list[count++] = n;

        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (text_compare(list[j]->data.name,
                                 list[j + 1]->data.name) > 0) {
                    Node *temp = list[j];
                    list[j] = list[j + 1];
                    list[j + 1] = temp;
                }
            }
        }

        char out[RESPONSE_SIZE] =
            "{\"success\":true,\"contacts\":[";
        int first = 1;

        for (int i = 0; i < count; i++)
            append_contact(out, sizeof out, &list[i]->data, &first);

        strncat(out, "]}", sizeof out - strlen(out) - 1);
        response_json(client, 200, out);
        return;
    }

    reply_message(client, 404, 0, "Unknown API endpoint.");
}

/* Handle browser requests */
static void handle_client(socket_t client) {
    char *request = calloc(1, REQUEST_SIZE);

    if (!request) {
        CLOSESOCKET(client);
        return;
    }

    int total = 0;
    int n;

    while (total < REQUEST_SIZE - 1 &&
           (n = recv(client, request + total,
                     REQUEST_SIZE - 1 - total, 0)) > 0) {

        total += n;
        request[total] = '\0';

        char *separator = strstr(request, "\r\n\r\n");

        if (separator) {
            char *cl = strstr(request, "Content-Length:");
            long needed = 0;

            if (cl && cl < separator)
                needed = strtol(cl + 15, NULL, 10);

            size_t header_length =
                (size_t)(separator + 4 - request);

            if ((long)(total - header_length) >= needed)
                break;
        }
    }

    
char method[10] = {0};
char url[512] = {0};

/* Parse the HTTP request line */
if (total == 0) {
    free(request);
    CLOSESOCKET(client);
    return;
}

int parsed = sscanf(request, "%9s %511s", method, url);

if (parsed != 2) {
    fprintf(stderr, "Invalid HTTP request received.\n");
    free(request);
    CLOSESOCKET(client);
    return;
}

fprintf(stderr, "DEBUG request: method='%s', url='%s'\n",
        method, url);

    char *query = strchr(url, '?');

    if (query)
        *query = '\0';

    char *separator = strstr(request, "\r\n\r\n");
    char *body = separator ? separator + 4 : "";

    if (!strcmp(method, "GET") && !strcmp(url, "/")) {
        response_file(client, "frontend/index.html",
                      "text/html; charset=utf-8");

    } else if (!strcmp(method, "GET") &&
               !strcmp(url, "/style.css")) {
        response_file(client, "frontend/style.css",
                      "text/css; charset=utf-8");

    } else if (!strcmp(method, "GET") &&
               !strcmp(url, "/app.js")) {
        response_file(client, "frontend/app.js",
                      "application/javascript; charset=utf-8");

    } else if (!strncmp(url, "/api/", 5) ||
               !strcmp(method, "OPTIONS")) {

        char fullpath[512];

        snprintf(fullpath, sizeof fullpath, "%s", url);

if (query) {
    strncat(fullpath, "?", sizeof fullpath - strlen(fullpath) - 1);
    strncat(fullpath, query + 1,
            sizeof fullpath - strlen(fullpath) - 1);
}

handle_api(client, method, fullpath, body);

    } else {
        response_json(client, 404,
            "{\"success\":false,\"message\":\"Not found\"}");
    }

    free(request);
    CLOSESOCKET(client);
}


int main(void) {
    seed_contacts();

#ifdef _WIN32
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "Winsock startup failed.\n");
        return 1;
    }
#endif

    int port = DEFAULT_PORT;
    const char *port_env = getenv("PORT");

    if (port_env != NULL && port_env[0] != '\0') {
        char *endptr;
        long configured_port = strtol(port_env, &endptr, 10);

        if (*endptr == '\0' &&
            configured_port > 0 &&
            configured_port <= 65535) {
            port = (int)configured_port;
        } else {
            fprintf(stderr, "Invalid PORT environment variable.\n");
            return 1;
        }
    }

    socket_t server = socket(AF_INET, SOCK_STREAM, 0);

    if (server == INVALID_SOCKET) {
        fprintf(stderr, "Could not create server socket.\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    int yes = 1;

#ifndef _WIN32
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
#else
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
               (const char *)&yes, sizeof yes);
#endif

    struct sockaddr_in address;
    memset(&address, 0, sizeof address);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((unsigned short)port);

    if (bind(server, (struct sockaddr *)&address, sizeof address)
            == SOCKET_ERROR ||
        listen(server, 10) == SOCKET_ERROR) {

        fprintf(stderr, "Could not start server on port %d.\n", port);
        CLOSESOCKET(server);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    printf("ContactHub server started on port %d\n", port);
    printf("Loaded sample contacts. Press Ctrl+C to stop.\n");
    fflush(stdout);

    for (;;) {
        socket_t client = accept(server, NULL, NULL);

        if (client == INVALID_SOCKET)
            continue;

        handle_client(client);
    }

    CLOSESOCKET(server);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
