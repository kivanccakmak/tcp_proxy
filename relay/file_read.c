#include "file_read_headers.h"
#include "file_read.h"

/*int main(int argc, char *argv[]){*/
    /**/
     /**this function reads a local file and prints it */
     /*file path, start_byte and block amount are assumed to*/
     /*be argument variables*/
     /**/ 
    /*if(argc != 4){*/
        /*printf("Wrong Usage\n");*/
        /*printf("Usage: %s filename start_byte block_number\n", argv[0]);*/
        /*printf("ex: %s kivanc.txt 10 100\n", argv[0]); */
        /*return 2;*/
    /*}*/
    /*int file_size;*/
    /*char *fname = argv[1];*/
    /*int start_byte = atoi(argv[2]);*/
    /*int block_number = atoi(argv[3]);*/
    /*char *buffer = (char *) malloc(block_number * BLOCKSIZE);*/
    /*memset(buffer, '\0', block_number * BLOCKSIZE);*/
    /*FILE *fp;*/
    /*fp = fopen(fname, "a+");*/
    /*fseek(fp, 0L, SEEK_END); */
    /*file_size = ftell(fp);*/
    /*printf("file size: %d\n", file_size);*/
    /*fseek(fp, 0L, SEEK_SET);*/
    /*read_file(fp, start_byte, block_number, buffer);*/
    /*printf("%s\n", buffer);*/
    /*free(buffer);*/
    /*fclose(fp);*/
/*}*/

int get_file_size(FILE *fp){
    int file_size;
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

void read_file(FILE *fp, int start_byte, int block_number, char *buffer){
    /* 
     * this function fills buffer with file pointer
     * */
    size_t flen;
    size_t r_size = BLOCKSIZE;
    size_t nmemb = block_number;
    int fseek_result;
    fseek(fp, 0L, SEEK_SET);
    fseek_result = fseek(fp, start_byte, SEEK_CUR);
    flen = fread(buffer, r_size, nmemb, fp);  
}
