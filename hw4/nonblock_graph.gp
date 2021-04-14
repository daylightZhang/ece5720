# 
#Set the output file type
set terminal postscript eps enhanced color solid colortext 18
# set terminal pdf
# set terminal pdf enhanced color solid

# Set the output file name
set output 'nonblock_time.eps'

# Now plot the data with lines and points
set title "Nonblocking time per byte"
set title font "Helvetica,18"
set xlabel "Log10 number of bytes"
set ylabel "Time in Seconds"
set grid
set key left top

# set style line 1 ps 2 pt 2 pi 5
# ps - point size, pt - point type (circle, square, etc.)
# pointinterval (pi 2) - plotevery second point
# w lp - with linepoints, lw - line width, lc - line color
# u - using

N = 2;
plot for [i=2:N] "nonblock_time.csv" u (column(1)):(column(i)) w lp\
     lw 3 title "s-".(i)
