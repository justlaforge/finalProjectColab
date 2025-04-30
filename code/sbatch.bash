#!/bin/bash
#SBATCH --job-name="stencil_serial"
#SBATCH --output="stencil_serial.%j.%N.out"
#SBATCH --partition=compute
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=2
#SBATCH --cpus-per-task=4
#SBATCH --mem=20GB
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
N_values=14
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
HYBRID_FILE="hybridTime.csv"

# Ensure the CSV files have headers
echo "Iterations, Rows, Cols, OverallTime, WorkTime, DiffTime," > $SERIAL_FILE
echo "Iterations, Rows, Cols, OverallTime, WorkTime, DiffTime, Processors" > $PTHREADS_FILE
echo "Iterations, Rows, Cols, OverallTime, WorkTime, DiffTime, Processors" > $OMP_FILE
echo "Iterations, Rows, Cols, OverallTime, WorkTime, DiffTime, Processors" > $MPI_FILE
echo "Iterations, Rows, Cols, OverallTime, WorkTime, DiffTime, Processors" > $HYBRID_FILE

# Compile files
make

# Loop over matrix sizes and thread counts
for C in "${C_values[@]}"; do
    ./make-2d A.bin $C

    echo "Running serial version for C=$C with N=${N_values} iterations."
    ./stencil-2d -n $N_values -i A.bin -o C.bin

    for P in "${P_values[@]}"; do
        echo "Running Pthreads version for C=$C, P=$P with N=${N_values} iterations."
        ./stencil-2d-pth -n $N_values -i A.bin -o C.bin -p $P

        echo "Running OpenMP version for C=$C, P=$P with N=${N_values} iterations."
        ./stencil-2d-omp -n $N_values -i A.bin -o C.bin -p $P

        # Split processors between -np and -p
        if [ $P -eq 1 ]; then
            NP=1
            PP=1
        else
            NP=$((P / 2))
            PP=2
        fi

        echo "Running MPI version for C=$C, P=$P with N=${N_values} iterations."
        mpirun -np $NP ./stencil-2d-mpi -n $N_values -i A.bin -o C.bin 

        echo "Running Hybrid MPI version for C=$C, P=$P with N=${N_values} iterations."
        mpirun -np $NP ./stencil-2d-hybrid -n $N_values -i A.bin -o C.bin -p $PP
    done
done

make clean

echo "Experiment complete. Results stored in $SERIAL_FILE, $PTHREADS_FILE, $OMP_FILE, $MPI_FILE, $HYBRID_FILE."
