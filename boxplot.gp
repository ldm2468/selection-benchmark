set style fill solid 0 border 0
set style data boxplot
set datafile separator ','
set datafile columnheaders

stats file u 0 nooutput

set xtics 1,2 format ""
do for[i=2:STATS_columns] {
    stats file every ::0::0 u (s=columnhead(i),1) nooutput
    set xtics add (s i)
}

set xrange [1:STATS_columns+1]
set yrange [0:]

set terminal pict2e texarrows size 8cm,6cm
set xtics noenhanced nomirror
set xtics rotate by 30 right
set xtics out
set ytics nomirror

set style boxplot pointtype 0

set grid ytics

unset key

plot for [i=2:STATS_columns] file using (i):i title columnhead(i)
