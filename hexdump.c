/**********************************************************************************
    hexdump - Show a binary file in Heximal formal
    Copyright (C) 2020  free6d1823@gmail.com

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static off_t hex2int(char *hex)
{
    off_t val = 0;
    while (*hex&& *hex != ' ') {
        // get current character then increment
        unsigned char byte = *hex++;
        if(byte == 'x'){
            val = 0;
            continue;
        }
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        else
            break;
        // shift 4 to make space for new digit, and add the 4 bits of the new digit
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

void usage(char* name)
{
    printf("NAME\n");
    printf("\tDisplay file in heximal format\n\n");
    printf("SYNOPSIS\n");
    printf("\t%s [-s START] [-t END] [-K KEYWORD] FILE\n\n", name);
    printf("DESCRIPTION\n");
    printf("\tDisplay the binary FILE in heximal code and visible characters.\n\n");
    printf("\t-s START\n");
    printf("\t\t Display the content from START bytes, in heximal fomate.\n\n");
    printf("\t-t END\n");
    printf("\t\t Display the content before END bytes, in heximal fomate.\n\n");

}

int main( int argc, char* argv[])
{
    int ch;
	unsigned char line[16];
	size_t offset = 0;
	size_t nRead;
    size_t nStart = 0;
    size_t nEnd = -1;
	int i;
    char* fileName = NULL;
	FILE* fpIn = NULL;

    while ((ch = getopt(argc, argv, "s:t:?")) != -1)
    {
        switch (ch)
        {
        case 's':
            nStart = hex2int(optarg);
            break;
        case 't':
            nEnd = hex2int(optarg);
            break;
        default:
            usage(argv[0]);
            exit(-1);
        }            
    }
    fileName = argv[optind];


    fpIn = fopen(fileName, "rb");
    if (!fpIn) {
        fprintf(stderr, "Failed to open file %s!\n", fileName);
        exit(-1);
    }

    do {
        nRead = fread(line, 1, sizeof(line), fpIn);
        if (offset+nRead > nStart && (nEnd == -1 || offset < nEnd)){
            printf("0x%08lX: ", offset);
            for (i=0; i< nRead; ++i) {
                printf("%02x ", line[i]);
                if ( i == 7)
                    printf("- ");
            }
            for (; i<sizeof(line); ++i ){
                printf (".. ");
                if ( i == 7)
                    printf("- ");
            }
            printf(" ");
            for (i=0; i< nRead; ++i) {
                if ( line[i]>= 0x20 && line[i] < 127)
                    printf("%c", line[i]);
                else
                    printf(" ");
            }
            printf("\n");
        }
        offset += nRead;
    } while (nRead == sizeof(line));

    fclose(fpIn);
	return 0;
}
