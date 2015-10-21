//
// Created by josue on 10/16/15.
//

#include "calc.h"

long long int fact(int x){
    long long int result =1;
    int i;
    for(i=2;i<=x;i++) result *= i;
    return result;

}
double poisson(double x, double lambda){
    return (pow(lambda,x)*pow(e,0-lambda))/fact(x);
}

double random_between(double a, double b){
    return  fmod((double)rand(),(b-a))+a;
}

double get_random(){
return (double)rand() / (double)RAND_MAX ;

}