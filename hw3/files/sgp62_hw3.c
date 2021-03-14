//Stefen Pegels, sgp62
//gcc -std=gnu99 -o sgp62_hw3 sgp62_hw3.c -fopenmp -lm
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define SOFT 1e-2f
#define NUM_BODY 50 /* the number of bodies                  */
#define MAX_X 90    /* the positions (x_i,y_i) are random    */
#define MAX_Y 50 
#define MIN_X 1     /* between MIN and MAX position          */
#define MIN_Y 1  
#define MAX_W 60    /* the weights w_i are random            */
#define MIN_W 1     /* between MIN and MAX wieght            */
#define MAX_V 20    /* the velocities (vx_i,vy_i) are random */
#define MIN_V 0     /* between MIN and MAX velocity          */
#define G 0.1       /* gravitational constant                */
#define DIM 2       /* 2 or 3 dimensions                     */

// position (x,y), velocity (vx,vy), acceleration (ax,ay), weight w
typedef struct { double x, y, vx, vy, ax, ay, m; } Body;

// kinetic and potential energy
typedef struct { double ke, pe; } Energy;

void bodyAcc(Body *r, double dt, int n);      // computes  a
void total_energy(Body *r, Energy *e, int n); // kinetic and potential
void center_of_momentum(Body *r, int n);      // center of momentum

int main(const int argc, const char** argv) {

  // record positions and velocities
  FILE *pvp = NULL;            
  pvp = fopen("pos_vel.txt", "w");

  // file to record energy (to check for correctness)
  FILE *tp = NULL;            
  tp = fopen("plot_nbody.csv", "w");
  
  int i;
  int nBodies;
  nBodies = atoi(argv[1]);

  const double dt = 0.01f; // time step
  const int nIters = 1000;  // simulation iterations
  double totalTime, avgTime;

/******************** (0) initialize N-body ******************/
  double *Bbuf = (double*)malloc(nBodies*sizeof(Body));
  double *Ebuf = (double*)malloc(sizeof(Energy));
  Body *r = (Body*) Bbuf;
  Energy *e = (Energy*)Ebuf;

/******************** (1) Get center of momentum *******************/
  center_of_momentum(r, nBodies);

/******************** (2) Get total energy *********************/
  total_energy(r, e, nBodies);

/******************** (3) Initial acceleration *****************/
  bodyAcc(r, dt, nBodies);            

  totalTime = omp_get_wtime();

/******************** Main loop over iterations **************/
  for (int iter = 1; iter <= nIters; iter++) {
// "half kick"
    
// "drift"

// update acceleration

// "half kick"

// record the position and velocity

// sanity check
    total_energy(r, e, nBodies);
    if (iter%10 == 0){
      fprintf(tp,"%4d %10.3e %10.3e %10.3e\n",
             iter,(*e).pe,(*e).ke, (*e).pe-(*e).ke);
    }
  }
  
  totalTime = omp_get_wtime() - totalTime;

  free(Bbuf); free(Ebuf);
  fclose(tp); fclose(pvp);
  
    FILE *fp = NULL;          // script for gnuplot which generates
                              // eps graph of timings
/* Create one way pipe line with call to popen() */
  if (( fp = popen("gnuplot plot_nbody.gp", "w")) == NULL)
  {
    perror("popen");
    exit(1);
  }
/* Close the pipe */
  pclose(fp); 

/* on Mac only */
  FILE *fpo = NULL;
  if (( fpo = popen("open plot_nbody.eps", "w")) == NULL)
  {
    perror("popen");
    exit(1);
  }
  pclose(fpo);

  return(0);

}

void center_of_momentum(Body *r, int n){
/*
 velocity v' in the center-of-momentum reference frame is
 v' = v - V_C where V_C = \sum_i(m_iv_i)/sum_i(m_i)
 (1) the total momentum in the center-of-momentum system venishes
 \sum_i(m_iv'_i) = \sum_i(m_i(v-V_C)) =
 \sum_i(m_iv_i) - \sum_i(m_i(\sum_j(m_jv_j)/(\sum_jm_j)) = 0
 (2) center of the mass C = \sum_i(m_ir_i)/\sum_(m_i) move with
 constant velocity
 https://en.wikipedia.org/wiki/N-body_problem
 https://en.wikipedia.org/wiki/Center-of-momentum_frame
*/
}

void total_energy(Body *r, Energy *e, int n){
// kinetic energy, (*e).ke = m*v^2/2;
// potential energy : (*e).pe = -\sum_{1\leq i < j \leq N}G*m_i*m_j/||r_j-r_i||
}

void bodyAcc(Body *r, double dt, int n) {
// F = G*sum_{i,j} m_i*m_j*(r_j-r_i)/||r_j-r_i||^3
// F = m*a thus a = F/m
  int i,j;
  double dx, dy, d, d3;
  #pragma omp parallel for private(i,j,dx,dy,d,d3) shared(r)
  for(i = 0; i < n; i++){
    for(j = 0; j < n; j++){
      if (j != i){
        dx = r[i].x - r[j].x;
        dy = r[i].y - r[j].y;
        d = sqrt(dx*dx + dy*dy); // Maybe alpha max beta min algorithm?
        d3 = d*d*d;
        r[i].ax -= ((G*r[i].m * r[j].m) / (d3 * (r[i].x-r[j].x))) / r[i].m;
        r[i].ay -= ((G*r[i].m * r[j].m) / (d3 * (r[i].y-r[j].y))) / r[i].m;
      }
    }
  }
}
