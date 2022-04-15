#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//지금 dataword size 4인 상태는 구현.

int char_xor(char a, char b){
    if(a==b){return '0';}
    else{return '1';}
}

void codeword_extend(char* codeword, int codeword_size, int datanum, int dataword_size){
    int i;
    //이진수 변환해서 앞쪽에 쓴다
    for(i=dataword_size-1;i>=0;i--){
        codeword[i]=datanum%2 + '0';
        datanum=datanum/2;
    }
    /* extension for crc. 뒤쪽에 쓴다 */
    for(i=dataword_size;i<codeword_size;i++){
        codeword[i]='0';
    }
    codeword[codeword_size]='\0';
}

void make_codeword(char* extended_dataword, int extended_dataword_size, char* generator, int generator_size){
    int dataword_size=extended_dataword_size - generator_size + 1;
    int i,start,cur_xor_idx;
    char* remainder_word=(char*)malloc(sizeof(char) * extended_dataword_size);
    for(i=0;i<extended_dataword_size;i++){
        remainder_word[i]=extended_dataword[i];
    }

    for(start=0;start<dataword_size;start++){
        if(remainder_word[start]=='0'){continue;} //최상위 비트가 1이 아니라면 넘어간다
        for(cur_xor_idx=start; cur_xor_idx<start+generator_size;cur_xor_idx++){
            remainder_word[cur_xor_idx] = char_xor(remainder_word[cur_xor_idx], generator[cur_xor_idx - start]);
        }
    }

    for(i=dataword_size;i<extended_dataword_size;i++){
        extended_dataword[i]=remainder_word[i];
    }
    //printf("%s\n", extended_dataword);
}

int main(int argc, char** argv) {
    if(argc!=5){
        //인자 수가 맞지 않으면
        printf("usage: ./crc_encoder input_file output_file generator dataword_size\n");
        exit(1);
    }
    char* input_file_name=argv[1];
    char* output_file_name=argv[2];
    char* generator=argv[3];
    int generator_size=strlen(generator);
    int dataword_size=strtol(argv[4], NULL, 10);
    int codeword_size;
    FILE* input_file, *output_file;
    //char input_file_name[]="/home/sunghyun/CLionProjects/network/temp.tx";
    int cur;
    int input_file_length, padding;
    int bit_index; //왼쪽을 0으로 할 때, 현재 바이트에서 몇 번째 비트를 현재 써야 하는가?
    char byte_to_write=0;

    printf("%s\n", output_file_name);
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
    if(!(dataword_size==4 || dataword_size==8)){
        printf("dataword size must be 4 or 8.\n");
        exit(1);
    }
    //coded word는 개당 dataword_size + generator_size - 1 만큼의 크기
    codeword_size=dataword_size + generator_size - 1;
    //입력 파일이 몇 글자인지
    input_file_length=0;
    while((cur= fgetc(input_file)) !=EOF){
        input_file_length++;
    }
    //printf("input file size : %d\n", input_file_length);
    padding=(input_file_length * (dataword_size == 4 ? 2 : 1)) * (codeword_size);
    padding=8 - (padding % 8);
    padding=padding % 8;
    //printf("%d padding\n", padding);
    fwrite(&padding, 1, 1, output_file);

    byte_to_write=0;
    bit_index=padding;
    //for(bit_index=0;bit_index<padding;bit_index++);
    //padding만큼의 왼쪽 비트 0으로 만들기. padding은 무조건 0보다 작다.
    //printf("byte to write : %d\n", byte_to_write);

    fseek(input_file, 0, SEEK_SET);

    while((cur= fgetc(input_file)) !=EOF){
        /* 먼저 dataword_size가 4인 경우부터 한다 */
        int first_datanum, second_datanum;
        char* first_codeword, *second_codeword;
        int i,j;
        //printf("%d %d\n", cur/16, cur%16);
        if(dataword_size==4){
            first_datanum=cur/16; //앞쪽 dataword
            second_datanum=cur%16;
            first_codeword=(char*)malloc(sizeof(char)*(codeword_size + 1));
            second_codeword=(char*)malloc(sizeof(char)*(codeword_size + 1));

            codeword_extend(first_codeword, codeword_size, first_datanum, dataword_size);
            codeword_extend(second_codeword, codeword_size, second_datanum, dataword_size);

            make_codeword(first_codeword, codeword_size, generator, generator_size);
            make_codeword(second_codeword, codeword_size, generator, generator_size);

            //이제 만들어진 codeword를 파일에 비트 단위로 써야 한다.
            for(i=0;i<codeword_size;i++){
                if(bit_index==8){
                    printf("%d\n",byte_to_write);
                    fwrite(&byte_to_write, 1, 1, output_file);
                    bit_index=0;
                    byte_to_write=0;
                }
                if(first_codeword[i]=='1'){
                    byte_to_write = byte_to_write | (1<<(7-bit_index));
                }
                bit_index++;
            }
            for(i=0;i<codeword_size;i++){
                if(bit_index==8){
                    printf("%d\n", byte_to_write);
                    fwrite(&byte_to_write, 1, 1, output_file);
                    bit_index=0;
                    byte_to_write=0;
                }
                if(second_codeword[i]=='1'){
                    byte_to_write = byte_to_write | (1<<(7-bit_index));
                }
                bit_index++;
            }
            if(bit_index==8){
                printf("%d\n", byte_to_write);
                fwrite(&byte_to_write, 1, 1, output_file);
                bit_index=0;
                byte_to_write=0;
            }
        }
        else{
            //dataword size is 8
            first_datanum=cur;

            first_codeword=(char*)malloc(sizeof(char)*(codeword_size + 1));

            codeword_extend(first_codeword, codeword_size, first_datanum, dataword_size);
            make_codeword(first_codeword, codeword_size, generator, generator_size);

            for(i=0;i<codeword_size;i++){
                if(bit_index==8){
                    printf("%d\n",byte_to_write);
                    fwrite(&byte_to_write, 1, 1, output_file);
                    bit_index=0;
                    byte_to_write=0;
                }
                if(first_codeword[i]=='1'){
                    byte_to_write = byte_to_write | (1<<(7-bit_index));
                }
                bit_index++;
            }
        }
    }

    return 0;
}
