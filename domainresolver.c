// Coded
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT_HTTPS 443

typedef struct Node {
    char ip[INET6_ADDRSTRLEN];  
    struct Node *next;
} Node;

Node *front = NULL, *rear = NULL;

void enqueue(const char *ip) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    strcpy(newNode->ip, ip);
    newNode->next = NULL;

    if (rear == NULL) {
        front = rear = newNode;
    } else {
        rear->next = newNode;
        rear = newNode;
    }
}

char *dequeue() {
    if (front == NULL) return NULL;

    Node *temp = front;
    char *ip = strdup(front->ip);
    
    front = front->next;
    if (front == NULL) rear = NULL;

    free(temp);
    return ip;
}

int isPortOpen(const char *ip, int port) {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    int result;

    WSAStartup(MAKEWORD(2, 2), &wsa);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return 0;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server.sin_addr);

    result = connect(sock, (struct sockaddr *)&server, sizeof(server));
    closesocket(sock);
    WSACleanup();

    return (result == 0) ? 1 : 0;
}

void resolveDomain(const char *domain) {
    WSADATA wsa;
    struct addrinfo hints, *res, *p;
    char ipStr[INET6_ADDRSTRLEN];

    WSAStartup(MAKEWORD(2, 2), &wsa);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  // Supports both IPv4 & IPv6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(domain, NULL, &hints, &res) != 0) {
        printf("[ERROR] Failed to resolve domain: %s\n", domain);
        WSACleanup();
        return;
    }

    printf("\n=== Resolved IPs for domain: %s ===\n", domain);

    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        if (p->ai_family == AF_INET) {  // IPv4
            addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
        } else {  // IPv6
            addr = &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr;
        }

        inet_ntop(p->ai_family, addr, ipStr, sizeof(ipStr));
        printf("[+] IP Found: %s (Added to Queue)\n", ipStr);
        enqueue(ipStr);
    }

    freeaddrinfo(res);
    WSACleanup();
}

void processQueue() {
    char *ip;
    
    while ((ip = dequeue()) != NULL) {
        printf("\nChecking IP: %s\n", ip);

        if (isPortOpen(ip, PORT_HTTPS))
            printf("[SUCCESS] HTTPS Port 443 is OPEN on %s\n", ip);
        else
            printf("[FAILURE] HTTPS Port 443 is CLOSED on %s\n", ip);

        free(ip);
    }
}

int main() {
    char domain[256];

    printf("Enter domain name: ");
    scanf("%255s", domain);

    resolveDomain(domain);
    processQueue();

    printf("\nPress Enter to exit...");
    getchar();
    getchar();
    return 0;
}
