#include "client_command_operations.h"
int write_command_to_server_fifo(int fd_client_cmd, char *command);

// Handle the list command
void handle_list_command(int *fd_client_cmd, int *fd_client_res, char** words, void (*cleanup)()) {
    // Write the command to the server
    char *files = (char*)calloc(1024, sizeof(char));
    if(!write_command_to_server_fifo(*fd_client_cmd, words[0]))
    {
        printf("You cannot list at the time being!\n");
        free(files);
        return;
    }

    printf("Files in the server directory:\n");
    // Read the response from the server
    char response[256]; // Buffer to store the response
    ssize_t bytes_read_res;
    while (1) {
        bytes_read_res = read(*fd_client_res, response, sizeof(response) - 1); // Leave space for null terminator
        if (bytes_read_res == -1) {
            perror("Failed to read response from server");
            cleanup();
            free(files);
            exit(EXIT_FAILURE);
        } else if (bytes_read_res == 0) {
            // End of file or no more data
            break;
        }
        response[bytes_read_res] = '\0'; // Null-terminate the string
        printf("%s", response); // Print the response
    
    }
    // printf("files: %s\n", files);    
    if (bytes_read_res == -1)
    {
        perror("Failed to read response from server");
        cleanup();
        free(files);
        exit(EXIT_FAILURE);
    }

    // Send a garbage value to signal the end of the response
    int garbage = 1;
    int bytes_written = write(*fd_client_cmd, &garbage, sizeof(int));
    if (bytes_written == -1) {
        perror("Failed to write to *fd_client_cmd");
        cleanup();
        free(files);
        exit(EXIT_FAILURE);
    }
    free(files);
    // /* */
    
}

// Handle the quit command
void handle_quit_command(int *fd_client_cmd, int *fd_client_res, char* command, void (*cleanup)()) {
    // Write the quit command to the server
    //printf("command: %s\n", command);
    if(!write_command_to_server_fifo(*fd_client_cmd, command))
    {
        printf("You cannot quit at the time being!\n");
    }

    // Read the response from the server
    int response;
    ssize_t bytes_read_res = read(*fd_client_res, &response, sizeof(int));
    if (bytes_read_res == -1)
    {
        perror("Failed to read response from server");
        cleanup();
        exit(EXIT_FAILURE);
    }
    else if(response == 1)
    {
        // Server acknowledged the quit command
        cleanup();
        printf("You are disconnected!\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        // Server did not acknowledge the quit command
        cleanup();
        printf("You are disconnected. Bye!\n");
    }
}

// Handle the killServer command
void handle_killServer_command(int *fd_client_cmd, int *fd_client_res, char* command, void (*cleanup)()) {
    //printf("command: %s\n", command);

    // Write the command to the server
    if(!write_command_to_server_fifo(*fd_client_cmd, command))
    {
        printf("You cannot kill the server at the time being!\n");
        return;
    }

    // Read the response from the server
    int response;
    ssize_t bytes_read_res = read(*fd_client_res, &response, sizeof(int));
    if (bytes_read_res == -1)
    {
        perror("Failed to read response from server");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Print the response
    if(response==1)
    {
        cleanup();
        printf("Server is killed!\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("You cannot kill the server at the time being!\n");
    }
}

// Handle the help command
void handle_help_command() {
    printf("Commands:\n");
    printf("list: List the files in the server directory\n");
    printf("quit: Disconnect from the server\n");
    printf("killServer: Kill the server\n");
    printf("help: Display this help message\n");
}

// Handle the readF command
void handle_readF_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)()) {
    // Check the number of arguments
    if(num_words < 2 || num_words > 3)
    {
        printf("Invalid number of arguments!\n");
        return;
    }

    printf("Read request received. Reading file...\nRead string:\n");
    // Create a readF_command struct
    readF_command readF;
    if(num_words == 2)
    {
        strcpy(readF.file, words[1]);
        readF.line_number = -1;
    }
    else
    {
        strcpy(readF.file, words[1]);
        readF.line_number = atoi(words[2]);
    }

    // Write words array to the FIFO with spaces separated form
    char fifo_message[256]; // Buffer to store the message. It includes command and parameters
    strcpy(fifo_message, words[0]);
    for (int i = 1; i < num_words; i++) {
        strcat(fifo_message, " ");
        strcat(fifo_message, words[i]);
    }
    write(*fd_client_cmd, fifo_message, strlen(fifo_message));

    // Read the response from the server
    char response[256]; // Buffer to store the response
    int ct = 0;
    ssize_t bytes_read_res;
    while (1) {
        bytes_read_res = read(*fd_client_res, response, sizeof(response) - 1); // Leave space for null terminator
        if (bytes_read_res == -1) {
            perror("Failed to read response from server");
            cleanup();
            exit(EXIT_FAILURE);
        } else if (bytes_read_res == 0) {
            // End of file or no more data
            break;
        }
        
        response[bytes_read_res] = '\0'; // Null-terminate the string
        if(strcmp(response, "NO_SUCH_FILE_05319346629") == 0)
        {
            printf("File does not exist on the server!\n");
            return;
        }
        printf("%s", response); // Print the response
        ct++;
    }
    
    // Send a garbage value to signal the end of response
    int garbage = 1;
    int bytes_written = write(*fd_client_cmd, &garbage, sizeof(int));
    if (bytes_written == -1) {
        perror("Failed to write to *fd_client_cmd");
        cleanup();
        exit(EXIT_FAILURE);
    }
    printf("\n");
}

// Handle the writeF command
void handle_writeF_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)()) {
    // Check the number of arguments
    if(num_words < 3 || num_words > 4)
    {
        printf("Invalid number of arguments!\n");
        return;
    }

    printf("Write request received. Writing to file...\n");
    // Create a writeF_command struct
    writeF_command writeF;
    if(num_words==3)
    {
        strcpy(writeF.file, words[1]);
        strcpy(writeF.string, words[2]);
    }
    else
    {
        strcpy(writeF.file, words[1]);
        writeF.line_number = atoi(words[2]);
        strcpy(writeF.string, words[3]);
    }

    // Write words array to the FIFO with spaces separated form
    char fifo_message[256]; // Buffer to store the message. It includes command and parameters
    strcpy(fifo_message, words[0]);
    for (int i = 1; i < num_words; i++) {
        strcat(fifo_message, " ");
        strcat(fifo_message, words[i]);
    }
    write(*fd_client_cmd, fifo_message, strlen(fifo_message)); // Write the message to the FIFO

    // Read the response from the server
    int garbage = 1;
    ssize_t bytes_read_res = read(*fd_client_res, &garbage, sizeof(int)); // Leave space for null terminator
    if (bytes_read_res == -1) {
        perror("Failed to read response from server");
        cleanup();
        exit(EXIT_FAILURE);
    } else if (bytes_read_res == 0) {
        // End of file or no more data
        return;
    }
    
    if(garbage == 1) printf("File has been changed successfuly!\n");
    else printf("Line number is not valid or there is no such file on the server!\n");
}

// Handle the upload command
void handle_upload_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)(), const char* server_req_fifo) {

    if(num_words != 2) {
        printf("Invalid number of arguments!\n");
        return;
    }

    printf("File transfer request received. Beginning file transfer...\n");
    upload_command upload;
    strcpy(upload.file, words[1]);

    // Check if the file exists in the current directory without opening the file
    if (access(words[1], F_OK) != 0) {
        printf("File does not exist!\n");
        return;
    }

    // Write words array to the FIFO with spaces separated form
    char fifo_message[256];
    strcpy(fifo_message, words[0]);
    for (int i = 1; i < num_words; i++) {
        strcat(fifo_message, " ");
        strcat(fifo_message, words[i]);
    }
    write(*fd_client_cmd, fifo_message, strlen(fifo_message));

    // Read the response from the server
    int response_;
    ssize_t bytes_read_res = read(*fd_client_res, &response_, sizeof(int));
    if (bytes_read_res == -1) {
        perror("Failed to read response from server");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Open the file upload.file in read mode and send it to the server
    FILE* file = fopen(upload.file, "r");
    if (file == NULL) {
        perror("Failed to open file for reading");
        cleanup();
        exit(EXIT_FAILURE);
    }
    
    ssize_t total_bytes = 0;
    char buffer[256];
    ssize_t bytes_read;
    while ((bytes_read = read(fileno(file), buffer, sizeof(buffer))) > 0) {
        // Write the buffer to the server
        ssize_t bytes_written = write(*fd_client_cmd, buffer, bytes_read);
        if (bytes_written == -1) {
            perror("Failed to write to *fd_client_cmd");
            fclose(file);
            cleanup();
            exit(EXIT_FAILURE);
        }

        total_bytes += bytes_written;
    }
    fclose(file);
    close(*fd_client_cmd);
    *fd_client_cmd = -1;

    bytes_read_res = read(*fd_client_res, &response_, sizeof(int));
    if (bytes_read_res == -1) {
        perror("Failed to read response from server");
        cleanup();
        exit(EXIT_FAILURE);
    }

    if (*fd_client_cmd == -1) {
        // printf("Client response FIFO is closed. Reopening...\n");
        *fd_client_cmd = open(server_req_fifo, O_WRONLY);
        if (*fd_client_cmd == -1) {
            perror("Failed to open client response FIFO");
            cleanup();
            exit(EXIT_FAILURE);
        }
    }

    printf("Successfully uploaded %ld bytes\n", total_bytes);
}

// Handle the download command
void handle_download_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)()) {
    if(num_words != 2) {
        printf("Invalid number of arguments!\n");
        return;
    }

    printf("File transfer request received. Beginning file transfer...\n");
    char target_dir[256];
    // Get current dir
    getcwd(target_dir, sizeof(target_dir));
    // target_dir = current directory + /archive
    strcat(target_dir, "/archive");
    
    download_command download; // Create a download_command struct
    strcpy(download.file, words[1]);

    // Construct the command message to be sent to the server
    char fifo_message[256]; // Buffer to store the message. It includes command and parameters

    // Intilize fifo_message as \0
    memset(fifo_message, '\0', 256);

    strcpy(fifo_message, words[0]);
    for (int i = 1; i < num_words; i++) {
        strcat(fifo_message, " ");
        strcat(fifo_message, words[i]);
    }

    // Put \0 at the end of the message
    strcat(fifo_message, "\0");
    // Write the command message to the server
    write(*fd_client_cmd, fifo_message, strlen(fifo_message));
    // printf("fifo_written: %ld and %d\n", fifo_written, strlen(fifo_message));

    FILE *file;
        struct stat buffer;
        int exists = stat(download.file, &buffer);
        char newFilename[259];

        if (exists == 0) {
            // File exists
            char *dot = strrchr(download.file, '.');
            if (dot) {
                strncpy(newFilename, download.file, dot - download.file); // Copy filename up to the dot
                sprintf(newFilename + (dot - download.file), "(1)%s", dot); // Append (1) and extension
            } else {
                // No extension found
                sprintf(newFilename, "%s(1)", download.file);
            }

            strcpy(download.file, newFilename);
        } 
        file = fopen(download.file, "w");
        if (file == NULL) {
            perror("Failed to open file for writing");
            cleanup();
            exit(EXIT_FAILURE);
        }
    

    char response[256]; // Buffer to store the response
    ssize_t total_bytes = 0; // Number of bytes read from the server
    while (1) {
        // Read the response from the server
        ssize_t bytes_read_res = read(*fd_client_res, response, sizeof(response) - 1); // Leave space for null terminator
        if (bytes_read_res == -1) {
            perror("Failed to read response from server");
            cleanup();
            exit(EXIT_FAILURE);
        } else if (bytes_read_res == 0) {
            // End of file or no more data
            break;
        }
        else total_bytes += bytes_read_res;

        // printf("%s", response); // Print the response

        // Check if the file doesn't exist on the server
        if(strcmp(response, "NO_SUCH_FILE_05319346629") == 0) 
        {
            response[bytes_read_res] = '\0'; // Null-terminate the string
            printf("File does not exist on the server!\n");
            fclose(file);
            char current_dir[256];
            getcwd(current_dir, sizeof(current_dir));
            strcat(current_dir, "/");
            strcat(current_dir, words[1]);
            remove(current_dir);
            int garbage = 7;
            write(*fd_client_cmd, &garbage, sizeof(int));
            return;
        }

        // Append the read response to the file
        write(fileno(file), response, bytes_read_res);
    }

    // Close the file
    fclose(file);

    int garbage = 7;
    int bytes_written = write(*fd_client_cmd, &garbage, sizeof(int));
    if (bytes_written == -1) {
        perror("Failed to write to *fd_client_cmd");
        cleanup();
        exit(EXIT_FAILURE);
    }
    printf("Successfully downloaded %ld bytes\n", total_bytes);
}

// Handle the archive command
void handle_arch_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)(), int (*write_command_to_server_fifo)(int, char*)) {
    // char * new_words[4];
    // new_words[0] = (char*)malloc(256);
    // strcpy(new_words[0], "list"); 

    // handle_list_command(fd_client_cmd, fd_client_res, new_words, cleanup, 1);
    // free(new_words[0]);
    // Check if the number of arguments is valid
    if(num_words != 2) {
        printf("Invalid number of arguments!\n");
        return;
    }

    printf("Archive request received. Server files are being archived...\n");
    // Create an arch_command struct and copy the file name
    arch_command arch;
    strcpy(arch.file, words[1]);

    // Construct the command message to be sent to the server
    char fifo_message[256];
    strcpy(fifo_message, words[0]);
    for (int i = 1; i < num_words; i++) {
        strcat(fifo_message, " ");
        strcat(fifo_message, words[i]);
    }

    // Write the command message to the server
    if(!write_command_to_server_fifo(*fd_client_cmd, fifo_message)) {
        printf("You cannot archive the file at the time being!\n");
        return;
    }

    int garbage = 1;
    read(*fd_client_res, &garbage, sizeof(int));
    
    char **new_words = (char**)malloc(2*sizeof(char*));
    for(int i = 0; i < 2; i++)
    {
        new_words[i] = (char*)calloc(256, sizeof(char));
    }

    strcpy(new_words[0], "download_arch");
    strcpy(new_words[1], words[1]);

    handle_download_command(fd_client_cmd, fd_client_res, new_words, 2, cleanup);
    for(int i = 0; i < 2; i++)
    {
        free(new_words[i]);
    }
    free(new_words);

    // Open the file words[1].tar in write mode

}