/*********************************************************
 * Your Name: Dylan Horowitz	
 * Partner Name: Wilson Housen
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



/*RLECompress algorithm as written at
http://paulbourke.net/dataformats/compress/rle.c*/

int RLECompress(unsigned char *output,unsigned char *input,int length)
{
   int count = 0,index,i;
   unsigned char pixel;
   int out = 0;

   while (count < length) {
      index = count;
      pixel = input[index++];
      while (index < length && index - count < 127 && input[index] == pixel)
         index++;
      if (index - count == 1) {
         /* 
            Failed to "replicate" the current pixel. See how many to copy.
            Avoid a replicate run of only 2-pixels after a literal run. There
            is no gain in this, and there is a risK of loss if the run after
            the two identical pixels is another literal run. So search for
            3 identical pixels.
         */
         while (index < length && index - count < 127
               && (input[index] != input[index-1]
               || index > 1 && input[index] != input[index-2]))
            index++;
         /* 
            Check why this run stopped. If it found two identical pixels, reset
            the index so we can add a run. Do this twice: the previous run
            tried to detect a replicate run of at least 3 pixels. So we may be
            able to back up two pixels if such a replicate run was found.
         */
         while (index < length && input[index] == input[index-1])
            index--;
         output[out++] = (unsigned char)(count - index);
         for (i=count;i<index;i++)
            output[out++] = input[i];
      } else {
         output[out++] = (unsigned char)(index - count);
         output[out++] = pixel;
      } /* if */
      count=index;
   } /* while */
   return(out);
}

void argument_overload_error(int argc, char *argv[]){
	fprintf(stderr, "Error: Unexpected arguments: ");
	for(int x = 2; x < argc; x++){fprintf(stderr, " %s ", argv[x]);}
	fprintf(stderr, "\n");
	exit(1);
}

void file_not_found_error(const char *filename){
	fprintf(stderr, "Error: file %s not found. \n", filename);
	exit(1);
}


int main(int argc, char *argv[]) {
	if(argc > 2){argument_overload_error(argc, argv);}
	if(access(argv[1], F_OK) != 0){file_not_found_error(argv[1]);}
	FILE *fp;
	fp = fopen(argv[1], "r");
	char input[100];
	char output[100];
	size_t bytes_read;

	bytes_read = fread(input, 5, 1, fp);
	fprintf(stdout, "%s \n", input);
}
