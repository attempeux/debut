#include <stdio.h>
#include <string.h>

int main ()
{
    long double x = 4543333333333333.92924447929292929;

    printf("<%*.*LG>\n", 15, 9, x);
    printf("<%*.*LE>\n", 15, 11, x);
    printf("<%*.*Lf>\n", 15, 11, x);
    printf("<%*c>\n", 15, ' ');
}



// *s Prints spaces to fill a * width requested
// *.*s
// ~ ~ characters to print
// minimum
