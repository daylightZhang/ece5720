#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
  FILE *fp = NULL;
  if (( fp = popen("gnuplot nonblock_graph.gp", "w")) == NULL){
      perror("popen");
      exit(1);
  }
    
  /* Close the pipe */
  pclose(fp); 
  return(0);
  //0, 6.830e-04, 
}