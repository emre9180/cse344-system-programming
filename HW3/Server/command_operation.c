#include "command_operation.h"
#include "server_util.h"


void handle_list_command(const char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo) {
    if (strcmp(command, "list") == 0)
    {
        

        printf("test");
        enterRegionReader(getSafeFile(dir_syncs, "05319346629"));
        printf("lst command\n");
        ftruncate(*client_res_fd, 0);
        DIR *dir;
        struct dirent *ent;

        if ((dir = opendir(dirname)) != NULL) 
        {
            int hasEntry = 0; // Flag to check if there is any file or folder
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type == DT_DIR || ent->d_type == DT_REG) {
                    char *name = ent->d_name;
                    // Exclude hidden files and directories
                    if (name[0] != '.') {
                        printf("Entry name: %s and fd %d\n", name, *client_res_fd);
                        int bytes_written = write(*client_res_fd, name, strlen(name));
                        if (bytes_written == -1) {
                            perror("Failed to write to client response FIFO");
                            close(server_fd);
                            close(*client_res_fd);
                            close(server_req_fd);
                            exitRegionReader(getSafeFile(dir_syncs, "05319346629"));
                            exit(EXIT_FAILURE);
                        }
                        printf("Bytes written: %d\n", bytes_written);
                        write(*client_res_fd, "\n", 1);
                        hasEntry = 1;
                    }
                }
            }

            closedir(dir);

            // If there is no file or folder, clear the FIFO
            if (!hasEntry) {
                const char* error_message = "Nothing has been found.";
                if (write(*client_res_fd, error_message, strlen(error_message)) == -1)
                {
                    perror("Failed to write error message to client response FIFO");
                    close(server_fd);
                    close(*client_res_fd);
                    close(server_req_fd);
                    exitRegionReader(getSafeFile(dir_syncs, "05319346629"));
                    exit(EXIT_FAILURE);
                }
            }
            close(*client_res_fd);
            *client_res_fd = -1;
        } 

        else
        {
            perror("Failed to open directory:");
            close(server_fd);
            close(*client_res_fd);
            close(server_req_fd);
            exitRegionReader(getSafeFile(dir_syncs, "05319346629"));
            exit(EXIT_FAILURE);
        }
        
        exitRegionReader(getSafeFile(dir_syncs, "05319346629"));

        // Open *client_res_fd again
        // if it is closed, open it: *client_res_fd
        // read from fifo
        int garbage;
        ssize_t num_read = read(server_req_fd, &garbage, sizeof(int));
        printf("num_read: %zd\n", num_read);

        if (*client_res_fd == -1)
        {
            printf("Client response FIFO is closed. Reopening...\n");
            *client_res_fd = open(client_res_fifo, O_WRONLY);
            printf("*client_res_fd: %d\n", *client_res_fd);
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
}

void handle_readF_command(char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo, int num_read) {
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
    strcpy(readF.file,words[1]);
    if(num_words>2) readF.line_number = atoi(words[2]);
    else readF.line_number = -1;
    printf("File1: %s\n", readF.file);

    if (num_read == -1)
    {
        perror("Failed to read from server request FIFO");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exit(EXIT_FAILURE);
    }

    printf("readf command\n");

    enterRegionReader(getSafeFile(dir_syncs, readF.file));

    // File name is the directory of server + file name
    char file_path[512];
    sprintf(file_path, "%s/%s", dirname, readF.file);

    FILE *file = fopen(file_path, "r");
    printf("File2: %s\n", file_path);
    if (file == NULL)
    {
        perror("Failed to open file");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exitRegionReader(getSafeFile(dir_syncs, readF.file));
        exit(EXIT_FAILURE);
    }

    // If the user wants a specific line
    if(readF.line_number!=-1)
    {
        handle_specific_line(file, readF, client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo);
    }
    
    // If the user wants the whole file
    else
    {
        handle_whole_file(file, readF, client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo);
    }

    printf("eof is reached\n");
}

void handle_specific_line(FILE *file, readF_command readF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo) {
    printf("Line number: %d\n", readF.line_number);
    char line[256];
    int line_number = 0;
    while (fgets(line, sizeof(line), file) != NULL)
    {
        line_number++;
        if (line_number == readF.line_number)
        {
            break;
        }
    }

    if (line_number != readF.line_number)
    {
        const char *error_message = "Line number is out of range";
        if (write(*client_res_fd, error_message, strlen(error_message)) == -1)
        {
            perror("Failed to write error message to client response FIFO");
            close(server_fd);
            close(*client_res_fd);
            close(server_req_fd);
            exitRegionReader(getSafeFile(dir_syncs, readF.file));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if (write(*client_res_fd, line, strlen(line)) == -1)
        {
            perror("Failed to write to client response FIFO");
            close(server_fd);
            close(*client_res_fd);
            close(server_req_fd);
            exitRegionReader(getSafeFile(dir_syncs, readF.file));
            exit(EXIT_FAILURE);
        }
        printf("Writing is ok:");
    }

    fclose(file);
    close(*client_res_fd);
    *client_res_fd = -1;
    exitRegionReader(getSafeFile(dir_syncs, readF.file));

    // read from fifo
    int garbage;
    ssize_t num_read = read(server_req_fd, &garbage, sizeof(int));
    printf("num_read: %zd\n", num_read);
    // if it is closed, open it: *client_res_fd
    if (*client_res_fd == -1)
    {
        printf("Client response FIFO is closed. Reopening...\n");
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

void handle_whole_file(FILE *file, readF_command readF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo) {
    printf("Whole file\n");
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (write(*client_res_fd, line, strlen(line)) == -1)
        {
            perror("Failed to write to client response FIFO");
            close(server_fd);
            close(*client_res_fd);
            close(server_req_fd);
            exitRegionReader(getSafeFile(dir_syncs, readF.file));
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    close(*client_res_fd);
    *client_res_fd = -1;
    exitRegionReader(getSafeFile(dir_syncs, readF.file));

    // read from fifo
    int garbage;
    ssize_t num_read = read(server_req_fd, &garbage, sizeof(int));
    printf("num_read: %zd\n", num_read);
    // if it is closed, open it: *client_res_fd
    if (*client_res_fd == -1)
    {
        printf("Client response FIFO is closed. Reopening...\n");
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

void handle_quit_command(int client_res_fd, int server_fd, int server_req_fd, struct client_info cli_info) {
    int response = 1;
    ssize_t num_written = write(client_res_fd, &response, sizeof(response));
    if (num_written == -1)
    {
        perror("Failed to write to client response FIFO");
        close(server_fd);
        close(client_res_fd);
        exit(EXIT_FAILURE);
    }
    close(server_fd);
    close(client_res_fd);
    close(server_req_fd);
    printf("Client %d is disconnected!\n", cli_info.pid);
    exit(EXIT_SUCCESS);
}

void handle_killServer_command(int client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, int parent_pid) {
    int response = 1;
    ssize_t num_written = write(client_res_fd, &response, sizeof(int));
    if (num_written == -1)
    {
        perror("Failed to write to client response FIFO");
        close(server_fd);
        close(client_res_fd);
        exit(EXIT_FAILURE);
    }
    close(server_fd);
    close(client_res_fd);
    close(server_req_fd);
    cleanup_server(server_fd, client_res_fd, server_req_fd, dir_syncs, parent_pid);
    printf("Server is killed!\n");
    // Send SIGTERM to the entire process group
    if (kill(-getpgid(0), SIGTERM) == -1) {
        perror("Failed to send SIGTERM to the process group");
        exit(EXIT_FAILURE);
    }
}

void handle_writeF_command(char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo, int num_read)
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

    writeF_command writeF;
    strcpy(writeF.file,words[1]);
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
    printf("File1: %s\n", writeF.file);

    if (num_read == -1)
    {
        perror("Failed to read from server request FIFO");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exit(EXIT_FAILURE);
    }

    printf("writef command\n");

    // // File name is the directory of server + file name
    // char file_path[512];
    // sprintf(file_path, "%s/%s", dirname, writeF.file);

    FILE *file;
    //  = fopen(file_path, "a+");
    // printf("File2: %s\n", file_path);
    // if (file == NULL)
    // {
    //     perror("Failed to open file");
    //     close(server_fd);
    //     close(*client_res_fd);
    //     close(server_req_fd);
    //     exitRegionWriter(getSafeFile(dir_syncs, writeF.file));
    //     exit(EXIT_FAILURE);
    // }

    // If the user wants a specific line
    if(writeF.line_number!=-1)
    {
        handle_specific_line_write(dirname, writeF, client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo);
    }
    
    // If the user wants the whole file
    else
    {
        handle_whole_file_write(dirname, writeF, client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo);
    }

    printf("eof is reached\n");
}

void handle_specific_line_write(char *dirname, writeF_command writeF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo) {
    printf("Line number: %d\n", writeF.line_number);
    char line[256];
    int line_number = 0;
    long int file_position = 0;
    int successful = 0;


    // File name is the directory of server + file name
    char file_path[512];
    sprintf(file_path, "%s/%s", dirname, writeF.file);
    printf("Line number: %d\n", writeF.line_number);

    enterRegionWriter(getSafeFile(dir_syncs, writeF.file));
    // Step 1: Open the Source File for Reading
    FILE *source_file = fopen(file_path, "r");
    if (source_file == NULL) {
        perror("Failed to open source file");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exitRegionWriter(getSafeFile(dir_syncs, file_path));
        exit(EXIT_FAILURE);
    }
    printf("Line number: %d\n", writeF.line_number);

    // Step 2: Create a Temporary File for Writing
    FILE *temp_file = fopen("temp.txt", "w");
    if (temp_file == NULL) {
        perror("Failed to create temporary file");
        fclose(source_file);
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exitRegionWriter(getSafeFile(dir_syncs, file_path));
        exit(EXIT_FAILURE);
    }
    printf("Line number: %d\n", writeF.line_number);

    // Step 3: Read from the Source File and Write to the Temporary File
    while (fgets(line, sizeof(line), source_file) != NULL) {
        if (ferror(source_file)) {
            perror("Failed to read from source file");
            fclose(source_file);
            fclose(temp_file);
            close(server_fd);
            close(*client_res_fd);
            close(server_req_fd);
            exitRegionWriter(getSafeFile(dir_syncs, file_path));
            exit(EXIT_FAILURE);
        }
        line_number++;
        if (line_number == writeF.line_number) {
            fputs(line, temp_file);
            fputs(writeF.string, temp_file);
            successful = 1;
        } else {
            fputs(line, temp_file);
        }
    }
    printf("Line number: %d\n", writeF.line_number);

    // Step 4: Replace the Original File
    fclose(source_file);
    fclose(temp_file);
    remove(file_path);
    rename("temp.txt", file_path);
    printf("Line number: %d\n", writeF.line_number);


    int response_ = 1;
    printf("client_res_fd: %d\n", *client_res_fd);
    int asdf = write(*client_res_fd, &response_, sizeof(int)) ;
    if (asdf == -1) {
        perror("Failed to write response to client");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exit(EXIT_FAILURE);
    }
    printf("asdf: %d\n", asdf);
    printf("Line number: %d\n", writeF.line_number);
    exitRegionWriter(getSafeFile(dir_syncs, writeF.file));

}


void handle_whole_file_write(char *dirname, writeF_command writeF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo) {
    printf("Whole file\n");
    char line[256];
    char file_path[512];
    int success = 0;
    sprintf(file_path, "%s/%s", dirname, writeF.file);
    if(getSafeFile(dir_syncs, writeF.file) == NULL) 
        addSafeFile(dir_syncs, writeF.file);
    enterRegionWriter(getSafeFile(dir_syncs, writeF.file));
    // Check if the file exists
    if (access(file_path, F_OK) == -1) {
        printf("File does not exist\n");
        // File does not exist, create it and write the string
        FILE *file = fopen(file_path, "w");
        if (file == NULL) {
            perror("Failed to create file");
            close(server_fd);
            close(*client_res_fd);
            close(server_req_fd);
            removeSafeFile(dir_syncs, writeF.file);
            exitRegionWriter(getSafeFile(dir_syncs, writeF.file));
            exit(EXIT_FAILURE);
        }
        fputs(writeF.string, file);
        fclose(file);
        success = 1;
    } else {
        printf("File exists\n");
    }
     if (write(*client_res_fd, &success, sizeof(int)) == -1) {
        perror("Failed to write to client response FIFO");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exitRegionWriter(getSafeFile(dir_syncs, writeF.file));
        exit(EXIT_FAILURE);
    }

    close(*client_res_fd);
    *client_res_fd = -1;
    exitRegionWriter(getSafeFile(dir_syncs, writeF.file));


}

void handle_download_command(const char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo, int num_read) {
   // Parse the command into separate words
    char *words[4];
    int num_words = 0;
    char *token = strtok(command, " ");
    while (token != NULL && num_words < 4) {
        words[num_words] = token;
        num_words++;
        token = strtok(NULL, " ");
    }

    download_command download;
    strcpy(download.file,words[1]);
    printf("File1: %s\n", download.file);

    if (num_read == -1)
    {
        perror("Failed to read from server request FIFO");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exit(EXIT_FAILURE);
    }

    printf("download command\n");

    if(getSafeFile(dir_syncs, download.file) == NULL) 
        addSafeFile(dir_syncs, download.file);
    enterRegionReader(getSafeFile(dir_syncs, download.file));

    // File name is the directory of server + file name
    char file_path[512];
    sprintf(file_path, "%s/%s", dirname, download.file);

    FILE *file = fopen(file_path, "r");
    printf("File2: %s\n", file_path);
    if (file == NULL)
    {
        perror("Failed to open file");
        const char *error_message = "NO_SUCH_FILE_05319346629";
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
        num_read = read(server_req_fd, &garbage, sizeof(int));
        printf("num_read: %zd\n", num_read);
        // if it is closed, open it: *client_res_fd
        if (*client_res_fd == -1)
        {
            printf("Client response FIFO is closed. Reopening...\n");
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
        return;
    }

    
printf("Whole file\n");
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (write(*client_res_fd, line, strlen(line)) == -1)
        {
            perror("Failed to write to client response FIFO");
            close(server_fd);
            close(*client_res_fd);
            close(server_req_fd);
            exitRegionReader(getSafeFile(dir_syncs, download.file));
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    close(*client_res_fd);
    *client_res_fd = -1;
    exitRegionReader(getSafeFile(dir_syncs, download.file));

    // read from fifo
    int garbage;
    num_read = read(server_req_fd, &garbage, sizeof(int));
    printf("num_read: %zd\n", num_read);
    // if it is closed, open it: *client_res_fd
    if (*client_res_fd == -1)
    {
        printf("Client response FIFO is closed. Reopening...\n");
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
    printf("eof is reached\n");
    
}

void handle_upload_command(const char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo, int num_read) {
    // Parse the command into separate words
    char *words[4];
    int num_words = 0;
    char *token = strtok(command, " ");
    while (token != NULL && num_words < 4) {
        words[num_words] = token;
        num_words++;
        token = strtok(NULL, " ");
    }

    upload_command upload;
    strcpy(upload.file,words[1]);
    printf("File1: %s\n", upload.file);

    if (num_read == -1)
    {
        perror("Failed to read from server request FIFO");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exit(EXIT_FAILURE);
    }

    printf("upload command\n");
    int success = 1;
    if (write(*client_res_fd, &success, sizeof(int)) == -1) {
        perror("Failed to write to client response FIFO");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exitRegionWriter(getSafeFile(dir_syncs, upload.file));
        exit(EXIT_FAILURE);
    }

    if(getSafeFile(dir_syncs, upload.file) == NULL) 
        addSafeFile(dir_syncs, upload.file);
    enterRegionWriter(getSafeFile(dir_syncs, upload.file));

    // File name is the directory of server + file name
    char file_path[512];
    sprintf(file_path, "%s/%s", dirname, upload.file);

    FILE *file = fopen(file_path, "w");
    printf("File2: %s\n", file_path);
    if (file == NULL)
    {
        perror("Failed to open file");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exitRegionWriter(getSafeFile(dir_syncs, upload.file));
        exit(EXIT_FAILURE);
    }

    printf("Whole file\n");
    char line[256];
    while (1)
    {
        ssize_t num_read = read(server_req_fd, line, sizeof(line));
        if (num_read == -1)
        {
            perror("Failed to read from server request FIFO");
            close(server_fd);
            close(*client_res_fd);
            close(server_req_fd);
            exitRegionWriter(getSafeFile(dir_syncs, upload.file));
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
    printf("Bytes written: %d\n", bytes_written);
    if (bytes_written == -1) {
        perror("Failed to write to client_res_fd");
        close(server_fd);
        close(*client_res_fd);
        close(server_req_fd);
        exitRegionWriter(getSafeFile(dir_syncs, upload.file));
        exit(EXIT_FAILURE);
    }
}