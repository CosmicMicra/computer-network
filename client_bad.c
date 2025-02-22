#include "protocol.h"

int sockfd;
struct sockaddr_in server_addr;
unsigned char client_id = 1;

// Send packet with timer and retry mechanism
int send_with_timer(struct data_packet *packet) {
    struct ack_packet ack;
    socklen_t server_len = sizeof(server_addr);
    int retries = 0;
    
    // Set up timer
    struct timeval tv;
    tv.tv_sec = ACK_TIMEOUT;
    tv.tv_usec = 0;
    
    while (retries < MAX_RETRIES) {
        // Set socket timeout
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            perror("Error setting timeout");
            return -1;
        }
        
        printf("\nSending Packet %d (Attempt %d):\n", packet->seg_no, retries + 1);
        print_packet_info(packet);
        
        // Send packet
        if (sendto(sockfd, packet, sizeof(struct data_packet), 0, 
                   (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("Send failed");
            return -1;
        }
        
        // Wait for response (ACK or REJECT)
        int recv_result = recvfrom(sockfd, &ack, sizeof(struct ack_packet), 0, 
                                 (struct sockaddr*)&server_addr, &server_len);
        
        if (recv_result < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                printf("Timeout for packet %d\n", packet->seg_no);
                retries++;
                
                if (retries == MAX_RETRIES) {
                    printf("Server does not respond\n");
                    return -1;
                }
                continue;
            }
            perror("Receive failed");
            return -1;
        }
        
        // Check packet type
        if (((struct ack_packet*)&ack)->type == ACK_PACKET) {
            printf("Received ACK for packet %d\n", packet->seg_no);
            return 1;
        } else if (((struct reject_packet*)&ack)->type == REJECT_PACKET) {
            printf("Received REJECT for packet %d - ", packet->seg_no);
            switch(((struct reject_packet*)&ack)->reject_subcode) {
                case REJECT_OUT_OF_SEQUENCE:
                    printf("Out of sequence\n");
                    break;
                case REJECT_LENGTH_MISMATCH:
                    printf("Length mismatch\n");
                    break;
                case REJECT_END_OF_PACKET_MISSING:
                    printf("End of packet missing\n");
                    break;
                case REJECT_DUPLICATE_PACKET:
                    printf("Duplicate packet\n");
                    break;
            }
            return -2;
        }
    }
    
    return 0;
}

// Function for Part 1: Normal packets
void send_normal_packets() {
    printf("\n=== Part 1: Sending 5 Normal Packets ===\n");
    for (int i = 1; i <= 5; i++) {
        struct data_packet packet;
        char payload[50];
        
        snprintf(payload, sizeof(payload), "Packet %d", i);
        create_data_packet(&packet, client_id, i, payload);
        
        if (send_with_timer(&packet) < 0) {
            printf("Failed to send packet %d\n", i);
            break;
        }
        sleep(1);
    }
}

// Function for Part 2: Error packets
void send_error_packets() {
    printf("\n=== Part 2: Testing Error Cases ===\n");
    struct data_packet packet;

    // Send one correct packet first
    printf("\nSending correct packet...\n");
    create_data_packet(&packet, client_id, 1, "First correct packet");
    send_with_timer(&packet);
    sleep(1);

    // Test out of sequence
    printf("\nTesting Out of Sequence Error...\n");
    create_data_packet(&packet, client_id, 3, "Out of sequence");
    send_with_timer(&packet);
    sleep(1);

    // Test length mismatch
    printf("\nTesting Length Mismatch Error...\n");
    create_data_packet(&packet, client_id, 2, "Max payload exceeded");
    packet.length = 0xFF;  
    send_with_timer(&packet);
    sleep(1);

    // Test end packet missing
    printf("\nTesting End of Packet Missing Error...\n");
    create_data_packet(&packet, client_id, 4, "Bad end packet");
    packet.end_id = 0xFFF0;
    send_with_timer(&packet);
    sleep(1);

    // Test duplicate packet
    printf("\nTesting Duplicate Packet Error...\n");
    create_data_packet(&packet, client_id, 1, "Duplicate");
    send_with_timer(&packet);
}

int main() {
    // Socket setup code
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Run Part 1
    send_normal_packets();
    
    // Small delay between parts
    sleep(2);
    
    // Run Part 2
    send_error_packets();
    
    // Display timer info
    printf("\n=== Part 3: Timer and Retry Mechanism ===\n");
    printf("Timer is set to %d seconds with %d retries\n", ACK_TIMEOUT, MAX_RETRIES);
    printf("Timer functionality was demonstrated throughout all sends\n");
    
    close(sockfd);
    return 0;
}