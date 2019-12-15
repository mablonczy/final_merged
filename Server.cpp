//
// Created by cis505 on 11/19/19.
//

#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <zconf.h>
#include "Server.h"
#include "ThreadHandler.h"

BigTableClient Server::bigTableClient;
bool Server::run = true;

void Server::startServer(char* portNumber) {
    int port_num = atoi(portNumber);
    //open socket
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Cannot open socket" << std::endl;
        exit(1);
    }
    //bind port
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(port_num);
    bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    //listen
    listen(sockfd, 100);
    pthread_t thread;
    bigTableClient.initialize("config.txt", 1, true);
    while (run) {
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        //accept connection
        int comm_fd = accept(sockfd, (struct sockaddr *) &clientaddr, &clientaddrlen);
        std::cerr << "[" << comm_fd << "] " << "New connection" << std::endl;
        pthread_create(&thread, NULL, ThreadHandler::worker, &comm_fd);
    }
    close(sockfd);
}
