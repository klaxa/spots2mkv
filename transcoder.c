#include <stdio.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "libspots.h"

struct SpotsFrame {
    uint8_t* image;
    long size;
    long timestamp;
    struct SpotsFrame *next;
} SpotsFrame;


void dump_list(struct SpotsFrame *head)
{
    struct SpotsFrame* cur = head;
    while ((cur = cur->next))
        printf("Timestamp:\t%li\tSize:\t%li\n", cur->timestamp, cur->size);
}

void dump_images(struct SpotsFrame *head)
{
    struct SpotsFrame* cur = head;
    int num = 0;
    while ((cur = cur->next)) {
        char *image_filename = (char*) malloc(16);
        sprintf(image_filename, "image%04d.jpg", num);
        FILE *out = fopen(image_filename, "wb");
        fwrite(cur->image, sizeof(uint8_t), cur->size, out);
        fclose(out);
        num++;
    }

}


int main(int argc, char *argv[])
{
    uint8_t ogg_header[27] = { 0 };
    if (argc < 3)
        return 1;
    char *input = argv[1];
    char *end;
    long fps = strtol(argv[2], &end, 10);
    if (fps <= 0)
        fps = 10;


    int n, i, i_fps;
    long stream_size = 0, offset = 0, seek = 0;
    printf("opening file %s\n", input);
    FILE *file = fopen(input, "rb");
    FILE *ffmpeg, *metadata;
    char cmd[1024], md_filename[1024];
    uint8_t *spots_data = NULL;
    struct OggStream ogg_stream;
    struct SpotsFrame *head = (struct SpotsFrame*) malloc(sizeof(struct SpotsFrame));
    head->image = NULL;
    head->timestamp = 0;
    head->next = NULL;
    struct SpotsFrame *cur = head;
    for (;;) {
        n = fread(ogg_header, sizeof(uint8_t), 27, file);
        if (n < 0)
            goto error;
        if (n == 0 && feof(file))
            break;
        ogg_parse_page(ogg_header, &ogg_stream, file);
        ogg_stream.magic[4] = 0;
        if (ogg_stream.serialno == 1) {
            //ogg_dump_header(&ogg_stream);
            if (ogg_stream.ts != -1) {
                cur->next = (struct SpotsFrame*) malloc(sizeof(struct SpotsFrame));
                if (!cur->next)
                    goto err_no_mem;
                cur = cur->next;
                cur->timestamp = ogg_stream.ts;
            }

            offset = stream_size;
            stream_size += ogg_stream.size;
            uint8_t *spots_data_new = (uint8_t*) realloc(spots_data, stream_size);
            if (!spots_data_new)
                goto err_no_mem;
            spots_data = spots_data_new;
            memcpy(spots_data + offset, ogg_stream.data, ogg_stream.size);

        }
    }

    fclose(file);

    printf("Stream size: %li\n", stream_size);

    spots_dump_header(spots_data);
    seek = SPOTS_HEADER_SIZE;
    cur = head;
    sprintf(md_filename, "%s.ffmetadata", input);
    metadata = fopen(md_filename, "w");
    fprintf(metadata, ";FFMETADATA1\n");
    i = 1;
    // if everything is right this should exactly match up with the number of images present
    while((cur = cur->next)) {
        cur->image = spots_get_image(spots_data, &seek, &cur->size, stream_size);
        if (!(cur == head->next)) {
            fprintf(metadata, "END=%li\ntitle=%d\n", (cur->timestamp), i);
            i++;
        }
        if (cur->size != 0)
            fprintf(metadata, "[CHAPTER]\nTIMEBASE=1/1000\nSTART=%li\n", cur->timestamp);
    }



    fclose(metadata);
    sprintf(cmd, "ffmpeg -i %s -f image2pipe -r %li -i - -i %s -map_metadata 2 -map 0:a -map 1 -c:v libx264 -crf 25 %s.mkv", input, fps, md_filename, input);

    ffmpeg = popen(cmd, "w");

    i_fps = 1000 / fps;
    cur = head->next;
    n = 0;
    for (i = 0; ; i += i_fps) {
        if (!cur || cur->size == 0)
            break;
        fwrite(cur->image, cur->size, sizeof(uint8_t), ffmpeg);
        if (cur->next)
            n = cur->next->timestamp;
        else
            n = INT_MAX;
        if (i > n)
            cur = cur->next;
    }

    pclose(ffmpeg);


    return 0;

error:
    return -1;
err_no_mem:
    return -2;
}
