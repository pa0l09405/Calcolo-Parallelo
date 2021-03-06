#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define up		0
#define down	1
#define left	2
#define right	3
#define dim		2
#define num_vicini	4

int main(int argc, char *argv[])
{
	int myrank, numprocs;
	int namelen;
	int i, j;
	int somma, count;
	float media;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int mio_valore;
	int coords[dim];
	int dims[dim]={3,3};
	int periods[dim]={1,1};				//Caso griglia periodica
	//int periods[dim]={0,0};			//Caso griglia non periodica
	int vicini[num_vicini];
	float* valore_medie;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	
	//Controlli di robustezza delegati al processo 0
	if (myrank == 0) {

		//Controllo numprocs coerente con definizione griglia
		if(numprocs!=9){
			printf("Errore! Il numero dei processi deve essere 9\n");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	//Definizione griglia di processi 9x9
	MPI_Comm griglia;
	MPI_Cart_create(MPI_COMM_WORLD, dim, dims, periods, 0, &griglia);
	MPI_Comm_rank(griglia, &myrank);
	MPI_Cart_coords(griglia, myrank, dim, coords);
	MPI_Get_processor_name(processor_name, &namelen);
	MPI_Status info;

	//Stampa in ordine dei processi
	for (i = 0; i < numprocs; i++) {

		if (myrank == i) {
			printf("Ciao sono il processo %d e mi chiamo %s. Le mie coordinate sono: (%d, %d)\n", i, processor_name, coords[0], coords[1]);
		}
		MPI_Barrier(griglia);
	}
	
	if(myrank==0){
		valore_medie = (float*) malloc (numprocs*sizeof(float));
	}
	
	
	//Inizializzazione valore da distribuire
	mio_valore = myrank;

	//Get rank dei miei vicini
	MPI_Cart_shift(griglia, 0, 1, &vicini[up], &vicini[down]);
	MPI_Cart_shift(griglia, 1, 1, &vicini[left], &vicini[right]);
	
	//Stampa in ordine dei processi
	for (i = 0; i < numprocs; i++) {

		if (myrank == i) {
			printf("Ciao sono il processo %d. I rank dei miei vicini sono U:%d, D:%d, L:%d, R:%d\n", myrank, vicini[up], vicini[down], vicini[left], vicini[right]);
		}
		MPI_Barrier(griglia);
	}


	//Calcolo media rank vicini
	somma = mio_valore;
	count = 1;
	for (i=0; i<num_vicini; i++){
		
		if (vicini[i]>=0){
			somma+=vicini[i];	
			count++;
		}
	}
	media = (float) somma/count;

	//Stampa in ordine
	for (i = 0; i < numprocs; i++) {

		if (myrank == i) {
			printf("Ciao sono il processo %d. La media dei rank dei miei vicini e' %f\n", myrank, media);
		}
		MPI_Barrier(griglia);
	}
	
	
	//Mando le medie a 0
	MPI_Gather(&media, 1, MPI_FLOAT, valore_medie, 1, MPI_FLOAT, 0, griglia);

	if (myrank==0){
		
		
		printf("Ciao sono il processo %d. Le medie sono: ", myrank);
		
		for (i = 0; i < numprocs; i++)
			printf("%f\n",valore_medie[i]);
		
		printf("\n");
		
	}

	//Libero la memoria nel processo root
	if(myrank==0)
		free(valore_medie);

	MPI_Finalize();
	return 0;
}
