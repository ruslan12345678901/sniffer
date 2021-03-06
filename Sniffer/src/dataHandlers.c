#include "references.h"
#include "headerStructs.h"


void banner(){
    char banner[] = "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++                                                                                \n"
                    "|    ______     __   __     __     ______   ______   ______     ______           |\n"
                    "|   /\\  ___\\   /\\ \"-.\\ \\   /\\ \\   /\\  ___\\ /\\  ___\\ /\\  ___\\   /\\  == \\          |\n"
                    "|   \\ \\___  \\  \\ \\ \\-.  \\  \\ \\ \\  \\ \\  __\\ \\ \\  __\\ \\ \\  __\\   \\ \\  __<          |\n"
                    "|    \\/\\_____\\  \\ \\_\\\\\"\\_\\  \\ \\_\\  \\ \\_\\    \\ \\_\\    \\ \\_____\\  \\ \\_\\ \\_\\        |\n"
                    "|     \\/_____/   \\/_/ \\/_/   \\/_/   \\/_/     \\/_/     \\/_____/   \\/_/ /_/        |\n"
                    "|                                                                           v1.0 |\n"
                    "|                                                               by Ruslan Gajiev |\n"
                    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
    printf("%s" ,banner);
}


void dumptraffic(const unsigned char *data_buffer, const unsigned int length) {
    unsigned char byte;
    unsigned int i, j;
    for(i = 0; i < length; i++){
        byte = data_buffer[i];
        printf("%02x ", byte);
        if (((i%16) == 15) || (i == length - 1)){
            printf(" | ");
            for(j = (i - (i%16)); j <= i; j++){
                byte = data_buffer[j];
                if ((byte > 31)&&(byte < 127))
                    printf("%c", byte);
                else
                    printf(".");
            }
            printf("\n");
        }
    }
}


struct sockaddr_ll createSaddr(const unsigned char *buffer){
    ETHER_HDR *ether_ptr = (ETHER_HDR *)buffer;
    struct sockaddr_ll saddr;
    memset((void*)&saddr, 0, sizeof(saddr));
    saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    saddr.sll_halen = ETHER_ADDR_LEN;
    memcpy(&saddr.sll_addr, ether_ptr->ether_dst_addr, ETHER_ADDR_LEN);
    saddr.sll_ifindex = if_nametoindex("wlp7s0");
    return saddr;
}


void saveInPCAP(unsigned char *buffer, const unsigned int lenght){
    //Dump of packet's data in "*.pcap" format
    pcap_t *handle = pcap_open_dead(DLT_EN10MB, 65535);
    char pcap_file_way[128] = {0};
    char y[10];
    printf("Save dump of packet's data? (y/n) ");
    fgets(y, 10, stdin);
    if (strncmp(y, "y\n", sizeof(y)) == 0) {
        printf("\nEnter the path for pcap-dump(For_example:/dir1/dir2/file.pcap): ");
        fgets(pcap_file_way, sizeof(pcap_file_way), stdin);
        char *pos = strrchr(pcap_file_way, '\n');
        if (pos)
            *pos = 0;
        pcap_dumper_t *dumper = pcap_dump_open(handle, pcap_file_way);
        struct pcap_pkthdr pcap_hdr;
        pcap_hdr.caplen = lenght;
        pcap_hdr.len = pcap_hdr.caplen;
        pcap_dump((unsigned char *) dumper, &pcap_hdr, buffer);
        pcap_close(handle);
        pcap_dump_close(dumper);
    } else if (strncmp(y, "n\n", sizeof(y)) == 0) {
        printf("\n\nData won't be saved\n");
    } else {
        printf("\n\nWhat do you mean?\n");
    }
}



void packetSender(int check, unsigned char *buffer, struct sockaddr_ll saddr, int socket, const unsigned int lenght){
    if(check == 0){
        printf("\nSend modified protocol? (y/n) ");
        char y[10];
        fgets(y, sizeof(y), stdin);
        if (strncmp(y, "y\n", sizeof(y)) == 0){
            ssize_t send_packet = sendto(socket, buffer, lenght, 0, (struct sockaddr *)&saddr, sizeof(saddr));
            if(send_packet != -1){
                printf("\n]----------------==={PACKET WAS SENT}===----------------[\n\n");
            }
            else{
                printf("%u\n", errno);
                printf("%s", strerror(errno));
                exit(SADNESS);
            }
        }
        else{
            printf("\n]----------------==={PACKET WAS NOT SENT}===-----------------[\n\n");
        }
    }
}


void showDumpTraffic(unsigned char *buffer, unsigned int lenght, int totalHeaderSize){
    unsigned char *packetData = buffer + totalHeaderSize;
    unsigned int dataLenght = lenght - totalHeaderSize;
    if(dataLenght > 0){
        printf("Был перехвачен payload размером %d байтов:\n\v", dataLenght);
        dumptraffic(packetData, dataLenght);
    }

    else
        printf("[*]Данные отсутствуют.\n\v");
}
