#include "protocol.h"

int wait_for_ack(int sockfd, struct data_packet *packet, 
                 struct sockaddr_in *server_addr, struct ack_packet *ack) {
    struct timeval tv;
    socklen_t server_len = sizeof(*server_addr);
    int retries = 0;
    
    // Set timer value
    tv.tv_sec = ACK_TIMEOUT;
    tv.tv_usec = 0;
    
    while (retries < MAX_RETRIES) {
        // Set socket timeout
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            perror("Error setting timeout");
            return -1;
        }

        // Wait for ACK
        int recv_result = recvfrom(sockfd, ack, sizeof(struct ack_packet), 0, 
                                 (struct sockaddr*)server_addr, &server_len);
                                 
        if (recv_result < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                printf("ACK timeout for packet %d (Attempt %d)\n", 
                       packet->seg_no, retries + 1);
                retries++;
                
                if (retries == MAX_RETRIES) {
                    printf("Server does not respond\n");
                    return -1;
                }
                
                // Resend packet
                if (sendto(sockfd, packet, sizeof(struct data_packet), 0, 
                       (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0) {
                    perror("Resend failed");
                    return -1;
                }
                continue;
            }
            perror("ACK receive failed");
            return -1;
        }
        
        // Check if received correct ACK
        if (ack->type == ACK_PACKET && ack->seg_no == packet->seg_no) {
            return 1;  // Success
        }
    }
    
    return 0;  // Failed after all retries
}