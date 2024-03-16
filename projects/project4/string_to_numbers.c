#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){ 
    char char_string[] = "1 023 0345 121111111";

    char *token = strtok(char_string, " ");
    int values[4];
    int i = 0;

    while (token != NULL && i < 4) {
        values[i] = atoi(token);
        token = strtok(NULL, " ");
        i++;
    }

    int third_value = values[2];
    int minutes = third_value / 100;
    int seconds = third_value % 100;

    char forth_value[256];
    sprintf(forth_value, "%d", values[3]);

    printf("first value %d\n", values[0]);
    printf("second value %d\n", values[1]);
    printf("third value %d:%02d\n", minutes, seconds);
    printf("forth value %s\n", forth_value);


    return 0;

}
