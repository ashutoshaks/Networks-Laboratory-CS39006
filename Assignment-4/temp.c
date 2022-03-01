#include <stdio.h>
#include <string.h>

const int PORT_1 = 50016;
const int PORT_2 = 50017;
const int MAX_MSG_LEN = 100;

int main() {
    char msg[MAX_MSG_LEN];
    memset(msg, 0, MAX_MSG_LEN);
    printf("Enter a message: ");
    // getline(&msg, MAX_MSG_LEN, stdin);
    scanf("%[^\n]s", msg);
    int msg_len = strlen(msg);
    printf("%d\n", msg_len);
    printf("%s\n", msg);
}