#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>

#include "types.h" // Contains user defined types

/*
 * Structure to store information required for
 * decoding secret file from stego Image
 */

typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *stego_image_fname; // To store the stego image fname
    FILE *fptr_stego_image;  // To store the address of the stego image

    /* Decoded Secret File Info */
    char *secret_fname;       // To store the output secret file name (without extn)
    FILE *fptr_secret;        // To store the address of the decoded secret file
    char extn_secret_file[5]; // To store the decoded secret file extension
    long size_secret_file;    // To store the decoded secret data size

} DecodeInfo;

/* Decoding function prototypes */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_files_decode(DecodeInfo *decInfo);

/* Decode Magic String */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo, int *size);

/* Decode secret file extension */
Status decode_secret_file_extn(int extn_size, DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo, long *file_size);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode a byte from LSBs of image data array */
char decode_byte_from_lsb(char *image_buffer);

/* Decode a size (int) from LSBs of image data array */
int decode_size_from_lsb(char *image_buffer);

#endif
