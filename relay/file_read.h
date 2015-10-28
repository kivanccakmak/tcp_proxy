#ifndef FILEREAD_H
#define FILEREAD_H

#define BLOCKSIZE 2 

void read_file(FILE *fp, int start_btye, int block_number, char * buffer);

int get_file_size(FILE *fp);

#endif 
