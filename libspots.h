#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define SPOTS_HEADER_SIZE 52
#define OGG_HEADER_SIZE 26


struct OggStream {
    char magic[5];
    int fresh;
    int bos;
    int eos;
    int64_t ts;
    int32_t serialno;
    int32_t page_counter;
    int64_t num;
    int64_t denom;
    int8_t shift;
    int num_segments;
    int size;
    uint8_t *data;
} OggStream;


struct SpotsData {
    char magic[4];
    uint8_t res_x, res_y;
    char *image_format;
} SpotsData;

void spots_dump_header(uint8_t *buf);
uint8_t *spots_get_image(uint8_t *buf, long *seek, long *size, long max_seek);
void ogg_parse_page(uint8_t *h, struct OggStream *os, FILE *file);
void ogg_dump_header(struct OggStream *os);
