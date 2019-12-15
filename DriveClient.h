#ifndef DRIVE_CLIENT_H_
#define DRIVE_CLIENT_H_

#include <sys/time.h>
#include "BigTableClient.h"
//
//std::string CONFIG_PATH = "/Users/murri/Desktop/FrontEnd/final_merged/config.txt";
//int NUMBER_OF_PRIMARIES = 1;
//bool PRINT_LOGS = true;

const std::string _DRIVE = "_DRIVE";
std::string FILETREE = "FILETREE";

const std::string TYPE_FILE = "TYPE_FILE";
const std::string TYPE_DIRECTORY = "TYPE_DIRECTORY";

const std::string DRIVE_CLIENT_OK = "DRIVE_CLIENT_OK";
const std::string DUPLICATE_NAME = "DUPLICATE_NAME";
const std::string FORBIDDEN_CHARS = "FORBIDDEN_CHARS";
const std::string DIRECTORY_NOT_EXIST = "DIRECTORY_NOT_EXIST";
const std::string BACKEND_DEAD = "BACKEND_DEAD";

const std::string BACKSLASH = "/";
std::string INIT_DIRECTORY = "[]()";
const std::string DRIVE_DELIMITER = ";;";
const std::string F_SQ_BRACKET = "[";
const std::string R_SQ_BRACKET = "]";
const std::string F_PARENTHESIS = "(";
const std::string R_PARENTHESIS = ")";


//TODO: HANDLE FAILED BACKEND SERVER (RETURNS -1)
class DriveClient {

private:

    bool PRINT_LOGS;
    BigTableClient big_table;
    std::string drive_id;
    std::hash<std::string> hash_fxn;

public:
    /**
     * Create a BigTable object
     * Initialize drive_id
     * If user does not yet exist, create a file tree and an empty root directory
     * @param usr_name
     */
    void initialize(const std::string usr_name) {
        big_table.initialize(CONFIG_PATH, NUMBER_OF_PRIMARIES, PRINT_LOGS);
        drive_id = usr_name + _DRIVE;
        std::string response;
        std::string hash;
        if (big_table.get(drive_id, FILETREE, response) == -1) {
            // Create a new drive
            hash = compute_hash();
            std::string FILETREE_INIT_STATE = "[](/root" + DRIVE_DELIMITER + hash + DRIVE_DELIMITER + "[]())";
            big_table.put(drive_id, FILETREE, FILETREE_INIT_STATE);
            big_table.put(drive_id, hash, INIT_DIRECTORY);
        }
    }

    /**
     * Store a new file into big_table
     * There are no restrictions on file type or size
     * @param directory: current directory to upload into
     * @param file_name: name of the file
     * @param file: reference to file in string form
     * @return: FORBIDDEN_CHARS if file_name contains forbidden characters
     * @return: DUPLICATE_NAME if file_name already exists
     * @return: DRIVE_CLIENT_OK otherwise
     */
    std::string upload(const std::string directory, std::string file_name, std::string& file) {

        /*Check for forbidden characters in file name*/
        if (check_forbidden_chars(file_name)) {
            return FORBIDDEN_CHARS;
        }

        /*Check if file name already exists*/
        if (check_duplicates(directory, file_name, TYPE_FILE)) return DUPLICATE_NAME;

        /*Create a unique hash value for each file*/
        std::string hash = compute_hash();

        /*Add file_name and hash to parent directory and the file tree*/
        update_add(directory, file_name, hash, TYPE_FILE);

        // TODO: Check return value
        /*Put the file into the file tree*/
        big_table.put(drive_id, hash, file);

        return DRIVE_CLIENT_OK;
    }

    std::string make_dir(std::string directory, std::string folder_name) {

        /*Checks if forbidden characters exist in folder name*/
        if (check_forbidden_chars(folder_name)) {
            return FORBIDDEN_CHARS;
        }

        /*Checks if folder name already exists*/
        if (check_duplicates(directory, folder_name, TYPE_DIRECTORY)) return DUPLICATE_NAME;

        /*Creates a unique hash value for each folder*/
        std::string hash = compute_hash();

        /*Updates parent directory and file tree*/
        update_add(directory, folder_name, hash, TYPE_DIRECTORY);

        // TODO: Check return value
        /*Updates big table value*/
        int retval = big_table.put(drive_id, hash, INIT_DIRECTORY);
        return DRIVE_CLIENT_OK;
    }

    std::string download(std::string directory, std::string file_name, std::string& file) {
        std::string hash = get_hash(directory, file_name, TYPE_FILE);
        big_table.get(drive_id, hash, file);
        return DRIVE_CLIENT_OK;
    }

    // return
     std::string display(const std::string directory, std::map<std::string, std::vector<std::string>>& current_contents) {
        current_contents[TYPE_FILE] = std::vector<std::string>();
        current_contents[TYPE_DIRECTORY] = std::vector<std::string>();

        std::string hash = get_hash(directory, "", TYPE_DIRECTORY);
        std::string contents;
        big_table.get(drive_id, hash, contents);

        size_t begin_file_list = contents.find(F_SQ_BRACKET) + 1;
        size_t end_file_list = contents.find(R_SQ_BRACKET);
        std::string file_list = contents.substr(begin_file_list, end_file_list);

        size_t begin_dir_list = contents.find(F_PARENTHESIS) + 1;
        size_t end_dir_list = contents.find(R_PARENTHESIS);
        std::string dir_list = contents.substr(begin_dir_list, end_dir_list);

        size_t delimiter_index;
        while ((delimiter_index = file_list.find(DRIVE_DELIMITER)) != std::string::npos) {
            std::string name = file_list.substr(0,delimiter_index);
            file_list.erase(0, delimiter_index + DRIVE_DELIMITER.length());
            current_contents[TYPE_FILE].push_back(name);
        }

        while ((delimiter_index = dir_list.find(DRIVE_DELIMITER)) != std::string::npos) {
            std::string name = dir_list.substr(0,delimiter_index);
            name.erase(0, 1);
            dir_list.erase(0, delimiter_index + DRIVE_DELIMITER.length());
            current_contents[TYPE_DIRECTORY].push_back(name);
        }
        return DRIVE_CLIENT_OK;
    }

    std::string rename(std::string directory, std::string old_name, std::string new_name, const std::string type) {

        /*Checks if forbidden characters exist in name*/
        if (check_forbidden_chars(new_name)) {
            return FORBIDDEN_CHARS;
        }

        /*Checks if name already exists*/
        if (check_duplicates(directory, new_name, TYPE_FILE)) return DUPLICATE_NAME;

        /*Get file tree from big table*/
        std::string file_tree;
        big_table.get(drive_id, FILETREE, file_tree);

        /*Get contents of directory*/
        std::string hash = get_hash(directory, "", TYPE_DIRECTORY);
        std::string contents;
        big_table.get(drive_id, hash, contents);


        /*Find the location of the directory in the file tree*/

        if (type == TYPE_FILE) {
            old_name = old_name + DRIVE_DELIMITER;
            new_name = new_name + DRIVE_DELIMITER;
            size_t location_tree = find_dir_in_tree(file_tree, directory);
            location_tree = file_tree.find(F_SQ_BRACKET, location_tree);
            location_tree = file_tree.find(old_name, location_tree);

            size_t location_contents = contents.find(F_SQ_BRACKET);
            location_contents = contents.find(old_name, location_contents);

            std::string first_half_tree = file_tree.substr(0, location_tree);
            std::string second_half_tree = file_tree.substr(location_tree + old_name.size());
            std::string new_file_tree = first_half_tree + new_name + second_half_tree;

            std::string first_half_contents = contents.substr(0, location_contents);
            std::string second_half_contents = contents.substr(location_contents + old_name.size());
            std::string new_contents = first_half_contents + new_name + second_half_contents;

            big_table.put(drive_id, FILETREE, new_file_tree);
            big_table.put(drive_id, hash, new_contents);
        } else {
            old_name = BACKSLASH + old_name;
            new_name = BACKSLASH + new_name;
            std::string current_directory = directory + old_name;

            size_t location_tree = find_dir_in_tree(file_tree, current_directory);

            std::string first_half_tree = file_tree.substr(0, location_tree);
            std::string second_half_tree = file_tree.substr(location_tree + old_name.size());
            std::string new_file_tree = first_half_tree + new_name + second_half_tree;
            big_table.put(drive_id, FILETREE, new_file_tree);

            size_t location_contents = contents.find(F_PARENTHESIS);
            location_contents = contents.find(old_name, location_contents);
            std::string first_half_contents = contents.substr(0, location_contents);
            std::string second_half_contents = contents.substr(location_contents + old_name.size());
            std::string new_contents = first_half_contents + new_name + second_half_contents;
            big_table.put(drive_id, hash, new_contents);
        }

        return DRIVE_CLIENT_OK;
    }

    std::string remove(std::string directory, std::string name, const std::string type) {

        /*Get file tree from big table*/
        std::string file_tree;
        big_table.get(drive_id, FILETREE, file_tree);

        /*Get contents of directory*/
        std::string hash_parent = get_hash(directory, "", TYPE_DIRECTORY);
        std::string contents;
        big_table.get(drive_id, hash_parent, contents);

        if (type == TYPE_FILE) {
            /* Delete from file tree */
            size_t location_tree = find_dir_in_tree(file_tree, directory);
            location_tree = file_tree.find(F_SQ_BRACKET, location_tree);
            name += DRIVE_DELIMITER;

            location_tree = file_tree.find(name, location_tree);


            /*Get the hash of the file*/
            size_t hash_begin = file_tree.find(DRIVE_DELIMITER, location_tree) + DRIVE_DELIMITER.length();
            size_t hash_end = file_tree.find(DRIVE_DELIMITER, hash_begin);
            std::string file_hash = file_tree.substr(hash_begin, hash_end - hash_begin);

            file_tree.erase(location_tree, hash_end + DRIVE_DELIMITER.length() - location_tree);
            big_table.put(drive_id, FILETREE, file_tree);

            /* Delete from directory content list */
            size_t location_list = contents.find(name);
            contents.erase(location_list, name.length());
            big_table.put(drive_id, hash_parent, contents);

            /* Delete from big table */
            big_table.del(drive_id, file_hash);

        } else {
            std::string current_directory = directory + BACKSLASH + name;

            size_t location_tree = find_dir_in_tree(file_tree, current_directory);

            /* Delete from directory content list */
            name = BACKSLASH + name + DRIVE_DELIMITER;
            size_t location_list = contents.find(name);
            contents.erase(location_list, name.length());
            big_table.put(drive_id, hash_parent, contents);

            /* Delete from file tree iteratively */

            size_t begin = file_tree.find(F_SQ_BRACKET, location_tree) + 1;

            size_t end = file_tree.find(F_PARENTHESIS, begin) + 1;

            int count = 1;

            for (size_t ii = end; ii < file_tree.length(); ii ++) {
                end ++;
                if (file_tree[ii] == '(') {
                    count ++;
                } else if (file_tree[ii] == ')') {
                    count --;
                }
                if (count == 0) {
                    break;
                }
            }
            std::string to_be_deleted = file_tree.substr(begin, end - begin);
            file_tree.erase(location_tree, end - location_tree);
            big_table.put(drive_id, FILETREE, file_tree);

            begin = to_be_deleted.find(DRIVE_DELIMITER);

            while(begin != std::string::npos) {
                begin += DRIVE_DELIMITER.length();
                end = to_be_deleted.find(DRIVE_DELIMITER, begin);
                std::string hash = to_be_deleted.substr(begin, end - begin);
                big_table.del(drive_id, hash);
                end += DRIVE_DELIMITER.length();
                begin = to_be_deleted.find(DRIVE_DELIMITER, end);
            }
        }
        return DRIVE_CLIENT_OK;
    }

    std::string move(std::string directory, std::string name, std::string to_directory, const std::string type) {

        /*Get file tree from big table*/
        std::string file_tree;
        big_table.get(drive_id, FILETREE, file_tree);

        /*Get contents of the old directory*/
        size_t from_dir_location_in_tree = find_dir_in_tree(file_tree, directory);
        std::string hash_old_dir = get_hash(directory, "", TYPE_DIRECTORY);
        std::string old_dir_contents;
        big_table.get(drive_id, hash_old_dir, old_dir_contents);

        /*Get contents of the new directory*/
        size_t to_dir_location_in_tree = find_dir_in_tree(file_tree, to_directory);
        std::string hash_new_dir = get_hash(to_directory, "", TYPE_DIRECTORY);
        if (hash_new_dir == DIRECTORY_NOT_EXIST) return DIRECTORY_NOT_EXIST;

        /*Checks if name already exists*/
        if (check_duplicates(to_directory, name, type)) return DUPLICATE_NAME;

        std::string new_dir_contents;
        big_table.get(drive_id, hash_new_dir, new_dir_contents);

        if (type == TYPE_FILE) {
            name += DRIVE_DELIMITER;
            size_t begin = file_tree.find(name, from_dir_location_in_tree);
            size_t end = file_tree.find(DRIVE_DELIMITER, begin) + DRIVE_DELIMITER.length();
            end = file_tree.find(DRIVE_DELIMITER, end) + DRIVE_DELIMITER.length();

            std::string file_in_tree = file_tree.substr(begin, end - begin);
            file_tree.erase(begin, end - begin);
            /*Returns an error if directory does not exist*/
            size_t to_dir_location_in_tree = find_dir_in_tree(file_tree, to_directory);

            size_t to_location = file_tree.find(F_SQ_BRACKET, to_dir_location_in_tree) + 1;

            file_tree = file_tree.insert(to_location, file_in_tree);

            big_table.put(drive_id, FILETREE, file_tree);

            /* Delete from old_directory content list */
            size_t location_list = old_dir_contents.find(name);
            old_dir_contents.erase(location_list, name.length());
            big_table.put(drive_id, hash_old_dir, old_dir_contents);

            /* Add to new_directory content list */
            location_list = new_dir_contents.find(F_SQ_BRACKET) + 1;
            new_dir_contents = new_dir_contents.insert(location_list, name);
            big_table.put(drive_id, hash_new_dir, new_dir_contents);

        } else {
            // Moving a directory

            std::string directory_to_move = directory + BACKSLASH + name;
            size_t dir_to_move_begin = find_dir_in_tree(file_tree, directory_to_move);

            size_t dir_to_move_end = file_tree.find(F_PARENTHESIS, dir_to_move_begin);

            int count = 0;

            for (size_t ii = dir_to_move_end; ii < file_tree.length(); ii ++) {
                if (file_tree[ii] == '(') {
                    count ++;
                } else if (file_tree[ii] == ')') {
                    count --;
                }
                if (count == 0) {
                    break;
                }
                dir_to_move_end ++;
            }
            std::string dir_contents = file_tree.substr(dir_to_move_begin, dir_to_move_end - dir_to_move_begin);

            /*Delete from file tree*/
            file_tree.erase(dir_to_move_begin, dir_to_move_end - dir_to_move_begin);

            size_t place_to_insert = file_tree.find(F_PARENTHESIS, to_dir_location_in_tree) + 1;
            file_tree.insert(place_to_insert, dir_contents);
            big_table.put(drive_id, FILETREE, file_tree);

            /* Delete from directory content list */
            name = BACKSLASH + name + DRIVE_DELIMITER;
            size_t location_old_dir = old_dir_contents.find(name);
            old_dir_contents.erase(location_old_dir, name.length());
            big_table.put(drive_id, hash_old_dir, old_dir_contents);

            /* Add to directory content list */
            size_t location_new_dir = new_dir_contents.find(F_PARENTHESIS) + 1;
            new_dir_contents.insert(location_new_dir, name);
            big_table.put(drive_id, hash_new_dir, new_dir_contents);
        }
        return DRIVE_CLIENT_OK;
    }

private:
    std::string get_hash(std::string directory, const std::string& file_name, const std::string& type) {

        /*Get file tree from big table*/
        std::string file_tree;
        big_table.get(drive_id, FILETREE, file_tree);

        /*Find the location of the directory in the file tree*/
        size_t location = find_dir_in_tree(file_tree, directory);
        if (location == -1) return DIRECTORY_NOT_EXIST;
        if (type == TYPE_FILE) {
            location = file_tree.find(F_SQ_BRACKET, location);
            location = file_tree.find(file_name, location);
            size_t hash_begin = file_tree.find(DRIVE_DELIMITER, location) + 2;
            size_t hash_end = file_tree.find(DRIVE_DELIMITER, hash_begin);
            std::string hash = file_tree.substr(hash_begin, hash_end - hash_begin);
            return hash;
        } else {
            /*Get the hash value of the parent directory*/
            size_t hash_begin = file_tree.find(DRIVE_DELIMITER, location) + 2;
            size_t hash_end = file_tree.find(DRIVE_DELIMITER, hash_begin);
            std::string hash = file_tree.substr(hash_begin, hash_end - hash_begin);
            return hash;
        }
    }

    const std::string& update_add (const std::string& directory, std::string name, const std::string& hash_value, const std::string& type) {

        /*Get file tree from big table*/
        std::string file_tree;
        big_table.get(drive_id, FILETREE, file_tree);

        /*Find the location of the directory in the file tree*/
        size_t loc_in_tree = find_dir_in_tree(file_tree, directory);

        /*Get the hash value of the parent directory*/
        size_t hash_begin = file_tree.find(DRIVE_DELIMITER, loc_in_tree) + 2;
        size_t hash_end = file_tree.find(DRIVE_DELIMITER, hash_begin);
        std::string dir_hash = file_tree.substr(hash_begin, hash_end - hash_begin);

        /*Get content from the directory*/
        std::string contents;
        big_table.get(drive_id, dir_hash, contents);

        /*Find the beginning of file/directory list to insert in tree and parent directory*/
        size_t list_begin;
        size_t file_tree_begin;
        if (type == TYPE_FILE) {
            list_begin = contents.find(F_SQ_BRACKET) + 1;
            file_tree_begin = file_tree.find(F_SQ_BRACKET, loc_in_tree) + 1;
        } else {
            name = BACKSLASH + name;
            list_begin = contents.find(F_PARENTHESIS) + 1;
            file_tree_begin = file_tree.find(F_PARENTHESIS, loc_in_tree) + 1;
        }

        /*Insert content into parent directory*/
        std::string list_first = contents.substr(0, list_begin);
        std::string list_second = contents.substr(list_begin);
        contents = list_first + name + DRIVE_DELIMITER + list_second;

        /*Insert content into parent filetree*/
        std::string file_tree_first = file_tree.substr(0, file_tree_begin);
        std::string file_tree_second = file_tree.substr(file_tree_begin);
        std::string new_file_tree;

        if (type == TYPE_FILE) {
            new_file_tree = file_tree_first + name + DRIVE_DELIMITER + hash_value + DRIVE_DELIMITER + file_tree_second;
        } else {
            new_file_tree = file_tree_first + name + DRIVE_DELIMITER + hash_value + DRIVE_DELIMITER + INIT_DIRECTORY + file_tree_second;
        }

        /*Store updated parent and filetree back to the big table*/
        big_table.put(drive_id, dir_hash, contents);
        big_table.put(drive_id, FILETREE, new_file_tree);

        return DRIVE_CLIENT_OK;
    }


    bool check_forbidden_chars(std::string& str) {
        return str.find(":") != std::string::npos ||
               str.find("/") != std::string::npos ||
               str.find("(") != std::string::npos ||
               str.find(")") != std::string::npos ||
               str.find("[") != std::string::npos ||
               str.find("]") != std::string::npos;
    }

    bool check_duplicates(std::string contents, std::string name, const std::string& type) {

        /*Get a list of files or a list of subdirectories*/
        size_t begin;
        size_t end;
        if (type == TYPE_FILE) {
            begin = contents.find(F_SQ_BRACKET) + 1;
            end = contents.find(R_SQ_BRACKET);
        } else {
            name = "/" + name;
            begin = contents.find(F_PARENTHESIS) + 1;
            end = contents.find(R_PARENTHESIS);
        }
        contents = contents.substr(begin, end - begin);

        /*Iterate through until a match is found or the end*/
        size_t token = contents.find(DRIVE_DELIMITER);
        while (token != std::string::npos) {
            std::string existing_name = contents.substr(0,token);
            if (existing_name == name) {
                return true;
            }
            contents.erase(0,token + 2);
            token = contents.find(DRIVE_DELIMITER);
        }

        return false;
    }

    std::string compute_hash() {

        struct  timeval  tv;
        gettimeofday(&tv,NULL);

        std::string hash_key = std::to_string(tv.tv_usec);
        std::size_t hash_value = hash_fxn(hash_key);

        std::string hash_value_str = std::to_string(hash_value);

        return hash_value_str;
    }

    size_t find_dir_in_tree(const std::string& file_tree, const std::string& directory) {
        int count = 0;
        size_t current_loc = 0;
        size_t prev = directory.find(BACKSLASH);
        size_t curr = directory.find(BACKSLASH, prev + 1);

        while(curr != std::string::npos) {
            std::string dir_name = directory.substr(prev, curr - prev);
            dir_name += DRIVE_DELIMITER;
            count = 0;
            size_t ii;
            for (ii = current_loc; ii < file_tree.length(); ii ++) {
                if (file_tree[ii] == '(') {
                    count ++;
                } else if (file_tree[ii] == ')') {
                    count --;
                } else if (file_tree[ii] == '/') {
                    if (count == 1 && file_tree.substr(ii, dir_name.length()) == dir_name) {
                        current_loc = ii;
                        break;
                    }
                }
                if (count < 0) {
                    /* If the directory is looped through but no file is found, return error*/
                    return -1;
                }
            }
            /* If the entire tree is looped through but no file is found, return error*/
            if (ii == file_tree.length()) return -1;
            prev = curr;
            curr = directory.find(BACKSLASH, prev + 1);
        }

        std::string dir_name = directory.substr(prev);
        dir_name += DRIVE_DELIMITER;
        count = 0;
        for (size_t ii = current_loc; ii < file_tree.length(); ii ++) {
            if (file_tree[ii] == '(') {
                count ++;
            } else if (file_tree[ii] == ')') {
                count --;
            } else if (file_tree[ii] == '/') {
                if (count == 1 && file_tree.substr(ii, dir_name.length()) == dir_name) {
                    current_loc = ii;
                    break;
                }
            }
        }

        return current_loc;
    }

};
#endif

