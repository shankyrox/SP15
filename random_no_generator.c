#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX 10000 //max limit of random integers generated
#define FILENAME "job_file"
#define DATA_LEN 100

int main() {
	int r, data_len = DATA_LEN; // no. of numbers to generate
	char *filename = FILENAME;
	srand(time(NULL));
	FILE *fp = NULL;
	fp = fopen(filename, "w");
	char str[6];
	fprintf(fp, "1 %d\n", data_len); 	//1 signifies operation is MAX, need to come up with a code
	while(data_len--){
		r = rand()%MAX;
		sprintf(str, "%d ", r);
		fprintf(fp, str);
		memset(str, 0, 6);
	}
	fclose(fp);
	return 0;
}
