#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

int PORT = 18080;
int flag = 0;
int DEBUG = 0;
char *HOST = "iot.dslab.pub.ds.open-cloud.xyz";

void removeSubstring(char *str, const char *sub) {
    size_t len = strlen(sub);
    char *p = str;
    
    while ((p = strstr(p, sub)) != NULL) {
        memmove(p, p + len, strlen(p + len) + 1);
    }
}

int main(int argc, char **argv) { 

    if (argc > 6) { 
        printf("argc is larger than 4\n");
        exit(1);
    }

    if (argc == 6) { 

        int count = 0;

        for (int i = 0; i < argc; i++) { 
            if (!strcmp(argv[i], "--host")){ 
                if (i < argc - 1) {
                    if (strncmp(argv[i+1], "--port", 6) && strncmp(argv[i+1], "--debug", 6)){ 
                        HOST = argv[i+1];
                        count++;
                    }
                    else { 
                        printf("wrong args\n");
                        exit(1);
                    }
                }
                else { 
                    printf("wrong args\n");
                    exit(1);
                }
            }

            if (!strcmp(argv[i], "--port")){ 
                if (i < argc - 1) {
                    if (strncmp(argv[i+1], "--host", 6) && strncmp(argv[i+1], "--debug", 6)){ 
                        PORT = atoi(argv[i+1]);
                        count++;
                    }
                    else { 
                        printf("wrong args\n");
                        exit(1);
                    }
                }
                else { 
                    printf("wrong args\n");
                    exit(1);
                }
            }

            if (!strcmp(argv[i], "--debug")){ 
                DEBUG = 1;
                count++;
            }
        }

        if (count != 3) { 
            printf("wrong args\n");
            exit(1);
        }
    }

    if (argc == 5) { 
        int count = 0;

        for (int i = 0; i < argc; i++) { 
            if (!strcmp(argv[i], "--host")){ 
                if (i < argc - 1) {
                    if (strncmp(argv[i+1], "--port", 6) && strncmp(argv[i+1], "--debug", 6)){ 
                        HOST = argv[i+1];
                        count++;
                    }
                    else { 
                        printf("wrong args\n");
                        exit(1);
                    }
                }
                else { 
                    printf("wrong args\n");
                    exit(1);

                }
            }

            if (!strcmp(argv[i], "--port")){ 
                if (i < argc - 1) {
                    if (strncmp(argv[i+1], "--host", 6) && strncmp(argv[i+1], "--debug", 6)){ 
                        PORT = atoi(argv[i+1]);
                        count++;
                    }
                    else { 
                        printf("wrong args\n");
                        exit(1);
                    }
                }
                else { 
                    printf("wrong args\n");
                    exit(1);
                }
            }
        }

        if (count != 2) { 
            printf("wrong args\n");
            exit(1);
        }


    }

    if (argc == 4) { 
        int count = 0;
        int arg_flag = 0;

        for (int i = 0; i < argc; i++) { 
            if (!strcmp(argv[i], "--host")){ 
                if (i < argc - 1) {
                    if (strncmp(argv[i+1], "--port", 6) && strncmp(argv[i+1], "--debug", 6)){ 
                        if (arg_flag == 0) {
                            HOST = argv[i+1];
                            count++;
                            arg_flag = 1;
                        }
                        else { 
                            printf("wrong args\n");
                            exit(1);
                        }
                    }
                }
            }

            if (!strcmp(argv[i], "--port")){ 
                if (i < argc - 1) {
                    if (strncmp(argv[i+1], "--host", 6) && strncmp(argv[i+1], "--debug", 6)){ 
                        if (arg_flag == 0) { 
                            PORT = atoi(argv[i+1]);
                            count++;
                            arg_flag = 1;
                        }
                        else { 
                            printf("wrong args\n");
                            exit(1);
                        }
                    }
                }
            }

            if (!strcmp(argv[i], "--debug")){ 
                DEBUG = 1;
                count++;
            }
        }

        if (count != 2) { 
            printf("wrong args\n");
            exit(1);
        }
    }

    if (argc == 3) { 
        int count = 0;
        int arg_flag = 0;

        for (int i = 0; i < argc; i++) { 
            if (!strcmp(argv[i], "--host")){ 
                if (i < argc - 1) {
                    if (strncmp(argv[i+1], "--port", 6) && strncmp(argv[i+1], "--debug", 6)){ 
                        if (arg_flag == 0) {
                            HOST = argv[i+1];
                            count++;
                            arg_flag = 1;
                        }
                        else { 
                            printf("wrong args 1\n");
                            exit(1);
                        }
                    }
                    else { 
                        printf("wrong args 2\n");
                        exit(1);
                    }
                }
                else { 
                    printf("wrong args 3\n");
                    exit(1);
                }
            }

            if (!strcmp(argv[i], "--port")){ 
                if (i < argc - 1) {
                    if (strncmp(argv[i+1], "--host", 6) && strncmp(argv[i+1], "--debug", 6)){ 
                        if (arg_flag == 0) { 
                            PORT = atoi(argv[i+1]);
                            count++;
                            arg_flag = 1;
                        }
                        else { 
                            printf("wrong args\n");
                            exit(1);
                        }
                    }
                    else { 
                        printf("wrong args\n");
                        exit(1);
                    }
                }
                else { 
                    printf("wrong args\n");
                    exit(1);
                }
            }

        }

        if (count != 1) { 
            printf("wrong args\n");
            exit(1);
        }
    }

    if (argc == 2) { 
        if (strcmp(argv[1], "--debug")){ 
            printf("Argv[1] is incorrect\n");
            exit(1);
        }

        DEBUG = 1;

    }

    struct hostent *server_host = gethostbyname(HOST);
    if (server_host == NULL) {
        perror("failed to resolve host name\n");
        exit(EXIT_FAILURE);
    }

    int network_socket;
    int max_fd = -1;

    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (network_socket < 0) { 
        perror(0);
        exit(1);
    }

    struct sockaddr_in server_address;
    fd_set mask;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    memcpy(&server_address.sin_addr, server_host->h_addr, server_host->h_length);

    int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    if (connection_status < 0) { 
        perror(0);
        exit(1);
    }

    while(1) { 

        FD_ZERO(&mask);

        FD_SET(network_socket, &mask);
        if (max_fd < network_socket) { 
            max_fd = network_socket;
        }

        FD_SET(STDIN_FILENO, &mask);
        if (max_fd < STDIN_FILENO) { 
            max_fd = STDIN_FILENO;
        }

        int select_ready = select(max_fd + 1, &mask, NULL, NULL, NULL);

        if(select_ready < 0) { 
            perror(0);
            exit(1);
        }
        char server_response[256];
        char buffer[101];


        if (FD_ISSET(STDIN_FILENO, &mask)){ 
            int n_read = read(STDIN_FILENO, buffer, 100);

            if (n_read < 0) { 
                perror(0);
                exit(1);
            }

            buffer[n_read] = '\0';

            if (n_read > 0 && buffer[n_read-1] == '\n') {
                buffer[n_read-1] = '\0';
            }

            if (DEBUG){ 
                printf("[DEBUG] sent %s\n", buffer);
            }

            if (n_read >= 4 && strncmp(buffer, "exit", 4) == 0) {
                close(network_socket);
                exit(0);
            }

            if (n_read >= 4 && strncmp(buffer, "help", 4) == 0) {
              printf("Available commands:\n");
              printf("'help', print this help message\n");
              printf("'exit', exit\n");
              printf("'get', retrieve sensor data\n");
              printf("'N name surname reason', ask permission to go out\n");
              continue;
            }

            if (n_read >= 3 && strncmp(buffer, "get", 3) == 0) {
                if (write(network_socket, &buffer, sizeof(buffer)) < 0) { 
                    perror(0);
                    exit(1);
                }
                flag = 1;
                continue;
            }

            if (write(network_socket, &buffer, sizeof(buffer)) < 0) { 
                perror(0);
                exit(1);
            }

            flag = 2;
            continue;
        }

        if (FD_ISSET(network_socket, &mask)){ 
            int m_read;

            if ((m_read = read(network_socket, &server_response, sizeof(server_response))) < 0) { 
                perror(0);
                exit(1);
            }


            if (!strncmp(server_response,"try again",strlen("try again"))) {
                server_response[10] = '\0';

                if (server_response[10] == '\n') {
                    server_response[10] = '\0';
                }

                if (DEBUG){
                    printf("[DEBUG] read %s\n", server_response);
                }

                memset(server_response, 0, sizeof(server_response));
                continue;
            }

            if (flag == 1) { 

                server_response[22] = '\0';

                if (server_response[22] == ' ') {
                    server_response[22] = '\0';
                }

                if (DEBUG){
                    printf("[DEBUG] read %s\n", server_response);
                }

                char *token = strtok(server_response, " ");
                int values[4];
                int i = 0;

                while (token != NULL && i < 4) { 
                    values[i] = atoi(token);
                    token = strtok(NULL, " ");
                    i++;
                }

                int third_value = values[2];
                int minutes = third_value / 100;
                int seconds = third_value % 100;

                char forth_value[256];
                sprintf(forth_value, "%d", values[3]);

                time_t rawtime = atoi(forth_value);
                struct tm *info = localtime(&rawtime);
                printf("------------\n");
                printf("Latest event:\n");
                printf("interval (%d)\n", values[0]);
                printf("Temperature is: %d.%d\n", minutes, seconds);
                printf("Light level is: %d\n", values[1]);
                printf("Timestamp is: %s\n", asctime(info));
                memset(server_response, 0, sizeof(server_response));
                continue;
            }

            if (flag ==  2) { 

                removeSubstring(server_response, "invalid code");

                server_response[m_read] = '\0';

                if (DEBUG){
                    printf("[DEBUG] read %s\n", server_response);
                }
                printf("%s\n", server_response);
                memset(server_response, 0, sizeof(server_response));
                continue;
            }
        }


    }

    return 0;

}
