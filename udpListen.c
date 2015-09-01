#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <zlib.h>
#include <unistd.h>
#include "imgStream.h"



void writeToFile(char buffer[1024 * 1024], unsigned int fileLen) {
    static unsigned int fileNum = 0;
    char  filename[14];
    FILE *jpgFile;
    sprintf(filename, "%0u.jpg", fileNum);
    printf("%s\n", filename);
    jpgFile = fopen(filename,"wb");
    fwrite(buffer, fileLen, 1, jpgFile);
    fclose(jpgFile);
    fileNum ++;
    return;
}

int main() {
    int recvlen;
    int socketfd; /*socket descriptor*/
    struct jpgPacket pktStruct;
	struct sockaddr_in myaddr;	/* our address */
    struct sockaddr_in sa_broadcast; /*socketaddr struct*/
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr); 
    struct hostent *target_addr;
    char buffer[sizeof(struct jpgPacket)] = {0};
    unsigned long crc;
    unsigned char fileBuffer[1024 * 1024] = {0};
    unsigned int writeCount = 0;
    unsigned int writeOffset = 0;
    unsigned int expectedPacket = 0;
    unsigned char state = GOODRX;
    unsigned int bytesRx = 0;
    /*create the socket*/
    socketfd = socket(AF_INET,SOCK_DGRAM,0);
    assert(socketfd > -1);
    /*bind to an address  */
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);
    assert(bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) > -1);
    for (;;) {
        recvlen = recvfrom(socketfd, buffer, sizeof(struct jpgPacket), 0, (struct sockaddr *)&remaddr, &addrlen);
	    memcpy((void *)&pktStruct, &buffer, sizeof(struct jpgPacket));
        crc = crc32(0,pktStruct.buffer,JPGBUF);
        if (pktStruct.seqNum == 0 && crc == pktStruct.checksum) {
            state = GOODRX;
        }
        if (expectedPacket == pktStruct.seqNum && crc == pktStruct.checksum && state == GOODRX) {
            expectedPacket ++;
            for (writeCount = 0; writeCount < pktStruct.bytesSent; writeCount ++) {
                fileBuffer[writeCount + writeOffset] = pktStruct.buffer[writeCount];
                bytesRx ++;
            }
            writeOffset += pktStruct.bytesSent;
        } else {
            state = BADRX;
        }
        if (state == GOODRX && pktStruct.lastPkt == 1) {
            writeToFile(fileBuffer,bytesRx);
            bytesRx = 0;
            expectedPacket = 0;
        }
    }
}
