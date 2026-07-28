set title   "Solar Intensity and Cumulative"
set xlabel  "Time [h]"
set ylabel  "Solar Intensity [%]"
set y2label "Solar Cumulative"

set grid
set ytics  5 nomirror
set y2tics 0.2

set yrange  [0:125]
set y2range [0:5]

# https://docs.w3cub.com/gnuplot/linetypes_colors_styles.html
set style line 3 lt rgb "grey" lw 1

#plot "gnuplot_data.txt" using 1:2 with linespoints title "XY Data"

plot  "gnuplot_data.txt" using 1:4 with lines      axes x1y1 title "Solar Intensity [%]", \
      "gnuplot_data.txt" using 1:5 with lines      axes x1y2 title "Solar Cumulative"
#     "gnuplot_data.txt" using 1:2 with lines ls 3 axes x1y1 title "Charge [Ah]"

#pause -1 "Press any key to exit..."
pause mouse
