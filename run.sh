waf_path="../waf"

${waf_path} --run "Exercise5" > Exercise5Log.out 2>&1
awk '$1 == "1" {print $2 "\t" $3}' Exercise5Log.out > Exercise5Flow1.dat
awk '$1 == "2" {print $2 "\t" $3}' Exercise5Log.out > Exercise5Flow2.dat