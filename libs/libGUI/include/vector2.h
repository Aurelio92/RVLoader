#ifndef VECTOR2_H
#define VECTOR2_H

class Vector2 {
public:
    int x, y;

    Vector2();
    Vector2(const int&, const int&);

    static Vector2 zero;
    static Vector2 one;
    static Vector2 right;
    static Vector2 left;
    static Vector2 up;
    static Vector2 down;

    //Operators overloading
    Vector2 operator + (const Vector2&) const;
    Vector2 operator - (const Vector2&) const;
    Vector2 operator += (const Vector2&);
    Vector2 operator -= (const Vector2&);

    Vector2 operator * (const Vector2&) const; //Dot product
    bool operator == (const Vector2&) const;
    bool operator != (const Vector2&) const;

    int operator [] (const int&) const;

    int sqrMagnitude();
};

//Other operators overloading
Vector2 operator * (const Vector2&, const int&);
Vector2 operator * (const int&, const Vector2&);
Vector2 operator / (const Vector2&, const int&);
Vector2 operator / (const int&, const Vector2&);

#endif // VECTOR2_H
