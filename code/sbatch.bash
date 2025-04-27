#!/bin/bash
#SBATCH --job-name="thread_matrices"
#SBATCH --output="thread_matrices.%j.%N.out"
#SBATCH --partition=compute
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=4
#SBATCH --mem=128GB
#SBATCH --account=ccu108
#SBATCH --export=ALL
#SBATCH -t 00:180:00

# Define the matrix sizes and thread counts
N_values=(256 512 1024 2048 4096) 
P_values=(1 2 4 8 16 32 64 128) # (1 2 4 8 16 32 64 128)

# Output CSV files
PARALLEL_FILE="parallelTime.csv"

# Ensure the CSV files have headers
echo "N,P,OverallTime,WorkTime" > $PARALLEL_FILE

# Loop over matrix sizes
for N in "${N_values[@]}"; do
    ./make_matrix A $N $N
    ./make_matrix X $N $N


    # Loop over thread counts for parallel execution
    for P in "${P_values[@]}"; do
        echo "Running parallel version for N=$N with P=$P threads"
        parallel_output=$(./omp_matrix_matrix A X Y2 $P)
        overall_time_p=$(echo "$parallel_output" | grep "Overall Time:" | awk '{print $3}')
        work_time_p=$(echo "$parallel_output" | grep "Work Time:" | awk '{print $3}')

    done

done

echo "Experiment complete. Results stored in $PARALLEL_FILE."
