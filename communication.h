#ifndef FINAL_COMMUNICATION_H
#define FINAL_COMMUNICATION_H


#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <strings.h>
#include "Payload.h"

inline int MAX_CONNECTIONS     = 1000;
inline int MAX_METADATA_LENGTH = 1000;
inline std::string OK = "+OK";


/* TCP Client-Server Connection Functions */

struct Node {
  std::string domain;
  int port;
  std::string domain_port;
  sockaddr_in address;
  int server_id;
};

inline int open_socket() {
  int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    fprintf(stderr, "Error: Could not open SOCKET\n");
    exit(EXIT_FAILURE);
  }
  return socket_fd;
}

inline void initialize_server(int socket, int port) {
  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htons(INADDR_ANY);
  servaddr.sin_port = htons(port);
  int socket_option = 1;
  if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(int)) < 0){
    fprintf(stderr, "Error: SETSOCKOPT failed\n");
    exit(EXIT_FAILURE);
  }
  if (bind(socket, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
    fprintf(stderr, "Error: Could not BIND socket\n");
    exit(EXIT_FAILURE);
  }
  if (listen(socket, MAX_CONNECTIONS) < 0) {
    fprintf(stderr, "Error: LISTEN failed\n");
    exit(EXIT_FAILURE);
  }
}

inline int accept_connection(int fd) {
  struct sockaddr_in client_address;
  socklen_t client_address_length = sizeof(client_address);
  int connection_fd = accept(fd, (struct sockaddr*)&client_address, &client_address_length);
  if (connection_fd < 0) {
    fprintf(stderr, "Error: Failed to ACCEPT new connection.\n");
    exit(EXIT_FAILURE);
  }
  return connection_fd;
}

inline int connect_address_to_socket(int socket_fd, sockaddr_in address){
  return connect(socket_fd, (struct sockaddr*)&address, sizeof(address));
}

inline sockaddr_in get_address_from_domain_and_port(std::string& domain, int& port) {
  struct sockaddr_in address;
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  inet_pton(AF_INET, domain.c_str(), &(address.sin_addr));
  return address;
}




/******* Read and Write payloads Functions *******/

/* Helper methods to do the actual reading and writing */
void write_string_to_socket(int socket_fd, std::string& str);
int read_string_from_socket(int socket_fd, std::string& str, int length);

inline void send_payload_via_socket(int socket_fd, Payload& payload) {
  /* Add data length key to payload metadata */
  if (payload.data.length() == 0) payload.data = EMPTY_DATA;
  std::string data_size = std::to_string(payload.data.length());
  payload.add(DATA_SIZE, data_size);

  /* Serialized payload metadata i.e. all fields except data */
  std::string serialized_metadata = payload.serialize_metadata();

  /* Send metadata message */
  write_string_to_socket(socket_fd, serialized_metadata);

  /* Read acknoledgementnotsurehowtospellthis message */
  std::string ack_message;
  int read = read_string_from_socket(socket_fd, ack_message,  OK.length());

  if (ack_message == OK) {
    write_string_to_socket(socket_fd, payload.data);
  }
}

inline int recieve_payload_from_socket(int socket_fd, Payload& payload) {
  /* Read metadata message (Note: this assume we can read in one chunk, fine for localhost) */
  char buffer[MAX_METADATA_LENGTH];
  int rlen = read(socket_fd, buffer, MAX_METADATA_LENGTH);
  if (rlen == 0) return 0;
  buffer[rlen] = '\0';
  std::string serialized_metadata = std::string(buffer);

  /* Deserialize metadata message into Payload object */
  payload.deserialize_metadata(serialized_metadata);

  /* Extract data payload size */
  int data_size = payload.data_size();

  /* Send acknowledgement to sender, giving it the green light to send the full payload message */
  write(socket_fd, OK.c_str(), OK.length());

  /* Read data */
  std::string data_str;
  rlen = read_string_from_socket(socket_fd, data_str, data_size);
  if (rlen == 0) return 0;
  payload.data = data_str;

  return rlen;
}

inline void write_string_to_socket(int socket_fd, std::string& str) {
  /* Initialize writing buffer */
  int length = str.length();
  const char *buffer = str.c_str();

  /* Read into buffer until expected length */
  int total_sent = 0;
  while (total_sent < length) {

    /* Read message */
    int sent = write(socket_fd, &buffer[total_sent], length - total_sent);

    /* Update total chars received thus far */
    total_sent += sent;
  }
}

inline int read_string_from_socket(int socket_fd, std::string& str, int length) {
  /* Initialize reading buffer */
  char* buffer = new char[length];

  /* Read into buffer until expected length */
  int total_recieved = 0;
  while (total_recieved < length) {

    /* Read message */
    int recieved = read(socket_fd, &buffer[total_recieved], length - total_recieved);
    if (recieved == 0) return 0;

    /* Update total chars received thus far */
    total_recieved += recieved;

  }

  str = std::string(buffer, length);

  return total_recieved;
}



/*******  Random Helpers *******/

inline std::string extract_domain_name_from_address(std::string address) {
    return address.substr(0, address.find(':'));
}

inline int extract_port_number_from_address(std::string address) {
  return stoi(address.substr(address.find(':') + 1, address.size() - 1));
}


#endif