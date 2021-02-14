
# Set the output file type to eps
set terminal postscript eps enhanced color solid colortext 18

# for *.pdf
# set terminal pdf enhanced color solid

# Set the output file name
set output 'mm_rbyc_plots.eps'

# Now plot the data with lines and points
# change titles and/or labels as needed
set title "row-by-column mm mult"
set title font "Helvetica,18"
set xlabel "log of matrix size"
set ylabel "time"
set grid

# put legend in the upper left corner
set key left top

plot "rbyc.csv" u 1:2 w lp lw 3 title "rbyc"

# some explanations of commands
# ps - point size, pt - point type (circle, square, etc.)
# pointinterval (pi 2) - plotevery second point
# w lp - with linepoints, lw - line width, lc - line color
# u - using

