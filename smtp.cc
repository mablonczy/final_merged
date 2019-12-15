#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <string.h> 
#include <sys/wait.h> 
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <deque>
#include <set> 
#include <signal.h>
#include <algorithm>
#include <regex>
#include <sys/file.h>
#include <mutex>
#include <map>
#include <functional>

#include "EmailClient.h"
EmailClient email_client_obj;
email_client_obj.initialize();
std::string CONFIG_PATH = "./config.txt";
int NUMBER_OF_PRIMARIES = 1;
bool PRINT_LOGS = false;
std::string colName = "METADATA";
//BigTableClient bigTable;

/* Server setup functions */
int  open_socket();
void bind_socket(int socket_fd, int port);
void begin_listening(int socket_fd, int max_connections);
int  accept_connection(int fd);

/* Helper functions */
bool is_valid_email_format(std::string &str);
bool is_mailbox(std::string path);
bool is_string_only_spaces(std::string &str);
int write_email_to_mailbox(int fd, std::string &email, std::string recipent);
int  lock_mailbox(int fd, std::string username);
int  unlock_mailbox(int fd, std::string username);
//void print_set(std::set<std::string> &s);

/* Variable decalarations */
std::string mailbox_path;
int socket_fd;
std::set<int> connections;
std::mutex open_mailboxes_mutex;
std::map<std::string, int> open_mailboxes;

bool v = true;
bool exited = false;
int MAX_CONNECTIONS    = 100;
int MAX_MESSAGE_LENGTH = 1000;
//const char CRLF[4]     = "\r\n"; 
std::string CRLF = "\r\n";
std::string SERVER_DOMAIN = "penncloud";

/* Messages definitions */
std::string A_FLAG_MSG = "T01\r\n";

std::string SYNTAX_ERROR = "500 Syntax error, command unrecognized" + CRLF;
std::string ARGS_SYNTAX_ERROR = "501 Syntax error in parameters or arguments" + CRLF;
std::string BAD_SEQUENCE_ERROR = "503 Bad sequence of commands" + CRLF;
std::string MAILBOX_NOT_FOUND = "550 No such user here" + CRLF;

std::string SERVICE_READY = "220 " + SERVER_DOMAIN + " Simple Mail Transfer Service Ready" + CRLF;
std::string SERVICE_CLOSING = "221 " + SERVER_DOMAIN + " Service closing transmission channel" + CRLF;
std::string OK = "250 OK" + CRLF;
std::string OK_DOMAIN = "250 " + SERVER_DOMAIN + CRLF;
std::string DATA_RECEIVED = "354 Start mail input; end with <CRLF>.<CRLF>" + CRLF;


std::string parse_command(std::string &message) {
	/* Returns command and erases it from message body */
	std::string command;
	std::size_t split = message.find(" ");

	if (split != std::string::npos) {
		command = message.substr(0, split);
		transform(command.begin(), command.end(), command.begin(), ::toupper);
		if (command == "MAIL" || command == "RCPT") {
			split = message.find(":");
			command = message.substr(0, split);
		}
		message.erase(0, split + 1);
	}
	else {
		command = message;
		message.clear();
	}
	transform(command.begin(), command.end(), command.begin(), ::toupper);
	return command;
}

void write_socket(int fd, std::string message) {
	if (v) fprintf(stderr, "[%d] S: %s", fd, message.c_str());
	send(fd, message.c_str(), message.size(), 0);
}

void read_socket(int fd, std::string &message) {
	char buf[MAX_MESSAGE_LENGTH];
	bzero(buf, MAX_MESSAGE_LENGTH);
	int recieved = read(fd, &buf, MAX_MESSAGE_LENGTH);
	if (recieved > 0) {
	 	std::string new_message(buf, recieved);
	 	if (v) fprintf(stderr, "[%d] C: %s", fd, new_message.c_str());
		message.append(new_message);
	}
}
/* HELO (? -> 1), MAIL FROM (1 -> 2), RCPT TO (2 -> 2) , RCPT TO, RCPT TO... DATA (2 -> 3 or 3 -> 3), DATA, DATA*/
void execute_helo(int fd, int &state, std::string &body) {
	/* Check command is acceptable in current state */
	// ?

	/* Check if command line arguments are acceptable */
	if (is_string_only_spaces(body)) {
		write_socket(fd, ARGS_SYNTAX_ERROR);
		return;
	}

	write_socket(fd, OK_DOMAIN);

	/* Update state if needed */
	state = 1;
}

/* Returns sender's email if message body is valid */
std::string execute_mail_from(int fd, int &state, std::string &body) {
	/* Check if command is executable from current state */
	if (state != 1) {
		write_socket(fd, BAD_SEQUENCE_ERROR);
		return "";
	}

	/* Check if message format is acceptable */
	if (!is_valid_email_format(body)){ 
		write_socket(fd, ARGS_SYNTAX_ERROR);
		return "";
	}

	/* Write back confirmation to sender */
	if (v) fprintf(stderr, "[%d] S: Sender's email is %s", fd, body.c_str());
	write_socket(fd, OK);

	/* Update state */
	state = 2;

	/* Return email of sender */
	return body.substr(body.find("<") + 1, body.find(">") - body.find("<") - 1);
}

/* Add recipent's email if message body is valid to recipients degque*/
void execute_rcpt_to(int fd, int &state, std::string &body, std::set<std::string> &recipients) {
	/* Check if command is executable on current state */
	if (state != 2) {
		write_socket(fd, BAD_SEQUENCE_ERROR);
		return;
	}

	/* Check if acceptable */
	if (!is_valid_email_format(body)){ 
		write_socket(fd, ARGS_SYNTAX_ERROR);
		return;
	}

	/* Get user and domain (user@domain) */
	std::string username = body.substr(body.find("<") + 1, body.find("@") - body.find("<") - 1);
	std::string domain   = body.substr(body.find("@") + 1, body.find(">") - body.find("@") - 1);
	
	/* Validate domain */
	if (domain != SERVER_DOMAIN) {
		if (v) fprintf(stderr, "[%d] S: RCPT domain not valid.\n", fd);
		write_socket(fd, MAILBOX_NOT_FOUND);
		return;
	}

	/* Validate username exits in server's mailbox */
	if (!is_mailbox(mailbox_path + username + ".mbox")) {
		if (v) fprintf(stderr, "[%d] S: RCPT username not valid.\n", fd);
		write_socket(fd, MAILBOX_NOT_FOUND);
		return;
	}

	/* Add username to recpients deque */
	if (v) fprintf(stderr, "[%d] S: Added recipent's with username: %s\n", fd, username.c_str());
	recipients.insert(username);

	/* Write back confirmation to sender */
	write_socket(fd, OK);
}

/* */
void execute_data(int fd, int &state, std::string &body) {
	/* Check if command is executable on current state */
	if (state != 2) {
		write_socket(fd, BAD_SEQUENCE_ERROR);
		return;
	}

	/* Check if acceptable */
	if (!is_string_only_spaces(body)){ 
		write_socket(fd, ARGS_SYNTAX_ERROR);
		return;
	}

	/* Write back confirmation to sender */
	write_socket(fd, DATA_RECEIVED);

	/* Update state */
	state = 3;
}

void execute_noop(int fd, std::string &body) {
	/* Check if command line arguments are acceptable */
	if (!is_string_only_spaces(body)) write_socket(fd, ARGS_SYNTAX_ERROR);
	write_socket(fd, OK);
}

void execute_quit(int fd, std::string &body) {
	/* Check command is acceptable in current state */
	// ?

	/* Check if command line arguments are acceptable */
	if (!is_string_only_spaces(body)) {
		write_socket(fd, ARGS_SYNTAX_ERROR);
		return;
	}

	/* Write to sender confirmation */
	write_socket(fd, SERVICE_CLOSING);

	/* Close connection */
	close(fd); 
	connections.erase(fd);
	if (v) fprintf(stderr, "[%d] Connection closed\n", fd);
	pthread_exit(NULL);

}

void *session_worker(void *arg) {
	int connection_fd = *(int *)arg;
	std::string message;
	int state = 0;

	std::set<std::string> recipients;
	std::string sender = "";
	std::string email_body = "";

	write_socket(connection_fd, SERVICE_READY);
	while (true) {
		/* Read from socket latest message arrival  */
		read_socket(connection_fd, message);

		/* Get index of CRLF if present */
		std::size_t message_idx = message.find(CRLF);

		/* If CRLF is present then message is ready to be excuted */
		while (message_idx != std::string::npos) {
			/* Get complete message up to CRLF */
			std::string body = message.substr(0, message_idx);

			/* Find message command and remove command from message body */
			std::string command = parse_command(body);

			/* Only execute command if server is not shutting down */
			if (!exited) {

				/* Execute command */
				if      (command == "HELO")      execute_helo(connection_fd, state, body);
				else if (command == "MAIL FROM") sender = execute_mail_from(connection_fd, state, body); 
				else if (command == "RCPT TO")   execute_rcpt_to(connection_fd, state, body, recipients);
				else if (command == "DATA")      execute_data(connection_fd, state, body);
				else if (command == "QUIT")      execute_quit(connection_fd, body);
				else if (command == "NOOP")      execute_noop(connection_fd, body);
				else if (command == "RSET") {   

						/* Check if we can accept RSET from current sate */
						if (state == 0) {
							write_socket(connection_fd, BAD_SEQUENCE_ERROR);
						}
						else {
							/* Check if command line arguments are acceptable */
							if (!is_string_only_spaces(body)) write_socket(connection_fd, ARGS_SYNTAX_ERROR);
							else {
								/* Update state to 1 */
								state = 1;

								/* Clear data */
								recipients.clear();
								sender.clear();
								email_body.clear();
								message.clear();

								/* Write to sender confirmation */
								write_socket(connection_fd, OK);
							}
						}
				}
				else if (state == 3) { /* i.e. Reading Message Body*/

					/*  Since we are reading the full message body we don't have the 
					    command + body format, we only have a line of the whole email body  */
					std::string email_line = message.substr(0, message_idx);

					/* Check if email_line conatins terminating sequence of characters*/
					if (email_line == ".") {
						/* Since this marks the end of the email body we now write it to the mboxs */

						/* Get email date */
						time_t current_time = time(NULL);
						struct tm * current_time_tm = localtime (&current_time);

						/* Add header to email body */
						email_body = "From <" + sender + "> " + asctime(current_time_tm) + email_body; 

						/* For each recipient write email to mailbox */
						bool has_failed = false;
						std::set<std::string>::iterator iterator = recipients.begin();
						while (iterator != recipients.end()) {
							int res = write_email_to_mailbox(connection_fd, email_body, *iterator);
							if (res < 0) {
								/* Write back failure to sender */
								has_failed = true;
								break;
							}
							iterator++;
						}
						recipients.clear();

						/* Write back confirmation to sender */
						if (! has_failed) write_socket(connection_fd, OK);	
						else write_socket(connection_fd, "450 Requested mail action not taken: mailbox unavailable\r\n");	
											
						/* Update state */
						state = 1;
					}
					else {
						/* Append email_line to email_body string */
						email_body += email_line + "\n";
					}

				}
				else {
					write_socket(connection_fd, SYNTAX_ERROR);
				}

				/* Erase executable_message from message queue */
				message.erase(0, message_idx + 2);

				/* Update new CRLF position */
				message_idx = message.find(CRLF);
			}
		}
	}
}

void interrupt_handler(sig_atomic_t s){ 
	/* Iterate through each open connection and close it*/
	exited = true;
	std::set<int>::iterator iter = connections.begin();
	while(iter != connections.end()) {
		int connection_fd = *iter++;
		std::string response = "421 " + SERVER_DOMAIN + " Service not available, closing transmission channel\r\n";
		write_socket(connection_fd, response);
		close(connection_fd);
		connections.erase(connection_fd);
	}
	/* Close serve socekt */
	close(socket_fd);
    exit(EXIT_SUCCESS); 
}
int main(int argc, char *argv[]) {
	//BigTableClient bigTable;
	//bigTable.initialize(CONFIG_PATH, NUMBER_OF_PRIMARIES, PRINT_LOGS);
	/* Use [getopt] to read in command-line options */
	int  p = 2500; 
	bool a = false; 
	v      = false;
	int flag;
	while ((flag = getopt(argc, argv, "vap:")) != -1) {
		switch (flag) {
			case 'p' : p = atoi(optarg); break;
			case 'a' : a = true; break;
			case 'v' : v = true; break;
			default :
				fprintf(stderr, "Invalid command-line options.\n");
				exit(EXIT_FAILURE);
		}
	}
	if (a) { fprintf(stderr, "%s", A_FLAG_MSG.c_str()); exit(EXIT_SUCCESS); }

	/* Get location of mailbox argument */ ///// CHECK IF IT EXITS ???
	if (argc == optind) {
		fprintf(stderr, "Missing mailbox location argument. Start server as follows: smpt [flags] [path/to/mailbox].\n");
		exit(EXIT_FAILURE);
	}
	mailbox_path = argv[optind];
	if (mailbox_path.at(mailbox_path.size() - 1) != '/') mailbox_path += '/';

	/* Catch Ctrl-C signal */
	signal(SIGINT, interrupt_handler);

	/* Start server */
	socket_fd = open_socket();
	bind_socket(socket_fd, p);
	begin_listening(socket_fd, MAX_CONNECTIONS);

	/* Accept connection and dispatch thread handler */
	while (true) {
		pthread_t thread;

		/* Block until new connection is trying to be established */
		int connection_fd = accept_connection(socket_fd);
		if (connection_fd > 0) {
			/* Add new connection file descriptor to set of open connections */
			connections.insert(connection_fd);

			/* Dispatch detached server thread */
			pthread_create(&thread, NULL, session_worker, &connection_fd);
			pthread_detach(thread);
		}

	}

	return EXIT_FAILURE;
}



















/* Server setup functions */
int open_socket() {
	int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		if (v) fprintf(stderr, "Error: Could not open socket\n");
		exit(EXIT_FAILURE);
	}
	return socket_fd;
}
void bind_socket(int socket_fd, int port) {
	struct sockaddr_in server_address;
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htons(INADDR_ANY);
	server_address.sin_port = htons(port);
	int socket_option = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(int)) < 0){
		if (v) fprintf(stderr, "Error: setsockopt failed\n");
		exit(EXIT_FAILURE);
	}
	if (bind(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address))) {
		if (v) fprintf(stderr, "Error: Could not bind socket\n");
		exit(EXIT_FAILURE);
	}
}
void begin_listening(int socket_fd, int max_connections) {
	if (listen(socket_fd, max_connections) < 0) {
		if (v) fprintf(stderr, "Error: Begin listen failed\n");
		exit(EXIT_FAILURE);
	}
}

int accept_connection(int fd) { /* Return -1 if could not accept connection */
	struct sockaddr_in client_address;
	socklen_t client_address_length = sizeof(client_address);
	int connection_fd = accept(fd, (struct sockaddr*)&client_address, &client_address_length);
	if (connection_fd < 0) { 
		if (v) fprintf(stderr, "Error: Failed to accept new connection.\n"); 
		return -1; 
	}
	else {
		if (v) fprintf(stderr, "[%d] New connection\n", connection_fd);
	}
	return connection_fd;
}









/* Helper functions */




/* Helper functions */
int write_email_to_mailbox(int fd, std::string &email, std::string recipent) {

	/*std::string rowName=recipent+"_email";

	std::string metadataContent=bigTable.get(rowName,colName);

	std::hash<std::string> hashFunction;
	std::size_t hashValue=hashFunction(email);

	//std::cout << valueEmail << std::endl;
	metadataContent=metadataContent+std::to_string(hashValue)+"\n";
	std::cout<<"putEmailmetadataContent="<<metadataContent<<"\n";
	bigTable.put(rowName,colName,metadataContent);
	std::string hashValueString=to_string(hashValue);
	bigTable.put(rowName,hashValueString,email);*/
    std::string tempLine;
    Email email_object;
    while (getline(email,tempLine)
    {
        if (tempLine.substr(0, 6) == "From: ")
        {
            email_object.sender=tempLine.substr(6,std::string::npos);

        }
        else if(tempLine.substr(0, 4) == "To: ")
        {
            email_object.recepients.push_back(tempLine.substr(4,std::string::npos));
        }
        else if (tempLine.substr(0, 6) == "Date: ")
        {
            email_object.timestamp=tempLine.substr(6,std::string::npos);
        }
        else if (tempLine.substr(0, 9) == "Subject: ")
        {
            email_object.subject=tempLine.substr(9,std::string::npos);
        }
        else if (tempLine.substr(0, 17) == "hashValueString: ")
        {
            email_object.hashValueString=tempLine.substr(17,std::string::npos);
        }
        else
        {
            email_object.content=email_object.content+tempLine+"\n";
        }
    }
    email_client_obj.putEmail(email_object);

}

int lock_mailbox(int fd, std::string username) {
	/* Go down on open_mailboxes_fd mutex */
	open_mailboxes_mutex.lock();

	/* Check if mailbox is already open in current session */
	if (open_mailboxes[username] > 0) {
		if (v) fprintf(stderr, "[%d] S: Mailbox is already locked in current server session.\n", fd );

		/* Release open_mailboxes_fd mutex */
		open_mailboxes_mutex.unlock();

		return -1;
	}

	/* Open mailbox */
	std::string user_mailbox = mailbox_path + username + ".mbox";
	int mailbox_fd = open(user_mailbox.c_str(), O_RDWR);

	/* Attempt to aquire mailbox lock */
	int flock_return = flock(mailbox_fd, LOCK_EX | LOCK_NB);

	/* Log and return error if mailbox could not be locked */
	if (flock_return != 0) {
		if (v) fprintf(stderr, "[%d] S: Could not lock mailbox. Errno: %d\n", fd , errno);

		/* Release open_mailboxes_fd mutex */
		close(mailbox_fd);
		open_mailboxes_mutex.unlock();

		return -1;
	}
	else {
		if (v) fprintf(stderr, "[%d] S: Locked mailbox for username: %s\n", fd , username.c_str());
	}
	
	/* Add user name to open mailboxes set */
	open_mailboxes[username] = mailbox_fd;

	/* Go up on open_mailboxes_fd mutex */
	open_mailboxes_mutex.unlock();

	return 1;
}

int unlock_mailbox(int fd, std::string username) {
	/* Go down on open_mailboxes_fd mutex */
	open_mailboxes_mutex.lock();

	/* Open mailbox */
	int mailbox_fd = open_mailboxes[username];

	/* Attempt to aquire mailbox lock */
	int flock_return = flock(mailbox_fd, LOCK_UN | LOCK_NB);

	/* Log and return error if mailbox could not be locked */
	if (flock_return != 0) {
		if (v) fprintf(stderr, "[%d] S: Could not unlock mailbox. Errno: %d\n", fd , errno);

		/* Go up on open_mailboxes_fd mutex */
		close(mailbox_fd);
		open_mailboxes_mutex.unlock();

		return -1;
	}
	else {
		if (v) fprintf(stderr, "[%d] S: Unlocked mailbox for username: %s\n", fd , username.c_str());
	}
	/* Add user name to open mailboxes set */
	close(mailbox_fd);
	open_mailboxes[username] = -1;

	/* Go up on open_mailboxes_fd mutex */
	open_mailboxes_mutex.unlock();

	return 1;
}



bool is_mailbox(std::string path) {
	/* Function return true if there exits a mailbox at specified path */

	/* Open file at specified path */
	std::ifstream mailbox(path);

	/* Mailbox file is good */
	bool is_good = mailbox.good();

	/* Close stream */
	mailbox.close();

	return is_good;
}

bool is_valid_email_format(std::string &str) {
	/* Checks if str is an accetable <user@domain> email */
										       // Message format is not valid if it meets any of the following
	if (str.find("<") == std::string::npos  || // Body contains '<'
		str.find(">") == std::string::npos  || // Body contains '>'
		str.find("@") == std::string::npos  || // Body contains '@'
		str.find(' ') != std::string::npos  || // Body does not contain spaces
	    str.at(str.size()-1) != '>'         || // Last character is '>' 
	    str.at(0) != '<'                    || // First character is '<' 
	    str.find("@") + 1 ==  str.find(">") || // Length of domain is 0
	   	str.find("@") - 1 ==  str.find("<") ){ // Length of username is 0
		return false;
	}
	return true;
}

bool is_string_only_spaces(std::string &str) {
	return str.find_first_not_of(' ') == std::string::npos;
}


// void print_set(std::set<std::string> &s)
// {
// 	for (std::string p: s)
//     {
//         std::cout << p << '\n';
//     }

// }

