#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include <libspe2.h>



#define SIZE    8
#define SPE_count 5
#define MAX_SPE_count 8
#define MAX_SIZE 8


float A[MAX_SIZE][MAX_SIZE]  __attribute__((aligned(16)));

float B[MAX_SIZE][MAX_SIZE]  __attribute__((aligned(16)));

float out[MAX_SPE_count][MAX_SIZE][MAX_SIZE] __attribute__((aligned(16)));

float final_out[MAX_SIZE][MAX_SIZE];


typedef struct {

   unsigned long long  A;
   
   unsigned long long  B;

   unsigned long long  out;

   unsigned int        size;
   
   unsigned int id;
   
   unsigned int SPE_num;

   int    pad[7];

} abs_params_t;


typedef struct {

      spe_context_ptr_t   spe;

      abs_params_t        *abs_params;

  } thread_arg_t;



abs_params_t abs_params[SPE_count] __attribute__((aligned(16)));



void *run_spe(void * thread_arg)
{
	int ret;
	thread_arg_t *arg = (thread_arg_t *) thread_arg;
	unsigned int entry;
	spe_stop_info_t stop_info;
	entry = SPE_DEFAULT_ENTRY;

    ret = spe_context_run(arg->spe, &entry, 0, arg->abs_params, NULL, &stop_info);

      if (ret < 0) {

          perror("spe_context_run");

          return NULL;

      }

}


int main()

{
    int i,k,j,ret,num,start,end;
	for(i = 0 ; i < SIZE ; i++){
		for(j = 0 ; j < SIZE ; j++){
			A[i][j] = (i==j) ? 1 : 0;
			B[i][j] = i * SIZE + j;
			}}
	/*		
	for(i = 0 ; i < SIZE ; i++)
		for(j = 0 ; j < SIZE ; j++)
			printf("%d %d %f %f\n" , i , j , A[i][j] , B[i][j]);
		
	*/	
    spe_context_ptr_t spe[SPE_count];
	pthread_t thread[SPE_count];
	thread_arg_t arg[SPE_count];
    spe_program_handle_t *prog;
    prog = spe_image_open("spu/spu_vector");
    if (!prog) {

         perror("spe_image_open");

         exit(1);

     }
	for (i = 0; i < SPE_count; i++) {
		 spe[i] = spe_context_create(0, NULL);
          if (!spe[i]) {
              perror("spe_context_create");
              exit(1);
          }
          ret = spe_program_load(spe[i], prog);
          if (ret) {
              perror("spe_program_load");
              exit(1);
          }	
	}
	time_t start, stop;

    start = clock();
	for (i = 0; i < SPE_count; i++) {
		 abs_params[i].A = (unsigned long) A;
		 abs_params[i].B = (unsigned long) B;
		 abs_params[i].out = (unsigned long) &out[i];
		 abs_params[i].size = (unsigned) SIZE;
		 abs_params[i].id = (unsigned) i;
		 abs_params[i].SPE_num = (unsigned) SPE_count;
		 arg[i].spe = spe[i];
		 arg[i].abs_params = &abs_params[i];		 
		 ret = pthread_create(&thread[i], NULL, run_spe, &arg[i]);
          if (ret) {
              perror("pthread_create");
              exit(1);
          }
	}
	
	for (i = 0; i < SPE_count; i++) {
		pthread_join(thread[i], NULL);
		ret = spe_context_destroy(spe[i]);
         if (ret) {
             perror("spe_context_destroy");
             exit(1);
         }
	}
	stop = clock();


     ret = spe_image_close(prog);

     if (ret) {

         perror("spe_image_close");

         exit(1);

     }
	 /*
	 for(i = 0 ; i < SPE_count ; i++)
		 for(j = 0 ; j < SIZE ; j++)
			 for( k = 0 ; k < SIZE ; k++)
				 printf("%d %d %d %f\n", i , j , k , out[i][j][k]);
	*/
	num = SPE_count;
	if(SPE_count > SIZE)
		num = SIZE;
	for(i = 0 ; i < num ; i++){
		end = i == num-1 ? SIZE : (i+1) * ( SIZE/num ) ;
		start = i * ( SIZE/num );
		for(j = start ; j < end ; j++)
			for(k = 0 ; k < SIZE ; k++)
				final_out[j][k] = out[i][j][k];
		
	}
	
	for(i = 0 ; i < SIZE ; i++)
		 for(j = 0 ; j < SIZE ; j++)
			 printf("%d %d %f\n" , i , j , final_out[i][j]);
		 
		 
	printf("time = %g\n" ,(stop - start) / (double)(CLOCKS_PER_SEC / 1000));
	


     return 0;

 }
 
 