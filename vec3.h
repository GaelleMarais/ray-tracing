#pragma once
#include <cmath>
#include <iostream>

template<typename T>
struct Vec3 {
	T x, y, z;
};

template<typename T>
bool operator==(const Vec3<T>& a, const Vec3<T>& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
};

template<typename T>
Vec3<T> operator*(const Vec3<T>& a, const Vec3<T>& b) {
	return Vec3<T>{ a.x * b.x, a.y * b.y, a.z * b.z };
};

template<typename T>
Vec3<T> operator*(const Vec3<T>& a, const float& scalar) {
	return Vec3<T>{ a.x * scalar, a.y * scalar, a.z * scalar };
};

template<typename T>
Vec3<T> operator*(const float& scalar, const Vec3<T>& a) {
	return a * scalar;
}

template<typename T>
Vec3<T> operator/(const Vec3<T>& a, const float& scalar) {
	return Vec3<T>{ a.x / scalar, a.y / scalar, a.z / scalar };
};

template<typename T>
Vec3<T> operator/(const float& scalar, const Vec3<T>& a) {
	return a / scalar;
}


template<typename T>
Vec3<T> operator/(const Vec3<T>& a, const Vec3<T>& b) {
	return Vec3<T>{ a.x / b.x, a.y / b.y, a.z / b.z };
};

template<typename T>
Vec3<T> operator-(const Vec3<T>& a, const Vec3<T>& b) {
	return Vec3<T>{ a.x - b.x, a.y - b.y, a.z - b.z };
};

template<typename T>
Vec3<T> operator+(const Vec3<T>& a, const Vec3<T>& b) {
	return Vec3<T>{ a.x + b.x, a.y + b.y, a.z + b.z };
};

template<typename T>
float norm(const Vec3<T>& v) {
	return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
};

template<typename T>
float dot(const Vec3<T>& a, const Vec3<T>&b) {
	return a.x* b.x + a.y * b.y + a.z * b.z;	
}

template<typename T>
Vec3<T> normalise(const Vec3<T>& a){
	float n = norm(a);
	if (!n) return {0, 0, 0};
	return {a.x/n, a.y/n, a.z/n};
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const Vec3<T>& v)
{
	stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return stream;
}