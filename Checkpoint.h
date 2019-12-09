
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <iostream>

std::string CHECKPOINT_INDEX = "CHECKPOINT_INDEX";
std::string CHECKPOINT_SEQ   = "CHECKPOINT_SEQ";

bool is_file(std::string& path) {
    std::ifstream f(path.c_str());
    return f.good();
}

std::string read_file(std::string& path) {
	/* Check if file exists else return "" */
	if (!is_file(path)){
		return "";
	}
	/* Open file */
	std::ifstream infile(path.c_str(), std::ifstream::binary);

	/* Get file size */
	infile.seekg(0, infile.end);
	long file_size = infile.tellg();
	infile.seekg(0);

	/* Initialize buffer to read file */
	char* buffer = new char[file_size];

	/* Read file */
	infile.read(buffer, file_size);

	/* Close file */
	infile.close();

	/* Return file as std::string 
	   Note: file_size is key! Since buffer might contain \0 */
	return std::string(buffer, file_size);
}

void write_string_to_file(std::string& string, std::string& path) {
	/* Open file */
	std::ofstream outfile(path.c_str(),std::ofstream::binary);

	/* Write file */
	outfile.write(string.c_str(), string.length());
 
 	/* Close file */
	outfile.close();
}

struct Checkpoint {
	std::map<std::string, std::map<std::string, std::string> > row_col_value;
	int write_sequence_number;
	int server_index;
};

std::string serialize_checkpoint(Checkpoint& checkpoint) {
	/* Create boost stringstream */
	std::stringstream ss;
//	boost::archive::text_oarchive archive(ss);
//	archive << checkpoint.row_col_value;

	return ss.str();
}

Checkpoint deserialize_checkpoint(std::string serialized_checkpoint) {
	std::map<std::string, std::map<std::string, std::string> > row_col_value;
	std::stringstream ss;
	ss.str(serialized_checkpoint);
//	boost::archive::text_iarchive archive(ss);
//	archive >> row_col_value;

	/* Re-build Checkpoint */
	Checkpoint checkpoint;
	checkpoint.row_col_value = row_col_value;
	checkpoint.server_index = std::stoi(row_col_value[CHECKPOINT_INDEX][CHECKPOINT_INDEX]);
	checkpoint.write_sequence_number = std::stoi(row_col_value[CHECKPOINT_SEQ][CHECKPOINT_SEQ]);

	return checkpoint;
}

/* Ask about temp file ???????????? */
void write_checkpoint_to_disk(Checkpoint& checkpoint) {

	/* First write to checkpoint to temp file */
	std::string temp_file_path = "./Checkpoints/server_" + std::to_string(checkpoint.server_index) + ".checkpoint";

	std::string serialized_checkpoint = serialize_checkpoint(checkpoint);

	write_string_to_file(serialized_checkpoint, temp_file_path);

	if (true) printf("[S] Wrote checkpoint with sequence number %d to disk.\n", checkpoint.write_sequence_number);
}

Checkpoint read_checkpoint(int server_index) {

	std::string temp_file_path = "./Checkpoints/server_" + std::to_string(server_index) + ".checkpoint";

	std::string serialized_checkpoint = read_file(temp_file_path);

	/* If its there are no old checkpoint return blank one */
	if (serialized_checkpoint.length() == 0) {
		std::cout << "[S] Creating new empty checkpoint." << std::endl;
		Checkpoint empty_checkpoint;
		empty_checkpoint.server_index = server_index;
		empty_checkpoint.write_sequence_number = 0;
		return empty_checkpoint;
	}

	return deserialize_checkpoint(serialized_checkpoint);

}

























