#define PORT 31337
#define BUFSIZE 256
#define JPGBUF 455
#define GOODRX 1
#define BADRX 0

struct jpgPacket {
    unsigned char lastPkt;
    long unsigned checksum;
    unsigned int seqNum;
    unsigned short bytesSent;
    unsigned char buffer[JPGBUF];
} __attribute__((__packed__));
