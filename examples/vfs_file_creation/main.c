#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

void list_files(void) {
    DIR *dir = opendir(".");
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s ", entry->d_name);
    }
    printf("\n");

    closedir(dir);
}

void new_file(const char *name) {
    FILE *f = fopen(name, "w");
    if (!f) {
        perror("fopen");
        return;
    }
    fclose(f);
}

int main(void) {
    printf("Listing before file creation:\n");
    list_files();

    new_file("new_file.txt");

    printf("Listing after file creation:\n");
    list_files();

    return 0;
}

