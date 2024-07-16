#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "types.h"

// Function to read and validate decode arguments
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if(argc > 2) // check if argmnts provided 
    {
	if(strcmp(strstr(argv[2], "."),".bmp") == 0) // Check if the file extension is .bmp
	{
	    decInfo->stego_image_fname = argv[2]; // Set stego image file name
	}
	else
	{
	    fprintf(stderr,"Error : Stego image %s format should be .bmp\n", argv[2]); // Print error message
	    return e_failure;
	}
    }
    else
    {
	fprintf(stderr,"Error : Arguments are misssing\n"); //print error msg
	printf("Test_encode: Encoding: ./a.out -e <.bmp file> <.txt file> [Output file]\n");
	printf("./Test_encode: Deconding: ./a.out -d <.bmp file> [output file]\n");
	return e_failure;
    }
    if(argc > 3)  
    {

	decInfo->output_file_name = argv[3]; // set output file name
    }
    else
    {
	decInfo->output_file_name = "out"; // et default output file name
    }

    return e_success;
}

// function to perform decoding
Status do_decoding(DecodeInfo *decInfo)
{
    printf("INFO : Decoding Procedure Started\n"); //prints info msg
    printf("INFO : Opening required files\n");
    // open file for decoding
    if(open_decode_files(decInfo) == e_success)
    {
	uint data;
	printf("Enter magic string\n");  // Prompt user for magic string
	scanf("%s",decInfo->password);  // Read user input
	printf("user input = %s\n", decInfo -> password); // print user input
	decInfo->password_size = strlen(decInfo->password); // calculate password size
	// decode magic string
	if(decode_magic_string(decInfo->password,decInfo->fptr_stego_image, decInfo) == e_success)
	{
	    printf("INFO : Magic string Decoded\n"); // prints info msg
	    if(decode_output_file_extn_size(decInfo->fptr_stego_image,decInfo ) == e_success) // decoding file extension size
	    {
		printf("Extension size successfully decoded\n");
		if(decode_output_file_extn(decInfo->output_file_extn_size,decInfo,decInfo->fptr_stego_image) == e_success) // decoding file extension
		{
		    strcat(decInfo->output_file_name,decInfo->output_file_extn); // Append file extension to output file name
		    //printf("%s\n",decInfo->output_file_name);
		    printf("Output file extension is decoded\n");
		    if(decode_file_size(decInfo,decInfo->fptr_stego_image) == e_success) // decode file ata size
		    {
			printf("File size is decoded\n");
			if(decode_file_data(decInfo) == e_success) // Decode file data

			{
			    printf("File data decoded\n");
			}
			else
			{
			    printf("Data not decoded\n");
			    return e_failure;
			}
		    }
		    else
		    {
			printf("File size not decoded\n");
			return e_failure;
		    }
		}
		else
		{
		    printf("Output file extension is not decoded\n");
		    return e_failure;
		}
	    }
	    else
	    {
		printf("Extension size not decoded\n");
		return e_failure;
	    }
	}
	else
	{
	    printf("Decoding magic string failed\n");
	    return e_failure;
	}
    return e_success;
    }
}
// Function to open files required for decoding
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"r"); // Open stego image file for reading
    if(decInfo->fptr_stego_image == NULL) // Check if file opening failed
    {
	perror("fopen");
	fprintf(stderr,"Error : unable to open file %s\n",decInfo->stego_image_fname);
	return e_failure;
    }
    else
    {
	printf("INFO : opened %s\n",decInfo->stego_image_fname);
    }
    return e_success;
}

// Function to decode magic string
Status decode_magic_string( char* password, FILE *fptr,DecodeInfo *decInfo)
{
    printf("%s\n", password); // prints password
    fseek(decInfo->fptr_stego_image,54,SEEK_SET);
    char arr[8];
    int i;
    for(i = 0;i<5;i++)		// Loop to read and decode magic string
    {
	fread(arr,8,1,fptr);	// read 8 bytes
	decode_lsb_to_byte(arr,&(decInfo->magic_string[i])); // Decode byte to magic string
    }
    decInfo -> magic_string[i] = '\0';   // Null-terminate the magic string
    printf("magic string = %s  password = %s\n", decInfo -> magic_string, password); // Print magic string and password
    if( strcmp(password,decInfo->magic_string) == 0 ) // Check if password matches magic string
    {
	printf("Password matched\n");
	return e_success;
    }
    else
    {
	printf("Password incorrect\n");
	return e_failure;
    }

}

// Function to decode least significant bit to byte
Status decode_lsb_to_byte(char*decode_data,char*magic_string)
{
    int i,j=0;
    *magic_string=0; // Initialize magic string byte

    for(i=7;i>=0;i--) // Loop to decode byte
    {
	*magic_string = (decode_data[j] & 1) << i | *magic_string; // Decode bit to byte
	j++;
    }
}

// Function to decode output file extension size
Status decode_output_file_extn_size(FILE* fptr,DecodeInfo* decInfo)
{
    char arr[32];
    fread(arr,32,1,fptr); // read 31 bytes
    decode_lsb_to_size(arr,&(decInfo->output_file_extn_size)); // decode size
}

Status decode_lsb_to_size(char* decode_data,long* size ) // Function to decode least significant bit to size
{
    int i,j=0;
    *size=0;  // initialize size
    for(i=31;i>=0;i--)  // loop to decode size
    {
	*size = (decode_data[j] & 1) << i | *size;  // decode bit to size
	j++;
    }
    return e_success;
}

// Function to decode output file extension
Status decode_output_file_extn(uint size,DecodeInfo *decInfo,FILE* fptr)
{
    int i;
    char arr[8];
    for(i=0;i< size; i++)
    {
	fread(arr,8,1,fptr);
	decode_lsb_to_byte(arr,&(decInfo->output_file_extn[i]));  // Decode byte to file extension
    }
    decInfo->output_file_extn[i] = '\0'; // Null-terminate the file extension
   
    return e_success;

}

// Function to decode file size
Status decode_file_size(DecodeInfo *decInfo,FILE *fptr)
{
    char arr[decInfo->output_file_extn_size*8];
    fread(arr,decInfo->output_file_extn_size*8,1,fptr); // Read bytes for file size
    decode_lsb_to_size(arr,&(decInfo->output_file_data_size)); // Decode file size
    
    return e_success;

}
// Function to decode file data
Status decode_file_data(DecodeInfo *decInfo)
{
    int i;
    char ch,arr[8];
    FILE* fptr_output_file = fopen(decInfo->output_file_name,"w"); // Open output file for writing
    for(i=0;i<decInfo->output_file_data_size;i++) // Loop to decode file data
    {
	fread(arr,8,1,decInfo->fptr_stego_image); // Read 8 bytes
	decode_lsb_to_byte(arr,&ch);
	fwrite(&ch,1,1,fptr_output_file); // write 8 bytes
    }

    return e_success;

}




