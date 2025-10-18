#include <stdio.h>
#include <math.h>
#include "calculator.h"

int main() {
    printf("=== Simple Calculator Demo ===\n");
    
    double a = 10.5, b = 3.2;
    
    printf("a = %.2f, b = %.2f\n", a, b);
    printf("Addition: %.2f + %.2f = %.2f\n", a, b, add(a, b));
    printf("Subtraction: %.2f - %.2f = %.2f\n", a, b, subtract(a, b));
    printf("Multiplication: %.2f * %.2f = %.2f\n", a, b, multiply(a, b));
    printf("Division: %.2f / %.2f = %.2f\n", a, b, divide(a, b));
    printf("Square root of %.2f = %.2f\n", a, square_root(a));
    
    return 0;
}