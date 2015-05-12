#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define PI 3.14159265358979323846
#define N 8
#define ROUND( a )      ( ( (a) < 0 ) ? (int) ( (a) - 0.5 ) : \
                                                  (int) ( (a) + 0.5 ) )

void init_block(int n_block);//αρχικοποιει το n_block 8x8 στον πίνακα input

void dct(int n_block);//μετασχηματίζει το n_block και το αποθηκεύει στον πίνακα output

void init_dct();//αρχικοποιεί τον πίνακα DCT

void print_dct(int n_block);//τυπώνει το n_block μετά το DCT με ακαίρεους συντελεστες

void print_block(int n_block);//τυπώνει το n_block

void print_quant(int n_block);//typwnei ton kvantopoihmeno pinaka

void print_runlength(int block_n);//typwnei to dianysma run length

void save_pic();//swzei to prwto kare tou video se .yuv

void quantize(int n_block);//kvantopoiei to n_block

void coding(int n_block);//zig_zag kai run length coding gia to n_block,apothikeyei to apotelesma sto dianisma run_length

void huffman();	//arxikopoiei to codebook meso algorithmou huffman

void encode(); //ginetai h kwdikopoihsh kai h apothikeysh se arxeio



//global metavlites 

unsigned char input[N][N]; //to block apo to yuv arxeio

double DCT[N][N],output[N][N]; //pinakas DCT kai pinakas tou block meta to dct

int quant[N][N];			//block meta apo kvantikopoihsh




struct r_l{	//dynamic vector structure
int *a;
int size;};


struct huffman_tree //huffman tree
        {
            int symbol;
            int freq;
            struct huffman_tree *parent,*left,*right;
        };

typedef struct huffman_tree *NODE;

struct dict         // dictionary structure
        {
			int symbol;
            char code[20];
        };


NODE newnode()  {   // returns a node of huffman tree structure type   
            NODE t;
            t = (NODE)malloc(sizeof(struct huffman_tree));
            if(t == NULL)
                return NULL;
            t -> symbol = 0;
            t -> freq = 0;
            t -> parent = NULL;
            t -> left = NULL;
            t -> right = NULL;
                return t;
        }




		
		//pinakes kvantikopoihshs
		
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

//voithitikos pinakas zig zag
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


		//huffman codebook
struct dict codebook[149];


		//run_length vector
struct r_l run_length;













int main(){	

	init_dct();
	huffman();
		
		//debug gia to block 82 pou parousiazei ypsilh paramorfwsh
		//print_block(82);
		//print_dct(82);
		//print_quant(82);

	encode();
}










void init_block(int n_block){
		FILE *fp;
  unsigned char *pointer;
  int n,i,j,offset;

  

  offset=0;
  pointer=(unsigned char *) malloc(1);

    fp = fopen("src14_ref__720x480_420.yuv", "rb");
 
  	if (fp == NULL){printf("file not found\n");}


	n=n_block/90; 
	offset=n*5040;
  
	 for(i=0;i<8;i++){

		fseek(fp, i*720 + n_block*8+offset, SEEK_SET);

		 for(j=0;j<8;j++){

			fread(pointer,1,1,fp);
			input[i][j]=*pointer;
		}	

	 }
		   fclose(fp);
		   free(pointer);

}

void dct(int n_block){
	double sum,temp[8][8];
	int i,j,k;

		init_block(n_block);
		

 // οριζόντιο DCT
 for(i=0;i<8;i++) 
  for(j=0;j<8;j++) {
   sum=0;
   for(k=0;k<8;k++) sum += DCT[i][k] * input[k][j];
   temp[i][j] = sum;
   }
 
 // κάθετο DCT
 for(i=0;i<8;i++)
  for(j=0;j<8;j++) {
   sum=0;
   for(k=0;k<8;k++) sum += temp[i][k] * DCT[j][k];
   output[i][j] = sum;
   }

}

void init_dct(){
	int i,j;
	double pi=PI;
		       for ( j = 0 ; j < 8 ; j++ ) {
        DCT[ 0 ][ j ] = 1.0 / sqrt( (float) 8 );
    }
    for ( i = 1 ; i < 8 ; i++ ) {
        for ( j = 0 ; j < 8 ; j++ ) {
            DCT[ i ][ j ] = sqrt( 2.0 / 8 ) *
                          cos( pi * ( 2 * j + 1 ) * i / ( 2.0 * 8 ) );
        }
}

}

void quantize(int n_block){
	int i,j;
	dct(n_block);


	if(n_block<5400){
			 for(i=0;i<N;i++)
		 for(j=0;j<N;j++){
			quant[i][j]=output[i][j]/Quant_Luminance[i][j];
			}
	}
	else{
			 for(i=0;i<N;i++)
		 for(j=0;j<N;j++){
			quant[i][j]=output[i][j]/Quant_Chrominance[i][j];
			}
	}
}

void coding(int n_block){
	int i,j,size=0,zeros=0;
	int zz[N*N];

	quantize(n_block);
				 for(i=0;i<N;i++)
		 for(j=0;j<N;j++){
			zz[N*i+j] = quant[zigzag[N*i+j][0]][zigzag[N*i+j][1]];
			}
		 j=0;

		 for(i=0;i<N*N;i++){
			 if(zz[i]==0){
				 zeros++;}
			 else{
				 size=size+2;
				 run_length.a=(int *)realloc(run_length.a,size*sizeof(int));
				 
				 run_length.a[j]=zeros;
				 run_length.a[j+1]=zz[i];
				
				 j=j+2;
				 zeros=0;
			 }
			 if(i==N*N-1 && zz[i]==0){//EOB
				  size=size++;
				  run_length.a=(int *)realloc(run_length.a,size*sizeof(int));

				  run_length.a[j]=256;
				  }
			 run_length.size=j+1;
		}

 
 
}

void huffman(){

		int i,j,k,node_num2,node_num=0;
	int flag=1;
	

	char code[100];

	NODE *list,temp,root;

	int symbols[512];//symbol[0]=-255 symbol[0]=-254 ... symbol[512]=EOB

	for(k=0;k<512;k++){symbols[k]=0;}

	for(i=0;i<8100;i++){
	coding(i);
	for(j=0;j<run_length.size;j++)
	for(k=0;k<512;k++){
		if(k-255==run_length.a[j])
			symbols[k]++;
	}
	}
	

	for(k=0;k<512;k++){
		if(symbols[k]!=0)node_num++;
	}
	

		list = (NODE *)malloc(node_num * sizeof(NODE));

		for(i=0,j=0;i<512;i++){
			if(symbols[i]!=0){

			list[j] = (NODE)malloc(sizeof(struct huffman_tree));

			list[j]->symbol=i-255;
			list[j]->freq=symbols[i];
			list[j]->left=NULL;
			list[j]->right=NULL;
			j++;
			}
		}

		  for(i = 0; i < 512; i++)        // sort
        for(j = node_num-1; j > i; j--)
            if(list[j]->freq < list[j-1]->freq)
            {
                temp = list[j-1];
                list[j-1] = list[j];
                list[j] = temp;
            }
			node_num;
		node_num2=node_num-1;i=0;
		
	for(i = 0; i < node_num2; i++)
    {
		//for(k = 0; k < node_num; k++) printf("%d freq %d symbol %d \n",k,list[k]->freq,list[k]->symbol);

        temp = newnode();
        if(temp == NULL)
            exit(-1);
		temp->symbol=123456789;
        temp->freq = list[0]-> freq + list[1] -> freq;
        temp -> left = list[0];
        temp -> right = list[1];
        list[0] -> parent = temp;
        list[1] -> parent = temp;
        
		list[0]=temp;
		for(j = 1; j < node_num-1; j++){
			list[j]=list[j+1];}
		for(j=0;j<node_num-2;j++){
			if(list[j]->freq >list[j+1]->freq){
				temp=list[j];
				list[j]=list[j+1];
				list[j+1]=temp;
			}
			
    }node_num--;

	}

		i=0;

	root = (NODE)malloc(sizeof(struct huffman_tree));
	root=list[0];
	list[0]=temp;

	 strcpy (code,"\0");

	while(temp->symbol!=666){

		if(temp->left!=NULL && temp->left->symbol!=666){
		temp=temp->left;
		strcat( code, "0" );
		}
		else if(temp->right!=NULL && temp->right->symbol!=666){
		temp=temp->right;
		strcat( code, "1" );
		}
		else
		{
			if(temp->symbol!=123456789){
				codebook[i].symbol=temp->symbol;
				strcpy (codebook[i].code,code);
				i++;
			}
			temp->symbol=666;
			 temp=root;
			strcpy (code,"\0");
			
		}
		
	}
	free(root);

}

void encode(){
		FILE *fp;
	int i,j,k,string_length=0;
	unsigned char bit_fld=0;
	char *code_string;
	fp=fopen("codebook.txt","w");


	

		for(j=0;j<149;j++)fprintf(fp,"%d %s \n",codebook[j].symbol,codebook[j].code);
		fclose(fp);

	fp=fopen("coded.txt","wb");

		code_string=(char*)malloc(1);
		strcpy(code_string,"\0");

	for(i=0;i<8100;i++){
		coding(i);
		for(j=0;j<run_length.size;j++){

//to "adynamo" shmeio tou kwdika apothikeyei se string to kwdikopoihmeno arxeio

		for(k=0;k<149;k++){
			if(codebook[k].symbol==run_length.a[j]){

				string_length=string_length+strlen(codebook[k].code);
				code_string=(char *)realloc(code_string,string_length*sizeof(char)+1);

				strcat(code_string,codebook[k].code); 
				break;
			}
			
		}
	}
			//apothikeysh se arxeio
	}
	for(i=0;code_string[i]!='\0';i++){
		k=i%8;
		if(code_string[i]=='1'){
		bit_fld |= (1 << 7-k);
		}
		if(k==7){
			//printf("%d ",bit_fld);
				fputc(bit_fld,fp);
				bit_fld=0;
		}

	}
	while(k<8)
	{
		k++;
		bit_fld &= ~(1 << 7-k);
	}
	fputc(bit_fld,fp);

	printf("the file is compressed!\n");
	fclose(fp);
}

void print_block(int n_block){
	int i,j;

	init_block(n_block);

		for(i=0;i<8;i++){
		 for(j=0;j<8;j++){
			 printf("%d  ",input[i][j]);
		 }
		  printf("\n");
		 }
}

void print_dct(int n_block){
	int i,j;

	dct(n_block);

		for(i=0;i<8;i++){
		 for(j=0;j<8;j++){
			 printf("%d  ",(int)output[i][j]);
		 }
		  printf("\n");
		 }
}

void print_quant(int n_block){
	int i,j;

	quantize(n_block);

		for(i=0;i<8;i++){
		 for(j=0;j<8;j++){
			 printf("%d  ",quant[i][j]);
		 }
		  printf("\n");
		 }
}

void save_pic(){
	int i;
	FILE *fp,*f;
    unsigned char *pointer;

	pointer=(unsigned char *) malloc(1);

	fp = fopen("src14_ref__720x480_420.yuv", "rb");
	f = fopen("src14_pic.yuv", "wb");

	for(i=0;i<518400;i++){

		fread(pointer,1,1,fp);
		fwrite(pointer,1,1,f);

	}

	fclose(fp);
	fclose(f);
		   free(pointer);

}

void print_runlength(int block_n){
	int i;
	coding(block_n);
	for(i=0;i<run_length.size;i++) printf("%d  \n",run_length.a[i]);
}

