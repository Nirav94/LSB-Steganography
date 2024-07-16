#include <stdio.h>
#include <string.h>
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

Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_stego_image) // copy header files from source file to stegofile
{
    char str[100];
    rewind(fptr_src_image); // set file pointer to the begin of the file
    fread(str,54,1,fptr_src_image); // read 54 header file
    fwrite(str,54,1,fptr_stego_image);// write header file to the stego image
    return e_success; // retunr success
}

OperationType check_operation_type(char *argv[]) //check the second char. for CLA for which type operation u perform
{
    if( argv[1][1] == 'e')
    {
	return e_encode;	// return encoding operation
    }
    else if( argv[1][1] == 'd')
    {
	return e_decode;	// return decoding operatiion	
    }
    else
    {
	return e_unsupported;
    }
}

Status encode_magic_string(const char* magic_string,EncodeInfo *encInfo)
{
    int i;
    char arr[8];
    for(i=0;magic_string[i] != 0;i++) //loop through  each char of magic string
    {
	fread(arr,8,1,encInfo->fptr_src_image); //read 8 bytes from source imamge
	encode_byte_to_lsb(magic_string[i],arr); // Encode the character into the LSB of each byte
	fwrite(arr,8,1,encInfo->fptr_stego_image); // Write the modified bytes to the stego image
    }
    return e_success;  // Return success status
}

Status encode_byte_to_lsb(char data, char* buffer_image)
{
    int j=0,i=0;
    for(i=7;i>=0;i--)
    {
	buffer_image[j] = ((data & (1<<i)) >> i) | (buffer_image[j] & (~1)); // Modify the LSB of each byte in buffer_image to match the corresponding bit in data
	j++;
    }
    return e_success;
}

Status encode_secret_file_extn_size(int size,EncodeInfo *encInfo) // secret file size from source image
{
    char arr[32];
    fread(arr,32,1,encInfo->fptr_src_image);  // Read 32 bytes from the source image
    encode_size_to_lsb(size,arr);  // Encode the size into the LSB of each byte
    fwrite(arr,32,1,encInfo->fptr_stego_image); // Write the modified bytes to the stego image
    return e_success;
}

Status encode_size_to_lsb( int size,char* buff_image)
{
    int i,j=0;
    for(i=31;i>=0;i--)
    {
	// Modify the LSB of each byte in buff_image to match the corresponding bit in size
	buff_image[j] = ((size & (1<<i)) >> i) | (buff_image[j] & (~1));
	j++;
    }
    return e_success;

}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char str[8];
    int i;
    for(i=0;file_extn[i] != 0;i++)
    {
	fread(str,8,1,encInfo->fptr_src_image); // read 8 bytes from source inmage for read char
	encode_byte_to_lsb(file_extn[i],str);
	fwrite(str,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}

uint get_file_size(FILE *fptr)  // get file size
{
    fseek(fptr,0,SEEK_END); // move file pointer to the end
    int len = ftell(fptr); // get length of  file
    return len;
}

Status encode_secret_file_size(long file_size,EncodeInfo *encInfo) // for encoding secret file size
{
    char arr[32];
    fread(arr,32,1,encInfo->fptr_src_image); // Read 32 bytes from the source image
    encode_size_to_lsb(file_size,arr); // Encode the file size into the LSB of each byte
    fwrite(arr,32,1,encInfo->fptr_stego_image); // Write the modified bytes to the stego image
    return e_success;

}

Status encode_secret_file_data(EncodeInfo *encInfo) // get secret fle data
{
    int i;
    rewind(encInfo->fptr_secret);
    char arr[encInfo->size_secret_file],str[8];
    fread(arr,encInfo->size_secret_file,1,encInfo->fptr_secret);
    for(i=0; i < encInfo->size_secret_file;i++)
    {
	fread(str,8,1,encInfo->fptr_src_image); // Read 8 bytes from the source image
	encode_byte_to_lsb(arr[i],str); // Encode the byte into the LSB of each byte in str
	fwrite(str,8,1,encInfo->fptr_stego_image); // Write the modified bytes to the stego image
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch,1,1,fptr_src) > 0) // Copy the remaining data from the source image to the stego image byte by byte
    {
	fwrite(&ch,1,1,fptr_dest);
    }
    return e_success;

}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image); // Get the image capacity

    strcpy(encInfo->extn_secret_file , strstr(encInfo->secret_fname,".")); // Get the file extension of the secret file

    int size = strlen(encInfo->extn_secret_file); // Get the length of the file extension

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret); // Get the size of the secret file

    uint img_size;

    img_size = get_image_size_for_bmp(encInfo->fptr_src_image);  // Get the image size

    printf("INFO : Image size = %u\n",img_size);

   // Check if the image has enough capacity to hold the encoded data
    if(encInfo->image_capacity > (54 + 40 + 32 + (size*8) + 32 + ( encInfo->size_secret_file * 8)))
    {
	return e_success; // Return success if there is enough capacity
    }
    else
    {
	return e_failure; // Return failure if there is not enough capacity
    }

}


uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
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

Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_success) // Open the files for encoding
    {
	printf("SUCCESS: %s function COMPLETED\n", "open_files" );
	if ( check_capacity(encInfo) == e_success) // Check if the image has enough capacity for encoding
	{
	    printf("Capacity OK\n");
	    // Copy the BMP header from the source image to the stego image
	    if (copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
	    {
		printf("Success\n");

		if(encode_magic_string(MAGIC_STRING,encInfo) == e_success)  //Call magic string function
		{
		    printf("Magic string encoded\n");
		}

		strcpy(encInfo->extn_secret_file , strstr(encInfo->secret_fname,".")); // Get the file extension of the secret file

		int size = strlen(encInfo->extn_secret_file); // Get the length of the file extension

		if(encode_secret_file_extn_size(size,encInfo) == e_success)    //Encode integer value
		{
		    printf("Secret file extn size is encoded\n");
		}

		// Encode the file extension into the stego image
		if(encode_secret_file_extn( encInfo->extn_secret_file, encInfo) == e_success)
		{
		    printf("Secret file extn is encoded\n");
		}

		encInfo->size_secret_file = get_file_size(encInfo->fptr_secret); // Get the size of the secret file

		// Encode the size of the secret file into the stego image
		if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_success)
		{
		    printf("Secret file size encoded\n");
		}

		// Encode the secret file data into the stego image
		if(encode_secret_file_data(encInfo) == e_success)
		{
		    printf("Secret file data encoded\n");
		}

		// Copy the remaining data from the source image to the stego image
	if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
		{
		    printf("Remaining data encoded\n");
		}

		return e_success;

	    }
	    else
	    {
		printf("Failure\n");
		return e_failure;
	    }
	}

    }
    else
    {
	printf("FAILED: %s function failure\n", "open_files" );
    }
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char arr[5];
    strcpy( arr,strstr(argv[2] , ".")); // Get the file extension of the source image file
    if(strcmp(arr,".bmp") == 0)
    {
	encInfo->src_image_fname = argv[2];
	//strcpy(encInfo->src_image_fname,argv[2]);
    }
    else
    {
	return e_failure; // Return failure if the source image file is not a BMP file
    }
    strcpy(arr,strstr(argv[3],".")); // Get the file extension of the secret file
    
    if ( (strcmp(arr,".txt") == 0) || (strcmp(arr,".c") == 0) || (strcmp(arr,".sh") == 0 ))
    {
	encInfo->secret_fname = argv[3];  // Set the secret file name
	//strcpy(encInfo->secret_fname,argv[3]);
    }
    else
    {
	return e_failure; // Return failure if the secret file is not a supported file type
    }
    if(argv[4] != 0)
    {
	strcpy(arr,strstr(argv[4],".")); // Get the file extension of the stego image file
	
	if(strcmp(arr,".bmp") == 0)
	{
	    encInfo->stego_image_fname = argv[4];
	    //strcpy(encInfo->stego_image_fname,argv[4]);
	    //printf("%s\n",encInfo->stego_image_fname);
	}
	else
	{
	    return e_failure;
	}
    }
    else
    {
	encInfo->stego_image_fname = "stego_img.bmp";
	//strcpy(encInfo->stego_image_fname,"stego_img.bmp\0");
    }

    return e_success;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    printf("%s\n",encInfo->stego_image_fname);
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

	return e_failure;
    }
    // No failure return e_success
    return e_success;
}
