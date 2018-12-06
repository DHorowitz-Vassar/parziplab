/*********************************************************
 * Your Name: Dylan Horowitz	
 * Partner Name: Wilson Housen
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FILE *fp;


/* Returns the Run Length Encoded string for the  
   source string src */
char* RLECompress(char* src) 
{ 
    int rLen; 
    int len = strlen(src); 
  
    /* If all characters in the source string are different,  
    then size of destination string would be twice of input string. 
    For example if the src is "abcd", then dest would be "a1b1c1d1" 
    For other inputs, size would be less than twice.  */
    char* dest = (char*)malloc(sizeof(char) * (len * 2 + 1)); 
  
    int i, j = 0; 
  
    /* traverse the input string one by one */
    for (i = 0; i < len; i++) { 
  
        /* Count the number of occurrences of the new character */
        rLen = 1; 
        while (src[i] == src[i + 1]) { 
            rLen++; 
            i++; 
        } 
  
        dest[j] = src[i];
        j++;
        dest[j] = rLen;
        j++;
    } 
  
    /*terminate the destination string */
    dest[j] = '\0'; 
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
	char* output = RLECompress(input);

	fwrite(output, 5, sizeof(output), stdout);
    printf("\n");
	fclose(fp);
}
