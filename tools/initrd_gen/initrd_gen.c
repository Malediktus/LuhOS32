// TODO:
// - use command line parser and redesign interface (provide folder to transform to image instead of single files)
// - implement subdirectories

// Implementation from http://www.jamesmolloy.co.uk/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>

struct initrd_header
{
    unsigned char magic;
    char name[64];
    unsigned int offset;
    unsigned int length;
} __attribute__((packed));

int main(int argc, char **argv)
{
    /*char *in_directory = NULL;
    char *out_filename = NULL;
    int c = 0;
    opterr = 0;

    while ((c = getopt(argc, argv, "i:o:")) != -1)
    {
        switch (c)
        {
        case 'i':
            in_directory = optarg;
            break;
        case 'o':
            out_filename = optarg;
            break;
        }
    }

    if (!in_directory)
    {
        fprintf(stderr, "no input directory specified\nuse -i {input directory} and -o {output file}\n");
        return 1;
    }

    if (!out_filename)
    {
        fprintf(stderr, "no output file specified\nuse -i {input directory} and -o {output file}\n");
        return 1;
    }*/
    int nheaders = (argc - 1) / 2;
    struct initrd_header headers[64];
    printf("size of header: %ld\n", sizeof(struct initrd_header));
    unsigned int off = sizeof(struct initrd_header) * 64 + sizeof(int);
    int i;
    for (i = 0; i < nheaders; i++)
    {
        printf("writing file %s->%s at 0x%x\n", argv[i * 2 + 1], argv[i * 2 + 2], off);
        strcpy(headers[i].name, argv[i * 2 + 2]);
        headers[i].offset = off;
        FILE *stream = fopen(argv[i * 2 + 1], "r");
        if (stream == 0)
        {
            printf("Error: file not found: %s\n", argv[i * 2 + 1]);
            return 1;
        }
        fseek(stream, 0, SEEK_END);
        headers[i].length = ftell(stream);
        off += headers[i].length;
        fclose(stream);
        headers[i].magic = 0xBF;
    }

    FILE *wstream = fopen("./initrd.img", "w");
    unsigned char *data = (unsigned char *)malloc(off);
    fwrite(&nheaders, sizeof(int), 1, wstream);
    fwrite(headers, sizeof(struct initrd_header), 64, wstream);

    for (i = 0; i < nheaders; i++)
    {
        FILE *stream = fopen(argv[i * 2 + 1], "r");
        unsigned char *buf = (unsigned char *)malloc(headers[i].length);
        fread(buf, 1, headers[i].length, stream);
        fwrite(buf, 1, headers[i].length, wstream);
        fclose(stream);
        free(buf);
    }

    fclose(wstream);
    free(data);

    return 0;
}
