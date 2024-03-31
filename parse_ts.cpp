// g++ -o build/parse_ts parse_ts.cpp

#include <iostream>
#include <stdio.h>

using namespace std;

bool parse_packet(FILE *fp) {

    unsigned char buf[2];
    int result;

    if ((result = fread(buf, 1, 2, fp)) != 2)
        return false;

    int pusi = (buf[0] & 0x40) >> 6;
    int pid  = ((buf[0] & 0x1F) << 8) + buf[1];

    printf("pusi=%i, pid=%i\n",pusi,pid);

    return true;
}

int main(int ac, char** av) {
    
    if (ac != 2) {
        printf("parse_ts <filename>\n");
        exit(1);
    }

FILE *fp;
if ( (fp = fopen(av[1],"r")) == NULL) {
    printf("error open file %s\n",av[1]);
    exit(1);
}

do {

    int result;
    unsigned char sync;
    if ((result = fread(&sync, 1, 1, fp)) != 1)
        break;

    if (sync == 0x47)
        if (!parse_packet(fp))
            break;
    
    break;

} while (true);

}