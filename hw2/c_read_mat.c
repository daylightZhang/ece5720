/* from matlab use                                                    *
 *  save('MyMatrix.txt', 'A', '-ascii', '-double', '-tabs')           *
 *  and then use the code below to input the matrix from MyMatrix.txt */

#include<stdio.h>
#include<stdlib.h>

void printData(int N, int M, float** A){
  if (N <= 16){
    for (int x=0; x<N; x++){
      printf("| ");
      for(int y=0; y<M; y++)
        printf("% 5.2e ", A[x][y]);
      printf("|\n");
      }
    }
  else{
    printf("\nMatrix and vector too large to print out.\n");
  }
}

void data_A_b(int N, float** A, float** b){
  int i, j;

  // for (i=0; i<N; i++){
  //   for (j=0; j<N; j++)
  //     A[i][j] = 1.0/(1.0*i + 1.0*j + 1.0);

  //   A[i][i] = A[i][i] + 1.0;
  // }

  for (i=0; i<N; i++){
    for (j=0; j<N; j++)
      A[i][j] = 1.0;

  }



/* create b, either as columns of the identity matrix, or */
/* when Nrhs = 1, assume x all 1s and set b = A*x         */

  // for(int i = 0; i < N; i++){
  //   for(int k = 0; k < N; k++){
  //     b[i][0] += A[i][k];
  //   }
  // }

  for(int i = 0; i < N; i++){
    for(int k = 0; k < N; k++){
      b[i][k] = (i == k)? 1.0 : 0.0;
    }
  }


  printf("here\n");
  fflush(stdout);

}

int main()
{
  int i,j,N;
  N = 16;

  float **A = (float **)malloc(N*sizeof(float*));
    for (int q=0; q < N; q++)
      A[q] = (float*)malloc(N*sizeof(float));

  float** b = (float**) malloc(N*sizeof(float*));
    for (int q=0; q < N; q++)
      b[q] = (float*)malloc(N*sizeof(float));



  data_A_b(N,A,b);
  printData(N,N,b);
}


