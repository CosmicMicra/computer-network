# COEN 233 Programming Assignment 1


## Overview
Client using customized protocol on top of UDP protocol for sending information to the server.

## Features
* UDP-based communication.
* Custom packet format with error handling.
* Acknowledgment mechanism.
* Implements `ack_timer` for retransmissions (3-second timeout, 3 retries).

## Files Included
`protocol.h`: Header file with protocol definitions.
`protocol.c`: Implementation of protocol functions.
`client.c`: Client implementation for sending data packets.
`server.c`: Server implementation that verifies and acknowledges packets.
`timer.c`: Implementation of timeout functionality.
`Makefile`: Build configuration.

## How to Run
1. **Compile**

```

make clean && make

```

2. **Run Server**

```

./server

```
3. **Run Client** (On different terminal)
```

./client

```

4. **Client sends packets to server**
   * First sends 5 normal packets
   * Then sends 5 packets with various errors

5. **Server Responds**
   *  ACK for correctly received packets
   *  REJECT for packets with errors:
      *    Out of sequence (0xFFF4)
        * Length mismatch (0xFFF5)
        * End of packet missing (0xFFF6)
        * Duplicate packet (0xFFF7)

6. **Timer functionality**
   * Client retransmits if no ACK received within 3 seconds
   * Displays "Server does not respond" after 3 failed retries

