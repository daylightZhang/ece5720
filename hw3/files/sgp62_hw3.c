//Stefen Pegels, sgp62
//gcc -std=gnu99 -o sgp62_hw3 sgp62_hw3.c -fopenmp -lm
//To run full matrix vs triangular, switch commented function at line 92 and 109 to triAcc or bodyAcc
//Note that striding up to 1024 bodies takes many minutes to execute
//Also note that momentum check code is included but commented out
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define SOFT 1e-2f
#define MIN_BODY 2 /* the number of bodies                  */
#define MAX_BODY 1024
#define MAX_X 100  /* the positions (x_i,y_i) are random    */
#define MAX_Y 100 
#define MIN_THREADS 1 //Mininum number of threads
#define MAX_THREADS 8 //Maximum number of threads
#define MIN_X 1     /* between MIN and MAX position          */
#define MIN_Y 1  
#define MAX_M 10    /* the weights w_i are random            */ //Changed to Mass
#define MIN_M 1     /* between MIN and MAX wieght         */ //Changed to Mass
#define MAX_V 5    /* the velocities (vx_i,vy_i) are random */
#define MIN_V 0     /* between MIN and MAX velocity          */
#define G 0.05       /* gravitational constant                */
#define DIM 2       /* 2 or 3 dimensions                     */

// position (x,y), velocity (vx,vy), acceleration (ax,ay), weight w(SWITCHED TO MASS M)
typedef struct { double x, y, vx, vy, ax, ay, m; } Body;

// kinetic and potential energy
typedef struct { double ke, pe; } Energy;

typedef struct { double flx, fly; } LocalF; //Struct for local force sums for triangular approach

void bodyAcc(Body *r, int n, int num_thrs);      // computes  a
void triAcc(Body *r, int n, int num_thrs, LocalF *f);        //Computes a with symmetry matrix
void total_energy(Body *r, Energy *e, int n, int num_thrs); // kinetic and potential
void center_of_momentum(Body *r, int n, int num_thrs);      // center of momentum

int main(const int argc, const char** argv) {

  // record positions and velocities
  FILE *pvp = NULL;            
  pvp = fopen("pos_vel.csv", "w");

  // file to record energy (to check for correctness)
  FILE *tp = NULL;            
  tp = fopen("plot_nbody.csv", "w");

  //file for timing analysis
  FILE *ftp = NULL;            
  ftp = fopen("time_nbody.csv", "w");
  
  int i;
  int nBodies;

  const double dt = 0.01f; // time step
  const int nIters = 5000;  // simulation iterations
  double totalTime, avgTime;

  double *Bbuf = (double*)malloc(MAX_BODY*sizeof(Body));
  double *Ebuf = (double*)malloc(sizeof(Energy));
  double *Fbuf = (double*)malloc(MAX_BODY*sizeof(LocalF));
  Body *r = (Body*) Bbuf;
  Energy *e = (Energy*)Ebuf;
  LocalF *f = (LocalF*)Fbuf;




//Outer loop over varying amounts of nBodies, doubling each time
  for(nBodies = MIN_BODY; nBodies <= MAX_BODY; nBodies += nBodies){
    fprintf(ftp, "%d, ", nBodies);
    //Inner loop over varying amounts of threads, doubling each time
    for(int num_thrs = MIN_THREADS; num_thrs <= MAX_THREADS; num_thrs += num_thrs){
        /******************** (0) initialize N-body ******************/
      for(int k = 0; k < nBodies; k++){
        r[k].x = (((double) rand()) / RAND_MAX) * MAX_X + MIN_X;
        r[k].y = (((double) rand()) / RAND_MAX) * MAX_Y + MIN_Y;
        r[k].vx = (((double) rand()) / RAND_MAX) * MAX_V + MIN_V;
        r[k].vy = (((double) rand()) / RAND_MAX) * MAX_V + MIN_V;
        r[k].ax = r[k].ay = 0;
        r[k].m = (((double) rand()) / RAND_MAX) * MAX_M + MIN_M;
      }

    /******************** (1) Get center of momentum *******************/
      center_of_momentum(r, nBodies, num_thrs);

    /******************** (2) Get total energy *********************/
      total_energy(r, e, nBodies, num_thrs);

    /******************** (3) Initial acceleration *****************/
      bodyAcc(r, nBodies, num_thrs);  //SELECT ONE
      //triAcc(r, nBodies, num_thrs, f);         

      totalTime = omp_get_wtime();
      /******************** Main loop over iterations **************/
      for (int iter = 1; iter <= nIters; iter++) {
        //fprintf(pvp, "%d, ", iter);
        #pragma omp parallel for private(i) shared(r) num_threads(num_thrs)
        for(i = 0; i < nBodies; i++){
          // "half kick"
          r[i].vx += (dt/2) * r[i].ax;
          r[i].vy += (dt/2) * r[i].ay;
          // "drift"
          r[i].x += dt*r[i].vx;
          r[i].y += dt*r[i].vy;
        }
        //Update accelerations based on new positions
        bodyAcc(r, nBodies, num_thrs);
        //triAcc(r, nBodies, num_thrs, f);  //SELECT ONE
        #pragma omp parallel for private(i) shared(r) num_threads(num_thrs)
        for(i=0; i < nBodies; i++){
          // "half kick"
          r[i].vx += (dt/2) * r[i].ax;
          r[i].vy += (dt/2) * r[i].ay;
        }

        center_of_momentum(r, nBodies, num_thrs);
        
          //Momentum check
        // double momentumx, momentumy;
        // for(int j = 0; j < nBodies; j++){
        //   momentumx += r[j].m * r[j].vx;
        //   momentumy += r[j].m * r[j].vy;
        // }
        // if(iter %1000 == 0){
        //   printf("X: %1.3e\n",momentumx);
        //   printf("Y: %1.3e\n",momentumy);
        // }

        
        //record the position and velocity
        if(iter %1000 == 0){
          for(int j = 0; j < nBodies; j++){
            fprintf(pvp, "%10.3e, %10.3e, %10.3e, %10.3e, ",r[j].x, r[j].y, r[j].vx, r[j].vy);
          }
        }

        

        //sanity check
        total_energy(r, e, nBodies, num_thrs);
        if (iter%10 == 0){
          fprintf(tp,"%4d, %10.3e, %10.3e, %10.3e\n",
                iter,(*e).pe,(*e).ke, (*e).pe-(*e).ke);
        }
      }
      totalTime = omp_get_wtime() - totalTime;
      fprintf(ftp, "%5.3e, ", totalTime);
    }
    fprintf(ftp, "\n");
  }
  
  

  free(Bbuf); free(Ebuf);
  fclose(tp); fclose(pvp); fclose(ftp);
  
    FILE *fp = NULL;          // script for gnuplot which generates
                              // eps graph of energy
/* Create one way pipe line with call to popen() */
  if (( fp = popen("gnuplot plot_nbody.gp", "w")) == NULL)
  {
    perror("popen");
    exit(1);
  }
/* Close the pipe */
  pclose(fp); 

    FILE *ttp = NULL;          // script for gnuplot which generates
                              // eps graph of timings
/* Create one way pipe line with call to popen() */
  if (( ttp = popen("gnuplot plot_bodytime.gp", "w")) == NULL)
  {
    perror("popen");
    exit(1);
  }
/* Close the pipe */
  pclose(ttp); 

  return(0);

}

void center_of_momentum(Body *r, int n, int num_thrs){
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
  #pragma omp parallel for private(i) shared(r) num_threads(num_thrs)
  for(i=0; i < n; i++){
    r[i].vx -= Vcx;
    r[i].vy -= Vcy;
  }
}

void total_energy(Body *r, Energy *e, int n, int num_thrs){
// kinetic energy, (*e).ke = m*v^2/2;
// potential energy : (*e).pe = -\sum_{1\leq i < j \leq N}G*m_i*m_j/||r_j-r_i||
  int i,j; //Big jump in kinetic energy at one point for nBodies > 50, not sure why
  
  (*e).ke = 0;
  (*e).pe = 0;
  #pragma omp parallel for private(i,j) shared(e,r) num_threads(num_thrs)
  for(i=0; i < n; i++){
    double pe, ke, dx, dy,sqt;
    pe = ke = 0;
    ke = 0.5 * r[i].m * ((r[i].vx * r[i].vx) + (r[i].vy * r[i].vy));
    for(j = 0; j < n; j++){
      if(i != j){
        dx = r[i].x - r[j].x;
        dy = r[i].y - r[j].y;
        sqt = sqrt(dx*dx+dy*dy);
        if (sqt == 0) sqt += 1e-32;
        pe -= 0.5 * G * (r[i].m * r[j].m) / sqt;
      }
    }
    #pragma omp critical
    {
      (*e).ke += ke;
      (*e).pe += pe;
    } 
  }
}

void bodyAcc(Body *r, int n, int num_thrs) {
// F = G*sum_{i,j} m_i*m_j*(r_j-r_i)/||r_j-r_i||^3
// F = m*a thus a = F/m
  int i,j;
  double dx, dy, d, d3;
  #pragma omp parallel for private(i,j,dx,dy,d,d3) shared(r) num_threads(num_thrs)
  for(i = 0; i < n; i++){
    r[i].ax = r[i].ay = 0;
    for(j = 0; j < n; j++){
      if (j != i){
        dx = r[i].x - r[j].x;
        dy = r[i].y - r[j].y;
        d = sqrt(dx*dx + dy*dy);
        d3 = d*d*d;
        if(d3 == 0) d3 += 1e-32;
        if(dx == 0) dx += 1e-32;
        if(dy == 0) dy += 1e-32;
        r[i].ax -= (((G*r[i].m * r[j].m) / d3) * (dx)) / r[i].m;
        r[i].ay -= (((G*r[i].m * r[j].m) / d3) * (dy)) / r[i].m;
      }
    }
  }
}

void triAcc(Body *r, int n, int num_thrs, LocalF *f){
  //Computes force matrix, taking advantage of upper triangular symmetry
  int i,j;
  double dx, dy, d, d3;
  #pragma omp parallel for private(i) shared(f) num_threads(num_thrs)
  for(i=0; i < n; i++){
    f[i].flx = f[i].fly = 0;
  }
  #pragma omp parallel for private(i,j,dx,dy,d,d3) shared(r,f) num_threads(num_thrs)
  for(i = 0; i < n; i++){
    r[i].ax = r[i].ay = 0;
    for(j = i+1; j < n; j++){
      dx = r[i].x - r[j].x;
      dy = r[i].y - r[j].y;
      d = sqrt(dx*dx + dy*dy);
      d3 = d*d*d;
      if(d3 == 0) d3 += 1e-32;
      if(dx == 0) dx += 1e-32;
      if(dy == 0) dy += 1e-32;
      //Local mass sums
      f[i].flx += (((G*r[i].m * r[j].m) / d3) * (dx));
      f[j].flx -= (((G*r[i].m * r[j].m) / d3) * (dx));
      f[i].fly += (((G*r[i].m * r[j].m) / d3) * (dy));
      f[j].fly -= (((G*r[i].m * r[j].m) / d3) * (dy));
    }
  }
  #pragma omp parallel for private(i) shared(r,f) num_threads(num_thrs)
  for(i = 0; i < n; i++){
    r[i].ax -= f[i].flx / r[i].m;
    r[i].ay -= f[i].fly / r[i].m;
  }  
}
