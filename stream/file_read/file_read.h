#ifndef FILEREAD_H
#define FILEREAD_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void read_file(
               FILE *fp, 
               int start_btye, 
               int end_byte, 
               char *buffer
              );

int get_file_size(FILE *fp);

#endif 
