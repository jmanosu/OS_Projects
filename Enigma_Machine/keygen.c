#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
  if(argc < 2){
    printf("ERROR need key size\n");
    exit(1);
  }
  srand(time(NULL));
  int i;
  int keySize = atoi(argv[1]);;
  char string[keySize + 2];
  string[keySize + 1] = '\0';
  for(i = 0; i < keySize; i++){
    int randNum = rand() % 27 + 65;
    if(randNum == 91){
      string[i] = ' ';
    }else{
      string[i] = randNum;
    }
  }
  string[keySize] = '\n';
  printf("%s", string);
}
