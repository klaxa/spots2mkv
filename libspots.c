#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "libspots.h"


void spots_dump_header(uint8_t *buf)
{
    printf("Header\n"
        "Image-format: %s\n",
        buf + 32);
}

uint8_t *spots_get_image(uint8_t *buf, long *seek, long *size, long max_seek)
{
    uint32_t offset;
    memcpy(&offset, buf + *seek, 4);

    *seek += offset;
    int c = 0;
    uint8_t temp = 0;
    uint16_t match = 0xffd9;
    uint16_t candidate = 0;
    long image_start = *seek;
    long image_end = 0;
    for(; *seek < max_seek; (*seek)++) {
        c = buf[*seek];
        temp = (uint8_t) c;
        candidate = ((candidate << 8) & 0xFF00) + temp;
        if (candidate == match) {
            (*seek)++;
            image_end = *seek;
            break;
        }
    }
    if (!(candidate == match))
        image_end = *seek;

    *size = image_end - image_start;
    printf("Packet\n"
        "Offset:       %d\n"
        "Image-format: %s\n"
        "Seek:         %li\n"
        "Size:         %li\n----\n",
        offset,
        buf + *seek + 4,
        *seek,
        *size);

    return buf + image_start;

}


void ogg_parse_page(uint8_t *h, struct OggStream *os, FILE *file)
{
    int i, size = 0;
    uint8_t c;
    memcpy(os->magic, h, 4);
    os->fresh = h[5] & 1;
    os->bos   = ((h[5] & 2) >> 1);
    os->eos   = ((h[5] & 4) >> 2);
    memcpy(&os->serialno, (h + 14), 4);
    memcpy(&os->page_counter, (h + 18), 4);
    memcpy(&os->ts, (h + 6), 8);
    os->num_segments = h[26];
    for (i = 0; i < os->num_segments; i++) {
        c = getc(file);
        //printf("%d\n", c);
        size += c;
    }
    os->size = size;
    os->data = (uint8_t*) malloc(size);
    fread(os->data, sizeof(uint8_t), size, file);

    if (os->serialno == 1) { // Format hopefully oggspots... uhh...
        if (!strncmp((char *) os->data, "SPOTS", 5)) {
            os->ts = -1;
            memcpy(&os->num, (os->data + 12), 8);
            memcpy(&os->denom, (os->data + 20), 8);
            os->shift = h[28];
        }
        if (os->ts != -1) { // hopefully vaild timestamp...
        //    printf("JPEG Timestamp?: %li\t\t", os->ts);
            // Calculate ts size here
        //    printf("(%li / %li)?\n", os->num, os->denom);
            os->ts = (1000 * os->ts * os->num) / os->denom; // calculate to ms
        }
    }

}

void ogg_dump_header(struct OggStream *os)
{
    printf(
        "magic:   \t%s\n"
        "serialno:\t%d\n"
        "is fresh:\t%d\n"
        "is start:\t%d\n"
        "is end:  \t%d\n"
        "count:   \t%d\n"
        "num_segs:\t%d\n"
        "size:    \t%d\n",
        os->magic,
        os->serialno,
        os->fresh,
        os->bos,
        os->eos,
        os->page_counter,
        os->num_segments,
        os->size);
}
