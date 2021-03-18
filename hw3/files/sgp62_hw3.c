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
#define MAX_M 6    /* the weights w_i are random            */ //Changed to Mass, W/g = M
#define MIN_M 0.1     /* between MIN and MAX wieght         */ //Changed to Mass
#define MAX_V 20    /* the velocities (vx_i,vy_i) are random */
#define MIN_V 0     /* between MIN and MAX velocity          */
#define G 0.1       /* gravitational constant                */
#define DIM 2       /* 2 or 3 dimensions                     */

// position (x,y), velocity (vx,vy), acceleration (ax,ay), weight w(SWITCHED TO MASS M)
typedef struct { double x, y, vx, vy, ax, ay, m; } Body;

// kinetic and potential energy
typedef struct { double ke, pe; } Energy;

void bodyAcc(Body *r, double dt, int n);      // computes  a
void total_energy(Body *r, Energy *e, int n); // kinetic and potential
void center_of_momentum(Body *r, int n);      // center of momentum

int main(const int argc, const char** argv) {

  // record positions and velocities
  FILE *pvp = NULL;            
  pvp = fopen("pos_vel.csv", "w");

  // file to record energy (to check for correctness)
  FILE *tp = NULL;            
  tp = fopen("plot_nbody.csv", "w");
  
  int i;
  int nBodies;
  //nBodies = atoi(argv[1]);
  nBodies = NUM_BODY;

  const double dt = 0.01f; // time step
  const int nIters = 1000;  // simulation iterations
  double totalTime, avgTime;

  double *Bbuf = (double*)malloc(nBodies*sizeof(Body));
  double *Ebuf = (double*)malloc(sizeof(Energy));
  Body *r = (Body*) Bbuf;
  Energy *e = (Energy*)Ebuf;

/******************** (0) initialize N-body ******************/
  for(int k = 0; k < nBodies; k++){
    r[k].x = rand() % MAX_X + MIN_X;
    r[k].y = rand() % MAX_Y + MIN_Y;
    r[k].vx = (double) (rand() % MAX_V) + MIN_V;
    r[k].vy = (double) (rand() % MAX_V) + MIN_V;
    r[k].ax = r[k].ay = 0;
    r[k].m = (double) (rand() % MAX_M) + MIN_M;
  }

/******************** (1) Get center of momentum *******************/
  center_of_momentum(r, nBodies);

/******************** (2) Get total energy *********************/
  total_energy(r, e, nBodies);

/******************** (3) Initial acceleration *****************/
  bodyAcc(r, dt, nBodies);            

  totalTime = omp_get_wtime();

/******************** Main loop over iterations **************/
  for (int iter = 1; iter <= nIters; iter++) {
    fprintf(pvp, "%d, ", iter);
    #pragma omp parallel for private(i) shared(r, dt, nBodies)
    {
      for(i = 0; i < nBodies; i++){
        // "half kick"
        r[i].vx += (dt/2) * r[i].ax;
        r[i].vy += (dt/2) * r[i].ay;
        // "drift"
        r[i].x += dt*r[i].vx;
        r[i].y += dt*r[i].vy;
      }
    }
    //Update accelerations based on new positions
    bodyAcc(r, dt, nBodies);
    #pragma omp parallel for private(i) shared(r, dt, nBodies)
    {
      for(i=0; i < nBodies; i++){
        // "half kick"
        r[i].vx += (dt/2) * r[i].ax;
        r[i].vy += (dt/2) * r[i].ay;
      }
    }
    // record the position and velocity
    for(int j = 0; j < nBodies; j++){
      fprintf(pvp, "%10.3e, %10.3e, %10.3e, %10.3e, ",r[j].x, r[j].y, r[j].vx, r[j].vy);
    }
    

//Check reflections in box (reverse velocity) Optional
/*
if (x[0] < XMIN) reflect(XMIN, x, v, a);
if (x[0] > XMAX) reflect(XMAX, x, v, a);
if (x[1] < YMIN) reflect(YMIN, x, v, a);
if (x[1] > YMAX) reflect(YMAX, x, v, a);
*/

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

// /* on Mac only */
//   FILE *fpo = NULL;
//   if (( fpo = popen("open plot_nbody.eps", "w")) == NULL)
//   {
//     perror("popen");
//     exit(1);
//   }
//   pclose(fpo);

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
  double Vcx, Vcy, topsumx, topsumy, bottomsum;
  for(int i = 0; i < n; i++){
    topsumx += r[i].m * r[i].vx;
    topsumy += r[i].m * r[i].vy;
    bottomsum += r[i].m; 
  }
  Vcx = topsumx / bottomsum;
  Vcy = topsumy / bottomsum;
  int i;
  #pragma omp parallel for private(i) shared(r)
  {
    for(i=0; i < n; i++){
      r[i].vx -= Vcx;
      r[i].vy -= Vcy;
    }
  }
}

void total_energy(Body *r, Energy *e, int n){
// kinetic energy, (*e).ke = m*v^2/2;
// potential energy : (*e).pe = -\sum_{1\leq i < j \leq N}G*m_i*m_j/||r_j-r_i||
  int i,j;
  double pe, ke, dx, dy;
  #pragma omp parallel for private(i, j, pe, ke, dx, dy) shared(e, r) 
  {
    for(i=0; i < n; i++){
      ke = 0.5 * r[i].m * ((r[i].vx * r[i].vx) + (r[i].vy * r[i].vy));
      for(j = 0; j < n; j++){
        if(i != j){
          dx = r[i].x - r[j].x;
          dy = r[i].y - r[j].y;
          pe -= G * (r[i].m * r[j].m) / (sqrt(dx*dx+dy*dy));
        }
      }
      #pragma omp critical
      {
        e[0].ke += ke;
        e[0].pe += pe;
      }
    }
  }
}

void bodyAcc(Body *r, double dt, int n) {//Is dt actually necessary?
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
