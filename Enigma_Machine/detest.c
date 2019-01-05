/* Testing C file */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include "otp_tools.h"

void encrypt(char *, char *);
void decrypt(char *, char *);
void formatSend(char *, char *, char *);
int main(int argc, char *argv[]) {
    char string[140000] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    char * string2 = & string[2];
    string[strlen(string)] = '\0';
    char key[140000] = "                           ";
    key[strlen(key)] = '\0';
    printf("      key: -%s- \n", key);
    printf("   string: -%s- \n", string);
    encrypt(string, key);
    printf("encrypted: -%s- \n", string);
    decrypt(string, key);
    printf("decrypted: -%s- \n", string);
    return 0;
}
