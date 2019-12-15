#ifndef FINAL_PAYLOAD_H
#define FINAL_PAYLOAD_H

#include <string>
#include <map>

inline std::string DELIMITER = "::";
inline std::string EMPTY_STRING = "";

inline std::string SENDER = "SENDER";
inline std::string CLIENT = "CLIENT";
inline std::string SERVER = "SERVER";

inline std::string HEAD         = "HEAD";
inline std::string TABLET_SIZE  = "TABLET_SIZE";

inline std::string TYPE         = "TYPE";
inline std::string PUT          = "PUT";
inline std::string GET          = "GET";
inline std::string CPUT         = "CPUT";
inline std::string DELETE 	     = "DELETE";
inline std::string GOLD 		 = "GOLD" ;
inline std::string GET_RESPONSE = "GET_RESPONSE";
inline std::string WRITE_RESPONSE = "WRITE_RESPONSE";
inline std::string LOGS         = "LOGS";
inline std::string CHECKPOINT   = "CHECKPOINT";
inline std::string SYNC_TABLET  = "SYNC_TABLET";

inline std::string STATUS       = "STATUS";
inline std::string STATUS_OK    = "STATUS_OK";
inline std::string STATUS_ERR   = "STATUS_ERR";

inline std::string DATA      = "DATA";
inline std::string DATA_SIZE = "DATA_SIZE";
inline std::string ROW       = "ROW";
inline std::string COLUMN    = "COLUMN";

inline std::string FILE_SIZE = "FILE_SIZE";
inline std::string FILE_DATA = "FILE_DATA";
inline std::string PING      = "PING";
inline std::string ON     	  = "ON";
inline std::string OFF       = "OFF";
inline std::string EMPTY_DATA = "EMPTY_DATA";
inline std::string RAW       = "RAW";
inline std::string STATS     = "STATS";
inline std::string SIZE     = "SIZE";

inline std::string SEQUENCE_NUMBER = "SEQUENCE_NUMBER";


class Payload {

private:
	/* Data structure that holds key-value massage pairs */
	std::map<std::string, std::string> metadata;

public:
	/* String that contains actual data i.e. file data, message etc */
	std::string data;

	/* Adds key-value pair to message */
	void add(std::string& key, std::string& value){
		metadata[key] = value;
	}

	/* Gets value for key in message */
	std::string get(std::string& key){
		return metadata[key]; 
	}

	/* Set data */
	void set_data(std::string& str) {
		data = str;
	}

	int data_size(){
		return std::stoi(metadata[DATA_SIZE]);
	}

	/* Serializes message key-value map to string */
	std::string serialize_metadata(){

		std::string serialized_message;

		/* Iterate through map and add key::value to serialized string */
		std::map<std::string, std::string>::iterator iterator = metadata.begin();
	 	while (iterator != metadata.end()) {

	 		/* Get key-value pair to be added to messaged */
			std::string key = iterator->first;
			std::string value = iterator->second;

			/* Add to serialized message as long its not the data field */
	 		if (key != DATA) {
	 			/* Add key-value pair separeted by DELIMITER */
		 		serialized_message += key + DELIMITER + value + DELIMITER;
	 		}

			iterator++;
		}
		return serialized_message;
	}

	/* Takes a serialized message string and writes it to class message map */
	void deserialize_metadata(std::string& serialized_message) {
		/* Iterate through serialize_message string and extract key-value pairs */
		size_t delimiter_index = 0;
		while ((delimiter_index = serialized_message.find(DELIMITER)) != std::string::npos) {
			/* Extract key */
		    std::string key = serialized_message.substr(0, delimiter_index);
		    serialized_message.erase(0, delimiter_index + DELIMITER.length());

		    /* Update delimiter index */
		    delimiter_index = serialized_message.find(DELIMITER);

		    /* Extract value */
		    std::string value = serialized_message.substr(0, delimiter_index);
		   	serialized_message.erase(0, delimiter_index + DELIMITER.length());

		   	/* Update message map */
		   	metadata[key] = value;
		}

	}


};

#endif