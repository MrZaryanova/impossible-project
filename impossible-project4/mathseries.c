#include "mathseries.h"

double sin(double x) {
    if (!(x > -PI && x < PI)) {
        if(x > 0) {
            return sin(x - (2 * PI));
            } else {
            return sin(x + (2 * PI));
            }
}
    if(x < -1.5)
        return 0.4 * x * x + 0.4 * PI * x;
    if(x > 1.5)
        return  -0.4 * x * x + 0.4 * PI * x;
    
    return x - (x * x * x)/6.0 + (x * x * x * x * x)/180;

}

double arcsin(double x) {
    if(x < -1)
        return -PI/2;
    else if(x > 1)
        return PI/2;
        
    return (0.25 * x * x * x * x * x * x * x) + (0.075 * x * x * x * x * x) + (0.16666 * x * x * x) + x;
}

