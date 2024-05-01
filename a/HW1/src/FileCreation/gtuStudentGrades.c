#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "gtuStudentGrades.h"
#include <sys/wait.h>
#include "../Log/log.h"

void gtuStudentGrades(const char *file)
{
    int fd = open(file, O_WRONLY | O_CREAT | O_EXCL, 0644); // Open the file in write-only mode, create if not exists, and fail if already exists
    if (fd == -1)
    {
        perror("open");
        char logMessage[100];
        sprintf(logMessage, "Error opening file: %s", file);
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }

    // strcpy(filename, file);
    printf("File has been created successfuly %s: \n", file);

    close(fd); // Close the file
    char logMessage[100];
    sprintf(logMessage, "File created: %s", file);
    logToFile(logMessage);
    exit(EXIT_SUCCESS);
}

// Function to display usage instructions
void usage()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Error: Fork failed.\n");
    }
    else if (pid == 0)
    {
        printf("Usage: [command] [arguments]\n");
        printf("Commands:\n");

        printf("  \"gtuStudentGrades\": Print usage of the system\n");
        printf("  \t\tExample using: gtuStudentGrades\n\n");

        printf("  gtuStudentGrades \"student.txt\": Create student.txt file.\n");
        printf("  \t\tExample using: gtuStudentGrades \"test.txt\"\n\n");

        printf("  addStudentGrade \"Name Surname\" \"Grade\" \"grades.txt\": Add a new student's grade.\n");
        printf("  \t\tFor example, addStudent Emre Yilmaz FF text.txt\"\n\n");
        printf("  \t\tIf a user has more than one name, his/her name and surname must be inside quote signs\n\t\tFor example, addStudentGrade \"Mehmet Ali Birand\" \"FF\" \"text.txt\"\n\n");

        printf("  searchStudent \"Name Surname\" \"grades.txt\": Search for a student's grade.\n");
        printf("  \t\tFor example, searchStudent Emre Yilmaz text.txt\"\n\n");
        printf("  \t\tIf a user has more than one name, his/her name and surname must be inside quote signs\n\t\tFor example, addStudentGrade \"Mehmet Ali Birand\" \"FF\" \"text.txt\"\n\n");

        printf("  sortAll \"grades.txt\": Sort students according to grade or name\n");
        printf("  \t\tExample using: sortAll \"grades.txt\"\n\n");

        printf("  showAll \"grades.txt\": Show all students with grades\n");
        printf("  \t\tExample using: showAll \"grades.txt\"\n\n");

        printf("  listGrades \"grades.txt\": Display the first five student grades.\n");
        printf("  \t\tExample using: listGrades \"grades.txt\"\n\n");

        printf("  listSome \"numEntries\" \"pageNumber\" \"grades.txt\": Display student grades based on page number and number of entries per page.\n");
        printf("  \t\tExample using: listSome 5 2 \"grades.txt\"\n\n");

        printf("  NOTE: Your inputs can be taken either within quotation marks or without quotation marks. It's up to you. However, if a student's name consists of more than one word, both their first name and last name should be within quotation marks.\n\n");

        exit(EXIT_SUCCESS);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status))
        {
            printf("Error: Child process terminated abnormally.\n");
        }
    }
}