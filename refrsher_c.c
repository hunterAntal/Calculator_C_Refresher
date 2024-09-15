#include <stdio.h>
#include <errno.h>   // Include errno


void devide (float num, float denom){
    float total = 0.0;

    // devide the num / denom

    // Perform floating-point division by casting one of the operands

    // Check for x/0
    while(denom == 0){
        printf("Please enter a non-zero denuminator: ");
            scanf("%f", &denom);
        }

    total = num / denom;
    printf("%.2f devided by %.2f gives you %.1f\n", num, denom, total);  // Print the integer
    
}

int chooseOpration(){
    int choice = 0;

    // dialogue
    printf(" ~~~~ Welcome to Calculator App! :) ~~~~\n");
    printf("---- ---- ---- ---- ---- ---- ----\n");
    printf("Please choose the operation you want to perform.\n");
    printf("For ADDITION please enter 1.\n");
    printf("For SUBTRACTION please enter 2.\n");
    printf("For MULTIPLICATION please enter 3.\n");
    printf("For DIVITION please enter 4.\n");
    printf("---- ---- ---- ---- ---- ---- ----\n");
    printf("Enter Choice: ");

    scanf("%d", &choice);

    while (choice <= 0 || choice > 4){
        printf("---- ---- ---- ---- ---- ---- ----\n");
        printf("Please choose operation\n");
        printf("Add: 1\n");
        printf("Subtract: 2\n");
        printf("Multiply: 3\n");
        printf("Divide: 4\n");
        printf("---- ---- ---- ---- ---- ---- ----\n");
        printf("Enter Choice: ");
        scanf("%d", &choice);
    }

    return choice;
}

void twoOperators(float *x, float *y){

    printf("Enter the first number: ");
    scanf("%f", x);  // Take an float input

    printf("Enter the second number: ");
    scanf("%f", y);
}

void add(float *x, float *y){
    float total = *x + *y;
    printf("Adding: %.2f + %.2f = %.2f\n", *x, *y, total);
}

void subtract(float *x, float *y){
    float total = *x - *y;
    printf("Subtracting: %.2f - %.2f = %.2f\n", *x, *y, total);
}

void mult(float *x, float *y){
    float total = *x * *y;
    printf("Multipling: %.2f * %.2f = %.2f\n", *x, *y, total);
    
}

void divide(float *x, float *y){

    // stops divition by zero
    while(*y == 0){
        printf("Please enter a non zero denominatior: ");
        scanf("%f", y);
    }
    

    if(*y != 0){
        float total = *x / *y;
        printf("Dividing: %.2f / %.2f = %.2f\n", *x, *y, total);
    }
    
}

int main() {
    int choice = 0;
    float x, y = 0;

    // prompt user to chose operation
    choice = chooseOpration();

    if (choice == 1){
        printf("---- You choose Addition (1) ----\n");
         // pass pointers to return x and y
        twoOperators(&x, &y);    // Testing 
        add(&x, &y);
    } else if (choice == 2){
        printf("---- You choose Subtraction (2) ----\n");
         // pass pointers to return x and y
        twoOperators(&x, &y);    // Testing 
        subtract(&x, &y);
    } else if (choice == 3){
        printf("---- You choose Multiply (3) ----\n");
         // pass pointers to return x and y
        twoOperators(&x, &y);    // Testing 
        mult(&x, &y);
    } else if (choice == 4){
        printf("---- You choose Divide (4) ----\n");
         // pass pointers to return x and y
        twoOperators(&x, &y);    // Testing 
        divide(&x, &y);
    }
    printf("---- ---- ---- ---- ---- ---- ----\n");

    //printf("the two numbers entered: %f and %f\n", x, y);
    

    return 0;
}
