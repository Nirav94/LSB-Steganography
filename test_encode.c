#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "common.h"
#include "types.h"

int main(int argc, char *argv[])
{
    if(argc == 1 || argc == 2)
    {
	printf("Error please pass ./a.out -e beautiful.bmp secret.txt\n");
    }
    
    else
    {
	int ret = check_operation_type(argv);

	if( ret == e_encode)
	{
	    EncodeInfo encInfo;
	    printf("Selected encoding\n");
	    if(read_and_validate_encode_args(argv,&encInfo) == e_success)
	    {
		printf("read and validate is successfully done\n");
		if(do_encoding(&encInfo)== e_success)
		{
		    printf("Encoding successfully done\n");
		}
		else
		{
		    printf("Encoding FAILURE\n");
		}
	    }
	    else
	    {
		printf("Read and validation is FAILURE\n");
	    }
	}
	
	else if(ret == e_decode)
	{
	    DecodeInfo DecInfo;
	    printf("Selected decoding\n");
	    if(read_and_validate_decode_args(argc,argv,&DecInfo) == e_success)
	    {
		printf("Read and validate is successfully done\n");
		if(do_decoding(&DecInfo)==e_success)
		{
		    printf("Decoding successfully done\n");
		}
		else
		{
		    printf("Decoding FAILURE\n");
		}
	    }
	    else
	    {
		printf("Read and validation is FAILURE\n");
	    }
	}
	else
	{
	    printf("Unsupported\n");
	}
    }
    return 0;
}
