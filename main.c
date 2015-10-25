#include "stdio.h"
#include "aux/list.h"
#include "aux/calc.h"
#include "pthread.h"
#include "unistd.h"

#define FILENAME "input.txt"
#define CHICKEN_LENGTH 0
#define WATER_MIN_RANGE 1
#define WATER_MAX_RANGE 2
#define FOOD_MIN_RANGE 3
#define FOOD_MAX_RANGE 4
#define WATER_POISSON 5
#define FOOD_POISSON 6
#define EGG_POISSON 7
#define FOOD_MIN_ALLOWED 8
#define FOOD_REFILL_AMOUNT 9
#define WATER_MIN_ALLOWED 10
#define WATER_REFILL_AMOUNT 11
#define EGG_MAX_ALLOWED 12
#define WATER_COST 13
#define FOOD_COST 14

#define NIVLEM_WAIT 21600


typedef struct {  // Tipo para mantener los hilos
    pthread_t id;
} Chickens;


int inputs[15];
list_t water_distribution;
list_t food_distribution;
list_t egg_distribution;

pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t water_cond=PTHREAD_COND_INITIALIZER;
pthread_cond_t food_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t nivlem_cond = PTHREAD_COND_INITIALIZER;

int food_amount,water_amount;
int egg_amount = 0;
int cost = 0;

Chickens * chicken_list;
pthread_t bot_thread;


void readInput(){
    FILE* file = fopen (FILENAME, "r");
    int i = 0;
    int j = 0;
    while (!feof (file)) {
        fscanf (file, "%d", &i);
        inputs[j] = i;
        j++;
    }
    fclose (file);
}

void calcWaterDistr(){
    double a=0;
    int i=1;
    while(a<0.99){
        double next = poisson(i,inputs[WATER_POISSON]);
        if(a+next>1.0) break;
        if(a+next < a+0.000001) break; // un aproximado para dejar de sacar probas pequenas
        a+=next;
        add(&water_distribution,a);
        i++;
    }
}

void calcFoodDistr(){
    double a=0;
    int i=1;
    while(a<0.99){
        double next = poisson(i,inputs[FOOD_POISSON]);
        if(a+next>1.0) break;
        if(a+next < a+0.000001) break; // un aproximado para dejar de sacar probas pequenas
        a+=next;
        add(&food_distribution,a);
        i++;
    }
}

void calcEggDistr(){
    double a=0;
    int i=1;
    while(a<0.99){
        double next = poisson(i,inputs[EGG_POISSON]);
        if(a+next>1.0) break;
        if(a+next < a+0.000001) break; // un aproximado para dejar de sacar probas pequenas
        a+=next;
        add(&egg_distribution,a);
        i++;
    }
}

int next_water_wait(){
    double random = get_random();
    int i;
    for (i = 0; i < water_distribution.size; i++) {
        if(random<get(&water_distribution,i)) return i+1;
    }
    return water_distribution.size;
}


int next_food_wait(){
    double random = get_random();
    int i;
    for (i = 0; i < food_distribution.size; i++) {
        if(random<get(&food_distribution,i)) return i+1;
    }
    return food_distribution.size;
}


int next_egg_wait(){
    double random = get_random();
    int i;
    for (i = 0; i < egg_distribution.size; i++) {
        if(random<get(&egg_distribution,i)) return i+1;
    }
    return egg_distribution.size;
}
void * water_proc(void* id_t){
    int id = *((int *)id_t);
    int next;
    int random;

    while(1){
        next = next_water_wait();
        //printf("La gallina %d se dormira %d \n",id+1,next);
        sleep(next);
        random = random_between(inputs[WATER_MIN_RANGE],inputs[WATER_MAX_RANGE]);
        pthread_mutex_lock(&mutex);
        water_amount-= random;
        if(water_amount <= inputs[WATER_MIN_ALLOWED])// enviar señal para aumentar la cantidad de agua y el costo
            pthread_cond_signal(&water_cond);
        pthread_mutex_unlock(&mutex);
        printf("La gallina %d tom'o %d de agua, queda %d del mismo\n", id+1,random,water_amount);
    }
    return NULL;
}
void * food_proc(void* id_t){
    int id = *((int *)id_t);
    int next;
    int random;

    while(1){
        next = next_food_wait();
       // printf("La gallina %d se dormira %d \n",id+1,next);
        sleep(next);
        random = random_between(inputs[FOOD_MIN_RANGE],inputs[FOOD_MAX_RANGE]);
        pthread_mutex_lock(&mutex);
        food_amount-= random;
        if(food_amount <= inputs[FOOD_MIN_ALLOWED]) // enviar señal para aumentar la cantidad de comida y el costo
            pthread_cond_signal(&food_cond);
        pthread_mutex_unlock(&mutex);
        printf("La gallina %d comio %d de concentrado, queda %d del mismo\n", id+1,random,food_amount);
    }
    return NULL;
}

void* chicken_process(void* id_t){
    int id = *((int *)id_t);
    int next;
    pthread_t food_process =(pthread_t) malloc(sizeof(pthread_t));
    pthread_t water_process =(pthread_t) malloc(sizeof(pthread_t));

    pthread_create(&food_process,NULL,food_proc, (void *) &id); // hilo que maneja el consumo de concentrado.
    pthread_create(&water_process,NULL,water_proc, (void *) &id); // hilo que maneja el consumo de agua.

    // el proceso de los huevos se maneja aqui.
    while(1){
        next = next_egg_wait();
        //printf("La gallina %d se dormira %d \n",id+1,next);
        sleep(next);
        pthread_mutex_lock(&mutex);
        egg_amount++;
        pthread_mutex_unlock(&mutex);
        printf("La gallina %d puso un huevo, hay %d en la canasta\n", id+1,egg_amount);
    }

    return NULL;
}


void nivlemProc() {
    clock_t last = clock();

    while (1) {
        clock_t current = clock();
        pthread_mutex_lock(&mutex);
        if ((current >= (last + NIVLEM_WAIT * CLOCKS_PER_SEC))) {
            printf("Han pasado 6 horas, se recogeran los huevos\n");
            egg_amount = 0;
            last = current;
        }
        else if (egg_amount >= EGG_MAX_ALLOWED) {
            printf("Se llego al maximo de huevos en la canasta, se recogeran\n");
            egg_amount = 0;
            last = current;
        }
        pthread_mutex_unlock(&mutex);
    }
}

void createChickens(){
    printf("Se crearon %d gallinas\n",inputs[CHICKEN_LENGTH]);
    int *id;
    chicken_list = calloc(inputs[CHICKEN_LENGTH],sizeof(pthread_t));
    int i;
    for (i = 0; i < inputs[CHICKEN_LENGTH]; i++) {  //crear los procesos para las gallinas con un id secuencial.
        id = malloc(sizeof(int));
        *id = i;
        if (pthread_create(&chicken_list[i].id, NULL, &chicken_process, (void *) id)) {
            printf("Error creando la gallina con id: %d\n",i+1);
        }
    }
}

void * food_bot_proc(){
    while(1) {
        pthread_mutex_lock(&mutex);
        while (food_amount > inputs[FOOD_MIN_ALLOWED])
            pthread_cond_wait(&food_cond, &mutex);

        printf("Se acabo el concentrado\n");
        food_amount += inputs[FOOD_REFILL_AMOUNT];
        cost += inputs[FOOD_COST];
        printf("El costo aumento a %d\n",cost);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}
void * bot(){
    //bot del concentrado
    // se manejan aparte y se usan condicionales diferentes
    pthread_t food_bot;
    pthread_create(&food_bot,NULL,food_bot_proc,NULL);
    // bot del agua..
    while(1) {
        pthread_mutex_lock(&mutex);
        while (water_amount > inputs[WATER_MIN_ALLOWED])
            pthread_cond_wait(&water_cond, &mutex);

        printf("Se acabo el agua\n");
        water_amount += inputs[WATER_REFILL_AMOUNT];
        cost += inputs[WATER_COST];
        printf("El costo aumento a %d\n",cost);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}
int main(){
    srand(time(NULL));
    readInput();  //leer el archivo de entrada, en el primer macro de este archivo se puede cambiar el nombre.
    //Llenar las variables por defecto.
    food_amount = inputs[FOOD_REFILL_AMOUNT];
    water_amount = inputs[WATER_REFILL_AMOUNT];

    // Se calculan las distribuciones con cada possion hasta que se aproxime a 0.99
    //y se guardan en una lista con el nombre [water|food|egg]_distribution;
    calcWaterDistr();
    calcFoodDistr();
    calcEggDistr();
    //Crear el proceso del bot
    pthread_create(&bot_thread,NULL,&bot,NULL);
    // Crear un proceso para cada gallina.
    sleep(1);
    createChickens();
    // correr el nivlem
    nivlemProc();
    return 0;

}

