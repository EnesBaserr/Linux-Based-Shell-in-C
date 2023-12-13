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
int main() {
    FILE *f = fopen("out.txt", "w");
    if (f == NULL) {
        printf("Error opening file \n");
        }
    const char *outputText = "Print this out to the txt file";
    int index = 0;
    while (index < 10) {
        printf("ben deamon \n");
        fprintf(f, "%s", outputText);
        fprintf(f, "and the index is %d \n", index);
        sleep(2);
        index++;
        fclose(f);
        f = fopen("out.txt", "a");
    }
    fclose(f);
    return 0;
 
}