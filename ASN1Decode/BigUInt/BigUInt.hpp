#pragma once
#include <stdint.h>
#include <ostream>

class BigUInt
{
public:
	BigUInt();
	BigUInt(const BigUInt& r);
	BigUInt(BigUInt&& r) noexcept;
	~BigUInt();

	static BigUInt pow2(const BigUInt& num);

	BigUInt modPow(BigUInt e, const BigUInt& m) const;
	BigUInt pow(uint64_t e) const;
	BigUInt operator+(const BigUInt& r) const;
	BigUInt operator-(const BigUInt& r) const;
	BigUInt operator*(const BigUInt& r) const;
	BigUInt operator%(const BigUInt& r) const;

	BigUInt& operator+=(const BigUInt& r);
	BigUInt& operator>>=(uint64_t r);

	bool operator<(const BigUInt& r) const;
	bool operator<=(const BigUInt& r) const;
	bool operator==(uint64_t r)const;
	bool operator!=(uint64_t r)const;

	BigUInt& operator=(uint64_t num);

	void loadFromByteReversArray(const uint8_t* arr, size_t size);
	BigUInt& operator=(const BigUInt& r);
	BigUInt& operator=(BigUInt&& r) noexcept;
	uint8_t operator[](size_t i) const;

	void reserve(size_t capacity);

//private:
	friend std::ostream& operator<<(std::ostream& out, const BigUInt& point);

	size_t capacity;
	uint8_t* integer;
};

std::ostream& operator<<(std::ostream& out, const BigUInt& point);