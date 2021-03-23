# Set the output file type
set terminal postscript eps enhanced color solid colortext 18
# Set the output file name
set output 'bodytime_plots.eps'

# Now plot the data with lines and points
set title "NBody Problem Time per num_bodies, for varying # Threads"
set title font "Helvetica,18"
set xlabel "nBodies"
set ylabel "time in seconds"
set grid
# put legend in the upper left corner
set key left top
N = 5
plot for [i=2:N] "time_nbody.csv" u (column(1)):((column(i))) w lp\
     lw 3 title "thrs-".(2**(i-2))

