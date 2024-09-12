#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFSIZE 4096
#define SERVERPORT 7799

void* counting(void* arg){
    int c_sock = *((int*)arg);
    char buf[BUFFSIZE];
    
    while(1){
        memset(buf,0,BUFFSIZE);
        if(recv(c_sock,buf,BUFFSIZE,0) <= 0){
            perror("Cant access server\n");
            close(c_sock);
            exit(1);
        }
        printf("%s\n",buf);
    }
    return NULL;
}

int main(int argc, char *argv[]){
    int c_sock;
    struct sockaddr_in server_addr;

    char buf[BUFFSIZE];
    char say[BUFFSIZE];

    if(argc != 2){
        printf("Usage: ./client <name>\n");
        return 1;
    }

    char *name = argv[1];

    c_sock = socket(AF_INET, SOCK_STREAM,0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(c_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("Failed to connect server\n");
        close(c_sock);
        exit(1);
    }

    pthread_t tid;
    if(pthread_create(&tid, NULL, counting, &c_sock) != 0){
        perror("Failed to create thread\n");
        close(c_sock);
        exit(1);
    }

    while(1){
        fgets(say, sizeof(say),stdin);

        size_t len = strlen(say);
        if(len > 0 && say[len - 1] == '\n'){
            say[len-1] = '\0';
        }

        if(strcmp(say,"exit") == 0){
            close(c_sock);
            exit(1);
        }

        memset(buf,0,sizeof(buf));
        strcpy(buf,name);
        strcat(buf,": ");
        strncat(buf, say, BUFFSIZE - strlen(buf) - 1);
      

        if(send(c_sock, buf, strlen(buf), 0) == -1){
            perror("Failed to send Message\n");
            close(c_sock);
            exit(1);
        }
    }

    pthread_join(tid,NULL);
    close(c_sock);

    return 0;
}