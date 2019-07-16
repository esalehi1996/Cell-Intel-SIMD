#include <stdio.h>

#include <spu_intrinsics.h>

#include <spu_mfcio.h>

 

#define SIZE    8
#define MAX_SIZE 8

 

float A_spe[MAX_SIZE][MAX_SIZE]  __attribute__((aligned(16)));
  
float B_spe[MAX_SIZE][MAX_SIZE]  __attribute__((aligned(16)));

float out_spe[MAX_SIZE][MAX_SIZE] __attribute__((aligned(16)));

 

typedef struct {

   unsigned long long  A;
   
   unsigned long long  B;

   unsigned long long  out;

   unsigned int        size;
   
   unsigned int id;
   
   unsigned int SPE_num;

   int    pad[7];

} abs_params_t;


 abs_params_t abs_params __attribute__((aligned(16)));



 int main(unsigned long long spe, unsigned long long argp, unsigned long long envp)

 {

     int i,j,k,temp,end,start,num;
	 

     int tag = 1;

     /* DMA Transfer 1 : GET input/output parameters */

     spu_mfcdma64(&abs_params, mfc_ea2h(argp), mfc_ea2l(argp),

                  sizeof(abs_params_t), tag, MFC_GET_CMD);

     spu_writech(MFC_WrTagMask, 1 << tag);

     spu_mfcstat(MFC_TAG_UPDATE_ALL);


     /* DMA Transfer 2 : GET input data */

     spu_mfcdma64(A_spe, mfc_ea2h(abs_params.A), mfc_ea2l(abs_params.A),

                  abs_params.size * abs_params.size * sizeof(float), tag, MFC_GET_CMD);

     spu_writech(MFC_WrTagMask, 1 << tag);

     spu_mfcstat(MFC_TAG_UPDATE_ALL);
	 
	 spu_mfcdma64(B_spe, mfc_ea2h(abs_params.B), mfc_ea2l(abs_params.B),

                  abs_params.size * abs_params.size * sizeof(float), tag, MFC_GET_CMD);

     spu_writech(MFC_WrTagMask, 1 << tag);

     spu_mfcstat(MFC_TAG_UPDATE_ALL);

     /* Calculate absolute values */
	 
	 /*
	 for (i = 0; i < abs_params.size; i++) 
		 for (j = 0; j < abs_params.size; j++)
			 printf("%d %d %d %f %f\n", abs_params.id , i , j , A_spe[i][j] , B_spe[i][j]);
	*/
	if(abs_params.SPE_num > abs_params.size && abs_params.id >= abs_params.size)
		return 0;
	num = abs_params.SPE_num;
	if(abs_params.SPE_num > abs_params.size)
		num = abs_params.size;
		
	
	end = abs_params.id == num-1 ? abs_params.size : (abs_params.id+1) * ( abs_params.size/num ) ;
	start = abs_params.id * ( abs_params.size/num );
	
	for (i = start; i < end; i++) {
		 for (j = 0; j < abs_params.size; j++){
			 temp = 0;
			 for( k = 0 ; k < abs_params.size ; k++)
				 temp = temp + A_spe[i][k]*B_spe[k][j];
			 out_spe[i][j] = temp;
		 }
	}
	 
     /* DMA Transfer 3 : PUT output data */

     spu_mfcdma64(out_spe, mfc_ea2h(abs_params.out), mfc_ea2l(abs_params.out),

                  MAX_SIZE * MAX_SIZE * sizeof(float), tag, MFC_PUT_CMD);

     spu_writech(MFC_WrTagMask, 1 << tag);

     spu_mfcstat(MFC_TAG_UPDATE_ALL);

	

     return 0;

 }
 
 