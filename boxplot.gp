set style fill solid 0 border linecolor "black"
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

set terminal pict2e texarrows size 10cm,8cm
set xtics noenhanced
set xtics rotate

set style boxplot fraction 1

unset key

plot for [i=2:STATS_columns] file using (i):i title columnhead(i)
