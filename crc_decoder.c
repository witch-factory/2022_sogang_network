#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
    if(argc!=6){
        //인자 수가 맞지 않으면
        printf("usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
        exit(1);
    }
    char* input_file_name=argv[1];
    char* output_file_name=argv[2];
    char* result_file_name=argv[3];
    char* generator=argv[4];
    int generator_size=strlen(generator);
    int dataword_size=strtol(argv[5], NULL, 10);
    int codeword_size;
    FILE* input_file, *output_file, *result_file;
    char padding_size, cur_byte;
    int bit_index, i;


    input_file=fopen(input_file_name, "r");
    if(input_file==NULL){
        printf("input file open error.\n");
        exit(1);
    }
    output_file= fopen(output_file_name, "w");
    if(output_file==NULL){
        printf("output file open error.\n");
        exit(1);
    }
    result_file=fopen(result_file_name, "w");
    if(result_file==NULL){
        printf("result file open error.\n");
        exit(1);
    }
    if(!(dataword_size==4 || dataword_size==8)){
        printf("dataword size must be 4 or 8.\n");
        exit(1);
    }
    fread(&padding_size, 1, 1, input_file);
    //printf("%d\n", padding_size);

    //패딩 제거
    bit_index=padding_size;
    codeword_size=dataword_size + generator_size - 1;

    char* current_codeword;
    while(1){
        current_codeword=(char*)malloc(codeword_size+1);
        for(i=0;i<codeword_size;i++){

        }
    }


    return 0;
}