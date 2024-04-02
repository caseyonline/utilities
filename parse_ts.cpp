// g++ -o build/parse_ts parse_ts.cpp

#include <iostream>
#include <stdio.h>
#include <stdint.h>

using namespace std;

bool pes_packet_details(int pid, unsigned char *buf, int idx) {

    if (idx < 0)
        return false;

    int streamId, len, indicator, pesLen;
    uint64_t pts=0, dts=0;

    streamId = buf[idx];
    len = (buf[idx+1] << 8) + buf[idx+2];
    indicator = (buf[idx+4] >> 6);

    idx += 5;
    pesLen = buf[idx];

    switch(indicator) {
        case 2: // PTS only
        case 3: // PTS and DTS
            pts += (uint64_t)(buf[idx+1] & 0x0E) << 29;
            pts += (uint64_t)(buf[idx+2] & 0xFF) << 22;
            pts += (uint64_t)(buf[idx+3] & 0xFE) << 14;
            pts += (uint64_t)(buf[idx+4] & 0xFF) <<  7;
            pts += (uint64_t)(buf[idx+5] & 0xFE) >>  1;

            if (indicator == 2) break;
            
            idx += 5;
            dts += (uint64_t)(buf[idx+1] & 0x0E) << 29;
            dts += (uint64_t)(buf[idx+2] & 0xFF) << 22;
            dts += (uint64_t)(buf[idx+3] & 0xFE) << 14;
            dts += (uint64_t)(buf[idx+4] & 0xFF) <<  7;
            dts += (uint64_t)(buf[idx+5] & 0xFE) >>  1;

            break;

    }

    printf("pid=%i streamId=%i Len=%i pesLen=%i PTS=%li DTS=%li\n", pid, streamId, len, pesLen, pts, dts);

    return true;
}

int search_start_code(unsigned char *buf, int len) {

    for(int i = 0; i < len - 3; i++) 
        if ((buf[i] == 0x00) && (buf[i+1] == 0x00) && (buf[i+2] == 0x01) )
            return i + 3;

    return -1;
}

bool parse_packet(FILE *fp) {

    unsigned char buf[2],buffer[185];
    int result, idx;

    if ((result = fread(buf, 1, 2, fp)) != 2)
        return false;

    int pusi = (buf[0] & 0x40) >> 6;
    int pid  = ((buf[0] & 0x1F) << 8) + buf[1];

    if ((result = fread(buffer, 1, 185, fp)) != 185)
        return false;

    if (pusi) {
        //printf("%0X-%0X\n",buf[0],buf[1]);
        idx = search_start_code(buffer, 185);
        pes_packet_details(pid, buffer, idx);
    }

    //printf("pusi=%i, pid=%i, idx=%i\n",pusi,pid,idx);

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

    if (sync == 0x47) {
        if (!parse_packet(fp))
            break;
    } else
        printf("expecting sync byte by received %u\n",sync);

} while (true);

}