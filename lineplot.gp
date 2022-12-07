set style fill solid 0 border 0
set style data linespoints
set datafile separator ','
set datafile columnheaders

stats file u 0 nooutput

set yrange [0:]

set xtics nomirror noenhanced
set xtics out
set ytics nomirror noenhanced

set grid ytics

set key outside above

plot for [i=2:STATS_columns] file using 1:i title columnhead(i)
