#include <sys/types.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


int main() { 

    char server_message[256] = "2 058 2950 1589989296\n";
    fd_set mask;

    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int binder = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    listen(server_socket, 5);

    int client_socket;

    int max_fd = -1;

    while((client_socket = accept(server_socket, NULL, NULL)) > 0) {
        
        FD_ZERO(&mask);

        FD_SET(server_socket, &mask);
        if (max_fd < server_socket) { 
            max_fd = server_socket;
        }

        FD_SET(client_socket, &mask);
        if (max_fd < client_socket) { 
            max_fd = client_socket;
        }

        int select_ready = select(max_fd + 1, &mask, NULL, NULL, NULL);
        

        if (FD_ISSET(client_socket, &mask)) { 
            send(client_socket, server_message, sizeof(server_message), 0);
        }

        if (FD_ISSET(server_socket, &mask)) { 
            char *server_message2;
            recv(server_socket, &server_message2, sizeof(server_message2), 0);
            send(client_socket, server_message2, sizeof(server_message2), 0);
        }




        close(server_socket);

        sleep(15);

    }

    return 0;




}
