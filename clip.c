#include <stdio.h>  /* fread() fwrite() fprintf() strerror() perror() fopen() fclose() ferror() feof() FILE remove() */
#include <stdlib.h> /* exit() 											     */
#include <stdint.h> /* uint_fast8_t uint64_t									     */
#include <string.h> /* strcmp() 										     */
#include <errno.h>  /* errno											     */

/* Constants */
#define DEFAULT   ".clipboard.clip"
#define TMP_FNAME ".temporary.clip"

/* Exit Codes */
#define SUCCESS 	    0
#define CLIPBOARD_OPEN_ERR  1
#define CLIPBOARD_CLOSE_ERR 2
#define TMP_OPEN_ERR        3
#define TMP_FFLUSH_ERR      4
#define TMP_REWIND_ERR      5
#define UNRECOGNISED_ARGS   6
#define NOT_ENOUGH_ARGS     7
#define BCP_MID_IO_ERR      8

int bcp(FILE * from, FILE * to, char * fromname, char * toname)
{
        #define BLOCK (1048576) 
	uint_fast8_t buffer[BLOCK];    /* Static buffer; faster than malloc(). Need only one-byte per element, but whatever's fastest. */
        uint64_t bytes_processed = 0;
        uint64_t bytes_read;

        while((bytes_read = fread(buffer,1,BLOCK,from)) == BLOCK && fwrite(buffer,1,BLOCK,to) == BLOCK) /* fwrite() won't execute if fread() fails */
		bytes_processed += BLOCK;

        if(ferror(from) || ferror(to)) /* If bytes read were less than BLOCK size, check for errors */
	{
            if(ferror(from))
                fprintf(stderr,"Failed : Unknown fatal error reading from \" %s \"\n",fromname);
            if(ferror(to))
                failed_fwrite : 
                fprintf(stderr,"Failed : Unknown fatal error writing to \" %s \"\n",toname);
            
            fprintf(stderr,"Forced to abandon at %llu bytes... exiting...\n",(long long unsigned)(bytes_processed));
            return BCP_MID_IO_ERR;
        }
        else 			       /* If no errors, we've reached EOF. If there's any leftover bytes (likely) , write them */
        {
            if(bytes_read && fwrite(buffer,1,bytes_read,to) != bytes_read)
                goto failed_fwrite;    /* In the unlikely case the last fwrite() failed, handle it the same way as previously */
        }
	return SUCCESS;
}

int main(int argc, char ** argv)
{
	/* Usage : clip [operation] [clipboard name (optional)] [clipboard name (optional)]... */
	if(argc >= 2)
	{
		              /* Process arguments */
		if(argc >= 3) /* If named clipboard(s) are given, convert any empty names to DEFAULT */
		{
			for(int i = 2; argv[i]; i++)
			{
				if(*argv[i] == '\0')
					argv[i] = DEFAULT;
			}
		}		
		else          /* argv[argc] is defined to be NULL. If no names are given, change it to point to DEFAULT, and suitably increment argc */
		{
			argv[argc] = DEFAULT;
			argc++;
		}

		switch(*argv[1])
		{
			case 'p': case 'P':           /* Paste */
			for(int i = 2; i < argc; i++) /* Loop over argv , and write every clipboard to stdout */
			{	
				FILE * from = fopen(argv[i],"rb");

				if(!from)
				{
					fprintf(stderr,"Failed : Error opening clipboard \" %s \" : %s\n",strcmp(argv[i],DEFAULT)? argv[i] : "default",errno? strerror(errno) : "Unknown error");
					exit(CLIPBOARD_OPEN_ERR);
				}

				int status = bcp(from,stdout, argv[i],"stdout");
				if(status) exit(status);
			}
			break;
			case 'c' : case 'C' : case 'a' : case 'A' : 		    /* Copy or append */
			{
				FILE * tmp = fopen(TMP_FNAME,"wb+"); 

				if(!tmp) 
				{
					perror("Failed : Error opening temporary file ");
					exit(TMP_OPEN_ERR);
				}

				int rval = bcp(stdin,tmp,"stdin","temporary file"); /* Copy stdin to the temp stream, as stdin is not seekable */
				if(rval) exit(rval);

				if(fflush(tmp))					    /* Flush temp stream to ensure all data is written to it by now */
				{
					perror("Failed : Error writing to temporary file ");
					exit(TMP_FFLUSH_ERR);
				}
				if(fseek(tmp,0,SEEK_SET)) 			    /* Rewind it so it can be read from the begining in the loop */
				{
					perror("Failed : Error rewinding temporary file ");
					exit(TMP_REWIND_ERR);
				}

				for(int i = 2; i < argc; i++) 			    /* Write data from temp stream to every clipboard */
				{	
					FILE * to = fopen(argv[i],(*argv[1] == 'c' || *argv[1] == 'C') ? "wb" : "ab");

					if(to == NULL)
					{
						fprintf(stderr,"Failed : Error opening clipboard \" %s \" : %s\n",strcmp(argv[i],DEFAULT)? argv[i] : "default",errno? strerror(errno) : "Unknown error");
						fclose(tmp) ;remove(TMP_FNAME);
						exit(CLIPBOARD_OPEN_ERR);
					}

					int status = bcp(tmp,to,"temporary file",argv[i]); 
					if(fseek(tmp,0,SEEK_SET)) 		    /* Rewind temp stream, so it can be re-read in next iteration */
					{
						perror("Failed : Error rewinding temporary file ");
						exit(TMP_REWIND_ERR);
					}

					if(fclose(to)) 				    /* Close this clipboard */
					{
						fprintf(stderr,"Failed : Error saving to clipboard \" %s \" : %s\n",strcmp(argv[i],DEFAULT)? argv[i] : "default",errno? strerror(errno) : "Unknown error");
						remove(TMP_FNAME);
						exit(CLIPBOARD_CLOSE_ERR);
					}

					if(status) { remove(TMP_FNAME); exit(status); }
				}
				fclose(tmp); remove(TMP_FNAME);
				break;
			}
			case 'r' : case 'R': /* Remove */
			{
				int rval = 0; 
				for(int i = 2; i < argc; i++)		
				{
					if(remove(argv[i]))
					{
						fprintf(stderr,"Failed : Error removing clipboard \" %s \" : %s\n",strcmp(argv[i],DEFAULT)? argv[i] : "default",errno? strerror(errno) : "Unknown error");
						rval = 7; /* do not immediately exit , as we may have other files to remove */
					}
				}
				exit(rval);
			}
			default:
				fprintf(stderr,"Failed : Unrecognizable arguments.\n");
				exit(UNRECOGNISED_ARGS);
				break;
		}

		exit(SUCCESS);
	}
	else
	{
		fprintf(stderr,"Failed : Not enough arguments\n");
		exit(NOT_ENOUGH_ARGS);
	}
}
