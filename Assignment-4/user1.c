// end will be indicated by closing the connection

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct s {
    int x;
    char c;
};

void func(char *buf) {
    printf("%s\n", buf);
}

int main() {
    // struct s *s1;
    // s1->x = 1;
    // s1->c = 'a';
    // printf("%d\n", s1->x);
    // printf("%c\n", s1->c);
    // struct s *s2;
    // s2 = s1;
    // s2->x = 2;
    // printf("%d\n", s1->x);
    // printf("%c\n", s1->c);
    // // free(s2);
    // s2 = NULL;
    // printf("%p\n", s1);
    // printf("%p\n", s2);

    // // printf("%d\n", s1->x);
    // // printf("%c\n", s1->c);

    char buf[100];
    memset(buf, 0, 100);
    strcpy(buf, "hello");
    func(buf);
    return 0;
}