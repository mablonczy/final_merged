// functions for admin page live here
#ifndef FINAL_ADMIN_H
#define FINAL_ADMIN_H

#include "Checkpoint.h"

struct ServerDataPoint {
    std::string row;
    std::string col;
    std::string partialData;
};

struct ServerInfo {
    int id ; 
    size_t load ;
    bool on;
    std::string address;
};

// --------------------helpers

bool switch_node(std::string nodeAddr, bool on) {
    std::string domain = extract_domain_name_from_address(nodeAddr);
    int port = extract_port_number_from_address(nodeAddr);
    sockaddr_in address = get_address_from_domain_and_port(domain, port);
    Payload payload;
    if (on)
	    payload.add(TYPE, ON);
    else 
        payload.add(TYPE, OFF);

    int socket_fd = open_socket(); 
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

// --------------------end of helpers

bool turn_off_node(std::string nodeAddr){
    return switch_node(nodeAddr, false);
}

bool turn_on_node(std::string nodeAddr){
    return switch_node(nodeAddr, true);
}

// returns raw data of given node
std::vector<ServerDataPoint> get_node_data(std::string nodeAddr) {
    std::vector<ServerDataPoint> datVec ;
    
    std::string domain = extract_domain_name_from_address(nodeAddr);
    int port = extract_port_number_from_address(nodeAddr);

    sockaddr_in address = get_address_from_domain_and_port(domain, port);

    // Define simple message with RAW command
	Payload payload;
	payload.add(TYPE, RAW);
	
    int socket_fd = open_socket(); 
	if (connect_address_to_socket(socket_fd, address) < 0) return datVec;
    if (PRINT_LOGS) printf("\n[%d] Connected to server.\n", socket_fd);

    Payload response;

	/* Send server payload */
	if (PRINT_LOGS) printf("[%d] Sending %s request to server.\n", socket_fd, payload.get(TYPE).c_str());
 	send_payload_via_socket(socket_fd, payload);

 	/* Wait for server to return requested data
                                     TODO: timeout? */
 	recieve_payload_from_socket(socket_fd, response);
 	if (PRINT_LOGS) printf("[%d] %s completed with %s.\n", socket_fd, response.get(TYPE).c_str(), response.get(STATUS).c_str());
    if (response.get(STATUS) != STATUS_OK) return datVec ;

    // parse data
	std::string data = response.data ;

	// deserialize 
    Checkpoint checkpoint = deserialize_checkpoint(response.data);

    // * code below can be used instead of making a checkpoint
    // std::map<std::string, std::map<std::string, std::string> > row_col_value;
	// std::stringstream ss;
	// ss.str(data);
	// boost::archive::text_iarchive archive(ss);
	// archive >> row_col_value;

	// iterate and parse into vector of ServerData
    std::map<std::string, std::map<std::string, std::string> >::iterator itRow = checkpoint.row_col_value.begin();
    while (itRow != checkpoint.row_col_value.end()) {
        std::string row = itRow->first;
        std::map<std::string, std::string> colMap = itRow->second;
        std::map<std::string, std::string> ::iterator itCol = colMap.begin();
        while (itCol != colMap.end()) {
            ServerDataPoint sdp ;
            sdp.row = row ;
            sdp.col = itCol->first ;
            sdp.partialData = itCol->second;
            datVec.push_back(sdp) ;

            itCol++;
        }
        itRow++;
    }

	return datVec ;
}

// returns names of all backend nodes and their statuses
std::vector<ServerInfo> get_back_nodes(BigTableClient big_table){
    std::vector<ServerInfo> result ;
    int id = 0 ; // server ID: just a counter
    std::map<int, std::vector<Node> > allServers = big_table.cluster_id_to_nodes ;

    // iterate through clusters and ping each server
    std::map<int, std::vector<Node> >::iterator it = allServers.begin();
    std::map<int, Node> connections;
    while (it != allServers.end()) {
        std::vector<Node> nodes = it->second;	

		for (int i = 0; i < nodes.size(); i++) { 

			/* Try to establish connection; those that cannot connect mark as OFF */
			int connection_fd = open_socket();
			if (connect_address_to_socket(connection_fd, nodes[i].address) == 0) {
                Payload payload;
	            payload.add(TYPE, STATS);
                payload.add(SENDER, CLIENT);

				connections[connection_fd] = nodes[i];
				
				send_payload_via_socket(connection_fd, payload);
				if (PRINT_LOGS) printf("[F] Sent read message to server %s on socket %d.\n", nodes[i].domain_port.c_str(), connection_fd);
			}
			else {
                ServerInfo current ;
                current.id = id ;
                current.on = false ;
                current.address = nodes[i].domain_port ;
                result.push_back(current) ;
                id++ ;
				if (PRINT_LOGS) printf("[F] Could not establish connection with server %s.\n", nodes[i].domain_port.c_str());
			}
		}

        it++ ;
    }

    /* Wait for all servers to respond */
    std::map<int, Node>::iterator itResp = connections.begin();
	while (itResp != connections.end()) {
        
        int conn = itResp->first;
        Node curNode = itResp->second;
		Payload response;
 
        // TODO: add check - if server died after connection was established
		recieve_payload_from_socket(conn, response);

		close(conn);

        if (PRINT_LOGS) printf("[F] closed connection\n");
        std::string strSize = response.get(SIZE);
        std::stringstream sstream(strSize);
        size_t sizetSize;
        sstream >> sizetSize;

        // replace '_' in curNode.domain_port with traditional ':'
        std::string addr = curNode.domain_port ;
        for (int i = 0; i < addr.length(); i++) {
            if (addr[i] == '_') {
                addr[i] = ':' ;
                break ;
            }
        }

        ServerInfo current ;
        current.id = id ;
        current.on = true ;
        current.address = addr ;
        current.load = sizetSize ;
        result.push_back(current) ;
        id++ ;

		if (PRINT_LOGS) printf("[F] Recieved STATS from server on socket %d.\n", conn);

        itResp++ ;
	}

    return result ;
}

#endif