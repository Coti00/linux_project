#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFSIZE 4096
#define SERVERPORT 7799
#define MAX_THREADS 4

pthread_t tid[MAX_THREADS];
int client_list[MAX_THREADS];
int count = 0;

void* counting(void *arg){
    int c_sock = *((int *) arg);

    char buf[BUFFSIZE];

    while(1){
        memset(buf,0,BUFFSIZE);

        if(recv(c_sock, buf, BUFFSIZE, 0) <= 0){
            printf("Client leaved: %d\n",c_sock);
            close(c_sock);

            for(int i =0; i < count;i++){
                if(client_list[i] == c_sock){
                    for(int j = i; j < count-1;j++){
                        client_list[j] = client_list[j+1];
                    }
                    count--;
                    break;
                }
            }
            pthread_exit(NULL);
        }

        printf("%s \n", buf);

        for(int i = 0 ; i <count;i++){
            if(client_list[i] != c_sock){
                if(send(client_list[i], buf, strlen(buf),0) == -1){
                    perror("Failed to send Message\n");
                }
            }
        }
    }
}

void *sendclient(void *arg){
    char say[BUFFSIZE];
    while(1){
        fgets(say,sizeof(say),stdin);

        size_t len = strlen(say);
        if(len > 0 && say[len - 1] == '\n'){
            say[len-1] = '\0';
        }
        
        for(int i = 0; i < count;i++){
            if(send(client_list[i],say,strlen(say),0) == -1){
                perror("Failed to send Message\n");
            }
        }
    }
    return NULL;
}

int main(void){
    int s_sock, *c_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size; 
    pthread_t input;
    
    s_sock = socket(AF_INET, SOCK_STREAM,0); 

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //소켓 이름 지정
    if(bind(s_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("Failed to bind a socket\n");
        close(s_sock);
        exit(1);
    }

    listen(s_sock,MAX_THREADS);

    printf("Server listening on port %d\n",SERVERPORT);

    if(pthread_create(&input,NULL,sendclient,NULL) !=0){
        perror("Failed to create\n");
        close(s_sock);
        exit(1);
    }
    
    c_addr_size = sizeof(struct sockaddr);
    while(1){
        c_sock = malloc(sizeof(int));
        *c_sock = accept(s_sock, (struct sockaddr*)&client_addr, &c_addr_size);
        if(*c_sock == -1){

            perror("Cant accept a connection\n");
            free(c_sock);
            continue;
        }
        printf("New client connected: %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        client_list[count++] = *c_sock;

        if(pthread_create(&tid[count-1], NULL, counting, c_sock) != 0){
            perror("Failed to create thread\n");
            close(*c_sock);
            free(c_sock);
            count--;
        }else{
            pthread_detach(tid[count-1]);
        }
    }

    close(s_sock);
    return 0;
}