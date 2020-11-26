#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define LADOWISKO 1
#define START 2
#define LOT 3
#define KONIEC_LOTU 4
#define KATASTROFA 5
#define TANKUJ 1000
#define REZERWA 500
int LADUJ = 1;
int NIE_LADUJ = 0;
int paliwo = 1000;
int iloscHelikopterow;
int iloscLadowisk = 3;
int iloscZajetychLadowisk = 0;
int tag = 1;
int wyslij[2];
int odbierz[2];
int n;
int p_id;
MPI_Status mpi_status;

void Pilot()
{
	sleep(2);
	if(odbierz[0] == LOT)
		printf("Pilot helikoptera nr %d przelatuje nad miastem \n", odbierz[1]);
	if(odbierz[0] == START)
		printf("Pilot helikoptera nr %d rozpoczyna lot \n", odbierz[1]);
	if(odbierz[0] == KONIEC_LOTU)
		printf("Pilot helikoptera nr %d musi niedługo wylądować \n", odbierz[1]);
	if(odbierz[0] == LADOWISKO)
		printf("Pilot helikoptera nr %d odpoczywa na lądowisku \n", odbierz[1]);
	if(odbierz[0] == KATASTROFA)
		printf("Pilot helikoptera nr %d zmarl \n", odbierz[1]);
}

void Wyslij(int stan, int nrHelikoptera)
{
	wyslij[0] = stan;
	wyslij[1] = nrHelikoptera;
	MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
	sleep(1);
}

void Ladowisko(int n)
{
	int nrHelikoptera;
	int stan;
	
	iloscHelikopterow = n - 1;
	
	while(iloscLadowisk <= iloscHelikopterow)
	{
		MPI_Recv(&odbierz,2,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &mpi_status);
		stan = odbierz[0];
		nrHelikoptera = odbierz[1];
		
		if(stan == LADOWISKO)
		{
			printf("Helikopter nr %d stoi na ladowisku \n", nrHelikoptera);
		}
			
		if(stan == START)
		{
			printf("Pozwolenia na start helikopterowi nr %d \n", nrHelikoptera);
			iloscZajetychLadowisk--;
		}
			
		if(stan == LOT)
		{
			printf("Helikopter nr %d lata \n", nrHelikoptera);
			Pilot();
		}
			
		if(stan == KONIEC_LOTU)
		{
			if(iloscZajetychLadowisk < iloscLadowisk)
			{
				iloscZajetychLadowisk++;
				MPI_Send(&LADUJ, 1, MPI_INT, nrHelikoptera, tag, MPI_COMM_WORLD);
			}
			else
			{
				MPI_Send(&NIE_LADUJ, 1, MPI_INT, nrHelikoptera, tag, MPI_COMM_WORLD);
			}
		}
		
		if(stan == KATASTROFA)
		{
			iloscHelikopterow--;
			printf("Katastrofa helikoptera nr %d, pozostalo %d statkow \n", nrHelikoptera, iloscHelikopterow);
		}
	}
}

void Helikopter()
{
	int stan = LOT;
	int suma;
	int i;
	
	while(1)
	{
		if(stan == LADUJ)
		{
			if(rand()%4 == 1)
			{
				stan = START;
				paliwo = TANKUJ;
				printf("Na ladowisku prosze o pozwolenie na start, Helikopter nr %d \n", p_id);
				
			}
			else
			{
				printf("Postoje sobie jeszcze troche \n");
			}
			Wyslij(stan, p_id);
		}
		
		if(stan == START)
		{
			printf("Wystartowalem, helikopter nr %d \n", p_id);
			stan = LOT;
			Wyslij(stan, p_id);
		}
		
		if(stan == LOT)
		{
			paliwo -= rand()%200;
			if(paliwo <= REZERWA)
			{
				stan = KONIEC_LOTU;
				Wyslij(stan, p_id);
			}
			else
			{
				sleep(3);
			}
		}
		
		if(stan == KONIEC_LOTU)
		{
			int st;
			MPI_Recv(&st, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
			if(st == LADUJ)
			{
				stan = LADOWISKO;
				printf("Wyladowalem, helikopter nr %d \n", p_id);
			}
			else
			{
				paliwo -= rand()%200;
				if(paliwo > 0)
				{
					Wyslij(stan, p_id);
				}
				else
				{
					stan = KATASTROFA;
					printf("Katastrofa \n");
					Wyslij(stan, p_id);
					return;
				}
			}
		}	
	}
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&p_id);
	MPI_Comm_size(MPI_COMM_WORLD,&n);
	srand(time(NULL));
	
	if(p_id == 0)
		Ladowisko(n);
	else
		Helikopter();
	
	MPI_Finalize();
	return 0;
}

