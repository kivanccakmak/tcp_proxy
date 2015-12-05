#include "file_read.h"

/**
 * @brief returns file size in 
 * terms of Bytes
 *
 * @param fp
 *
 * @return 
 */
int get_file_size(FILE *fp){
    int file_size;
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

/**
 * @brief 
 *
 * Fills pointer input with file descriptor.
 *
 * @param[in] fp
 * @param[in] start_byte
 * @param[in] end_byte
 * @param[out] buffer
 */
void read_file(FILE *fp, int start_byte, int end_byte, char *buffer){
    size_t flen;
    size_t r_size = 1;
    size_t nmemb = end_byte - start_byte;
    int fseek_result;

    fseek(fp, 0L, SEEK_SET);
    fseek_result = fseek(fp, start_byte, SEEK_CUR);
    flen = fread(buffer, r_size, nmemb, fp);  
}
