/*********************************************************
 * Your Name: Dylan Horowitz
 * Partner Name: Wilson Housen
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FILE *fp;

char* RLEUncompress(char* src)
{
   signed char count;
   int len = strlen(src);

   char* dest = (char*)malloc(sizeof(char) * (len * 2 + 1));

   int i, j, k = 0;

   for(i = 0; i < len; i++){
        count = src[i];
        i++;
        for(j = 0; j < count; j++){
           dest[k] = src[i];
           k++; 
        }
   }   

   return dest;
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

    fp = fopen(argv[1], "r");
    fseek(fp, 0L, SEEK_END);
    char input[ftell(fp)];
    rewind(fp);
    fread(input, 5, sizeof(input), fp);
    char *output = RLEUncompress(input);

    fwrite(output, 5, sizeof(output), stdout);
    printf("\n");
	fclose(fp);
}
