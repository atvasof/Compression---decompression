#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define N 8
#define PI 3.14159265358979323846
#define ROUND( a )      ( ( (a) < 0 ) ? (int) ( (a) - 0.5 ) : \
                                                  (int) ( (a) + 0.5 ) )

int bin2dec(char *bin);//binary se dekadiko

int show_bits(int from_bit, int n_bits);

void init_buffer();//apokodikopoihsh kai apothikeysh ston pinaka xarakthrwn bin_string
void init_decode_table();//arxikopoiei toys pinakes table kai bits
void zz_vector_init();	//arxikopoihsh tou run_length vector
void zz_vector(int n_block);//paragei to dianysma run length pou antistoixei sto n_block
void dequantize(int n_block); //apokvantikopoihsh
void init_dctt();			//arxikopoihsh toy antistrofou dct
void dctt(int n_block);		//antistrofos dct

float DCTt[N][N];

int table[65536];	//pinakes apokvdikopoihshs symfvna me thn ekdoxh 3 tvn diafaniwn
int bits[65536];	//
char *bin_string;	

int *rle;
int rle_length;

int zz[N*N];
int quant[N][N];
int output[N][N];
unsigned char block[N][N];

int zigzag[ N*N ][ 2 ] =
{
    {0, 0},
    {0, 1}, {1, 0},
    {2, 0}, {1, 1}, {0, 2},
    {0, 3}, {1, 2}, {2, 1}, {3, 0},
    {4, 0}, {3, 1}, {2, 2}, {1, 3}, {0, 4},
    {0, 5}, {1, 4}, {2, 3}, {3, 2}, {4, 1}, {5, 0},
    {6, 0}, {5, 1}, {4, 2}, {3, 3}, {2, 4}, {1, 5}, {0, 6},
    {0, 7}, {1, 6}, {2, 5}, {3, 4}, {4, 3}, {5, 2}, {6, 1}, {7, 0},
    {7, 1}, {6, 2}, {5, 3}, {4, 4}, {3, 5}, {2, 6}, {1, 7},
    {2, 7}, {3, 6}, {4, 5}, {5, 4}, {6, 3}, {7, 2},
    {7, 3}, {6, 4}, {5, 5}, {4, 6}, {3, 7},
    {4, 7}, {5, 6}, {6, 5}, {7, 4},
    {7, 5}, {6, 6}, {5, 7},
    {6, 7}, {7, 6},
    {7, 7}
};

int Quant_Luminance[N][N]=
{{16, 11, 10, 16, 24, 40, 51, 61},
{12, 12, 14, 19, 26, 58, 60, 55},
{14, 13, 16, 24, 40, 57, 69, 56},
{14, 17, 22, 29, 51, 87, 80, 62},
{18, 22, 37, 56, 68, 109, 103, 77},
{24, 35, 55, 64, 81, 104, 113, 92},
{49, 64, 78, 87, 103, 121, 120, 101},
{72, 92, 95, 98, 112, 100, 103, 99}};

int Quant_Chrominance[N][N]=
{{17, 18, 24, 47, 99, 99, 99, 99},
{18, 21, 26, 66, 99, 99, 99, 99},
{47, 66, 99, 99, 99, 99, 99, 99},
{99, 99, 99, 99, 99, 99, 99, 99},
{99, 99, 99, 99, 99, 99, 99, 99},
{99, 99, 99, 99, 99, 99, 99, 99},
{99, 99, 99, 99, 99, 99, 99, 99},
{99, 99, 99, 99, 99, 99, 99, 99}};


int main(){
	/*int i,j;
	init_dctt();
	init_buffer();
	init_decode_table();
	dctt(82);
	for(i=0;i<8;i++){
	for(j=0;j<8;j++)
	printf("%d ",output[i][j]);
	printf("\n");}*/
	int i,j,n_block,offset,n,k;
	FILE *fp;
	offset=0;
	n_block=0;

	
	init_dctt();
	init_decode_table();
	init_buffer();
	zz_vector_init();
	

	fp = fopen("src14_cmp.yuv", "wb");

	printf("...saving file\n");
	while(offset<8100){

		for(j=0;j<8;j++){
			for(n_block=offset; n_block<offset+90; n_block++){
				dctt(n_block);
				for(i=0;i<8;i++){
				fputc(block[j][i],fp);
					
		}
			}
				}
		offset=offset+90;
	}
	printf("The file is decompressed\n");
	fclose(fp);

	free(bin_string);
}






int bin2dec(char *bin)
{
int b, k, m, n;
int len, sum = 0;

while(strlen(bin)!=16){
	strcat(bin,"0");
}

len = strlen(bin) - 1;

for(k = 0; k <= len; k++)
{
n = (bin[k] - '0'); // char to numeric value
if ((n > 1) || (n < 0))
{
puts("\n\n ERROR! BINARY has only 1 and 0!\n");
return (0);
}
for(b = 1, m = len; m > k; m--)
{
// 1 2 4 8 16 32 64 ... place-values, reversed here
b *= 2;
}
// sum it up
sum = sum + n * b;
//printf("%d*%d + ",n,b); // uncomment to show the way this works
}
return(sum);
}

void init_decode_table(){

char string_code[17];
	int j=0;
	int i=0;
	FILE *fp;



	struct dictionary{
		int code;
		int symbol;
		int binary_length;
	};

	struct dictionary	decode_book[149];

	fp=fopen("codebook.txt","r");


	

			while(fscanf(fp,"%d %s \n",&decode_book[i].symbol,string_code)==2){
				decode_book[i].binary_length=strlen(string_code);
				decode_book[i].code=bin2dec(string_code);
				i++;
			}
		fclose(fp);
		
		
		
		
		for(i=0;i<65536;i++){
			if(i==decode_book[j].code)j++;
			table[i]=decode_book[j-1].symbol;
			bits[i]=decode_book[j-1].binary_length;
		}
		
		//debug
		//for(i=0;i<149;i++)printf("symbol %d code %d length %d\n",decode_book[i].symbol,decode_book[i].code,decode_book[i].binary_length);
		//for(i=0;i<16455;i++)printf("i %d symbol %d bits %d\n",i,table[i],bits[i]);
}

int show_bits(int from_bit, int n_bits){
	char *buffer;
	int i,val;
	buffer=(char *)malloc(n_bits*sizeof(char)+1);

	for(i=0;i< n_bits;i++){
	buffer[i]= bin_string[from_bit+i];
	}
	buffer[i]='\0';

	val=bin2dec(buffer);
	free(buffer);
	return val;

}

void init_buffer(){
		unsigned char buffer;
	char string_buffer[9];
	
	int i,k;
	int file_length=0;
	int string_length=0;
	FILE *fp;

	fp=fopen("coded.txt","rb");


		while(fread(&buffer,sizeof(char),1,fp))
		{
			file_length++;
		}

		fclose(fp);

		fp=fopen("coded.txt","rb");

				bin_string=(char *)malloc(1);
				strcpy(bin_string,"\0");

				for(i=1;i<=file_length;i++){

				bin_string=(char *)realloc(bin_string,i*8*sizeof(char)+1);
				fread(&buffer,sizeof(char),1,fp);

				itoa ((int)buffer,string_buffer,2);
				k=8-strlen(string_buffer);
				while(k!=0){
					strcat(bin_string,"0");
					k--;
				}
				strcat(bin_string,string_buffer); 
			}
				fclose(fp);
				//printf("%s",bin_string);
}
void zz_vector_init(){
	int i,j,val,k,n;
	
	i=0;
	j=0;
	k=0;
	n=0;

	rle=(int*)malloc(sizeof(int));
	while(bin_string[i]!='\0')
	{	
	val=show_bits(i,16);
	rle[j]=table[val];
	k=bits[val];
	i=k+i;
	j++;
	rle=(int*)realloc(rle,(j+1)*sizeof(int));
	}
	rle_length=j;

}


void zz_vector(int n_block){
		int i,j,val,k,n;


	
	i=0;
	j=0;
	k=0;
	n=0;

	for(val=0; val<rle_length; val++){
		if(rle[val]==256) n++;
		if(n==n_block) break;
	}
	if(val!=0)val++;

		for(n=0,i=val;rle[i]!=256;i++,n++){

			if(n%2==0) {
				for(j=0;j<rle[i];j++){
					zz[k]=0;
					k++; 
				}
					
			}
			else {

			zz[k]=rle[i];
			k++;
			}
		}

		for(i=k;i<N*N;i++){
			zz[k]=0;
			k++;
		}
		
		//free(rle);
//for(i=0;i<64;i++)printf("%d ",zz[i]);
}

void dequantize(int n_block){


		int i,j;


	zz_vector(n_block);

				 for(i=0;i<N;i++)
		 for(j=0;j<N;j++){
			 quant[zigzag[N*i+j][0]][zigzag[N*i+j][1]] = zz[N*i+j];
			}


	if(n_block<5400){
			 for(i=0;i<N;i++)
		 for(j=0;j<N;j++){
			output[i][j]=quant[i][j]*Quant_Luminance[i][j];
			}
	}
	else{
			 for(i=0;i<N;i++)
		 for(j=0;j<N;j++){
			output[i][j]=quant[i][j]*Quant_Chrominance[i][j];
			}
	}
}

void init_dctt(){
	int i,j;
	float DCT[N][N];
	float pi=PI;
		       for ( j = 0 ; j < 8 ; j++ ) {
        DCT[ 0 ][ j ] = 1.0 / sqrt( (float) 8 );
		DCTt[ j ][ 0 ] = DCT[ 0 ][ j ];
    }
    for ( i = 1 ; i < 8 ; i++ ) {
        for ( j = 0 ; j < 8 ; j++ ) {
            DCT[ i ][ j ] = sqrt( 2.0 / 8 ) *
                          cos( pi * ( 2 * j + 1 ) * i / ( 2.0 * 8 ) );
			DCTt[ j ][ i ] = DCT[ i ][ j ];
        }
}

}

void dctt(int n_block){
	double sum,temp[8][8];
	int i,j,k;

		dequantize(n_block);

		

 // οριζόντιο DCT
 for(i=0;i<8;i++) 
  for(j=0;j<8;j++) {
   sum=0;
   for(k=0;k<8;k++) sum += DCTt[i][k]*output[k][j];
   temp[i][j] = sum;
   }
 
 // κάθετο DCT
 for(i=0;i<8;i++)
  for(j=0;j<8;j++) {
   sum=0;
   for(k=0;k<8;k++) sum +=temp[i][k]* DCTt[j][k];
   block[i][j] = (unsigned char) ROUND(abs(sum));
   }

}