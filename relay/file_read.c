#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKSIZE 2 

int main(int argc, char *argv[]){
    if(argc != 4){
        printf("Wrong Usage\n");
        printf("Usage: %s filename start_byte block_number\n", argv[0]);
        printf("ex: %s kivanc.txt 10 100\n", argv[0]); 
        return 2;
    }
    char *fname;
    fname = argv[1];
    int start_byte = atoi(argv[2]);
    int block_number = atoi(argv[3]);
    FILE *fp;
    char *buffer;
    buffer = (char *) malloc(block_number * BLOCKSIZE);
    fp = fopen(fname, "a+");
    size_t flen;
    size_t r_size = BLOCKSIZE;
    size_t nmemb = block_number;
    int fseek_result;
    fseek_result = fseek(fp, start_byte, SEEK_CUR);
    flen = fread(buffer, r_size, nmemb, fp);  
    printf("%s\n", buffer);
    fclose(fp);
}
