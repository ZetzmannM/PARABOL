#pragma once

#define MAX(A,B) ((A<B)?B:A)
#define MIN(A,B) ((A>B)?B:A)


#define PI 3.14159265358979323846264
#define PIF 3.14159265358979323846264f

#define RAD(A) (A/180.0 * PI)
#define RADF(A) (A/180.0f * PIF)

#define DEG(A) (A/PI * 180.0)
#define DEGF(A) (A/PIF * 180.0f)