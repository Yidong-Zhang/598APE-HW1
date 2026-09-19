#include<string.h>
#include<stdio.h>
#include<limits>
#include<math.h>
#include<stdlib.h>
//#include <printf.h>
#include <stddef.h>
#include "vector.h"

Vector::Vector(double a, double b, double c) : x(a), y(b), z(c) {
}
void Vector::operator -= (const Vector rhs) {
   x-=rhs.x; y-=rhs.y; z-=rhs.z;
}
void Vector::operator += (const Vector rhs) {
   x+=rhs.x; y+=rhs.y; z+=rhs.z;
}
void Vector::operator *= (const double rhs) {
   x*=rhs; y*=rhs; z*=rhs;
}
void Vector::operator *= (const float rhs) {
   x*=rhs; y*=rhs; z*=rhs;
}
void Vector::operator *= (const int rhs) {
   x*=rhs; y*=rhs; z*=rhs;
}
void Vector::operator /= (const double rhs) {
   x/=rhs; y/=rhs; z/=rhs;
}
void Vector::operator /= (const float rhs) {
   x/=rhs; y/=rhs; z/=rhs;
}
void Vector::operator /= (const int rhs) {
   x/=rhs; y/=rhs; z/=rhs;
}


Vector Vector::operator - (const Vector rhs) {
   return Vector(x-rhs.x, y-rhs.y, z-rhs.z);
}
Vector Vector::operator + (const Vector rhs) {
   return Vector(x+rhs.x, y+rhs.y, z+rhs.z);
}
/*
Vector Vector::operator * (const Vector a) {
   return Vector(y*a.z-z*a.y, z*a.x-x*a.z, x*a.y-y*a.x);
}*/
Vector Vector::operator * (const double rhs) {
   return Vector(x*rhs, y*rhs, z*rhs);
}
Vector Vector::operator * (const float rhs) {
   return Vector(x*rhs, y*rhs, z*rhs);
}
Vector Vector::operator * (const int rhs) {
   return Vector(x*rhs, y*rhs, z*rhs);
}
Vector Vector::operator / (const double rhs) {
   return Vector(x/rhs, y/rhs, z/rhs);
}
Vector Vector::operator / (const float rhs) {
   return Vector(x/rhs, y/rhs, z/rhs);
}
Vector Vector::operator / (const int rhs) {
   return Vector(x/rhs, y/rhs, z/rhs);
}
Vector Vector::cross(const Vector a) {
   return Vector(y*a.z-z*a.y, z*a.x-x*a.z, x*a.y-y*a.x);
}
double Vector::mag2(){
   return x*x+y*y+z*z; 
}
double Vector::mag(){
   return sqrt(x*x+y*y+z*z); 
}
double Vector::dot(const Vector a){
   return x*a.x+y*a.y+z*a.z;
}
Vector Vector::normalize(){
   double m = mag();
   return Vector(x/m, y/m, z/m); 
}
Vector solveScalers(
    const Vector& v1,
    const Vector& v2,
    const Vector& v3,
    const Vector& C
){
    return Vector(
        C.x * v1.x + C.y * v1.y + C.z * v1.z,
        C.x * v2.x + C.y * v2.y + C.z * v2.z,
        C.x * v3.x + C.y * v3.y + C.z * v3.z
    );
}

// Vector solveScalers(
//     const Vector& v1,
//     const Vector& v2,
//     const Vector& v3,
//     const Vector& C
// ){
//     const double denom =
//         (v1.z * v2.y - v1.y * v2.z) * v3.x +
//         (v1.x * v2.z - v1.z * v2.x) * v3.y +
//         (v1.y * v2.x - v1.x * v2.y) * v3.z;

//     const double a =
//         (C.z * v2.y - C.y * v2.z) * v3.x +
//         (C.x * v2.z - C.z * v2.x) * v3.y +
//         (C.y * v2.x - C.x * v2.y) * v3.z;

//     const double b =
//         (C.y * v1.z - C.z * v1.y) * v3.x +
//         (C.z * v1.x - C.x * v1.z) * v3.y +
//         (C.x * v1.y - C.y * v1.x) * v3.z;

//     const double c =
//         (C.z * v1.y - C.y * v1.z) * v2.x +
//         (C.x * v1.z - C.z * v1.x) * v2.y +
//         (C.y * v1.x - C.x * v1.y) * v2.z;

//     const double invDenom = 1.0 / denom;

//     return Vector(
//         a * invDenom,
//         b * invDenom,
//         c * invDenom
//     );
// }

Ray::Ray(const Vector& po, const Vector& ve): point(po), vector(ve){}
