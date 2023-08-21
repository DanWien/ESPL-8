#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int digit_cnt (char* str) {
    int output = 0;
    while (*str) {
        if('0' <= *str && *str <= '9')
            output++;
        str++;
    }
    return output;
}

int main(int argc , char** argv) {
    printf("%d" ,digit_cnt("0123456789"));
}