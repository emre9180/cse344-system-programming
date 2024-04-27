#include "client_command_operations.h"


void handle_list_command(int *fd_client_cmd, int *fd_client_res, char** words, void (*cleanup)()) {
    // Write the command to the server
    printf("len: %d\n", strlen(words[0]));
    if(!write_command_to_server_fifo(*fd_client_cmd, words[0]))
    {
        printf("You cannot list at the time being!\n");
        return;
    }
    // Read the response from the server
    char response[256];
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
        printf("%s", response); // Print the response
    }
    if (bytes_read_res == -1)
    {
        perror("Failed to read response from server");
        cleanup();
        exit(EXIT_FAILURE);
    }

    int garbage = 1;
    int bytes_written = write(*fd_client_cmd, &garbage, sizeof(int));
    if (bytes_written == -1) {
        perror("Failed to write to *fd_client_cmd");
        cleanup();
        exit(EXIT_FAILURE);
    }
    printf("Successfully written %d bytes\n", bytes_written);
}

void handle_quit_command(int *fd_client_cmd, int *fd_client_res, char* command, void (*cleanup)()) {
    // Write the command to the server
    if(!write_command_to_server_fifo(*fd_client_cmd, command))
    {
        printf("You cannot quit at the time being!\n");
        return;
    }

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
        cleanup();
        printf("You are disconnected!\n");
        exit(EXIT_SUCCESS);
    }

    else
    {
        printf("You cannot disconnect at the time being!\n");
    }
}

void handle_killServer_command(int *fd_client_cmd, int *fd_client_res, char* command, void (*cleanup)()) {
    printf("command: %s\n", command);
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

void handle_help_command() {
    printf("Commands:\n");
    printf("list: List the files in the server directory\n");
    printf("quit: Disconnect from the server\n");
    printf("killServer: Kill the server\n");
    printf("help: Display this help message\n");
}

void handle_readF_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)()) {
    if(num_words < 2 || num_words > 3)
    {
        printf("Invalid number of arguments!\n");
        return;
    }

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
    char fifo_message[256];
    strcpy(fifo_message, words[0]);
    for (int i = 1; i < num_words; i++) {
        strcat(fifo_message, " ");
        strcat(fifo_message, words[i]);
    }
    write(*fd_client_cmd, fifo_message, strlen(fifo_message));

    // Read the response from the server
    char response[256];
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
        printf("%s", response); // Print the response
    }
    
    int garbage = 1;
    int bytes_written = write(*fd_client_cmd, &garbage, sizeof(int));
    if (bytes_written == -1) {
        perror("Failed to write to *fd_client_cmd");
        cleanup();
        exit(EXIT_FAILURE);
    }
    printf("Successfully written %d bytes\n", bytes_written);
}

void handle_writeF_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)()) {
    writeF_command writeF;
    if(num_words < 3 || num_words > 4)
    {
        printf("Invalid number of arguments!\n");
        return;
    }

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
    char fifo_message[256];
    strcpy(fifo_message, words[0]);
    for (int i = 1; i < num_words; i++) {
        strcat(fifo_message, " ");
        strcat(fifo_message, words[i]);
    }
    write(*fd_client_cmd, fifo_message, strlen(fifo_message));

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
    
    printf("Successfully written %d bytes\n", bytes_read_res);
}

void handle_upload_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)(), struct dir_sync* dir_syncs, const char* server_req_fifo) {
    if(num_words!=2)
    {
        printf("Invalid number of arguments!\n");
        return;
    }

    upload_command upload;
    strcpy(upload.file, words[1]);

    // Check is there a file in the current dir without opening file
    if (access(words[1], F_OK)!=0) {
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
    if (bytes_read_res == -1)
    {
        perror("Failed to read response from server");
        cleanup();
        exit(EXIT_FAILURE);
    }

    if(getSafeFile(dir_syncs, upload.file) == NULL) 
        addSafeFile(dir_syncs, upload.file);
        
    enterRegionWriter(getSafeFile(dir_syncs, upload.file));
    // Open the file upload.file in read mode and send it to the server
    FILE* file = fopen(upload.file, "r");
    if (file == NULL) {
        perror("Failed to open file for reading");
        exitRegionWriter(getSafeFile(dir_syncs, upload.file));
        cleanup();
        exit(EXIT_FAILURE);
    }
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Write the buffer to the server
        ssize_t bytes_written = write(*fd_client_cmd, buffer, strlen(buffer));
        if (bytes_written == -1) {
            perror("Failed to write to *fd_client_cmd");
            exitRegionWriter(getSafeFile(dir_syncs, upload.file));
            cleanup();
            exit(EXIT_FAILURE);
        }
    }
    exitRegionWriter(getSafeFile(dir_syncs, upload.file));
    close(*fd_client_cmd);
    *fd_client_cmd = -1;

    bytes_read_res = read(*fd_client_res, &response_, sizeof(int));
    if (bytes_read_res == -1)
    {
        perror("Failed to read response from server");
        cleanup();
        exit(EXIT_FAILURE);
    }

    if (*fd_client_cmd == -1)
    {
        printf("Client response FIFO is closed. Reopening...\n");
        *fd_client_cmd = open(server_req_fifo, O_WRONLY);
        if (*fd_client_cmd == -1)
        {
            perror("Failed to open client response FIFO");
            cleanup();
            exit(EXIT_FAILURE);
        }
    }
}

void handle_download_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)()) {
    if(num_words!=2)
    {
        printf("Invalid number of arguments!\n");
        return;
    }

    download_command download;
    strcpy(download.file, words[1]);

    // Write words array to the FIFO with spaces separated form
    char fifo_message[256];
    strcpy(fifo_message, words[0]);
    for (int i = 1; i < num_words; i++) {
        strcat(fifo_message, " ");
        strcat(fifo_message, words[i]);
    }
    write(*fd_client_cmd, fifo_message, strlen(fifo_message));
                    
    // Open the file in write mode
    FILE* file = fopen(words[1], "w");
    if (file == NULL) {
        perror("Failed to open file for writing");
        cleanup();
        exit(EXIT_FAILURE);
    }

    char response[256];
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
        printf("%s", response); // Print the response
        
        // Append the read response to the file
        
        if(strcmp(response, "NO_SUCH_FILE_05319346629") == 0)
        {
            break;
        }
        fprintf(file, "%s", response);
    }

    // Close the file
    fclose(file);
                    
    int garbage = 1;
    int bytes_written = write(*fd_client_cmd, &garbage, sizeof(int));
    if (bytes_written == -1) {
        perror("Failed to write to *fd_client_cmd");
        cleanup();
        exit(EXIT_FAILURE);
    }
    printf("Successfully written %d bytes\n", bytes_written);
}

void handle_arch_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)(), int (*write_command_to_server_fifo)(int, const char*)) {
    if(num_words != 2)
    {
        printf("Invalid number of arguments!\n");
        return;
    }

    arch_command arch;
    strcpy(arch.file, words[1]);

    // Write the command to the server
    if(!write_command_to_server_fifo(*fd_client_cmd, words[0]))
    {
        printf("You cannot archive the file at the time being!\n");
        return;
    }

    // Write the arch parameters to the server
    ssize_t bytes_written = write(*fd_client_cmd, &arch, sizeof(arch));
    if (bytes_written == -1)
    {
        perror("Failed to write command to server");
        cleanup();
        exit(EXIT_FAILURE);
    }

    char response[256];
    // Read the response from the server
    ssize_t bytes_read_res = read(*fd_client_res, response, sizeof(response));
    if (bytes_read_res == -1)
    {
        perror("Failed to read response from server");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Print the response
    printf("%s\n", response);
}