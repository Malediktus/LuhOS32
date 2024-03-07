#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define BLOCK_SIZE 128

struct file_entry
{
    char filename[10];
    uint16_t num_blocks;
    uint32_t real_size;
    uint16_t start_block;
} __attribute__((packed));

struct fs_header
{
    uint32_t size;
    uint32_t signature;
    struct file_entry file_table[28];
} __attribute__((packed));

int main(int argc, char **argv)
{
    assert(sizeof(struct fs_header) == 512);

    if (argc % 2 != 1)
    {
        fprintf(stderr, "Usage: %s host_file1 guest_file1 host_file2 guest_file2 ...\n", argv[0]);
        return 1;
    }

    FILE *img = fopen("initrd.img", "wb");
    if (!img)
    {
        fprintf(stderr, "Error: failed to open file initrd.img");
        return 1;
    }

    printf("Writing initrd to initrd.img\n");

    fseek(img, 512, SEEK_SET);

    struct fs_header header = {};
    header.signature = 0xAFA0010D;

    uint32_t num_files = 0;
    uint32_t current_block = 0;

    for (int i = 1; i < argc; i += 2)
    {
        char *host_file = argv[i];
        char *guest_file = argv[i + 1];

        printf("%s => %s\n", host_file, guest_file);

        FILE *fp = fopen(host_file, "r");
        if (!fp)
        {
            fprintf(stderr, "Error: failed to open file %s", host_file);
            return 1;
        }

        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        struct file_entry file_entry = {};
#pragma GCC diagnostic ignored "-Wstringop-truncation"
        strncpy(file_entry.filename, guest_file, 10);

        file_entry.real_size = file_size;
        file_entry.num_blocks = (file_size + (BLOCK_SIZE - 1)) / BLOCK_SIZE;
        file_entry.start_block = current_block;
        current_block += file_entry.num_blocks;

        header.file_table[num_files++] = file_entry;

        uint8_t *data = malloc(file_entry.num_blocks * BLOCK_SIZE);
        memset(data, 0, file_entry.num_blocks * BLOCK_SIZE);
        fread(data, file_size, 1, fp);

        fwrite(data, file_entry.num_blocks * BLOCK_SIZE, 1, img);
    }

    header.size = ftell(img);

    fseek(img, 0, SEEK_SET);
    fwrite(&header, 512, 1, img);

    return 0;
}
