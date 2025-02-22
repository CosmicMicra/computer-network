#include "protocol.h"

int sockfd;
struct sockaddr_in server_addr, client_addr;
int expected_seq_no = 1;
int received_packets[255] = {0};




void reset_server_state() {
    expected_seq_no = 1;
    memset(received_packets, 0, sizeof(received_packets));
    printf("\n=== Resetting server state. Starting new sequence ===\n");
}




// Function to send ACK
void send_ack(unsigned char client_id, unsigned char seg_no) {
    struct ack_packet ack;
    create_ack_packet(&ack, client_id, seg_no);
    
    if (sendto(sockfd, &ack, sizeof(ack), 0,
           (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
            perror("Send ACK failed");}

    printf("Sent ACK for packet %d\n", seg_no);
}


//Reject Function
void send_reject(unsigned char client_id, unsigned char seg_no, 
                unsigned short reject_subcode) {
    struct reject_packet reject;
    create_reject_packet(&reject, client_id, seg_no, reject_subcode);
    
    if (sendto(sockfd, &reject, sizeof(reject), 0,
               (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
        perror("Send REJECT failed");
    }
    
    // Print reject reason
    printf("Sent REJECT for packet %d - ", seg_no);
    switch(reject_subcode) {
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



}

int check_packet_errors(struct data_packet *packet) {
    // Check for duplicate packet
    if (received_packets[packet->seg_no] == 1) {
        printf("Error: Duplicate packet %d\n", packet->seg_no);
        send_reject(packet->client_id, packet->seg_no, REJECT_DUPLICATE_PACKET);
        return -1;
    }
    
    // Check sequence number
    if (packet->seg_no != expected_seq_no) {
        printf("Error: Out of sequence - Expected %d, got %d\n", 
            expected_seq_no, packet->seg_no);
        send_reject(packet->client_id, packet->seg_no, REJECT_OUT_OF_SEQUENCE);
        return -1;
    }
    
    // Check packet length
    if (strlen(packet->payload) != packet->length) {
        printf("Error: Length mismatch - Expected %d, got %ld\n", 
            packet->length, strlen(packet->payload));
        send_reject(packet->client_id, packet->seg_no, REJECT_LENGTH_MISMATCH);
        return -1;
    }
    
    // Check end of packet identifier
    if (packet->end_id != END_PACKET) {
        printf("Error: Invalid end packet identifier\n");
        send_reject(packet->client_id, packet->seg_no, REJECT_END_OF_PACKET_MISSING);
        return -1;
    }
    
    return 0;  // No errors
}



int main() {
    socklen_t client_len = sizeof(client_addr);
    struct data_packet recv_packet;
    //int packets_received = 0;
    
    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    
    // Bind socket
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }


    //prints
    
   printf("Server started on port %d...\n", SERVER_PORT);
    
    expected_seq_no = 1;
    memset(received_packets, 0, sizeof(received_packets));

    while (1) {  


        // Receive packet
        printf("\nWaiting for packet (Expected Seq No: %d)...\n", expected_seq_no);
        ssize_t n = recvfrom(sockfd, &recv_packet, sizeof(struct data_packet), 0,
                            (struct sockaddr*)&client_addr, &client_len);
        
        if (n < 0) {
            perror("Receive failed");
            continue;
        }
        
        // Print received packet info
        printf("\nReceived Packet:\n");
        print_packet_info(&recv_packet);
        if (!validate_packet(&recv_packet)) {
            printf("Received invalid packet\n");
            continue;
        }

    
        if (check_packet_errors(&recv_packet) == 0) {
            send_ack(recv_packet.client_id, recv_packet.seg_no);
            received_packets[recv_packet.seg_no] = 1;
            expected_seq_no = recv_packet.seg_no + 1; 
            printf("Packet accepted. Next expected sequence number: %d\n", expected_seq_no);

        if (expected_seq_no == 6) {
        reset_server_state();
        {printf("\n=== Starting new sequence (Part 2) ===\n");};  // This sets expected_seq_no to 1 and clears received_packets
    }

        } else {
            // Error was found and reject was sent by check_packet_errors
            printf("Packet rejected. Still expecting sequence number: %d\n", expected_seq_no);
        }

        
        printf("Server continuing to run...\n");
    }
    
    close(sockfd);
    return 0;
}



