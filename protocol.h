#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>



/* Protocol Constants */
#define START_PACKET 0XFFFF      // Marks start of packet
#define END_PACKET 0XFFFF        // Marks end of packet
#define MAX_CLIENT_ID 0XFF       // Max client ID (255)
#define MAX_LENGTH 0XFF          // Max payload length (255)




/* Packet Types */
#define DATA_PACKET 0XFFF1       
#define ACK_PACKET 0XFFF2        
#define REJECT_PACKET 0XFFF3     

/* Reject Sub Codes - for error handling */
#define REJECT_OUT_OF_SEQUENCE 0XFFF4    // Packet received out of order
#define REJECT_LENGTH_MISMATCH 0XFFF5    // Payload length doesn't match length field
#define REJECT_END_OF_PACKET_MISSING 0XFFF6  // End packet identifier missing
#define REJECT_DUPLICATE_PACKET 0XFFF7    // Duplicate packet received

/* Timer Constants */
#define ACK_TIMEOUT 3    // 3 seconds timeout
#define MAX_RETRIES 3    // Maximum 3 retries


/* Network Settings */
#define SERVER_PORT 8080
#define BUFFER_SIZE 512


struct data_packet {
    unsigned short start_id;    // 2 bytes
    unsigned char client_id;    // 1 byte
    unsigned short type;        // 2 bytes
    unsigned short seg_no;      // 2 bytes
    unsigned char length;       // 1 byte
    char payload[255];   // Variable length array (max 255 bytes)
    unsigned short end_id;      // 2 bytes
};


struct ack_packet {
    unsigned short start_id;    // 2 bytes
    unsigned char client_id;    // 1 byte
    unsigned short type;        // 2 bytes
    unsigned char seg_no;       // 1 byte
    unsigned short end_id;      // 2 bytes
};


struct reject_packet {  // Make sure this structure is defined
    unsigned short start_id;
    unsigned char client_id;
    unsigned short type;
    unsigned char seg_no;
    unsigned short reject_subcode;  // For storing the reject reason
    unsigned short end_id;
};

/* Function Declarations */
// Creates a new data packet with given parameters
void create_data_packet(struct data_packet *packet, 
                       unsigned char client_id,
                       unsigned short seg_no, 
                       const char *payload);

// Creates an acknowledgment packet
void create_ack_packet(struct ack_packet *packet,
                      unsigned char client_id,
                      unsigned char seg_no);

// Validates a received packet's format
int validate_packet(const struct data_packet *packet);

// Utility function to print packet contents (for debugging)
void print_packet_info(const struct data_packet *packet);


//Timer 
int wait_for_ack(int sockfd, struct data_packet *packet, 
                 struct sockaddr_in *server_addr, struct ack_packet *ack);


void create_reject_packet(struct reject_packet *packet, unsigned char client_id,
                         unsigned char seg_no, unsigned short reject_subcode); 


#endif