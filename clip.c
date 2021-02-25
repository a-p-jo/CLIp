#include <stdio.h> // fread() fwrite() fprintf() perror() 
#include <stdlib.h>// malloc() free()
#include <stdint.h>// uint_fast8_t uint_fast64_t
#include <string.h>// strcpy() strcat() strcmp()

#define DEFAULT ".clipboard.clip"
#define TMP_FILE_NAME ".temporary.clip"

#define CLEAN_TMP fclose(tmp); remove(TMP_FILE_NAME);

#define BLOCK (1*1048576) // Size of buffer for bcp() in bytes . 1 MiB by default.
int bcp(FILE *, FILE*, char *, char *);	// bcp == bytecopy, copies one stream to the other (here stdin/stdout and clipboard file) byte-accurately through a buffer

int main(int argc, char * argv[])
{
	if(argc >= 2) // Usage : clip [operation] [clipboard name (optional)] [clipboard name (optional)]...
	{
		// Process arguments
		if(argc >= 3) // If named clipboard(s) are given, convert any empty names to DEFAULT
		{
			for(uint_fast8_t i = 2; argv[i] != NULL; i++)
			{
				if(strlen(argv[i]) == 0)
					argv[i] = DEFAULT;
			}
		}		
		else // argv[argc] is defined to exist (NULL). If no names are given, change it to point to DEFAULT, and suitably increment argc
		{
			argv[argc] = DEFAULT;
			argc++;
		}
		//Now we can iterate over argv and use the strings directly in the rest of the code, without rewriting the above for each scenario.

		if(!strcmp(argv[1],"p") || !strcmp(argv[1],"paste"))
		{
			for(uint_fast8_t i = 2; i < argc; i++) // Loop over argv , and write every clipboard to stdout
			{	
				FILE * from = fopen(argv[i],"rb");

				if(from == NULL)
				{
					fprintf(stderr,"Failed : Error opening clipboard \" %s \" : ",argv[i]);
					perror("");
					return 1;
				}

				int status = bcp(from,stdout, argv[i],"stdout"); // try to copy all bytes from clipboard to stdout

				fclose(from);

				if(status) 
					return status; //exit at first failed failed paste
			}
		}

		// copying and appending is essentially the same thing, the difference only being an "ab" instead of "wb" to fopen.
		else if (!strcmp(argv[1],"c") || !strcmp(argv[1],"copy") || !strcmp(argv[1], "a") || !strcmp(argv[1], "append"))
		{
			FILE * tmp = fopen(TMP_FILE_NAME,"w+"); // Create a temp stream which can be written to and also read 

			if(tmp == NULL) 
			{
				perror("Failed : Error opening temporary file ");
				return 8;
			}

			int rval = bcp(stdin,tmp,"stdin","temporary file"); // Copy stdin to the temp stream, as stdin is not rewindable

			if(rval)
				return rval;

			fflush(tmp); // Flush temp stream to ensure all data is written to it by now
			rewind(tmp); // Rewind it so it can be read from the begining in the loop

			for(uint_fast8_t i = 2; i < argc; i++) // Write data from temp stream to every clipboard
			{	
				FILE * to = fopen(argv[i],(*argv[1] == 'c') ? "wb" : "ab"); // Open to write or append as asked to

				if(to == NULL)
				{
					fprintf(stderr,"Failed : Error opening clipboard \" %s \" : ",argv[i]);
					perror("");
					CLEAN_TMP
					return 3;
				}

				int status = bcp(tmp,to,"temporary file",argv[i]); // Copy from temp stream to clipboard
				rewind(tmp); // Rewind temp stream, so it can be re-read in next iteration

				if(fclose(to)) // Close this clipboard
				{
					fprintf(stderr,"Failed : Error saving to clipboard \" %s \" : ",argv[i]);
					perror("");
					CLEAN_TMP
					return 4;
				}

				if(status)
					return status;
			}
			CLEAN_TMP // Cleanup , i.e., close the temp stream and delete the file from disk.
		}

		else if (!strcmp(argv[1],"remove") || !strcmp(argv[1],"r")) 
		{
			int rval = 0; 
			for(uint_fast8_t i = 2; i < argc; i++)	// Remove every clipboard given		
			{
				if(remove(argv[i])) // remove() returns non-zero on failure 
				{
					fprintf(stderr,"Failed : Error removing clipboard \" %s \" : ",argv[i]);
					perror("");
					rval = 7; // do not immediately return , as we may have other files to set.
				}
			}
			return rval;
		}
		else
		{
			fprintf(stderr,"Failed : Unrecognizable arguments.\n");
			return 9;
		}

		return 0; // If we've reached here, we were successful
	}

	else
	{
		fprintf(stderr,"Failed : Not enough arguments\n");
		return 10;
	}
}

int bcp(FILE * from, FILE * to, char * fromname, char * toname)
{
        uint_fast8_t buffer[BLOCK]; // Buffer . Static - on stack or possibly cache , potentially faster than malloc(). Need only have one-byte elements, but whatever's fastest.
        uint_fast64_t bytes_processed = 0;
        uint_fast64_t bytes_read; // Stores number of bytes read in each iteration , so write *only* as much as read when less than BLOCK bytes read.

        while((bytes_read = fread(buffer,1,BLOCK,from)) == BLOCK && fwrite(buffer,1,BLOCK,to) == BLOCK) // "&&" Means fwrite() won't execute if fread() condition fails, so no garbage
		bytes_processed += BLOCK;

        if(ferror(from) || ferror(to)) // If bytes read were less than BLOCK size, check for errors 
	{
            if(ferror(from))
                fprintf(stderr,"Failed : Unknown fatal error reading from \" %s \"\n",fromname);
            if(ferror(to))
                failed_fwrite : 
                fprintf(stderr,"Failed : Unknown fatal error writing to \" %s \"\n",toname);
            
            fprintf(stderr,"Forced to abandon at %llu bytes... exiting...\n",(long long unsigned)(bytes_processed));
            return 11;
        }

        else // If no errors, we've reached EOF. If there's any leftover bytes (likely) , write them
        {
            if(bytes_read && fwrite(buffer,1,bytes_read,to) != bytes_read) // "&&" Means fwrite() won't wastefully execute if bytes_read == 0
                goto failed_fwrite; // In the unlikely case the last fwrite() failed, handle it the same way as above
        }
	return 0; // If we got this far without returning, program was successful. Return 0.
}
