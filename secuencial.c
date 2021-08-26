#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SEMANAS_EN_ANIO 52

/*
-- CADA ARBOL ESTA REPRESENTADO POR 4 POSICIONES DE MEMORIA EN LA FILA DE LA MATRIZ
-- EN LA POSICION 0 SE ENCUENTRA EL ESTADO DEL ARBOL
-- EN LA POSICION 1 SE ENCUENTRA LA EDAD DEL ARBOL
-- EN LA POSICION 2 SE ENCUENTRA SI POSEE HERIDAS ABIERTAS O NO
-- EN LA POSICION 3 SE ENCUENTRA UN CONTADOR AUXILIAR DE SEMANAS PARA CIERTOS CASOS
*/

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
    for (i = 0; i < filas; i++)
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

short **vecindarioDeMoore(short *matrix[], short filas, short columnas, short posX, short posY, short *vecindario[])
{
    short i,j;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 9; j+=4)
        {
            short coordX = posX - 4 + j;
            short coordY = posY - 1 + i;
            /*
            caso 1 = ambos positivos
            caso 2 = alguno es negativo
            caso 3 = alguno se paso de rango
            se asigna -1 a todos los atributos de un vecino que no existe
            */
            if(coordX>=0 && coordY>=0)
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
  for (i = 0; i <  filas; i++){
    for (j = 0; j < columnas; j+=4)
        printf(" -- %d %d %d %d", matrix[i][j], matrix[i][j+1], matrix[i][j+2], matrix[i][j+3]);
    printf("\n");
  }
  printf("\n");
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int random;
    float probContagio;
    short filas = atoi(argv[1]), columnas = atoi(argv[1]) * 4; //DIMENSION DE LA MATRIZ
    short generacion, i, j;
    short semanas = atoi(argv[2]);
    short **matrix = newMatrix(filas, columnas);
    short **matrixProxima = newMatrix(filas, columnas);
    short **vecindario = newMatrix(3,12);
    short **auxiliar;

    //COMIENZO A MEDIR EL TIEMPO
    struct timespec begin, end; 
    clock_gettime(CLOCK_REALTIME, &begin);
    
    inicializar(matrix, filas, columnas);
    for(generacion=0; generacion<semanas; generacion++)
    {
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
                    probContagio = (porcentajeConSintomas(vecindarioDeMoore(matrix, filas, columnas, j, i, vecindario)) + susceptibilidad(matrix[i][j+1], matrix[i][j+2])) * 0.6 + 0.05;
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
        auxiliar = matrix;
        matrix = matrixProxima;
        matrixProxima = auxiliar;
    }

    //DETENGO LA MEDICION DEL TIEMPO
    clock_gettime(CLOCK_REALTIME, &end);
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    double elapsed = seconds + nanoseconds*1e-9;
    printf("Tiempo medido: %.3f segundos.\n", elapsed);
    freeMatrix(matrix,filas);
    freeMatrix(matrixProxima,filas);
    return 0;

}
