#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define numero_processori 4
#define a 0.0
#define b 1.0
#define num_intervalli 100000


//Funzioni di utilità usate
double funzione(double);
double formula_trapezoidale(double, double, int, double);



int main(int argc, char *argv[])
{
	int myrank, numprocs;
	int namelen;
	int i, j;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	const int num_punti = num_intervalli + 1;
	double *x = NULL, *part_x = NULL;
	double risultato = 0.0, part_risultato = 0.0;
	double t1, t2;
	int *displs = NULL, *dimRecv = NULL;

	const double h = (double)(b - a) / num_intervalli;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Get_processor_name(processor_name, &namelen);
	MPI_Status info;


	//Controlli di robustezza delegati al processo 0
	if (myrank == 0) {

		if (numprocs != numero_processori) {
			printf("Errore! Il numero dei processi deve essere pari a %d\n", numero_processori);
			MPI_Abort(MPI_COMM_WORLD, 1);
		}

		if (num_intervalli <= 0) {
			printf("Errore! Il numero di intervalli non è un numero positivo\n");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}

		if (b < a) {
			printf("Errore! L'intervallo non è coerente.\n");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}

		if (num_intervalli < numprocs) {
			printf("Errore! La dimensione non è coerente con il numero di processori.\n");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}
	}//Fine controlli di robustezza
	MPI_Barrier(MPI_COMM_WORLD);

	printf("Ciao sono il processo %d e mi chiamo %s\n", myrank, processor_name);

	
	//Allocazione vettori comuni a tutti i processori
	x = (double*)calloc(num_punti, sizeof(double));
	dimRecv = (int*)calloc(numprocs, sizeof(int));
	displs = (int*)calloc(numprocs + 1, sizeof(int));


	//Inizializzazione parametri per scatterv delegati al processo 0
	if (myrank == 0) {

		//Inizializzazione vettore di punti da distribuire	

		for (i = 0;i < num_punti - 1;i++) {
			x[i] = a + i * h;
		}
		x[num_punti - 1] = b;


		//Inizializzazione dimRecv

		for (i = 0; i < numprocs; i++) {

			if (i < (num_punti%numprocs)) {
				//Devo ricevere i+1 elementi
				dimRecv[i] = (int)num_punti / numprocs + 1;
			}
			else {
				//Devo ricevere i elementi
				dimRecv[i] = (int)num_punti / numprocs;
			}
		}

		//Inizializzazione displs

		displs[0] = 0;
		for (i = 1; i < numprocs; i++) {

			displs[i] = displs[i - 1] + dimRecv[i - 1];
		}
		displs[numprocs] = num_punti - 1;
		
	}//Fine inizializzazione parametri scatterv

	//Broadcast
	MPI_Bcast(dimRecv, numprocs, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(displs, numprocs + 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(x, num_punti, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	if (myrank == 0) {

		//Inizio tempo
		t1 = MPI_Wtime();
	}

	//Calcolo integrale in parallelo
	part_risultato = formula_trapezoidale(x[displs[myrank]], x[displs[myrank + 1]], dimRecv[myrank], h);

	//Stampa in ordine del prodotto scalare i-esimo
	for (i = 0; i < numprocs; i++) {
		if (myrank == i) {
			printf("Sono il processo %d. Il mio integrale parziale è: %.16lf\n", i, part_risultato);
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}

	//Reduce dei risultati parziali
	MPI_Reduce(&part_risultato, &risultato, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	//Stampa risultato finale
	if (myrank == 0) {

		//Fine tempo
		t2 = MPI_Wtime();

		printf("Sono il processo %d. L'integrale è pari a : %.16lf.\nSono il processo %d : tempo di esecuzione totale di %.16lf secondi.\n", myrank, risultato, myrank, t2 - t1);
	}

	//Libero la memoria in tutti i processi 
	free(x);
	x = NULL;
	free(dimRecv);
	dimRecv = NULL;
	free(displs);
	displs = NULL;


	MPI_Finalize();
	return EXIT_SUCCESS;
}


//Funzione da integrare
double funzione(double x) {

	double y = x / (pow(x, 2) + 5);

	return y;
}



//Formula trapezoidale composita per il calcolo integrale
double formula_trapezoidale(double pto_iniziale, double pto_finale, int n, double h) {

	double risultato_parziale = funzione(pto_iniziale) + funzione(pto_finale);
	double somma = 0.0;
	double risultato = 0.0;
	int i = 0;
	double k = pto_iniziale;

	for (i = 1; i < n; i++) {
		k = k + h;
		somma += funzione(k);
	}

	risultato = (h / 2)*(risultato_parziale + 2 * somma);

	return risultato;
}
