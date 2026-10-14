#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    // need at least the mode flag + one file to do anything useful
    if (argc < 3)
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  Encode: %s -e <src.bmp> <secret_file> [dest.bmp]\n", argv[0]);
        fprintf(stderr, "  Decode: %s -d <stego.bmp> [output_file_base]\n", argv[0]);
        return 1;
    }

    // figure out if user wants to encode or decode
    OperationType op = check_operation_type(argv[1]);

    if (op == e_encode)
    {
        if (argc < 4)
        {
            fprintf(stderr, "ERROR: Insufficient arguments for encoding\n");
            fprintf(stderr, "Usage: %s -e <src.bmp> <secret_file> [dest.bmp]\n", argv[0]);
            return 1;
        }

        printf("INFO: Selected encoding\n");

        EncodeInfo encInfo; // holds everything needed across the whole encode process

        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("INFO: Read and validated encode arguments successfully\n");
        }
        else
        {
            fprintf(stderr, "ERROR: Invalid arguments for encoding\n");
            return 1;
        }

        if (do_encoding(&encInfo) == e_success)
        {
            printf("INFO: Encoding completed successfully\n");
        }
        else
        {
            fprintf(stderr, "ERROR: Encoding failed\n");
            return 1;
        }
    }
    else if (op == e_decode)
    {
        printf("INFO: Selected decoding\n");

        DecodeInfo decInfo; // same idea, just for pulling the secret back out

        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("INFO: Read and validated decode arguments successfully\n");
        }
        else
        {
            fprintf(stderr, "ERROR: Invalid arguments for decoding\n");
            return 1;
        }

        if (do_decoding(&decInfo) == e_success)
        {
            printf("INFO: Decoding completed successfully\n");
        }
        else
        {
            fprintf(stderr, "ERROR: Decoding failed\n");
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Unsupported operation type %s\n", argv[1]);
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  Encode: %s -e <src.bmp> <secret_file> [dest.bmp]\n", argv[0]);
        fprintf(stderr, "  Decode: %s -d <stego.bmp> [output_file_base]\n", argv[0]);
        return 1;
    }

    return 0;
}

// just maps the -e/-d flag to an enum, anything else is unsupported
OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
