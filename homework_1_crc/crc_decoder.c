#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int char_xor(char a, char b){
    if(a==b){return '0';}
    else{return '1';}
}

int check_codeword(char* extended_dataword, int extended_dataword_size, char* generator, int generator_size){
    //만약 codeword에 에러가 있었다면 remainder가 0이 아니다. 이 함수는 remainder를 반환한다.
    int remainder=0;
    int dataword_size=extended_dataword_size - generator_size + 1;
    int i,start,cur_xor_idx;
    char* remainder_word=(char*)malloc(sizeof(char) * (extended_dataword_size+1));
    for(i=0;i<extended_dataword_size;i++){
        remainder_word[i]=extended_dataword[i];
    }

    for(start=0;start<dataword_size;start++){
        if(remainder_word[start]=='0'){continue;} //최상위 비트가 1이 아니라면 넘어간다
        for(cur_xor_idx=start; cur_xor_idx<start+generator_size;cur_xor_idx++){
            remainder_word[cur_xor_idx] = char_xor(remainder_word[cur_xor_idx], generator[cur_xor_idx - start]);
        }
    }
    for(i=0;i<extended_dataword_size;i++){
        if(remainder_word[i]=='1'){
            remainder += 1 << (extended_dataword_size - i - 1);
        }
    }

    /*for(i=dataword_size;i<extended_dataword_size;i++){
        extended_dataword[i]=remainder_word[i];
    }*/
    free(remainder_word);
    return remainder;
}

int main(int argc, char** argv){
    if(argc!=6){
        //인자 수가 맞지 않으면
        printf("usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
        exit(1);
    }
    char* input_file_name, *output_file_name, *result_file_name, *generator;
    int generator_size, dataword_size, codeword_size;
    FILE* input_file, *output_file, *result_file;
    int padding_size, cur_byte;
    int bit_index, i;
    int input_file_length=0;
    // 만약 dataword size가 4이면, 2개를 붙여서 디코딩해야 한다. 따라서 한 바이트가 다 채워졌는지를 나타내는 변수
    //4이면 1바이트의 반만 차 있다
    //0이면 1바이트가 온전히 비어있다는 뜻
    int byte_filled=0;
    char byte_to_write=0;

    input_file_name=argv[1];
    output_file_name=argv[2];
    result_file_name=argv[3];
    generator=argv[4];
    generator_size=strlen(generator);
    dataword_size=strtol(argv[5], NULL, 10);
    codeword_size=dataword_size + generator_size - 1;

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
    //read the padding size from the input file
    fread(&padding_size, 1, 1, input_file);

    input_file_length=0;
    //padding을 제외한 나머지 인코딩된 파일 길이 계산
    while(fread(&cur_byte, sizeof(cur_byte), 1, input_file)==1){
        input_file_length++;
    }
    //printf("%d\n", input_file_length);

    fseek(input_file, 0, SEEK_SET);
    fread(&padding_size, 1, 1, input_file);
    //패딩 제거
    //bit_index는 이번에 읽을 비트 번지수
    bit_index=padding_size;

    int codeword_num=0, error_codeword_num=0;
    char* current_codeword;
    //read the first byte
    cur_byte= fgetc(input_file);
    byte_filled=0;
    while(cur_byte!=EOF){
        current_codeword=(char*)malloc(codeword_size+1);
        for(i=0;i<codeword_size;i++){
            if(bit_index==8){
                cur_byte= fgetc(input_file);
                if(cur_byte==EOF){
                    //현재 읽은 바이트가 파일 끝이면 다 읽은 것이다
                    break;
                }
                bit_index=0;
            }
            if(cur_byte & (1 << (7-bit_index))){
                current_codeword[i]='1';
            }
            else{
                current_codeword[i]='0';
            }
            bit_index++;
        }
        if(cur_byte==EOF){break;}
        codeword_num++;
        //printf("codeword num %d ", codeword_num);
        //널 문자 삽입
        current_codeword[codeword_size]='\0';
        if(check_codeword(current_codeword, codeword_size, generator, generator_size)){
            error_codeword_num++; //crc 코드 복원중 에러 발생.
        }
        if(dataword_size==4){
            if(byte_filled==0){
                //첫 바이트를 채운다.
                for(i=0;i<dataword_size;i++){
                    if(current_codeword[i]=='1'){
                        byte_to_write+=(1 << (dataword_size - i - 1 + 4));
                    }
                }
                byte_filled+=4;
            }
            else{
                //이미 반 차 있다.
                for(i=0;i<dataword_size;i++){
                    if(current_codeword[i]=='1'){
                        byte_to_write+=(1 << (dataword_size - i - 1));
                    }
                }
                fprintf(output_file,"%c", byte_to_write);
                //printf("now decoded : %d\n", byte_to_write);
                byte_filled=0;
                byte_to_write=0;
            }
        }
        else{
            //dataword size is 8
            for(i=0;i<dataword_size;i++){
                if(current_codeword[i]=='1'){
                    byte_to_write+=(1 << (dataword_size - i - 1));
                }
            }
            fprintf(output_file,"%c", byte_to_write);
            //printf("now decoded : %d\n", byte_to_write);
            byte_to_write=0;
        }
        free(current_codeword);
    }

    fprintf(result_file, "%d %d\n", codeword_num, error_codeword_num);

    fclose(input_file);
    fclose(output_file);
    fclose(result_file);
    return 0;
}