#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
// width/height live at byte offset 18 in a bmp, gives us how many pixels we can hide bits in
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

// jump to end, note position, jump back - old trick to get file size
uint get_file_size(FILE *fptr)
{
    long curr = ftell(fptr);
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    fseek(fptr, curr, SEEK_SET);
    return size;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

// makes sure user actually passed a bmp + a real secret file before we touch anything
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *ptr = strstr(argv[2], ".bmp");
    if (ptr != NULL && strcmp(ptr, ".bmp") == 0)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        fprintf(stderr, "ERROR: %s is not a .bmp file\n", argv[2]);
        return e_failure;
    }

    // grab whatever extension the secret file has so we can rebuild it later on decode
    ptr = strstr(argv[3], ".");
    if (ptr != NULL)
    {
        strcpy(encInfo->extn_secret_file, ptr);
        encInfo->secret_fname = argv[3];
    }
    else
    {
        fprintf(stderr, "ERROR: %s does not have an extension\n", argv[3]);
        return e_failure;
    }

    if (argv[4] != NULL)
    {
        ptr = strstr(argv[4], ".bmp");
        if (ptr != NULL && strcmp(ptr, ".bmp") == 0)
        {
            encInfo->dest_image_fname = argv[4];
        }
        else
        {
            fprintf(stderr, "ERROR: %s is not a .bmp file\n", argv[4]);
            return e_failure;
        }
    }
    else
    {
        // no output name given, just fall back to a default
        encInfo->dest_image_fname = "stego_output.bmp";
    }

    return e_success;
}

// opens all three files we need - bail early if any one of them fails
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    encInfo->fptr_dest_image = fopen(encInfo->dest_image_fname, "wb");
    if (encInfo->fptr_dest_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->dest_image_fname);
        return e_failure;
    }

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // each byte we hide eats 8 pixel bytes (1 bit per byte), plus header stuff - gotta make sure it fits
    uint required = (strlen(MAGIC_STRING) * 8) + 32 +
                     (strlen(encInfo->extn_secret_file) * 8) + 32 +
                     (encInfo->size_secret_file * 8);

    if (encInfo->image_capacity > required)
    {
        return e_success;
    }
    return e_failure;
}

// bmp header is always 54 bytes, just copy it over untouched before we start hiding stuff
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);
    char header[54];

    fread(header, 1, 54, fptr_src_image);
    fwrite(header, 1, 54, fptr_dest_image);

    return e_success;
}

// writes "#*" into the lsbs first so decode can later check this is actually a stego image
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    for (unsigned int i = 0; i < strlen(magic_string); i++)
    {
        char image_buffer[8];

        fread(image_buffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], image_buffer);
        fwrite(image_buffer, 1, 8, encInfo->fptr_dest_image);
    }

    return e_success;
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char image_buffer[32];

    fread(image_buffer, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb(size, image_buffer);
    fwrite(image_buffer, 1, 32, encInfo->fptr_dest_image);

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    for (unsigned int i = 0; i < strlen(file_extn); i++)
    {
        char image_buffer[8];

        fread(image_buffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], image_buffer);
        fwrite(image_buffer, 1, 8, encInfo->fptr_dest_image);
    }

    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char image_buffer[32];

    fread(image_buffer, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb((int)file_size, image_buffer);
    fwrite(image_buffer, 1, 32, encInfo->fptr_dest_image);

    return e_success;
}

// the actual hiding happens here, one byte of secret -> 8 bytes of image
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret);

    // load the whole secret into memory first, easier than juggling two file pointers
    char *file_data = (char *)malloc(encInfo->size_secret_file);
    if (file_data == NULL)
    {
        fprintf(stderr, "ERROR: Memory allocation failed\n");
        return e_failure;
    }
    fread(file_data, 1, encInfo->size_secret_file, encInfo->fptr_secret);

    for (long i = 0; i < encInfo->size_secret_file; i++)
    {
        char image_buffer[8];

        fread(image_buffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_data[i], image_buffer);
        fwrite(image_buffer, 1, 8, encInfo->fptr_dest_image);
    }

    free(file_data);
    return e_success;
}

// this is the core trick - stuff one data bit into the last bit of each pixel byte, invisible to the eye
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = (data >> (7 - i)) & 1; // pull out bit i, starting from the msb

        image_buffer[i] = image_buffer[i] & ~1; // clear whatever lsb was already there
        image_buffer[i] = image_buffer[i] | bit; // drop our bit in
    }

    return e_success;
}

// same idea as above but for a 4-byte int (needs 32 pixel bytes instead of 8)
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        int bit = (size >> (31 - i)) & 1;

        imageBuffer[i] = imageBuffer[i] & ~1;
        imageBuffer[i] = imageBuffer[i] | bit;
    }

    return e_success;
}

// once secret is fully hidden, just dump whatever's left of the image as-is
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) > 0)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }

    return e_success;
}

// glues all the steps together in order - header, magic string, extension, size, then the actual data
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_success)
        printf("INFO: Files opened successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to open files\n");
        return e_failure;
    }

    if (check_capacity(encInfo) == e_success)
        printf("INFO: Image has sufficient capacity to encode the secret file\n");
    else
    {
        fprintf(stderr, "ERROR: Image does not have sufficient capacity\n");
        return e_failure;
    }

    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_dest_image) == e_success)
        printf("INFO: BMP header copied successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to copy BMP header\n");
        return e_failure;
    }

    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
        printf("INFO: Magic string encoded successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to encode magic string\n");
        return e_failure;
    }

    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_success)
        printf("INFO: Secret file extension size encoded successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to encode secret file extension size\n");
        return e_failure;
    }

    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
        printf("INFO: Secret file extension encoded successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to encode secret file extension\n");
        return e_failure;
    }

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
        printf("INFO: Secret file size encoded successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to encode secret file size\n");
        return e_failure;
    }

    if (encode_secret_file_data(encInfo) == e_success)
        printf("INFO: Secret file data encoded successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to encode secret file data\n");
        return e_failure;
    }

    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_dest_image) == e_success)
        printf("INFO: Remaining image data copied successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to copy remaining image data\n");
        return e_failure;
    }

    return e_success;
}
