#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <zlib.h>
#include "imgStream.h"

/*
struct jpgPacket {
    long unsigned checksum;
    unsigned int seqNum;
    unsigned char buffer[458];
} __attribute__((__packed__));
*/

int main() {
    struct jpgPacket packetStruct;
    int broadcastPermission = 1;
    int socketfd; /*socket descriptor*/
    int fileSize;
    unsigned long crc;
	struct sockaddr_in myaddr;	/* our address */
    struct sockaddr_in sa_broadcast; /*socketaddr struct*/
    struct hostent *target_addr;
    unsigned char buffer[1024 * 1024] = {0};
    char *target_host = "255.255.255.255";
    int bytesSent = 0;
    /*create the socket*/
    socketfd = socket(AF_INET,SOCK_DGRAM,0);
    assert(socketfd > -1);
    /*bind to an address  */
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);
    assert(bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) > -1);
    assert(setsockopt(socketfd, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission,sizeof(broadcastPermission)) > -1);
    int i,j,k = 0;
    /*hose out the sockaddr struct and set options*/
    memset((char *) &sa_broadcast, 0, sizeof(sa_broadcast));
    sa_broadcast.sin_family = AF_INET;
    sa_broadcast.sin_port  = htons(31337); 
    target_addr = gethostbyname(target_host);
    assert(inet_aton("255.255.255.255", &sa_broadcast.sin_addr) != 0);
    /*set the target address in the packet*/
	memcpy((void *)&sa_broadcast.sin_addr, target_addr->h_addr_list[0], target_addr->h_length);
    assert(connect(socketfd, (struct sockaddr *)&sa_broadcast, sizeof(sa_broadcast)) > -1);
    /*open the file we'll be sending*/
    FILE *jpgFile;
    jpgFile=fopen("bad-cat.jpg","rb");
    /*get file size*/
    fseek(jpgFile,0L,SEEK_END);
    fileSize = ftell(jpgFile);
    fseek(jpgFile,0L,SEEK_SET);
    /*copy the file into the buffer*/
    fread(&buffer,fileSize,1,jpgFile);
    fclose(jpgFile);
    packetStruct.seqNum = 0;
    packetStruct.lastPkt = 0;
    for (j = 0; j < fileSize; j += JPGBUF) {
        for (k = 0; k < JPGBUF; k ++) {
            bytesSent ++;
            packetStruct.buffer[k] = buffer[j + k];
            if ((j + k) == (fileSize - 1)) {
                packetStruct.lastPkt = 1;
            }
        }
        packetStruct.bytesSent = bytesSent;
        bytesSent = 0;
        crc = crc32(0,packetStruct.buffer,JPGBUF);
        packetStruct.checksum = crc;
        sendto(socketfd, &packetStruct, sizeof(struct jpgPacket) , 0 , (struct sockaddr *) &sa_broadcast, sizeof(sa_broadcast));
        memset((char *) &packetStruct.buffer, 0, JPGBUF);
        printf("Sending packet %d\n", packetStruct.seqNum);
        packetStruct.seqNum ++;
    }
	return 0;
}
