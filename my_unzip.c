/*********************************************************
 * Your Name: Dylan Horowitz
 * Partner Name: Wilson Housen
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FILE *fp;

void* RLEUncompress(char* src)
{
   
   int count = *((int*) src);
   char ch = (src[4]);

   char* dest = (char*)malloc(count);
    
   for(int i = 0; i < count; i++){
        dest[i] = ch;
   }
   fwrite(dest, count, 1, stdout);
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
    while(fread(input, 5, 1, fp) > 0){
        RLEUncompress(input);
    }

    printf("\n");
	fclose(fp);
}
