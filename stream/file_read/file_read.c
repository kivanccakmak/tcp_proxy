/*
 * Copyright (C) 2016  Kıvanç Çakmak <kivanccakmak@gmail.com>
 * Author: Kıvanç Çakmak <kivanccakmak@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "file_read.h"

/**
 * @brief returns file size in 
 * terms of Bytes
 *
 * @param[in] fp
 *
 * @return num bytes in file
 */
int get_file_size(FILE *fp)
{
    int file_size;
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

/**
 * @brief Fill file content to buffer.
 *
 * @param[in] fp
 * @param[in] start_byte
 * @param[in] end_byte
 * @param[out] buffer
 */
void read_file(FILE *fp, int start_byte, int end_byte, char *buffer)
{
    size_t flen;
    size_t r_size = 1;
    size_t nmemb = end_byte - start_byte;
    int fseek_result;

    fseek(fp, 0L, SEEK_SET);
    fseek_result = fseek(fp, start_byte, SEEK_CUR);
    flen = fread(buffer, r_size, nmemb, fp);  
}
