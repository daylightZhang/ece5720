# Set the output file type
set terminal postscript eps enhanced color solid colortext 18
# Set the output file name
set output 'fixed_graph.eps'

# Now plot the data with lines and points
set title "NxN Tiled Matrix Multiply for Varying Block Sizes"
set title font "Helvetica,18"
set xlabel "2^N Block Size"
set ylabel "time in msec"
set grid
set logscale y 10
# put legend in the upper left corner
set key left top
N = 2;
plot for [i=2:N] "fixedN.csv" u (column(1)):((column(i))) w lp\
     lw 3 title "s-".(i)