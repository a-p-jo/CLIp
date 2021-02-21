#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SUFFIX ".temporary_clipboard"

char * combine(char *, char *);
int bcp(FILE *, FILE*);

int main(int argc, char * argv[])
{
	if(argc >= 2)
	{
		uint_fast8_t clip_named = (argc >= 3);

		if(!strcmp(argv[1],"paste") || !strcmp(argv[1],"p"))
		{
			char * infile = "";

			if(clip_named)
				infile = combine(argv[2],SUFFIX);
			else
				infile = SUFFIX;

			if(infile != NULL)
			{	
				FILE * from = fopen(infile,"rb");

				if(from == NULL)
				{
					perror("Failed : Error opening clipboard ");
					return 1;
				}

				int status = bcp(from,stdout);
				if(clip_named)
					free(infile);

				fclose(from);
				return status;
			}
			else
				goto malloc_error;

		}

		else if (!strcmp(argv[1],"copy") || !strcmp(argv[1],"c"))
		{
			char * outfile = "";

			if(clip_named)
				outfile = combine(argv[2],SUFFIX);
			else
				outfile = SUFFIX;

			if(outfile != NULL)
			{	
				FILE * to = fopen(outfile,"wb");

				if(to == NULL)
				{
					perror("Failed : Error opening clipboard ");
					return 3;
				}

				int status = bcp(stdin,to);
				if(clip_named)
					free(outfile);

				if(fclose(to) != 0)
				{
					perror("Failed : Error saving to clipboard ");
					return 4;
				}

				return status;
			}
			else
			{
				malloc_error :
				fprintf(stderr,"Failed : Error allocating memory for clipboard filename\n");
				return 6;
			}
		}

		else if (!strcmp(argv[1],"remove") || !strcmp(argv[1],"r"))
		{
			char * torm = "";

			if(clip_named)
				torm = combine(argv[2],SUFFIX);
			else
				torm = SUFFIX;

			int returnval = 0;

			if(remove(torm))
			{
				perror("Failed ");
				returnval = 7;
			}
			
			if(clip_named)
				free(torm);

			return returnval;
		}
		else
		{
			fprintf(stderr,"Failed : Unrecognizable arguments.\n");
			return 8;
		}
	}

	else
	{
		fprintf(stderr,"Failed : Not enough arguments");
		return 9;
	}
}

char * combine(char * original, char * toadd)
{
	char * result = malloc( strlen(original) + strlen(toadd) + sizeof('\0') );
	// Must free later

	if(result == NULL) return result;

	strcpy(result,original);
	strcat(result,toadd);

	return result;
}

#define BLOCK (1*1048576)

int bcp(FILE * from, FILE * to)
{
        uint_fast8_t buffer[BLOCK];
        uint_fast64_t bytes_processed = 0;
        uint_fast64_t bytes_read;

        while((bytes_read = fread(buffer,1,BLOCK,from)) == BLOCK && fwrite(buffer,1,BLOCK,to) == BLOCK)
		bytes_processed += BLOCK; 

        if(ferror(from) || ferror(to))
	{
            if(ferror(from))
                fprintf(stderr,"Failed : Unknown fatal error reading\n");
            if(ferror(to))
                failed_fwrite : 
                fprintf(stderr,"Failed : Unknown fatal error writing\n");
            
            fprintf(stderr,"Forced to abandon at %llu bytes... exiting...\n",(long long unsigned)(bytes_processed));
            return -2;
        }

        else
        {
            if(bytes_read && fwrite(buffer,1,bytes_read,to) != bytes_read)
                goto failed_fwrite;
        }
	return 0;
}
