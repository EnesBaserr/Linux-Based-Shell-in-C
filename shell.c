#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <time.h>
#include <sys/sysinfo.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ALIAS_SIZE 256
int reversed_fd;
int shouldReversed = 0;
char last_command[256];
char second_last_command[256];
int p_count = 2;
void trimWhitespace(char *str)
{
    // Find the first non-whitespace character
    char *start = str;
    while (isspace((unsigned char)*start))
    {
        start++;
    }

    // Move non-whitespace characters to the beginning of the string
    if (start != str)
    {
        size_t i = 0;
        while (*start)
        {
            str[i++] = *start++;
        }
        str[i] = '\0';
    }

    // Remove trailing whitespace
    size_t length = strlen(str);
    while (length > 0 && isspace((unsigned char)str[length - 1]))
    {
        length--;
    }

    // Null-terminate the trimmed string
    str[length] = '\0';
}
void trimWhitespace_alias(char *str)
{
    // Find the first non-whitespace character
    char *start = str;
    while (isspace((unsigned char)*start) || (*start == '"'))
    {
        printf("Current char is :%c", *start);
        start++;
    }

    // Move non-whitespace characters to the beginning of the string
    if (start != str)
    {
        size_t i = 0;
        while (*start)
        {
            str[i++] = *start++;
        }
        str[i] = '\0';
    }

    // Remove trailing whitespace
    size_t length = strlen(str);
    int count = 1;

    while (length > 0 && count != 0 && (isspace((unsigned char)str[length - 1]) || (str[length - 1] == '"')))
    {
        length--;
        count--;
    }

    // Null-terminate the trimmed string
    str[length] = '\0';
}
void print_prompt(const char *username, const char *hostname, const char *current_directory)
{
    printf("%s@%s %s --- ", username, hostname, current_directory);
}
void read_aliases(char *aliases[MAX_ALIAS_SIZE][2], int *alias_count)
{
    FILE *alias_file = fopen("aliases.txt", "r");
    if (alias_file == NULL)
    {
       // perror("Error opening alias file");
        return;
    }

    char line[MAX_INPUT_SIZE];
    *alias_count = 0;

    while (fgets(line, sizeof(line), alias_file) != NULL && *alias_count < MAX_ALIAS_SIZE)
    {
        line[strcspn(line, "\n")] = '\0'; // Remove newline character

        char *alias_name = strtok(line, "=");
        char *command = strtok(NULL, "=");

        if (alias_name != NULL && command != NULL)
        {
            aliases[*alias_count][0] = strdup(alias_name);
            aliases[*alias_count][1] = strdup(command);
            (*alias_count)++;
        }
    }

    fclose(alias_file);
}

// Function to write aliases to a file
void write_aliases(char *aliases[MAX_ALIAS_SIZE][2], int count)
{
    FILE *alias_file = fopen("aliases.txt", "w");
    if (alias_file == NULL)
    {
        perror("Error opening alias file");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        char *alias_name = aliases[i][0];
        char *alias_command = aliases[i][1];
        printf("Alias name is %s\n", alias_name);
        printf("Alias command is %s\n", alias_command);
        trimWhitespace_alias(alias_name);
        trimWhitespace_alias(alias_command);
        printf("Alias command after trim is %s\n", alias_command);
        printf("Alias name after trim is %s\n", alias_name);

        fprintf(alias_file, "%s=%s\n", alias_name, alias_command);
    }

    fclose(alias_file);
}

// Function to reverse a string
void reverse_string(char *str)
{

    int length = strlen(str);
    for (int i = 0, j = length - 1; i < j; ++i, --j)
    {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

int main()
{
    // printf("31\n");
    //  Get the current username
    struct passwd *pw = getpwuid(getuid());
    const char *username = pw->pw_name;
    const char *home_location = pw->pw_dir;

    // Get the machine name (hostname)
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    char *aliases[MAX_ALIAS_SIZE][2];
    int alias_count = 0;
    read_aliases(aliases, &alias_count);

    // Main shell loop
    while (1)
    {
        int background_process = 0;
        // Get the current directory

        char current_directory[256];
        getcwd(current_directory, sizeof(current_directory));
        // Print the custom shell prompt
        
        //print_prompt(username, hostname, current_directory);
         if (strncmp(current_directory, home_location, strlen(home_location)) == 0)
        {
        // Replace the home directory portion with ~
        printf("%s@%s ~%s --- ", username, hostname, current_directory + strlen(home_location));
        }
        else
        {
        // If not, print the full current directory
        printf("%s@%s %s --- ", username, hostname, current_directory);
         }

        // Read user input
        char input[MAX_INPUT_SIZE];
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            // Handle EOF or error
            perror("Error reading input");
            break;
        }
       
       // printf("input is : %s", input);

        // printf("String is : %s\n",input);
        //  Remove newline character from input
       
        if(strcmp(input,"\n")==0){
            continue;
        }
        input[strcspn(input, "\n")] = '\0';
        for (int i = 0; i < alias_count; i++)
        {
            // printf("searching allies!!!\n");
            char *alias_name = aliases[i][0];
            char *alias_command = aliases[i][1];
            // printf("Alias name is : %s\n",aliases[i][0]);
            // printf("length input %ld\n",strlen(input));

            // printf("length alias_name %ld\n",strlen(alias_name));
            // trimWhitespace_alias(alias_name);
            // trimWhitespace_alias(alias_command);
            trimWhitespace(input);
            char temp_inp[MAX_INPUT_SIZE];
            strcpy(temp_inp, input);
            strtok(temp_inp, " ");

            // printf("length alias_name after trimWhiteSpace %ld\n",strlen(alias_name));
            if (strncmp(temp_inp, alias_name, strlen(temp_inp)) == 0 && strlen(input)>0)
            {
                // printf("input name is : %s\n",input);
                // printf("alias name is %s\n",alias_name);
                // printf("alias command is %s\n",alias_command);
                // Replace the alias with its command
                char *rest_of_input = input + strlen(alias_name); // Get the portion of input after the alias
                char temp[MAX_INPUT_SIZE];
                sprintf(temp, "%s%s", alias_command, rest_of_input);
                strcpy(input, temp);

                break; // Once replaced, exit the loop
            }
        }
         if (strcmp(input, "bello") != 0)
        {

            strcpy(last_command, "");
        }
        
        if (strcmp(input, "bello") != 0)
            strcat(last_command, input);
         
        //printf("input is : %s", input);
        if (input[strlen(input) - 1] == '&')
        {
           // printf("41 kcoaeli\n");
            p_count++;
            background_process = 1;
            // Remove the '&' symbol from the input
            input[strlen(input) - 1] = '\0';
           //  printf("& input is %s\n",input);
        }
        else
        {
            background_process = 0;
        }
       

        // Check for exit command
        if (strcmp(input, "exit") == 0)
        {
            break; // Exit the shell
        }
        if (strncmp(input, "alias ", 6) == 0)
        {
            // Extract alias name and command
            char *alias_name = strtok(input + 6, "=");
            char *command = strtok(NULL, "=");

            if (alias_name != NULL && command != NULL)
            {
                // Add alias to the list
                aliases[alias_count][0] = strdup(alias_name);
                aliases[alias_count][1] = strdup(command);
                alias_count++;

                // Write aliases to file
                write_aliases(aliases, alias_count);

                printf("Alias created: %s=%s\n", alias_name, command);
                continue;
            }
        }

        // Check for re-redirection operator ">>>"
        int reverse_output = 0;
        int extraction_op_2_output = 0;
        int extraction_op_1_output = 0;
        char *reverse_operator = ">>>";
        char *extraction_op_2 = ">>";
        char *extraction_op_1 = ">";
        char *reverse_position = strstr(input, reverse_operator);
        char *extraction_op_2_position = strstr(input, extraction_op_2);
        char *extraction_op_1_position = strstr(input, extraction_op_1);
        

        if (reverse_position != NULL)
        {
            reverse_output = 1;
            *reverse_position = '\0';                     // Remove the ">>>" part from the command
            reverse_position += strlen(reverse_operator);
            trimWhitespace(reverse_position); // Move past ">>>"
            // Remove any extra spaces after ">>>"
            while (*reverse_position == ' ')
            {
                reverse_position++; // Skip spaces
            }
        }
        
        
        

        else if (extraction_op_2_position != NULL)
        {
            extraction_op_2_output = 1;
            *extraction_op_2_position = '\0';                    // Remove the ">>" part from the command
            extraction_op_2_position += strlen(extraction_op_2);
            trimWhitespace(extraction_op_2_position); // Move past ">>"
            // Remove any extra spaces after ">>"
            while (*extraction_op_2_position == ' ')
            {
                extraction_op_2_position++; // Skip spaces
            }
        }
        else if (extraction_op_1_position != NULL)
        {
            extraction_op_1_output = 1;
            *extraction_op_1_position = '\0';                    // Remove the ">" part from the command
            extraction_op_1_position += strlen(extraction_op_1); 
            trimWhitespace(extraction_op_1_position);// Move past ">"
            // Remove any extra spaces after ">>"
            while (*extraction_op_1_position == ' ')
            {
                extraction_op_1_position++; // Skip spaces
            }
        }
        // printf("inp is %s",input);
        //  Fork a new process
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1)
        {
            perror("Pipe error");
            exit(EXIT_FAILURE);
            break;

        }
        pid_t pid = fork();

        if (pid == -1)
        {
            // Fork error
            perror("Fork error");
        }
        else if (pid == 0)
        {
            // Child process (execute the command with arguments)
            char *args[MAX_INPUT_SIZE];
            int i = 0;
            int inQuotes = 0; // Flag to indicate if currently inside quotes

            for (char *token = strtok(input, " "); token != NULL; token = strtok(NULL, " "))
            {
                if (strcmp(token, "&") == 0)
                {

                    // args[i++]=token;
                }
                if (extraction_op_2_output)
                {
                    if (strcmp(token, ">>") == 0)
                    {
                        args[i++] = token;
                        // Skip the ">>" token
                        continue;
                    }
                }
                if (extraction_op_1_output)
                {
                    if (strcmp(token, ">") == 0)
                    {
                        args[i++] = token;
                        // Skip the ">" token
                        continue;
                    }
                }
                // Handle quotes
                //  if (token[0] == '"'&&reverse_output) {
                //    inQuotes = !inQuotes;
                // Skip the quote character
                //  token++;
                //}
                int tokenLength = strlen(token);
                if (tokenLength > 0 && token[tokenLength - 1] == '"' && reverse_output)
                {
                    // token[tokenLength - 1] = '\0';
                }
                else if (tokenLength > 0 && token[tokenLength - 1] == '"' && extraction_op_2_output)
                {
                    // token[tokenLength - 1] = '\0';
                }
                else if (tokenLength > 0 && token[tokenLength - 1] == '"' && extraction_op_1_output)
                {
                    //  token[tokenLength - 1] = '\0';
                }

                // Concatenate tokens if inside quotes
                if (!inQuotes && reverse_output)
                {
                    args[i++] = token;
                }
                else
                {

                    // Reverse the token outside of quotes
                    //  if(reverse_output){
                    // reverse_string(token);
                    //  }
                    args[i++] = token;
                }
            }

            args[i] = NULL;
            int fd;

            // int fd_temp;
            char buffer[4096]; // File descriptor for redirection
                               // printf("32\n");
            if (reverse_output || extraction_op_2_output || extraction_op_1_output)
            {
                printf("reverse position is %s",reverse_position);
                // Open the file for writing with truncation, create if not exists
                // fd = open(reverse_position, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                if (reverse_position)
                {
                    
                    
                   
                        close(pipe_fd[0]); // Close the read end of the pipe
                        dup2(pipe_fd[1], STDOUT_FILENO);
                        close(pipe_fd[1]);
                        // Close the write end of the pipe
                    
                    
                }
                else if (extraction_op_2_position)
                {
                    fd = open(extraction_op_2_position, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
                    // fd_temp = open("temp.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    extraction_op_2_position = NULL;
                }
                else if (extraction_op_1_position)
                {
                    fd = open(extraction_op_1_position, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    // fd_temp = open("temp.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    extraction_op_1_position = NULL;
                }
                // dup2(fd, STDOUT_FILENO);
                // close(fd);
            }

            // printf("length of bello is %ld above if and input is : %s\n",strlen(input),input);
            if (strcmp(input, "bello") == 0)
            {

                char *shell_name = getenv("SHELL");
                char *tty_name;
                int fd;
                fd = 0;
                tty_name = ttyname(fd);

                time_t current_time;
                struct tm *time_info;
                time(&current_time);
                time_info = localtime(&current_time);
                char time_str[256];
                strftime(time_str, sizeof(time_str), "%c", time_info);
                struct utsname uname_data;

                struct sysinfo sys_info;
                long num_processes = sys_info.procs;
                char *termtype = getenv("TERM");

                printf("1. Username: %s\n", username);
                printf("2. Hostname: %s\n", hostname);
                printf("3. Last Executed Command: %s\n", last_command);
                printf("4. TTY Name: %s\n", tty_name);
                printf("5. Current Shell Name: %s\n", shell_name); // TODO : Show not env variable but current executing shell.
                printf("6. Home Location: %s\n", home_location);
                printf("7. Current Time and Date: %s\n", time_str);
                printf("8. Current number of processes: %d\n", p_count);
                exit(EXIT_SUCCESS);
            }

            execvp(args[0], args);

            // printf("bedo");

            // If execvp fails
            perror("Command not found");
            exit(EXIT_FAILURE);
        }
        else
        {
             pid_t zombie_child;
              while ((zombie_child = waitpid(-1, NULL, WNOHANG)) > 0) {
                  p_count--;
              }
              if(p_count<2){
                  p_count=2;
              }
              if (!background_process)
            {
                waitpid(pid, NULL, 0);
            }
            if(reverse_output){
                    pid_t grand_child = fork();
                    if(grand_child==-1){
                         perror("Grandchild fork error");
                        exit(EXIT_FAILURE);
                    }
                    else if(grand_child == 0 ){
                        close(pipe_fd[1]);

                         // Read from the read end of the pipe and reverse the content
                        char buffer[4096];
                        ssize_t bytesRead;

                        while ((bytesRead = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
                            // Reverse the content in the buffer
                            for (ssize_t i = 0, j = bytesRead - 1; i < j; ++i, --j) {
                                char temp = buffer[i];
                                buffer[i] = buffer[j];
                                buffer[j] = temp;
                            }

                            // Write the reversed content to the standard output

                           int new_fd;
                        // printf("reverse_position is %s",reverse_position);
                            new_fd=open(reverse_position, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
                            write(new_fd, buffer, bytesRead);
                            close(pipe_fd[0]);
                            close(new_fd);
                            exit(EXIT_SUCCESS);
                        }
                        


                    }
                
                }
            // sonradan eklendi
            
            
            
        }
    }

    return 0;
}