#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define DATA_FILE "/dev/urandom"
#define SERVER_ADDR "172.30.1.61"
#define SERVER_PORT 9300
//#define DATA_FILE "/home/janghoon/examples.desktop"

typedef struct {
    unsigned int buffer_size;
    unsigned int num_times;
} flags;

int allocate_buffer(char **p_buff, unsigned int buff_size){
    *p_buff = (char *)malloc(buff_size);
    if(p_buff == NULL){
        perror("Memeory allocation failed.");
        return 0;
    }else{
        return 1;
    }
}
int read_data(int fp, char **p_buff, unsigned int buff_size){
    if (read(fp, *p_buff, buff_size) == -1){
        perror("Read data failed.");
        return 0;
    }else{
        return 1;
    }
}
int copy_buff(char *p_buff_src, char *p_buff_dst, unsigned int buff_size){
    memcpy(p_buff_dst, p_buff_src, buff_size);
    return 1;
}
int send_message(char *p_buff, unsigned int buff_size, int sockfd, struct sockaddr_in servAddr){
    if(sendto(sockfd, p_buff, buff_size, 0, (struct sockaddr*)&servAddr, sizeof(servAddr)) != buff_size) {
        perror("Sendto failed.");
        return 0;
    }
    return 1;
}
int main(int argc, char **argv) {
    int opt;
    int i;
    int fp;
    int sockfd;
    struct sockaddr_in servAddr;
    

    char *p_buff_file = NULL;
    char *p_buff_udp = NULL;
    flags conf;

    conf.buffer_size = 4096;
    conf.num_times = 1;

    while((opt = getopt(argc, argv, "n:b:")) != -1){
        switch(opt){
            case 'b':
                conf.buffer_size = atoi(optarg);
                break;
            case 'n':
                conf.num_times = atoi(optarg);
                break;
        }
    }
    fp = open(DATA_FILE,O_RDONLY);
    if (!fp){
        perror("Failed to open the data file.");
        exit(1);
    }
    if((sockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
       perror("Failed to create a UDP socket.");
       exit(1);
    }
    memset(&servAddr, 0, sizeof(servAddr));
    /* servAddr에 IP 주소와 포트 번호를 저장 */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    servAddr.sin_port = htons(SERVER_PORT);

    for (i = 0; i < conf.num_times; i++){
        // buffer allocation. 
        // p_buff_file : data from DATA_FILE
        // p_buff_udp : data to send to UDP
        allocate_buffer(&p_buff_file, conf.buffer_size);
        allocate_buffer(&p_buff_udp, conf.buffer_size);
        read_data(fp, &p_buff_file, conf.buffer_size);
        copy_buff(p_buff_file, p_buff_udp, conf.buffer_size);
        send_message(p_buff_udp, conf.buffer_size, sockfd, servAddr);
        free((void *)p_buff_file);
        free((void *)p_buff_udp);
        lseek(fp, 0, SEEK_SET); 
    }
    close(fp);
    
    //printf("SRC String : %s\n", p_buff_file);
    //printf("DST String : %s\n", p_buff_udp);
    printf("Completed. buffer_size : %i, num_times : %i\n", conf.buffer_size, conf.num_times);
    exit(0);
}