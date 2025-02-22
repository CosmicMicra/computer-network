#include "protocol.h"

// Global variables
int sockfd;
struct sockaddr_in server_addr;
unsigned char client_id = 1;  // Using client ID 1

// Function to send a packet and wait for ACK
int send_and_wait_ack(struct data_packet *packet) {
    struct ack_packet ack;
    struct reject_packet reject;
    socklen_t server_len = sizeof(server_addr);
    int retries = 0;
    
    // Send packet
    printf("\nSending Packet %d:\n", packet->seg_no);
    printf("Client ID: %d\n", packet->client_id);
    printf("Segment No: %d\n", packet->seg_no);
    printf("Length: %d\n", packet->length);
    printf("Payload: %s\n", packet->payload);
    
    if (sendto(sockfd, packet, sizeof(struct data_packet), 0, 
               (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Send failed");
        return -1;
    }
    
    int result = wait_for_ack(sockfd, packet, &server_addr, &ack);
    if (result > 0) {
        printf("Received ACK for packet %d\n", ack.seg_no);
    }
    return result;
}

    // Wait for ACK
    //if (recvfrom(sockfd, &ack, sizeof(struct ack_packet), 0, 
    //             (struct sockaddr*)&server_addr, &server_len) < 0) {
     //   perror("ACK receive failed");
    //    return -1;}
    
    // Verify ACK
    //if (ack.type == ACK_PACKET && ack.seg_no == packet->seg_no) {
    //   printf("Received ACK for packet %d\n", ack.seg_no);
     // return 1;}
    

int main() {
    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    printf("Part 1: Sending 5 normal packets to server...\n");
    
    // Create and send 5 packets
    for (int i = 1; i <= 5; i++) {
        struct data_packet packet;
        char payload[50];
        
        // Prepare payload
        snprintf(payload, sizeof(payload), "This is packet %d", i);
        
        // Create packet using protocol function
        create_data_packet(&packet, client_id, i, payload);
        
        // Send packet and wait for ACK
        if (send_and_wait_ack(&packet) < 0) {
            printf("Failed to send packet %d\n", i);
            break;
        }
        
        // Small delay between packets
        sleep(1);
    }
    
    close(sockfd);
    return 0;
}