#include "command_operation.h"
#include "server_util.h"

// Handles the "list" command.
void handle_list_command(const char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo) {
    // Check if the command is "list"
    if (strcmp(command, "list") == 0)
    {
        // Enter the reader region of the synchronization file
        enterRegionReader(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));
        
        // Clear the client response FIFO
        ftruncate(*client_res_fd, 0);
        
        DIR *dir;
        struct dirent *ent;

        // Open the specified directory
        if ((dir = opendir(dirname)) != NULL) 
        {
            int hasEntry = 0; // Flag to check if there is any file or folder
            
            // Iterate over each entry in the directory
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type == DT_DIR || ent->d_type == DT_REG) {
                    char *name = ent->d_name;
                    
                    // Exclude hidden files and directories
                    if (name[0] != '.') {
                        // printf("Entry name: %s and fd %d\n", name, *client_res_fd);
                        
                        // Write the entry name to the client response FIFO
                        int bytes_written = write(*client_res_fd, name, strlen(name));
                        if (bytes_written == -1) {
                            perror("Failed to write to client response FIFO");
                            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
                            exitRegionReader(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));
                            exit(EXIT_FAILURE);
                        }

                        // Write a newline character to separate entries
                        write(*client_res_fd, "\n", 1);
                        
                        hasEntry = 1;
                    }
                }
            }

            closedir(dir);

            // If there is no file or folder, write an error message to the client response FIFO
            if (!hasEntry) {
                const char* error_message = "Nothing has been found.";
                if (write(*client_res_fd, error_message, strlen(error_message)) == -1)
                {
                    perror("Failed to write error message to client response FIFO");
                    cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
                    exitRegionReader(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));
                    exit(EXIT_FAILURE);
                }
            }
            
            // Close the client response FIFO
            close(*client_res_fd);
            *client_res_fd = -1;
        } 

        else
        {
            perror("Failed to open directory:");
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            exitRegionReader(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));
            exit(EXIT_FAILURE);
        }
        
        // Exit the reader region of the synchronization file
        exitRegionReader(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));

        int garbage;
        ssize_t num_read = read(server_req_fd, &garbage, sizeof(int));
        if(num_read == -1) {
            perror("Failed to read from server request FIFO");
        }

        if (*client_res_fd == -1)
        {
            // printf("Client response FIFO is closed. Reopening...\n");
            // Reopen the client response FIFO
            *client_res_fd = open(client_res_fifo, O_WRONLY);            
            if (*client_res_fd == -1)
            {
                // Failed to reopen the client response FIFO
                perror("Failed to reopen client response FIFO");
                cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
                exitRegionReader(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));
                exit(EXIT_FAILURE);
            }
        }
    }

    char log_message[512];
    sprintf(log_message, "Client requested to list the files in the server directory.\n");
    write_log_file(log_message, dir_syncs);
}

// Handles the "readF" command.
void handle_readF_command(char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo) {
    // Parse the command into separate words
    char *words[4];
    int num_words = 0;
    char *token = strtok(command, " ");
    while (token != NULL && num_words < 4) {
        words[num_words] = token;
        num_words++;
        token = strtok(NULL, " ");
    }

    readF_command readF;
    strcpy(readF.file, words[1]);
    if (num_words > 2) {
        readF.line_number = atoi(words[2]);
    } else {
        readF.line_number = -1;
    }
    // printf("File1: %s\n", readF.file);

    // printf("readf command\n");

    // File name is the directory of server + file name
    char file_path[512];
    sprintf(file_path, "%s/%s", dirname, readF.file);

    FILE *file = fopen(file_path, "r");
    // printf("File2: %s\n", file_path);
    if (file == NULL) {
        // perror("Failed to open file");

        // Write fifo error message
        const char *error_message = "NO_SUCH_FILE_05319346629";
        write(*client_res_fd, error_message, strlen(error_message));
        return;
    }
    
    enterRegionReader(getSafeFile(dir_syncs, readF.file)); // Enter the reader region of the synchronization file

    // If the user wants to read a specific line
    if (readF.line_number != -1) {
        handle_specific_line(file, readF, client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo);
    }
    
    // If the user wants to read the whole file
    else {
        handle_whole_file(file, readF, client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo);
    }

    // printf("eof is reached\n");
}

// Handles reading a specific line from a file.
void handle_specific_line(FILE *file, readF_command readF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo) {
    // printf("Line number: %d\n", readF.line_number);
    char line[256]; // Buffer to store the line read from the file
    int line_number = 0;

    // Read each line from the file until the desired line number is reached
    while (fgets(line, sizeof(line), file) != NULL)
    {
        line_number++;
        if (line_number == readF.line_number)
        {
            break;
        }
    }

    // If the desired line number is out of range, send an error message to the client
    if (line_number != readF.line_number)
    {
        const char *error_message = "Line number is out of range";
        if (write(*client_res_fd, error_message, strlen(error_message)) == -1)
        {
            perror("Failed to write error message to client response FIFO");
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            exitRegionReader(getSafeFile(dir_syncs, readF.file));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Write the line to the client response FIFO
        if (write(*client_res_fd, line, strlen(line)) == -1)
        {
            perror("Failed to write to client response FIFO");
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            exitRegionReader(getSafeFile(dir_syncs, readF.file));
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    close(*client_res_fd);
    *client_res_fd = -1;
    exitRegionReader(getSafeFile(dir_syncs, readF.file));

    // Read from the server request FIFO to synchronize with the client
    int garbage;
    ssize_t num_read = read(server_req_fd, &garbage, sizeof(int));
    if(num_read == -1) {
        perror("Failed to read from server request FIFO");
    }

    // If the client response FIFO is closed, reopen it
    if (*client_res_fd == -1)
    {
        // printf("Client response FIFO is closed. Reopening...\n");
        *client_res_fd = open(client_res_fifo, O_WRONLY);
        if (*client_res_fd == -1)
        {
            perror("Failed to open client response FIFO");
            close(server_fd);
            close(*client_res_fd);
            close(server_req_fd);
            exit(EXIT_FAILURE);
        }
    }
}

// Handles sending the entire contents of a file to the client.
void handle_whole_file(FILE *file, readF_command readF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo) {
    // printf("Whole file\n");
    char line[256]; // Buffer to store the line read from the file

    // Read each line from the file and send it to the client
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (write(*client_res_fd, line, strlen(line)) == -1)
        {
            perror("Failed to write to client response FIFO");
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            exitRegionReader(getSafeFile(dir_syncs, readF.file));
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    close(*client_res_fd);
    *client_res_fd = -1;
    exitRegionReader(getSafeFile(dir_syncs, readF.file));

    // Read from the server request FIFO to synchronize with the client
    int garbage;
    ssize_t num_read = read(server_req_fd, &garbage, sizeof(int));
    if(num_read == -1) {
        perror("Failed to read from server request FIFO");
    }

    // If the client response FIFO is closed, reopen it
    if (*client_res_fd == -1)
    {
        // printf("Client response FIFO is closed. Reopening...\n");
        *client_res_fd = open(client_res_fifo, O_WRONLY);
        if (*client_res_fd == -1)
        {
            perror("Failed to open client response FIFO");
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            exit(EXIT_FAILURE);
        }
    }
}

// Handles the "quit" command.
// Sends a response to the client indicating successful execution.
// Closes the server file descriptors and client response file descriptor.
// Removes the client from the client list.
// Prints a message indicating that the client is disconnected.
// Exits the program with a success status.
void handle_quit_command(int client_res_fd, int server_fd, int server_req_fd, struct client_info cli_info, struct client_list_wrapper *client_list, struct dir_sync *dir_syncs) {
    int response = 1;
    ssize_t num_written = write(client_res_fd, &response, sizeof(response));
    if (num_written == -1)
    {
        perror("Failed to write to client response FIFO");
        cleanup(server_fd, client_res_fd, server_req_fd, dir_syncs);
        exit(EXIT_FAILURE);
    }

    // Close the server file descriptors and client response file descriptor
    // cleanup(server_fd, client_res_fd, server_req_fd, dir_syncs);
    
    remove_client(client_list, cli_info.pid); // Remove the client from the client list
    printf("Client %d is disconnected!\n", cli_info.pid);
    char log_message[512];
    sprintf(log_message, "Client %d is disconnected.\n", cli_info.pid);
    write_log_file(log_message, dir_syncs);
    exit(EXIT_SUCCESS);
}

// Handles the "killServer" command.
void handle_killServer_command(int client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, struct client_list_wrapper *client_list) {
    int response = 1;
    ssize_t num_written = write(client_res_fd, &response, sizeof(int));
    if(num_written == -1) {
        perror("Failed to write to client response FIFO");
    }

    // Close the server file resources and client response file descriptor
   cleanup(server_fd, client_res_fd, server_req_fd, dir_syncs);

    // Traverse and send SIGINT signal to all child pids in the client list
    for (int i = 0; i < client_list->counter; i++)
    {
        printf("Killing client %d\n", client_list->clients[i]);
        kill(client_list->clients[i], SIGINT);
    }

    // Send SIGTERM to the entire process group
    if (kill(-getpgid(0), SIGINT) == -1) {
        perror("Failed to send SIGTERM to the process group");
        exit(EXIT_FAILURE);
    }
}

// Handles the "writeF" command.
void handle_writeF_command(char* command, char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs)
{
    // Parse the command into separate words
    char *words[4];
    int num_words = 0;
    char *token = strtok(command, " ");
    while (token != NULL && num_words < 5) {
        words[num_words] = token;
        num_words++;
        token = strtok(NULL, " ");
    }

    writeF_command writeF; // Structure to store the writeF command
    strcpy(writeF.file,words[1]); // Copy the file name to the structure
    if(num_words>3) 
    {
        writeF.line_number = atoi(words[2]);
        strcpy(writeF.string, words[3]);
    }

    else 
    {
        writeF.line_number = -1;
        strcpy(writeF.string, words[2]);
    }

    // printf("File1: %s\n", writeF.file);

    // If the user wants a specific line
    if(writeF.line_number!=-1)
    {
        handle_specific_line_write(dirname, writeF, client_res_fd, server_fd, server_req_fd, dir_syncs);
    }
    
    // If the user wants the whole file
    else
    {
        handle_whole_file_write(dirname, writeF, client_res_fd, server_fd, server_req_fd, dir_syncs);
    }

    // printf("eof is reached\n");
}

// Handles writing a specific line to a file.
void handle_specific_line_write(char *dirname, writeF_command writeF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs) {
    // printf("Line number: %d\n", writeF.line_number);
    char line[256]; // Buffer to store the line read from the file
    int line_number = 0; // Counter to keep track of the line number
    // long int file_position = 0; // Position in the file
    int successful = 0; // Flag to indicate if the write operation was successful


    // File name is the directory of server + file name
    char file_path[512];
    sprintf(file_path, "%s/%s", dirname, writeF.file);
    // printf("Line number: %d\n", writeF.line_number);

    if(getSafeFile(dir_syncs, writeF.file) == NULL) 
        addSafeFile(dir_syncs, writeF.file);

    // Step 1: Open the Source File for Reading
    FILE *source_file = fopen(file_path, "r");
    if (source_file == NULL) {
        perror("Failed to open source file");
        fflush(stdout);
        int resp = -1;
        write(*client_res_fd, &resp, sizeof(int));
        removeSafeFile(dir_syncs, writeF.file);
        exitRegionWriter(getSafeFile(dir_syncs, file_path));
    }
    // printf("Line number: %d\n", writeF.line_number);

    // Step 2: Create a Temporary File for Writing
    FILE *temp_file = fopen("temp.txt", "w");
    if (temp_file == NULL) {
        perror("Failed to create temporary file");
        fclose(source_file);
        cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
        exitRegionWriter(getSafeFile(dir_syncs, file_path));
        exit(EXIT_FAILURE);
    }
    // printf("Line number: %d\n", writeF.line_number);

    // Step 3: Read from the Source File and Write to the Temporary File
    while (fgets(line, sizeof(line), source_file) != NULL) {
        if (ferror(source_file)) {
            perror("Failed to read from source file");
            fclose(source_file);
            fclose(temp_file);
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            exitRegionWriter(getSafeFile(dir_syncs, file_path));
            exit(EXIT_FAILURE);
        }
        line_number++;

        // Check if the current line is the line to be replaced
        if (line_number == writeF.line_number) {
            fputs(line, temp_file);
            fputs(writeF.string, temp_file);
            successful = 1;
        } else {
            fputs(line, temp_file);
        }
    }
    // printf("Line number: %d\n", writeF.line_number);

    // Step 4: Replace the Original File
    fclose(source_file);
    fclose(temp_file);
    remove(file_path);
    rename("temp.txt", file_path);
    // printf("Line number: %d\n", writeF.line_number);
    exitRegionWriter(getSafeFile(dir_syncs, writeF.file));

    int asdf = write(*client_res_fd, &successful, sizeof(int)) ;
    if (asdf == -1) {
        perror("Failed to write response to client");
        cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
        exit(EXIT_FAILURE);
    }
    
    char log_message[512];
    sprintf(log_message, "Line %d of file %s has been written by the client.\n", writeF.line_number, writeF.file);
    write_log_file(log_message, dir_syncs);
}

// Handles writing the entire contents of a file.
void handle_whole_file_write(const char *dirname, writeF_command writeF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs) {
    // printf("Whole file\n");
    char file_path[512]; // Path to the file
    int success = 0; // Flag to indicate if the write operation was successful

    // File name is the directory of server + file name
    sprintf(file_path, "%s/%s", dirname, writeF.file);

    // If there is no file name in the synchronization file, add it
    if(getSafeFile(dir_syncs, writeF.file) == NULL) 
        addSafeFile(dir_syncs, writeF.file);

    // Enter the writer region of the synchronization file
    enterRegionWriter(getSafeFile(dir_syncs, writeF.file));

    // Check if the file exists
    if (access(file_path, F_OK) == -1) {
        // printf("File does not exist\n");

        // File does not exist, create it and write the string
        FILE *file = fopen(file_path, "w");
        if (file == NULL) {
            perror("Failed to create file");
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            removeSafeFile(dir_syncs, writeF.file); // Remove the file from the synchronization file since there is no such file
            exitRegionWriter(getSafeFile(dir_syncs, writeF.file));
            exit(EXIT_FAILURE);
        }
        fputs(writeF.string, file);
        fclose(file);
        success = 1; // Set the success flag to true
    }

    else {
        // File exists, open it and write the string
        FILE *file = fopen(file_path, "a");
        if (file == NULL) {
            perror("Failed to open file");
            exitRegionWriter(getSafeFile(dir_syncs, writeF.file));
        }
        fputs(writeF.string, file);
        fclose(file);
        success = 1; // Set the success flag to true
    }

    exitRegionWriter(getSafeFile(dir_syncs, writeF.file));

     if (write(*client_res_fd, &success, sizeof(int)) == -1) {
        perror("Failed to write to client response FIFO");
        cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
        exitRegionWriter(getSafeFile(dir_syncs, writeF.file));
        exit(EXIT_FAILURE);
    }

    char log_message[512];
    sprintf(log_message, "File %s has been written by the client.\n", writeF.file);
    write_log_file(log_message, dir_syncs);
}

// Handles the "download" command.
void handle_download_command(char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo) {
    // Parse the command into separate words
    char *words[4];
    int num_words = 0;
    char *token = strtok(command, " ");
    while (token != NULL && num_words < 2) {
        words[num_words] = calloc(strlen(token) + 1, sizeof(char));
        memset(words[num_words], '\0', strlen(token) + 1);
        strcpy(words[num_words], token);
        num_words++;
        token = strtok(NULL, " ");
    }

    download_command download; // Structure to store the download command
    strcpy(download.file,words[1]); // Copy the file name to the structure
    // printf("File1: %s - %d\n", download.file, strlen(download.file));

    // printf("download command\n");

    // If there is no file name in the synchronization file, add it
    if(getSafeFile(dir_syncs, download.file) == NULL) 
        addSafeFile(dir_syncs, download.file);
    enterRegionReader(getSafeFile(dir_syncs, download.file));

    // File name is the directory of server + file name
    char file_path[512];
    sprintf(file_path, "%s/%s", dirname, download.file);

    FILE *file = fopen(file_path, "r");
    // printf("File2: %s\n", file_path);
    if (file == NULL)
    {
        perror("Failed to open file");
        const char *error_message = "NO_SUCH_FILE_05319346629";
        char log_message[512];
        sprintf(log_message, "Client requested file %s, but it does not exist.\n", download.file);
        write_log_file(log_message, dir_syncs);
        if (write(*client_res_fd, error_message, strlen(error_message)) == -1)
        {
            perror("Failed to write error message to client response FIFO");
            close(server_fd);
        }
        close(*client_res_fd);
        *client_res_fd = -1;
        exitRegionReader(getSafeFile(dir_syncs, download.file));
        removeSafeFile(dir_syncs, download.file);

        int garbage;
        ssize_t num_read = read(server_req_fd, &garbage, sizeof(int));
        if(num_read == -1) {
            perror("Failed to read from server request FIFO");
        }
        // printf("num_read: %zd\n", num_read);
        // if it is closed, open it: *client_res_fd
        if (*client_res_fd == -1)
        {
            // printf("Client response FIFO is closed. Reopening...\n");
            *client_res_fd = open(client_res_fifo, O_WRONLY);
            if (*client_res_fd == -1)
            {
                for(int i = 0; i < num_words; i++) {
                    free(words[i]);
                }
                perror("Failed to open client response FIFO");
                cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
                exit(EXIT_FAILURE);
            }
        }
        return;
    }

    char buffer[256]; // Buffer to store the data read from the file
    ssize_t num_read;
    ssize_t total_bytes = 0;
    while ((num_read = read(fileno(file), buffer, sizeof(buffer))) > 0)
    {
        if (total_bytes += write(*client_res_fd, buffer, num_read) == -1)
        {
            for(int i = 0; i < num_words; i++) {
                free(words[i]);
            }
            perror("Failed to write to client response FIFO");
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            exitRegionReader(getSafeFile(dir_syncs, download.file));
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    close(*client_res_fd);
    *client_res_fd = -1;
    exitRegionReader(getSafeFile(dir_syncs, download.file));
    /// Read from the server request FIFO to synchronize with the client
    int garbage;
    num_read = read(server_req_fd, &garbage, sizeof(int));
    // printf("garbage readed: %d\n", garbage);
    fflush(stdout);
    if(num_read == -1) {
        perror("Failed to read from server request FIFO");
    }
    // printf("num_read: %zd\n", num_read);

    // if it is closed, open it: *client_res_fd
    if (*client_res_fd == -1)
    {
        // printf("Client response FIFO is closed. Reopening...\n");
        *client_res_fd = open(client_res_fifo, O_WRONLY);
        if (*client_res_fd == -1)
        {
            for(int i = 0; i < num_words; i++) {
                free(words[i]);
            }
            perror("Failed to open client response FIFO");
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            exit(EXIT_FAILURE);
        }
    }
    for(int i = 0; i < num_words; i++) {
        free(words[i]);
    }
    char log_message[512];
    sprintf(log_message, "File %s has been downloaded by the client. Total %ld bytes is downloaded.\n", download.file, total_bytes);
}

// Handles the "upload" command.
void handle_upload_command(char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs) 
{
    // Parse the command into separate words
    char *words[4];
    int num_words = 0;
    char *token = strtok(command, " ");
    while (token != NULL && num_words < 4) {
        words[num_words] = token;
        num_words++;
        token = strtok(NULL, " ");
    }

    upload_command upload; // Structure to store the upload command
    strcpy(upload.file,words[1]); // Copy the file name to the structure
    // printf("File1: %s\n", upload.file);

    // printf("upload command\n");
    int success = 1;
    if (write(*client_res_fd, &success, sizeof(int)) == -1) {
        perror("Failed to write to client response FIFO");
        cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
        exitRegionWriter(getSafeFile(dir_syncs, upload.file));
        exit(EXIT_FAILURE);
    }

    if(getSafeFile(dir_syncs, upload.file) == NULL) 
        addSafeFile(dir_syncs, upload.file);
    
    // Enter the writer region of the synchronization file
    enterRegionWriter(getSafeFile(dir_syncs, upload.file));
    enterRegionWriter(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));

    // File name is the directory of server + file name
    char file_path[512];
    sprintf(file_path, "%s/%s", dirname, upload.file);

    FILE *file = fopen(file_path, "w");
    // printf("File2: %s\n", file_path);
    if (file == NULL)
    {
        perror("Failed to open file");
        cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
        exitRegionWriter(getSafeFile(dir_syncs, upload.file));
        exitRegionWriter(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));
        exit(EXIT_FAILURE);
    }

    // printf("Whole file\n");
    char line[256]; // Buffer to store the line read from the file
    ssize_t total_bytes = 0;
    while (1)
    {
        ssize_t num_read = read(server_req_fd, line, sizeof(line));
        total_bytes += num_read;
        if (num_read == -1)
        {
            perror("Failed to read from server request FIFO");
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            exitRegionWriter(getSafeFile(dir_syncs, upload.file));
            exitRegionWriter(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));
            exit(EXIT_FAILURE);
        }
        if (num_read == 0)
        {
            break;
        }
        if (write(fileno(file), line, num_read) == -1)
        {
            perror("Failed to write to file");
            close(server_fd);
        }
    }

    fclose(file);
    int garbage = 1;
    int bytes_written = write(*client_res_fd, &garbage, sizeof(int));
    // printf("Bytes written: %d\n", bytes_written);
    if (bytes_written == -1) {
        perror("Failed to write to client_res_fd");
        cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
        exitRegionWriter(getSafeFile(dir_syncs, upload.file));
        exitRegionWriter(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));
        exit(EXIT_FAILURE);
    }

    exitRegionWriter(getSafeFile(dir_syncs, upload.file));
    exitRegionWriter(getSafeFile(dir_syncs, UPLOAD_AND_LIST_SYNC_FILE));

    char log_message[512];
    sprintf(log_message, "File %s has been uploaded by the client with %ld bytes\n", upload.file, total_bytes);
    write_log_file(log_message, dir_syncs);
}

// Handles the "archive" command.
void handle_archive_command(char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs)
{
    // Parse the command into separate words
    char *words[4];
    int num_words = 0;
    char *token = strtok(command, " ");
    while (token != NULL && num_words < 4) {
        words[num_words] = token;
        num_words++;
        token = strtok(NULL, " ");
    }

    arch_command archive;
    strcpy(archive.file,words[1]);
    // printf("File1: %s\n", archive.file);
    // printf("archive command\n");

    if(getSafeFile(dir_syncs, archive.file) == NULL) 
        addSafeFile(dir_syncs, archive.file);
    putAllFilesToCriticalSection(dir_syncs);

    // File name is the directory of server + file name
    char file_path[512];
    sprintf(file_path, "%s/%s", dirname, archive.file);

    // Fork and use "execve" function in child. Archive all files in directory and save them into [file_path].tar
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Failed to fork child process");
        cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
        removeAllFilesFromCriticalSection(dir_syncs);
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        if (chdir(dirname) != 0) {
            cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
            removeAllFilesFromCriticalSection(dir_syncs);
            exit(EXIT_FAILURE);
        }

        char *args[] = {"tar", "-cf", archive.file, ".", NULL};
        execvp("tar", args);
        perror("Failed to execute tar");
        cleanup(server_fd, *client_res_fd, server_req_fd, dir_syncs);
        removeAllFilesFromCriticalSection(dir_syncs);
        exit(EXIT_FAILURE);
    }

    // Wait for the child process to finish
    int status;
    waitpid(pid, &status, 0);
    removeAllFilesFromCriticalSection(dir_syncs);

    int response = 1;
    ssize_t num_written = write(*client_res_fd, &response, sizeof(response));
    if(num_written == -1) {
        perror("Failed to write to client response FIFO");
    }
}

// Handles the "help" command.
void handle_help_command(int client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs){
const char *help_message = "Commands:\n"
                               "list: List all files and directories in the server directory\n"
                               "readF <file> [line_number]: Read the file\n"
                               "writeF <file> <line_number> <string>: Write the string to the file at the specified line number\n"
                               "download <file>: Download the file\n"
                               "upload <file>: Upload the file\n"
                               "archive <file>: Archive the server directory\n"
                               "quit: Disconnect from the server\n"
                               "killServer: Kill the server\n";
    if (write(client_res_fd, help_message, strlen(help_message)) == -1) {
        perror("Failed to write to client response FIFO");
        cleanup(server_fd, client_res_fd, server_req_fd, dir_syncs);
        exit(EXIT_FAILURE);
    }
}


/*
* Returns string array of files in server directory
* @param command The command string.
* @param dirname The name of the directory to list.
* @param client_res_fd The file descriptor for the client response FIFO.
* @param server_fd The file descriptor for the server FIFO.
* @param server_req_fd The file descriptor for the server request FIFO.
* @param dir_syncs An array of dir_sync structures.
* @param client_res_fifo The name of the client response FIFO.
*/
void get_files_list(const char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char *client_res_fifo);