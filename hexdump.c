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
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define LINE_LENGTH 16

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
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        else
            break;
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

static int hextobin(unsigned char* bin, char* hex, int maxLen)
{
    int k = 0;

    for (int i=0; *hex; i++) {
        // get current character then increment
        char byte = *hex++;
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        else
            break;
        bin[k] = (bin[k]  << 4) | (byte & 0xF);
        if (i%2 ==1) {
            k++;
            if(k>= maxLen)
                break;
            bin[k] = 0;
     
        }
    }    
    return k;
}

static void usage(char* name)
{
    printf("\nNAME\n");
    printf("\tDisplay file in heximal format\n\n");
    printf("SYNOPSIS\n");
    printf("\t%s [-s START] [-t END] [-K KEYWORD] FILE\n\n", name);
    printf("DESCRIPTION\n");
    printf("\tDisplay the binary FILE in heximal code and visible characters.\n\n");
    printf("\t-s START\n");
    printf("\t\t Display the content from START bytes, in heximal fomate.\n\n");
    printf("\t-t END\n");
    printf("\t\t Display the content before END bytes, in heximal fomate.\n\n");
    printf("\t-K KEYWORD\n");
    printf("\t\t Show the line containing KEYWORD in visible string.\n\n");
    printf("\t-k HEX_KEYWORD\n");
    printf("\t\t Show the line containing HEX_KEYWORD in octval (MSB first).\n\n");
}

static void ShowLine(size_t offset, unsigned char* line, int length) 
{
    int i;
    printf("0x%08lX: ", offset);
    for (i=0; i< length; ++i) {
        printf("%02x ", line[i]);
        if ( i == (LINE_LENGTH/2-1))
            printf("- ");
    }
    for (; i< LINE_LENGTH; ++i ){
        printf (".. ");
        if ( i == (LINE_LENGTH/2-1))
            printf("- ");
    }
    printf(" ");
    for (i=0; i< length; ++i) {
        if ( line[i]>= 0x20 && line[i] < 127)
            printf("%c", line[i]);
        else
            printf(" ");
    }
    printf("\n");
}

/* return:
    -1  no found
    0   found
    1   found, include next line
*/
static int FoundKeyword(unsigned char* b0, int l0, unsigned char* b1, int l1, unsigned char* key, int lk)
{
    unsigned char buffer[2*LINE_LENGTH+1];
    int len = l0+l1 - lk;
    if (lk <=0)
        return -1;
    memcpy(buffer, b0, l0);
    if (l1 > 0 && l1 <= LINE_LENGTH)
        memcpy(buffer + l0, b1, l1);

    unsigned char* pStart = buffer;
    if (len > l0) len = l0;
 
    int i;
    for (i=0; i<len; i++){
        if (memcmp(pStart, key, lk) == 0) {
            return (i+lk > l0)?1:0;
        }
        pStart++;
    }
    return -1;
}

int main( int argc, char* argv[])
{
    int ch;
	unsigned char buffer[2][LINE_LENGTH];
    unsigned char* line0, *line1;
    unsigned char keyword[LINE_LENGTH];
	size_t offset = 0;
	size_t nRead0;
	size_t nRead1;
    size_t nStart = 0;
    size_t nEnd = -1;
    int nKeyword = 0;

    char* fileName = NULL;
	FILE* fpIn = NULL;

    while ((ch = getopt(argc, argv, "k:K:s:t:?")) != -1)
    {
        switch (ch)
        {
        case 's':
            nStart = hex2int(optarg);
            break;
        case 't':
            nEnd = hex2int(optarg);
            break;
        case 'K':
            strncpy((char*)keyword, optarg, LINE_LENGTH-1);
            nKeyword = strlen((char*)keyword);
            break;
        case 'k':
            nKeyword = hextobin(keyword, optarg, LINE_LENGTH);
            break;
        default:
            usage(argv[0]);
            exit(-1);
        }            
    }
    fileName = argv[optind];

    if ( !fileName) {
        usage(argv[0]);
        exit(-1);
    }
    fpIn = fopen(fileName, "rb");
    if (!fpIn) {
        fprintf(stderr, "Failed to open file %s!\n", fileName);
        exit(-1);
    }
    
    line0 = buffer[0];
    line1 = buffer[1];
    nRead1 = fread(line1, 1, LINE_LENGTH, fpIn);
    int showNext = 0;
    int ret;
    do {
        unsigned char* temp = line0;
        line0 =  line1;
        line1 = temp;
        nRead0 = nRead1;

        if(nRead0 < LINE_LENGTH) {
            line1 = NULL;
            nRead1 = 0;
        }else {

            nRead1 = fread(line1, 1, LINE_LENGTH, fpIn);
        }

        if (offset+nRead0 > nStart && (nEnd == -1 || offset < nEnd)){
            ret = FoundKeyword(line0, nRead0, line1, nRead1, keyword,nKeyword);
            if (showNext == 1 || ret >= 0) {
                ShowLine(offset, line0, nRead0);
            }
            showNext =ret;
        }
        offset += nRead0;
    } while (nRead0 == LINE_LENGTH);

    fclose(fpIn);
	return 0;
}
