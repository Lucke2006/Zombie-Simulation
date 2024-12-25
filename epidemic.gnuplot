set term x11
set title "Infected Population Over Time"
set xlabel "time (s)"
set ylabel "Infected"

plot "epidemic.dat" using 1:2 with lines title "susceptible",\
"epidemic.dat" using 1:3 with lines title "infected",\
"epidemic.dat" using 1:4 with lines title "dead"
