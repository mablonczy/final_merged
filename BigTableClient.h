#ifndef FINAL_BIGTABLECLIENT_H
#define FINAL_BIGTABLECLIENT_H

#include <map>
#include <fstream>
#include <sys/time.h>
#include <vector>
#include "communication.h"

inline int SUCCESS =  1;
inline int FAILURE = -1;

class BigTableClient {

private:

	bool PRINT_LOGS;
	int  number_of_primaries;

	// std::map<int, std::vector<Node> > cluster_id_to_nodes;

public: 
	std::map<int, std::vector<Node> > cluster_id_to_nodes;
	
	void initialize(std::string config_file_path, int num_of_primaries, bool print_logs) {
		PRINT_LOGS = print_logs;
		number_of_primaries = num_of_primaries;
		initialize_rand();

		int index = 0;
		std::ifstream config_file(config_file_path);
		std::string address;
		while (std::getline(config_file, address)) {
			Node node;
			node.domain  = extract_domain_name_from_address(address);
			node.port    = extract_port_number_from_address(address);
			node.domain_port = node.domain + "_" + std::to_string(node.port);
			node.address = get_address_from_domain_and_port(node.domain, node.port);

			cluster_id_to_nodes[index++ % num_of_primaries].push_back(node);
		} 
	}

	int put(std::string& row, std::string& column, std::string& value) {
		/* Define PUT message */
		Payload payload;
		payload.add(TYPE, PUT);
		payload.add(ROW, row);
		payload.add(COLUMN, column);   
		payload.add(SENDER, CLIENT); 
		payload.set_data(value);

		/* Connect to primary node */
		int cluster_id = row_to_cluster_id(row);

		/* Connect to first server i.e. primary */
		int socket_fd = connect_to_primary_server(cluster_id);
 		if (socket_fd == FAILURE) return FAILURE;

 		Payload response = sender_server_request(socket_fd, payload);

 		if (PRINT_LOGS) printf("[%d] Closing connection to server.\n", socket_fd);
 		close(socket_fd);

 		if (response.get(STATUS) == STATUS_OK) return SUCCESS;
 		else return FAILURE;
	}

	int cput(std::string& row, std::string& column, std::string& value1, std::string& value2) {
		/* Define CPUT message */
		Payload payload;
		payload.add(TYPE, CPUT);
		payload.add(ROW, row);
		payload.add(COLUMN, column);   
		payload.add(SENDER, CLIENT); 
		payload.add(GOLD, value1); 
		payload.set_data(value2);
		

		/* Connect to primary node */
		int cluster_id = row_to_cluster_id(row);

		/* Connect to first server i.e. primary */
		/* TODO: Check if first one is avaliable else go down the line */
		int socket_fd = connect_to_server(cluster_id_to_nodes[cluster_id][0]);

 		Payload response = sender_server_request(socket_fd, payload);

 		if (PRINT_LOGS) printf("[%d] Closing connection to server.\n", socket_fd);
 		close(socket_fd);

 		if (response.get(STATUS) == STATUS_OK) return SUCCESS;
 		else return FAILURE;
	}

	/* Fills in value string */
	int get(std::string& row, std::string& column, std::string& value) {
		/* Declare payload to send to server */
		Payload payload;
		payload.add(TYPE, GET);
		payload.add(ROW, row);
		payload.add(COLUMN, column);
		payload.add(SENDER, CLIENT);    
 
		/* Connect to primary node */
		int cluster_id = row_to_cluster_id(row);

		/* Connect to a random server in correct cluster */
		int socket_fd = connect_to_random_replica_server(cluster_id);
 		if (socket_fd == FAILURE) return FAILURE;

 		Payload response = sender_server_request(socket_fd, payload);

		if (PRINT_LOGS) printf("[%d] Closing connection to server.\n", socket_fd);
 		close(socket_fd);

 		/* Write data to value string */
 		value = response.data;

 		if (response.get(STATUS) == STATUS_OK) return SUCCESS;
 		else return FAILURE;
	}

	int del(std::string& row, std::string& column) {
		Payload payload;
		payload.add(TYPE, DELETE);
		payload.add(ROW, row);
		payload.add(COLUMN, column);   
		payload.add(SENDER, CLIENT); 
 
		/* Connect to primary node */
		int cluster_id = row_to_cluster_id(row);

		/* Connect to first server i.e. primary */
		/* TODO: Check if first one is avaliable else go down the line */
		int socket_fd = connect_to_server(cluster_id_to_nodes[cluster_id][0]);
 		if (socket_fd == FAILURE) return FAILURE;

 		Payload response = sender_server_request(socket_fd, payload);

 		if (PRINT_LOGS) printf("[%d] Closing connection to server.\n", socket_fd);
 		close(socket_fd);

 	 	if (response.get(STATUS) == STATUS_OK) return SUCCESS;
 		else return FAILURE;
	}

private:

	Payload sender_server_request(int socket_fd, Payload& payload) {
		Payload response;

		/* Send server payload */
	 	if (PRINT_LOGS) printf("[%d] Sending %s request to server.\n", socket_fd, payload.get(TYPE).c_str());
 		send_payload_via_socket(socket_fd, payload);

 		/* Wait for server to complete reqeust */
 		recieve_payload_from_socket(socket_fd, response);
 		if (PRINT_LOGS) printf("[%d] %s completed with %s.\n", socket_fd, response.get(TYPE).c_str(), response.get(STATUS).c_str());

 		return response;
	}

	int connect_to_primary_server(int cluster_id) {
		/* First online server is guarnateed to be primary */
		std::vector<Node> servers = cluster_id_to_nodes[cluster_id];
		for (int i = 0; i < servers.size(); i++) {
			/* Tries to establish connection with all servers in order */
			int socket_fd = connect_to_server(servers[i]);
			if (socket_fd != FAILURE) return socket_fd;
		}
		if (PRINT_LOGS) printf("\n[C] Could not connect to any servers.\n");
		return FAILURE;
	}

	int connect_to_random_replica_server(int cluster_id) {
		std::vector<Node> replicas = cluster_id_to_nodes[cluster_id];
		while (replicas.size() > 0) {
			/* Get random index of replica */
			int random_index = rand_number_up_to_int(replicas.size());

			/* Get random replica and remove it */
			Node random_replica = replicas[random_index];
			replicas.erase(replicas.begin() + random_index);

			/* Try to connect */
			int socket_fd = connect_to_server(random_replica);
			if (socket_fd != FAILURE) {
				return socket_fd;
			}
		}
		if (PRINT_LOGS) printf("\n[C] Could not connect to any servers.\n");
		return FAILURE;
	}

	int connect_to_server(Node node) {
		/* Open socket */
		int socket_fd = open_socket(); 
		if (connect_address_to_socket(socket_fd, node.address) < 0) return FAILURE;
		if (PRINT_LOGS) printf("\n[%d] Connected to server (%s).\n", socket_fd, node.domain_port.c_str());
		return socket_fd;
	}

	int row_to_cluster_id(std::string& row) {
		std::hash<std::string> hasher;
		return hasher(row) % number_of_primaries;
	}

	void initialize_rand() {
		struct  timeval  tv;
		struct  timezone tz;
		gettimeofday(&tv,&tz);
		std::srand(tv.tv_usec);
	}

	int rand_number_up_to_int(int max) {
		return std::rand() % max;
	}

};

#endif