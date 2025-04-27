#!/bin/bash
#SBATCH --job-name="stencil_serial"
#SBATCH --output="stencil_serial.%j.%N.out"
#SBATCH --partition=compute
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=4
#SBATCH --mem=128GB
#SBATCH --account=ccu108
#SBATCH --export=ALL
#SBATCH -t 00:90:00

# Define the matrix sizes and thread counts
N_values=8
C_values=(5000 10000 20000 40000)  # rows and columns
P_values=(1) # (1 2 4 8 16 32 64 128)

# Output CSV files
SERIAL_FILE="serialTime.csv"

# Ensure the CSV files have headers
echo "Iterations,Rows,Cols,OverallTime,WorkTime" > $SERIAL_FILE

# Loop over matrix sizes
for C in "${C_values[@]}"; do
    ./make-2d A $C

    echo "Running serial version for C=$C with N=${N_values} iterations."
    ./stencil-2d -n 8 -i A -o Out

done

echo "Experiment complete. Results stored in $SERIAL_FILE."
