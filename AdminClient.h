#ifndef ADMIN_CLIENT_H_
#define ADMIN_CLIENT_H_

#include <sys/time.h>
#include "BigTableClient.h"
#include <sstream>

/* Data structure containing a row-col-value tuple to be displayed in the front end*/
struct ServerDataPoint {
    std::string row;
    std::string col;
    std::string partialData;
};

/* Data strucutre containing the server metatdata */
struct ServerMetadata {
	int id; 
	size_t load;
	bool on;
	std::string address;
	std::vector<ServerDataPoint> content;
	Node node;
};

class AdminClient {

	private:
	    bool PRINT_LOGS;
	    BigTableClient big_table;
		std::vector<ServerMetadata> servers_metadata;

	public:

	    void initialize() {
	        big_table.initialize(CONFIG_PATH, NUMBER_OF_PRIMARIES, PRINT_LOGS);
	        servers_metadata.clear();
	    }

		std::vector<ServerMetadata> get_servers_info() {

			for (int i = 0; i < big_table.nodes.size(); i++) {

				Node node = big_table.nodes.at(i);

				ServerMetadata server_metadata;
				server_metadata.node = node;
				server_metadata.id = i;
				server_metadata.address = node.domain_port;
				std::cout << "Address: " << node.domain_port << std::endl;

				Payload response;
				if (big_table.get_head(node, response)) {
					server_metadata.on = true;

					server_metadata.load = std::stoul(response.get(TABLET_SIZE));
					std::string serialized_tablet_content = response.data;

					std::cout << "Size: " << server_metadata.load << std::endl;
					std::istringstream content_stream(serialized_tablet_content);
					for (std::string line; std::getline(content_stream, line); ){
					    std::cerr << line << std::endl;
						ServerDataPoint data_point;
						data_point.row = line.substr(0, line.find("$"));
						data_point.col = line.substr(line.find("$") + 1, line.find("&") - line.find("$") - 1);
						data_point.partialData = line.substr(line.find("&") + 1, std::string::npos);
						server_metadata.content.push_back(data_point);
						std::cout << "Content: " << data_point.row << " | " << data_point.col << " | " << data_point.partialData << std::endl;

					}	
				}
				else {
					server_metadata.on = false;
					server_metadata.load = 0;
				}
				servers_metadata.push_back(server_metadata);
		    }

		    return servers_metadata;
		}

		std::vector<ServerDataPoint> get_server_content(int server_id){
			return servers_metadata[server_id].content;
		}

		bool turn_off_node(int server_id){
		    return switch_node(servers_metadata[server_id].node, false);
		}

		bool turn_on_node(int server_id){
			return switch_node(servers_metadata[server_id].node, true);
		}

		bool switch_node(Node& node, bool turn_on) { 

			int port;
			Payload payload;
			if (turn_on) {
				payload.add(TYPE, ON);
				port = node.port + 1000;
			}
			else {
				payload.add(TYPE, OFF);
				port = node.port;
			}

			int socket_fd = open_socket(); 
			sockaddr_in address = get_address_from_domain_and_port(node.domain, port);
			if (connect_address_to_socket(socket_fd, address) < 0) return false;
			if (PRINT_LOGS) printf("\n[%d] Connected to server.\n", socket_fd);

			Payload response;

			/* Send server payload */
			if (PRINT_LOGS) printf("[%d] Sending %s request to server.\n", socket_fd, payload.get(TYPE).c_str());
			send_payload_via_socket(socket_fd, payload);

			/* Wait for server to return status of the request 
			                        TODO: add timeout */
			recieve_payload_from_socket(socket_fd, response);
			if (PRINT_LOGS) printf("[%d] %s completed with %s.\n", socket_fd, response.get(TYPE).c_str(), response.get(STATUS).c_str());
			if (response.get(STATUS) != STATUS_OK) return false ;

			return true ;
		}

};

#endif