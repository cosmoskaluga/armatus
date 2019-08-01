ARMATUS=src/armatus
#for gamma in 1.5
#do
    echo $gamma
#mpiexec -np 4 ./$ARMATUS -r 20000 -i examples/Hi-C_armatus.txt.gz -g ${gamma} -o output/test_gamma${gamma} -m -s 0.5
mpiexec -np 4 ./$ARMATUS -r 20000 -i examples/Hi-C_armatus.txt.gz -g 1.5 -o output/test_gamma${gamma} -m -s 0.5
#done
