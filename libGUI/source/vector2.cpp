#include "vector2.h"

Vector2 Vector2::zero = Vector2(0, 0);
Vector2 Vector2::one = Vector2(1, 1);
Vector2 Vector2::right = Vector2(1, 0);
Vector2 Vector2::left = Vector2(-1, 0);
Vector2 Vector2::up = Vector2(0, -1);
Vector2 Vector2::down = Vector2(0, 1);

Vector2::Vector2() {
    x = 0;
    y = 0;
}

Vector2::Vector2(const int& X, const int& Y) {
    x = X;
    y = Y;
}

Vector2 Vector2::operator + (const Vector2& vec) const {
    Vector2 temp = *this;
    temp.x += vec.x;
    temp.y += vec.y;
    return temp;
}

Vector2 Vector2::operator - (const Vector2& vec) const {
    Vector2 temp = *this;
    temp.x -= vec.x;
    temp.y -= vec.y;
    return temp;
}

Vector2 Vector2::operator += (const Vector2& vec) {
    x += vec.x;
    y += vec.y;
    return *this;
}

Vector2 Vector2::operator -= (const Vector2& vec) {
    x -= vec.x;
    y -= vec.y;
    return *this;
}

Vector2 Vector2::operator * (const Vector2& vec) const {
    Vector2 temp;
    temp.x = x * vec.x;
    temp.y = y * vec.y;
    return temp;
}

Vector2 operator * (const Vector2& vec, const int& scalar) {
    Vector2 temp = vec;
    temp.x = temp.x * scalar;
    temp.y = temp.y * scalar;
    return temp;
}

Vector2 operator * (const int& scalar, const Vector2& vec) {
    Vector2 temp = vec;
    temp.x = temp.x * scalar;
    temp.y = temp.y * scalar;
    return temp;
}

Vector2 operator / (const Vector2& vec, const int& scalar) {
    Vector2 temp = vec;
    temp.x = temp.x / scalar;
    temp.y = temp.y / scalar;
    return temp;
}

Vector2 operator / (const int& scalar, const Vector2& vec) {
    Vector2 temp = vec;
    temp.x = scalar / temp.x;
    temp.y = scalar / temp.y;
    return temp;
}

bool Vector2::operator == (const Vector2& vec) const {
    if (x == vec.x && y == vec.y)
        return true;
    return false;
}

bool Vector2::operator != (const Vector2& vec) const {
    if (x != vec.x || y != vec.y)
        return true;
    return false;
}

int Vector2::operator [] (const int& index) const {
    if (index == 0)
        return x;
    else if (index == 1)
        return y;

    return 0;
}

//Returns the squared magnitude of the Vector
int Vector2::sqrMagnitude() {
    return (x * x + y * y);
}
