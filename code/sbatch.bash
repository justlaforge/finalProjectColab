#!/bin/bash
#SBATCH --job-name="thread_matrices"
#SBATCH --output="thread_matrices.%j.%N.out"
#SBATCH --partition=compute
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=4
#SBATCH --mem=16GB
#SBATCH --account=ccu108
#SBATCH --export=ALL
#SBATCH -t 00:30:00

# Define the matrix sizes and thread counts
N_values=(8) 
C_values=(40000)  # rows and columns
P_values=(1) # (1 2 4 8 16 32 64 128)

# Output CSV files
SERIAL_FILE="serialTime.csv"

# Ensure the CSV files have headers
echo "Iterations,Rows,Cols,OverallTime,WorkTime" > $SERIAL_FILE

# Loop over matrix sizes
for C in "${C_values[@]}"; do
    ./make-2d A $C

    echo "Running serial version for C=$C with N=8 iterations."
    ./stencil-2d -n 8 -i A -o Out

done

echo "Experiment complete. Results stored in $SERIAL_FILE."
