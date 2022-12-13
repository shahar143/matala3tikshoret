//
// Created by shahar on 12/4/22.
//


#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define SERVER_PORT 5060
#define FILE_SIZE_IN_BYTES 1979599
#define CHUNK_SIZE 10000


void receive_file(int client_socket, int server_socket_fd, char* file_name);
void send_message_to_client(char* message, int client_socket, int server_socket_fd, int message_len);

int main(){
    int dtaz1 = 0700;
    int dtaz2 = 8577;
    int xor = dtaz1 ^ dtaz2;

    signal(SIGPIPE, SIG_IGN);

    //creating the server socket with the function socket()
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket_fd == -1){
        printf("Could not create socket: %d\n", errno);
    }


    int enableReuse = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) == -1){
        printf("setsockopt() failed with error code : %d\n", errno);
        return 1;
    }

    //creating the struct with the name and the address of the server
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY; // any IP at this port (Address to accept any incoming messages)
    server_address.sin_port = htons(SERVER_PORT);  // network order (makes byte order consistent)

    int bind_res = bind(server_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    //if bind worked, bind_res == 0
    if(bind_res == -1){
        printf("Bind failed with error code : %d", errno);
        // close the socket
        close(server_socket_fd);
        return -1;
    }

    int listen_value = listen(server_socket_fd, 1);
    //if listen worked, then listen_value == 0.
    if(listen_value == -1){
        printf("listen didn't work with code: %d", errno);
        close(server_socket_fd);
        exit(1);
    }

    //declaring a new struct for the client port and ip. accept will fill these in the struct
    printf("Waiting for incoming TCP-connections...\n");
    struct sockaddr_in client_address;  //
    socklen_t client_address_len = sizeof(client_address);


    memset(&client_address, 0, sizeof(client_address));
    client_address_len = sizeof(client_address);
    int client_socket = accept(server_socket_fd, (struct sockaddr *)&server_address, &client_address_len) ;
    if(client_socket == -1){
        printf("accept didn't work with code: %d", errno);
        close(server_socket_fd);
        exit(1);
    }

    char* message = "Welcome to our TCP-server\n";
    int message_len = strlen(message) + 1;

    send_message_to_client(message, client_socket, server_socket_fd, message_len);

    receive_file(client_socket, server_socket_fd, "destbut.txt");

    message = "look at me!! I've received the first part :)\n";
    message_len = strlen(message) + 1;

    send_message_to_client(message, client_socket, server_socket_fd, message_len);

    ///////////////////////////////////////////////////////////////////////////////////////////////////

    int num_message = xor;

    int bytes_sent = send(client_socket, &num_message, sizeof(num_message), 0);
    if (bytes_sent == -1) {
        printf("send() failed with error code : %d\n", errno);
        close(server_socket_fd);
        close(client_socket);
        exit(1);
    }
    else if (bytes_sent == 0) {
        printf("peer has closed the TCP connection prior to send().\n");
    }
    else if (bytes_sent < sizeof(num_message)) {
        printf("sent only %d bytes from the required %d.\n", num_message, bytes_sent);
    }
    else {
        printf("message was successfully sent.\n");
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    sleep(3);

    receive_file(client_socket, server_socket_fd, "destbut2.txt");

    message = "look at me!! I've received the second part :)\n";
    message_len = strlen(message) + 1;

    send_message_to_client(message, client_socket, server_socket_fd, message_len);

    close(server_socket_fd);

    return 0;
}

void receive_file(int client_socket, int server_socket_fd, char* file_name){
    int bytes_counter = 0;
    int bytes_received = 0;

    char* receive_space = (char *)calloc((FILE_SIZE_IN_BYTES/2), sizeof(char));
    if(receive_space == NULL){
        printf("place wasn't allocated!!\n");
        exit(-1);
    }

    printf("starting to receive\n");
    time_t time_stamp1 =time(NULL);

    while(bytes_counter + CHUNK_SIZE <= FILE_SIZE_IN_BYTES/2){
        bytes_received = 0;
        bytes_received = recv(client_socket, receive_space, CHUNK_SIZE, 0);
        if (bytes_received == -1) {
            printf("recv failed with error code : %d\n", errno);
            // close the sockets
            close(server_socket_fd);
            close(client_socket);
            exit(1);
        }
        bytes_counter += bytes_received;
    }

    bytes_received = recv(client_socket, receive_space, FILE_SIZE_IN_BYTES/2 - bytes_counter, 0);
    if (bytes_received == -1) {
        printf("recv failed with error code : %d\n", errno);
        // close the sockets
        close(server_socket_fd);
        close(client_socket);
        exit(1);
    }

    time_t time_stamp2 = time(NULL);

    printf("time of sending: %ld\n", time_stamp1);
    printf("time of receiving: %ld\n", time_stamp2);
    time_t sent_time = time_stamp2- time_stamp1;
    printf("sending the file took %ld seconds\n", sent_time);

    printf("bytes received: %d\n", bytes_counter + bytes_received);

    FILE *file = fopen(file_name, "w");

    // write the char array to the file
    fwrite(receive_space, sizeof(char), FILE_SIZE_IN_BYTES/2, file);

    // close the file
    fclose(file);

    printf("check that first file is good: %s\n", file_name);

    free(receive_space);
}

void send_message_to_client(char* message, int client_socket, int server_socket_fd, int message_len){
    int bytes_sent = send(client_socket, message, message_len, 0);
    if (bytes_sent == -1) {
        printf("send() failed with error code : %d\n", errno);
        close(server_socket_fd);
        close(client_socket);
        exit(1);
    }
    else if (bytes_sent == 0) {
        printf("peer has closed the TCP connection prior to send().\n");
    }
    else if (bytes_sent < message_len) {
        printf("sent only %d bytes from the required %d.\n", message_len, bytes_sent);
    }
    else {
        printf("message was successfully sent.\n");
    }
}

