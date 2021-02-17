
# Set the output file type
# for *.eps
set terminal postscript eps enhanced color solid colortext 18
# for *.pdf
# set terminal pdf enhanced color solid
# Set the output file name
set output 'cache_plots.eps'

# Now plot the data with lines and points
# adjust titles and labels as needed
set title "time per stride"
set title font "Helvetica,18"
set xlabel "log of stride"
set ylabel "average time in nanoseconds"
set grid

# some explanations of commands
# ps - point size, pt - point type (circle, square, etc.)
# pointinterval (pi 2) - plotevery second point
# w lp - with linepoints, lw - line width, lc - line color
# u - using

# put legend in the upper left corner
set key left top

# set N to the number of columns in the array stored in time_cache.csv
N = 6;
plot for [i=2:N] "time_cache.csv" u (column(1)):(column(i)) w lp\
     lw 3 title "s-".(2**(i-2))
# "s-" is a label, (2**(i-2)) indexes the label "s-", 
# "**" means "to the power", "i" is from the "for" loop
