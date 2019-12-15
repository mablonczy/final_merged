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
};

class AdminClient {

private:
    bool PRINT_LOGS;
    BigTableClient big_table;
    std::vector<Node> servers;
    std::vector<ServerMetadata> servers_metadata;

public:

    void initialize() {
        big_table.initialize(CONFIG_PATH, NUMBER_OF_PRIMARIES, PRINT_LOGS);
    }

    std::vector<ServerMetadata> get_servers_info() {

        for (int i = 0; i < big_table.nodes.size(); i++) {

            Node node = big_table.nodes.at(i);

            ServerMetadata server_metadata;
            server_metadata.id = i;
            server_metadata.address = node.domain_port;
            std::cout << "Address: " << node.domain_port << std::endl;

            Payload response;
            if (big_table.get_head(node, response)) {
                server_metadata.on = true;

                server_metadata.load = std::stoul(response.get(TABLET_SIZE));
                std::string serialized_tablet_content = response.data;

                std::cout << "Size: " << server_metadata.load << std::endl;
                std::cout << "Content: " << serialized_tablet_content << std::endl;
                std::istringstream content_stream(serialized_tablet_content);
                for (std::string line; std::getline(content_stream, line); ){
                    std::cout << "Line: " << line << std::endl;

                    ServerDataPoint data_point;
                    data_point.row = line.substr(0, line.find('$'));
                    data_point.col = line.substr(line.find("$") + 1, line.find("&") - line.find("$") - 1);
                    data_point.partialData = line.substr(line.find("&") + 1, std::string::npos);
                    server_metadata.content.push_back(data_point);
                    std::cout << data_point.row << " | " << data_point.col << " | " << data_point.partialData << std::endl;

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

    bool turn_off_node(std::string nodeAddr){
        return true;
    }

    bool turn_on_node(std::string nodeAddr){
        return true;
    }

};

#endif