# Image Steganography in C

## About the Project

This project is about hiding secret files inside BMP images using C programming. It uses image steganography to hide data inside an image and allows the hidden file to be extracted later.

The project has two main operations: encoding and decoding.

## Features

- Hide a secret file inside a BMP image
- Extract the hidden file from an encoded image
- Save the output image with a custom filename
- Check input files and command-line arguments
- Use separate functions for encoding and decoding

## Technologies Used

- C programming
- GCC compiler
- Makefile
- File handling and bitwise operations

## Project Structure

```text
Steganography/
├── main.c
├── encode.c
├── encode.h
├── decode.c
├── decode.h
├── common.h
├── types.h
├── Makefile
├── beautiful.bmp
├── beautiful1.bmp
├── secret.txt
├── stego.bmp
└── decoded.txt.txt
```

## How to Compile

Make sure GCC and Make are installed on your system. Open the terminal in the project folder and run:

```bash
make
```

This creates the executable file named `lsb_steg`.

To clean the generated files, run:

```bash
make clean
```

You can also compile the program manually using GCC:

```bash
gcc -Wall -Wextra -g main.c encode.c decode.c -o lsb_steg
```

## How to Run

### Encoding

Run the following command to hide a secret file inside a BMP image:

```bash
./lsb_steg -e beautiful.bmp secret.txt stego.bmp
```

Here, `beautiful.bmp` is the input image, `secret.txt` is the file to hide, and `stego.bmp` is the output image.

### Decoding

Use the following command to extract the hidden file:

```bash
./lsb_steg -d stego.bmp decoded
```

The program extracts the hidden data and saves it using the specified output filename and the recovered file extension.

## How It Works

1. The program reads the input BMP image and secret file.
2. It checks the input files and required arguments.
3. The encoding function hides the secret data inside the image.
4. The encoded image is saved as a new BMP file.
5. The decoding function extracts the hidden data from the encoded image.

## What I Learned

- Working with files in C
- Using pointers and bitwise operations
- Handling binary image data
- Writing modular C programs
- Understanding encoding and decoding techniques

## Author

C Manu

## Note

This project was developed for learning and educational purposes. Steganography hides the presence of data but does not automatically encrypt it. Avoid using it to hide confidential information without proper security measures.
