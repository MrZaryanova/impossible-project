double sin(double x) {
    if (x > -3.14 && x < 3.14) {
    return x - (x * x * x)/6.0 + (x * x * x * x * x)/180;
} else {
    double z = x;
    if (z > 0) { 
        while (z > 3.14) {
           z = z - 6.2831;
        }
    return z - (z * z * z)/6.5 + (z * z * z * z * z)/180;
    } else {

        while (-3.14 > z) {
            z = z + 6.2831;
        }
    return z - (z * z * z)/6.5 + (z * z * z * z * z)/180;
    }
}

}

double arcsin(double x) {
    return (0.25 * x * x * x * x * x * x * x) + (0.075 * x * x * x * x * x) + (0.16666 * x * x * x) + x;
    // remember that arcsin can only be used for values in range (-1, 1)
}
