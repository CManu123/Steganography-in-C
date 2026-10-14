#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "types.h"
#include "common.h"

// pretty much just checking we got a bmp to pull the secret out of
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *ptr = strstr(argv[2], ".bmp");
    if (ptr != NULL && strcmp(ptr, ".bmp") == 0)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        fprintf(stderr, "ERROR: %s is not a .bmp file\n", argv[2]);
        return e_failure;
    }

    // user can name the output file, extension gets tacked on automatically later
    if (argv[3] != NULL)
    {
        decInfo->secret_fname = argv[3];
    }
    else
    {
        decInfo->secret_fname = "decoded_output";
    }

    return e_success;
}

Status open_files_decode(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

// reverse of encode_byte_to_lsb - pull the last bit out of 8 pixel bytes and rebuild the original byte
char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;
    for (int i = 0; i < 8; i++)
    {
        int bit = image_buffer[i] & 1; // grab just the lsb
        data = (data << 1) | bit; // shove it back into place
    }
    return data;
}

// same trick but for a 32-bit int (extension size / file size fields)
int decode_size_from_lsb(char *image_buffer)
{
    int size = 0;
    for (int i = 0; i < 32; i++)
    {
        int bit = image_buffer[i] & 1;
        size = (size << 1) | bit;
    }
    return size;
}

// if this doesn't match "#*" then the image was never encoded by us, better stop here
Status decode_magic_string(DecodeInfo *decInfo)
{
    int magic_len = strlen(MAGIC_STRING);
    char decoded[magic_len + 1];

    for (int i = 0; i < magic_len; i++)
    {
        char image_buffer[8];
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image);
        decoded[i] = decode_byte_from_lsb(image_buffer);
    }
    decoded[magic_len] = '\0';

    if (strcmp(decoded, MAGIC_STRING) == 0)
    {
        return e_success;
    }
    return e_failure;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo, int *size)
{
    char image_buffer[32];
    fread(image_buffer, 1, 32, decInfo->fptr_stego_image);
    *size = decode_size_from_lsb(image_buffer);
    return e_success;
}

Status decode_secret_file_extn(int extn_size, DecodeInfo *decInfo)
{
    for (int i = 0; i < extn_size; i++)
    {
        char image_buffer[8];
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image);
        decInfo->extn_secret_file[i] = decode_byte_from_lsb(image_buffer);
    }
    decInfo->extn_secret_file[extn_size] = '\0';

    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo, long *file_size)
{
    char image_buffer[32];
    fread(image_buffer, 1, 32, decInfo->fptr_stego_image);
    *file_size = decode_size_from_lsb(image_buffer);
    return e_success;
}

// this is where the recovered file actually gets written to disk, byte by byte
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    // stitch the name and extension back together, e.g. "recovered" + ".txt"
    char full_fname[256];
    snprintf(full_fname, sizeof(full_fname), "%s%s", decInfo->secret_fname, decInfo->extn_secret_file);

    decInfo->fptr_secret = fopen(full_fname, "wb");
    if (decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", full_fname);
        return e_failure;
    }

    for (long i = 0; i < decInfo->size_secret_file; i++)
    {
        char image_buffer[8];
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image);
        char byte = decode_byte_from_lsb(image_buffer);
        fwrite(&byte, 1, 1, decInfo->fptr_secret);
    }

    fclose(decInfo->fptr_secret);
    return e_success;
}

// mirrors do_encoding but in reverse - check magic, then pull extension, size, data in order
Status do_decoding(DecodeInfo *decInfo)
{
    if (open_files_decode(decInfo) == e_success)
        printf("INFO: File opened successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to open file\n");
        return e_failure;
    }

    // header's just pixel-less metadata, skip past it straight to the image data
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    if (decode_magic_string(decInfo) == e_success)
        printf("INFO: Magic string matched, valid stego image\n");
    else
    {
        fprintf(stderr, "ERROR: Magic string did not match. Not a valid stego image\n");
        return e_failure;
    }

    int extn_size;
    if (decode_secret_file_extn_size(decInfo, &extn_size) == e_success)
        printf("INFO: Secret file extension size decoded successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to decode secret file extension size\n");
        return e_failure;
    }

    if (decode_secret_file_extn(extn_size, decInfo) == e_success)
        printf("INFO: Secret file extension decoded successfully: %s\n", decInfo->extn_secret_file);
    else
    {
        fprintf(stderr, "ERROR: Failed to decode secret file extension\n");
        return e_failure;
    }

    if (decode_secret_file_size(decInfo, &decInfo->size_secret_file) == e_success)
        printf("INFO: Secret file size decoded successfully: %ld\n", decInfo->size_secret_file);
    else
    {
        fprintf(stderr, "ERROR: Failed to decode secret file size\n");
        return e_failure;
    }

    if (decode_secret_file_data(decInfo) == e_success)
        printf("INFO: Secret file data decoded successfully\n");
    else
    {
        fprintf(stderr, "ERROR: Failed to decode secret file data\n");
        return e_failure;
    }

    return e_success;
}
