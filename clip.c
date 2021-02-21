#include <stdio.h> // fread() fwrite() fprintf() perror() 
#include <stdlib.h>// malloc() free()
#include <stdint.h>// uint_fast8_t uint_fast64_t
#include <string.h>// strcpy() strcat() strcmp()

#define SUFFIX ".clipboard.clip" // suffix + clipboard name (optional) = filename of clipboard's file.

char * combine(char *, char *); // combine's two strings one-after-the-other into a sufficiently sized new string, which it returns 
int bcp(FILE *, FILE*);	// bcp == bytecopy, copies one stream to the other (here stdin/stdout and clipboard file) byte-accurately through a buffer

int main(int argc, char * argv[])
{
	 /* Usage Examples :
	  * dmesg | clip c                 # Save output of dmesg to default clipboard in current working directory (cwd)
	  * lsblk | clip c drives          # Save output of lsblks to a clipboard named drives in cwd 
	  * 
	  * clip p | gzip > dmesg.zip      # Pipe default clipboard in cwd to gzip , which saves it to a .zip file
	  * clip p drives >> drive_log.txt # Pipe clipboard named dmesg in cwd to drive_log.txt 
	  *
	  * clip r			   # Remove the default clipboard in the cwd
	  * clip r drives		   # Remove the clipboard named drives in the cwd
	  */
	if(argc >= 2) 
	{
		uint_fast8_t clip_named = (argc >= 3);

		if(!strcmp(argv[1],"paste") || !strcmp(argv[1],"p"))
		{
			char * clip_name = ""; // initialise string to empty, null-terminated.

			if(clip_named)
				clip_name = combine(argv[2],SUFFIX); // if clipboard is named , add the suffix to it
			else
				clip_name = SUFFIX; // if clipboard is not named, it's name IS suffix.

			if(infile != NULL)
			{	
				FILE * from = fopen(clip_name,"rb");

				if(from == NULL)
				{
					perror("Failed : Error opening clipboard ");
					return 1;
				}

				int status = bcp(from,stdout); // try to copy all bytes from clipboard to stdout

				if(clip_named)
					free(clip_name); // combine() returns a malloc'd char * , which we have to free 

				fclose(from); 
				return status;
			}
			else
				goto malloc_error; // if combine() returns NULL , malloc() in it had returned NULL , i.e. , failed.

		}

		else if (!strcmp(argv[1],"copy") || !strcmp(argv[1],"c"))
		{
			char * clip_name = "";

			if(clip_named)
				clip_name = combine(argv[2],SUFFIX);
			else
				clip_name = SUFFIX;

			if(clip_name != NULL)
			{	
				FILE * to = fopen(clip_name,"wb");

				if(to == NULL)
				{
					perror("Failed : Error opening clipboard ");
					return 3;
				}

				int status = bcp(stdin,to);
				if(clip_named)
					free(clip_name);

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
				fprintf(stderr,"Failed : Error allocating memory for clipboard filename\n"); // Only print to stderr , as stdout must not be tampered with
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

			if(remove(torm)) // remove() returns non-zero on failure 
			{
				perror("Failed ");
				returnval = 7; // do not immediately return , as we may need to free clip_name 
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

char * combine(char * original, char * toadd) // combine contents of two strings into a new one exactly big enough to accomodate both + \0 and return it
{
	char * result = malloc( strlen(original) + strlen(toadd) + sizeof('\0') );
	// Must free later

	if(result == NULL) return result;

	strcpy(result,original); // copy 1st str to new string
	strcat(result,toadd);    // append 2nd string to new string

	return result;           // return new string
}

#define BLOCK (1*1048576) // Size of buffer for bcp() in bytes . 1 MiB by default.

int bcp(FILE * from, FILE * to)
{
        uint_fast8_t buffer[BLOCK]; // Buffer . Static - on stack or possibly cache , potentially faster than malloc(). Need only have one-byte elements, but whatever's fastest.
        uint_fast64_t bytes_processed = 0;
        uint_fast64_t bytes_read; // Stores number of bytes read in each iteration , so write *only* as much as read when less than BLOCK bytes read.

        while((bytes_read = fread(buffer,1,BLOCK,from)) == BLOCK && fwrite(buffer,1,BLOCK,to) == BLOCK) // "&&" Means fwrite() won't execute if fread() condition fails, so no garbage
		bytes_processed += BLOCK;

        if(ferror(from) || ferror(to)) // If bytes read were less than BLOCK size, check for errors 
	{
            if(ferror(from))
                fprintf(stderr,"Failed : Unknown fatal error reading\n");
            if(ferror(to))
                failed_fwrite : 
                fprintf(stderr,"Failed : Unknown fatal error writing\n");
            
            fprintf(stderr,"Forced to abandon at %llu bytes... exiting...\n",(long long unsigned)(bytes_processed));
            return 10;
        }

        else // If no errors, we've reached EOF. If there's any leftover bytes (likely) , write them
        {
            if(bytes_read && fwrite(buffer,1,bytes_read,to) != bytes_read) // "&&" Means fwrite() won't wastefully execute if bytes_read == 0
                goto failed_fwrite; // In the unlikely case the last fwrite() failed, handle it the same way as above
        }
	return 0; // If we got this far without returning, program was successful. Return 0.
}
