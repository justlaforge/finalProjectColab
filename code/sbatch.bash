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

module purge
module load cpu
module load slurm
module load gcc/10.2.0
module load openmpi/4.1.3
module load mpip/3.5

# Define the matrix sizes and thread counts
N_values=8
C_values=(5000 10000 20000 40000)  # rows and columns
P_values=(1 2 4 8 16) # 32 64 128)

# Clear Output CSV files
> serialTime.csv
> pthTime.csv
> ompTime.csv
> mpiTime.csv

SERIAL_FILE="serialTime.csv"
PTHREADS_FILE="pthTime.csv"
OMP_FILE="ompTime.csv"
MPI_FILE="mpiTime.csv"

# Ensure the CSV files have headers
echo "Iterations, Rows, Cols, OverallTime, WorkTime" > $SERIAL_FILE
echo "Iterations, Rows, Cols, OverallTime, WorkTime, Processors" > $PTHREADS_FILE
echo "Iterations, Rows, Cols, OverallTime, WorkTime, Processors" > $OMP_FILE
echo "Iterations, Rows, Cols, OverallTime, WorkTime, Processors" > $MPI_FILE

# Compile files
make

# Loop over matrix sizes
for C in "${C_values[@]}"; do
    ./make-2d A $C

    echo "Running serial version for C=$C with N=${N_values} iterations."
    ./stencil-2d -n 8 -i A -o Out

    # echo "Running Pthreads version for C=$C with N=${N_values} iterations."
    # for P in "${P_values[@]}"; do
    #     ./stencil-2d -n 8 -i A -o Out -p $P
    # done

    echo "Running OpenMP version for C=$C with N=${N_values} iterations."
    for P in "${P_values[@]}"; do
        ./stencil-2d -n 8 -i A -o Out -t $P
    done

    echo "Running MPI version for C=$C with N=${N_values} iterations."
    for P in "${P_values[@]}"; do
        mpirun -np $P ./stencil-2d -n 8 -i A -o Out -m
    done

done

make clean

echo "Experiment complete. Results stored in $SERIAL_FILE, $PTHREADS_FILE, $OMP_FILE, $MPI_FILE."
