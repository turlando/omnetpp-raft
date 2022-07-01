#/bin/sh

mkdir -p results/csv

cd results/sca

for N in *; do
    opp_scavetool x "$N/*.sca" -o ../csv/$N.csv
done
