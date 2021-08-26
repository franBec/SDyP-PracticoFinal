#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>
#define SEMANAS_EN_ANIO 52

short **newMatrix(short filas, short columnas)
{
    short i;
    short **arr = (short **)malloc(filas * sizeof(short *));
    for (i=0; i<filas; i++)
         arr[i] = (short *)malloc(columnas * sizeof(short));
    return arr;
}

void freeMatrix (short** m, short filas) {
    short i;
    for (i = 0; i < filas; i++)
    {
        short* currentIntPtr = m[i];
        free(currentIntPtr);
    }
    free(m);
}

int randInRange(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

void inicializar(short *matrix[], short filas, short columnas)
{
    short i, j;
    for (i = 1; i < filas-1; i++)
    {
        for (j = 0; j < columnas; j+=4)
        {
            //estados
            int random = randInRange(0,100);
            if(random<=50)
            {
                matrix[i][j] = 4;
                matrix[i][j+3] = -1;
            }
            else
            {
                random = randInRange(0,100);
                if(random<=20)
                {
                    matrix[i][j] = 2;
                    matrix[i][j+3] = 6;
                }
                else if(random<=70){
                    matrix[i][j] = 3;
                    matrix[i][j+3] = randInRange(0,6);
                }
                else{
                    matrix[i][j] = 1;
                    matrix[i][j+3] = randInRange(6,8);
                }
            }
            //edades
            random = randInRange(0,100);
            if(random<=30){
                matrix[i][j+1] = randInRange(0, 2*SEMANAS_EN_ANIO);
            }
            else if(random <= 84){
                matrix[i][j+1] = randInRange(2*SEMANAS_EN_ANIO + 1, 38*SEMANAS_EN_ANIO);
            }
            else{
                matrix[i][j+1] = randInRange(38*SEMANAS_EN_ANIO + 1, 70*SEMANAS_EN_ANIO);
            }

            //heridas
            matrix[i][j+2] = randInRange(0,1);
        }
    }
}

short **vecindarioDeMoore(short *matrix[], short filas, short columnas, short posX, short posY, int id, int nproc, short *vecindario[])
{
    short i,j;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 12; j+=4)
        {
            short coordX = posX - 4 + j;
            short coordY = posY - 1 + i;
            /*
            caso 1 = ambos positivos
            caso 2 = alguno es negativo
            caso 3 = alguno se paso de rango
            se asigna -1 a todos los atributos de un vecino que no existe
            */
            if((id==0 && posY == 1 && i == 0) || (id==nproc && posY == filas && i == 2))
            {
            	vecindario[i][j] = -1;
                vecindario[i][j+1] = -1;
                vecindario[i][j+2] = -1;
                vecindario[i][j+3] = -1;
            }
            else if(coordX>=0 && coordY>=0)
            {
                if(coordX<columnas && coordY<filas)
                {
                    //caso 1
                    vecindario[i][j] = matrix[coordY][coordX];      //asigno estado
                    vecindario[i][j+1] = matrix[coordY][coordX+1];  //asigno edad
                    vecindario[i][j+2] = matrix[coordY][coordX+2];  //asigno herida
                    vecindario[i][j+3] = matrix[coordY][coordX+3];  //asigno contador tiempo
                }
                else
                {
                    //caso 3
                    vecindario[i][j] = -1;
                    vecindario[i][j+1] = -1;
                    vecindario[i][j+2] = -1;
                    vecindario[i][j+3] = -1;
                }
            }
            else
            {
                //caso 2
                vecindario[i][j] = -1;
                vecindario[i][j+1] = -1;
                vecindario[i][j+2] = -1;
                vecindario[i][j+3] = -1;
            }
        }
    }
    return vecindario;
}

float susceptibilidad(short edad, short heridas)
{
    float retorno;
    if(edad < 2*SEMANAS_EN_ANIO)
    {
        retorno = 0.3;
    }
    else if(edad < 38*SEMANAS_EN_ANIO)
    {
        retorno = 0.2;
    }
    else
    {
        retorno = 0.5;
    }
    if(heridas){
        retorno += 0.15;
    }
    return retorno;
}

float porcentajeConSintomas(short *matrix[])
{
    short cont = 0, i, j;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 12; j+=4)
        {
            if ((i != 1 && j != 4) && matrix[i][j] == 2)
            {
                cont++;
            }
        }
    }
    return cont/8;
}

void printMatrix(short* matrix[], short filas, short columnas){
  short i, j;
  for (i = 1; i <=  filas; i++){
    for (j = 0; j < columnas; j+=4)
        printf(" -- %d %d %d %d", matrix[i][j], matrix[i][j+1], matrix[i][j+2], matrix[i][j+3]);
    printf("\n");
  }
  printf("\n");
}

/*
Esta pensado para 4 procesadores
*/

void main(int argc, char *argv[])
{
    int mytid, nproc;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &mytid);
    MPI_Status status;

    srand(time(NULL)+mytid);
    int random;
    float probContagio;
    short filas, columnas = atoi(argv[1]) * 4; //DIMENSION DE LA MATRIZ
    short generacion, i, j;
    short modulo = atoi(argv[1])%nproc;
    if(modulo!=0){
        for (i = 0; i < modulo; ++i)
        {
            if (mytid==i)
            {
                filas = (atoi(argv[1])/nproc)+1;
            }
        }
        if(mytid>=i){
            filas = atoi(argv[1])/nproc;
        }   
    }
    else{
        filas = atoi(argv[1])/nproc;
    }   
    short semanas = atoi(argv[2]);
    short threads = atoi(argv[3]);
    double tInicio, tFin;
    short **matrix = newMatrix(filas+2, columnas);
    short **matrixProxima = newMatrix(filas+2, columnas);
    short **vecindario = newMatrix(3,12);
    short **auxiliar;

    //INICIO MEDICION DE TIEMPOS
    tInicio = MPI_Wtime();

    inicializar(matrix, filas+2, columnas);

    for(generacion = 0; generacion<semanas; generacion++)
    {

    	/*
		ENVIO DE INFORMACION PUNTO A PUNTO ENTRE PROCESOS
		si soy p0
			enviar fila (fila) a p1
			recibir	de p1 en la fila (fila + 1)

		si soy un pi
			recibir del anterior en la fila 0
			enviar fila 1 al anterior
			enviar fila fila al siguiente
			recibir del siguiente en la fila fila+1

		si soy pn
			recibir del anterior en la fila 0
			enviar fila 1 al anterior
		*/
		MPI_Barrier(MPI_COMM_WORLD);
		//caso p0
		if(mytid==0){
			//enviar fila (fila) a p1
			MPI_Send(matrix[filas], columnas, MPI_SHORT, 1, 99, MPI_COMM_WORLD);

			//recibir de p1 en la fila (fila + 1)
			MPI_Recv(matrix[filas+1], columnas, MPI_SHORT, 1, 99, MPI_COMM_WORLD, &status);
		}

		//caso pi
		else if(mytid>0 && mytid<nproc-1){
			//recibir del anterior en la fila 0
			MPI_Recv(matrix[0], columnas, MPI_SHORT, mytid-1, 99, MPI_COMM_WORLD, &status);

			//enviar fila 1 al anterior
			MPI_Send(matrix[1], columnas, MPI_SHORT, mytid-1, 99, MPI_COMM_WORLD);

			//enviar fila (fila) al siguiente
			MPI_Send(matrix[filas], columnas, MPI_SHORT, mytid+1, 99, MPI_COMM_WORLD);

			//recibir del siguiente en la fila fila+1
			MPI_Recv(matrix[filas+1], columnas, MPI_SHORT, mytid+1, 99, MPI_COMM_WORLD, &status);
		}

		//caso pn
		else{
			//recibir del anterior en la fila 0
			MPI_Recv(matrix[0], columnas, MPI_SHORT, mytid-1, 99, MPI_COMM_WORLD, &status);

			//enviar fila 1 al anterior
			MPI_Send(matrix[1], columnas, MPI_SHORT, mytid-1, 99, MPI_COMM_WORLD);			
		}
        #pragma omp parallel num_threads(threads) private(random, probContagio,vecindario)
        {
            #pragma omp parallel for collapse(2)
            for (i = 0; i < filas; i++)
            {
                for (j = 0; j < columnas; j += 4)
                {
                    matrixProxima[i][j + 1] = matrix[i][j+1] + 1; //Agrego una semana a la edad

                    switch (matrix[i][j])
                    {
                    case 0: //podado
                        if(matrix[i][j+3] == 7){
                            matrixProxima[i][j] = 4;
                            matrixProxima[i][j+3] = -1;
                        }
                        else{
                            matrixProxima[i][j+3] = matrix[i][j+3] + 1;
                        }
                        break;
                    case 1:
                        random = randInRange(0, 100);
                        if (matrix[i][j + 1] <= 2 * SEMANAS_EN_ANIO)
                        {
                            if (random > 1)
                            {
                                matrixProxima[i][j] = 4;
                                matrixProxima[i][j + 3] = -1;
                            }
                            else
                            {
                                matrixProxima[i][j] = 0;
                                matrixProxima[i][j + 3] = -1;
                            }
                        }
                        else if (matrix[i][j + 1] <= 38 * SEMANAS_EN_ANIO)
                        {
                            if (random > 10)
                            {
                                matrixProxima[i][j] = 4;
                                matrixProxima[i][j + 3] = -1;
                            }
                            else
                            {
                                matrixProxima[i][j] = 0;
                                matrixProxima[i][j + 3] = -1;
                            }
                        }
                        else
                        {
                            if (random > 45)
                            {
                                matrixProxima[i][j] = 4;
                                matrixProxima[i][j + 3] = -1;
                            }

                            else
                            {
                                matrixProxima[i][j] = 4;
                                matrixProxima[i][j + 1] = 52;
                                matrixProxima[i][j + 3] = -1;
                            }
                        }
                        break;
                    case 2: //enfermo con sintomas
                        if (randInRange(0, 100) <= 90)
                        {
                            matrixProxima[i][j] = 1;
                        }
                        else
                        {
                            matrixProxima[i][j] = 2;
                        }
                        matrixProxima[i][j + 3] = matrix[i][j+3] + 1;
                        break;
                    case 3: //enfermo sin sintomas
                        if (matrix[i][j + 3] >= 6)
                        {
                            matrixProxima[i][j] = 2;
                        }
                        else
                        {
                            matrixProxima[i][j] = 3;
                        }
                        matrixProxima[i][j + 3] = matrix[i][j+3] + 1;
                        break;
                    case 4: //sano
                        //puede pasar a enfermo sin sintomas
                        probContagio = (porcentajeConSintomas(vecindarioDeMoore(matrix, filas, columnas, j, i, mytid, nproc,vecindario)) + susceptibilidad(matrix[i][j+1], matrix[i][j+2])) * 0.6 + 0.05;
                        if ((randInRange(0, 100) / 100) <= probContagio)
                        {
                            matrixProxima[i][j] = 3;
                        }
                        matrixProxima[i][j + 3] = -1;
                        break;
                    }
                    matrixProxima[i][j + 2] = randInRange(0, 1); //Genero heridas aleatoriamente
                }
            }
            #pragma omp barrier
        }
        auxiliar = matrix;
        matrix = matrixProxima;
        matrixProxima = auxiliar;
    }

    //DETENGO LA MEDICION DEL TIEMPO
    MPI_Barrier(MPI_COMM_WORLD);
    tFin = MPI_Wtime();
    if(mytid==0){
        printf("Tiempo medido: %.3f segundos.\n",tFin-tInicio);
    }
    freeMatrix(matrix,filas);
    freeMatrix(matrixProxima,filas);
    MPI_Finalize();
}
