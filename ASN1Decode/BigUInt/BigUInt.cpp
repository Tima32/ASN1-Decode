#include <memory.h>
#include <iomanip>
#include <iostream>
#include <chrono>

#include "BigUInt.hpp"

using namespace std;

BigUInt::BigUInt():
	capacity(1), integer(new uint8_t[capacity])
{
	integer[0] = 0;
}
BigUInt::BigUInt(const BigUInt& r):
	capacity(r.capacity), integer(new uint8_t[capacity])
{
	memcpy(integer, r.integer, capacity);
}
BigUInt::BigUInt(BigUInt&& r) noexcept:
	capacity(r.capacity), integer(r.integer)
{
	r.integer = nullptr;
}
BigUInt::~BigUInt()
{
	delete[] integer;
}

BigUInt BigUInt::pow2(const BigUInt& num)
{
	BigUInt result, temp;

	size_t num_size = 1;
	for (size_t i = num.capacity - 1; i != -1; --i)
	{
		if (num[i] != 0)
		{
			num_size = i + 1;
			break;
		}
	}

	result.reserve(num_size * 2);
	temp.reserve(num_size * 2);

	for (size_t i_m = 0; i_m < num_size; ++i_m)
	{
		size_t counter = 0;
		uint16_t overflow = 0;
		while (counter < num_size)
		{
			uint16_t rank = (uint16_t)num[counter] * (uint16_t)num[i_m] + overflow;
			overflow = rank >> 8;

			temp.integer[counter + i_m] = (uint8_t)rank;
			counter++;
		}

		if (overflow)
		{
			temp.integer[counter + i_m] = (uint8_t)overflow;;
		}

		result = result + temp;
		memset(temp.integer, 0, temp.capacity);
	}
	return result;
}

BigUInt BigUInt::modPow(BigUInt e, const BigUInt& m) const
{
	// https://codetown.ru/plusplus/algoritm-vozvedeniya-v-stepen/
	BigUInt a = *this;

	BigUInt r;
	r = 1;

	while (e != 0)
	{
		if ((e.integer[0] & 1) == 1)
		{
			r = r * a % m;
		}
		e >>= 1;
		a = a * a % m;
	}

	return r;
}
BigUInt BigUInt::pow(uint64_t e) const
{
	BigUInt result = *this;

	for (size_t i = 0; i < e; ++i)
	{
		result = result * *this;
	}

	return result;
}
BigUInt BigUInt::operator+(const BigUInt& r) const
{
	BigUInt temp;

	size_t max_capacity = capacity;
	if (capacity < r.capacity)
		max_capacity = r.capacity;
	temp.reserve(max_capacity);

	size_t counter = 0;
	uint16_t overflow = 0;


	//сложить
	while (capacity > counter || r.capacity > counter)
	{
		uint16_t rank = (uint16_t)operator[](counter) + (uint16_t)r[counter] + overflow;
		overflow = rank >> 8;

		temp.integer[counter] = (uint8_t)rank;
		counter++;
	}

	if (overflow)
	{
		temp.reserve(temp.capacity + 1);
		temp.integer[counter] = (uint8_t)overflow;
	}

	return temp;
}
BigUInt BigUInt::operator-(const BigUInt& r) const
{
	BigUInt temp;

	size_t max_capacity = capacity;
	if (capacity < r.capacity)
		max_capacity = r.capacity;
	temp.reserve(max_capacity);

	size_t counter = 0;
	uint16_t overflow = 0;


	//сложить
	while (capacity > counter || r.capacity > counter)
	{
		uint16_t rank = (uint16_t)operator[](counter) + (uint16_t)r[counter] + overflow;
		overflow = rank >> 8;

		temp.integer[counter] = (uint8_t)rank;
		counter++;
	}

	if (overflow)
	{
		temp.reserve(temp.capacity + 1);
		temp.integer[counter] = (uint8_t)overflow;
	}

	return temp;
}
BigUInt BigUInt::operator*(const BigUInt& r) const
{
	BigUInt result;

	size_t size_multiplied = -1;//размер умножаемого
	size_t size_multiplier = -1; //размер множителя

	//Размеры чисел
	for (size_t i = capacity - 1; i != -1; --i)
	{
		if (integer[i] != 0)
		{
			size_multiplied = i;
			break;
		}
	}
	for (size_t i = r.capacity - 1; i != -1; --i)
	{
		if (r.integer[i] != 0)
		{
			size_multiplier = i;
			break;
		}
	}

	//проверка умножения на 0
	if (size_multiplied == -1 || size_multiplier == -1)
		return result;

	//установка размера
	size_t size_result = size_multiplied + 1 + size_multiplier + 1;
	BigUInt temp;
	temp.reserve(size_result);
	result.reserve(size_result);

	for (size_t i_multipler = 0; i_multipler < size_multiplier + 1; ++i_multipler)
	{
		uint16_t overflow = 0;
		for (size_t i_multipled = 0; i_multipled < size_multiplied + 1; ++i_multipled)
		{
			uint16_t rank = (uint16_t)integer[i_multipled] * (uint16_t)r.integer[i_multipler] + overflow;
			overflow = rank >> 8;
			temp.integer[i_multipled + i_multipler] = (uint8_t)rank;
		}
		temp.integer[size_multiplied + 1 + i_multipler] = (uint8_t)overflow;

		result += temp;
		for (size_t i_multipled = 0; i_multipled < size_multiplied + 1; ++i_multipled)
		{
			temp.integer[i_multipled + i_multipler] = 0;
		}
		temp.integer[size_multiplied + 1 + i_multipler] = 0;
	}

	return result;
}
BigUInt BigUInt::operator%(const BigUInt& r) const
{
	BigUInt result;

	size_t size_divisible = -1;//размер деллимого
	size_t size_divider = -1; //размер делителя


	for (size_t i = capacity - 1; i != -1; --i)
	{
		if (integer[i] != 0)
		{
			size_divisible = i;
			break;
		}
	}
	for (size_t i = r.capacity - 1; i != -1; --i)
	{
		if (r.integer[i] != 0)
		{ 
			size_divider = i;
			break;
		}
	}

	if (size_divider == -1)//деление на ноль
	{
		result.reserve(capacity);
		for (size_t i = 0; i < result.capacity; ++i)
			result.integer[i] = 0xFF;
		return result;
	}

	if (size_divider > size_divisible) //делитель больше делимого
		return *this;                 //возврат делимого
	else if (size_divider == size_divisible && r.integer[size_divider] > integer[size_divisible])//делитель больше делимого при равном количестве разрядов
		return *this;                 //возврат делимого


	size_t pos_div = size_divisible - size_divider; //откуда делить
	size_t size_div = size_divider;                //размер делителя
	for (size_t i = size_divider; i != -1; --i)
	{
		if (integer[pos_div + i] > r.integer[i])
			break;
		if (integer[pos_div + i] < r.integer[i])
		{
			--pos_div;
			break;
		}
	}

	result = *this;
	for (size_t pd = pos_div; pd != -1; --pd)//перемещение к началу числа
	{
		OperatorModCheckSub:
		//можно вычесть?
		size_t pos_divisible_ceck = pd + size_divider;
		size_t pos_devider_ceck = size_divider;
		size_t additional_digit = 0; //делимый кусок на 1 разряд больше делителя
		if (result[pd + size_divider + 1] == 0)
		{
			do
			{
				if (pos_divisible_ceck == -1)
					break;

				if (result.integer[pos_divisible_ceck] < r.integer[pos_devider_ceck])
					goto OperatorModEndSub;
				if (result.integer[pos_divisible_ceck] > r.integer[pos_devider_ceck])
					break;
				pos_divisible_ceck--;
				pos_devider_ceck--;
			} while (true);
		}
		else
		{
			additional_digit = 1;
		}

		//вычитание
		{
			uint8_t debt = 0;
			size_t pos_sub = pd;
			for (; pos_sub < pd + size_divider + 1; ++pos_sub)//вычетание делителя
			{
				uint8_t old = result.integer[pos_sub];
				result.integer[pos_sub] = result.integer[pos_sub] - r.integer[pos_sub - pd] - debt;
				debt = old < result.integer[pos_sub] || (old == result.integer[pos_sub] && r.integer[pos_sub - pd] != 0x00);
			}
			if (additional_digit)
				result.integer[pos_sub] = result.integer[pos_sub] - debt;

			goto OperatorModCheckSub;
		}

	OperatorModEndSub:
		{}
	}
	return result;
}

BigUInt& BigUInt::operator+=(const BigUInt& r)
{
	if (capacity < r.capacity)
		reserve(r.capacity);

	size_t counter = 0;
	uint16_t overflow = 0;


	//сложить
	while (capacity > counter || r.capacity > counter)
	{
		uint16_t rank = (uint16_t)operator[](counter) + (uint16_t)r[counter] + overflow;
		overflow = rank >> 8;

		integer[counter] = (uint8_t)rank;
		counter++;
	}

	if (overflow)
	{
		reserve(capacity + 1);
		integer[counter] = (uint8_t)overflow;
	}

	return *this;
}
BigUInt& BigUInt::operator>>=(uint64_t r)
{
	for (uint64_t ei = 0; ei < r; ++ei)
	{
		integer[0] >>= 1;
		for (size_t i = 1; i < capacity; ++i)
		{
			integer[i - 1] |= integer[i] << 7;
			integer[i] >>= 1;
		}
	}
	return *this;
}

bool BigUInt::operator<(const BigUInt& r) const
{
	size_t min_capacity = capacity;
	if (capacity > r.capacity)
	{
		min_capacity = r.capacity;
		for (size_t i = r.capacity; i < capacity; ++i)
			if (integer[i] != 0)
				return false;
	}
	else if (capacity < r.capacity)
	{
		min_capacity = r.capacity;
		for (size_t i = capacity; i < r.capacity; ++i)
			if (r.integer[i] != 0)
				return true;
	}

	for (size_t i = min_capacity - 1; i != -1; --i)
	{
		if (integer[i] < r.integer[i])
			return true;
		else if (integer[i] > r.integer[i])
			return false;
	}

	return false;
}
bool BigUInt::operator<=(const BigUInt& r) const
{
	size_t min_capacity = capacity;
	if (capacity > r.capacity)
	{
		min_capacity = r.capacity;
		for (size_t i = r.capacity; i < capacity; ++i)
			if (integer[i] != 0)
				return false;
	}
	else if (capacity < r.capacity)
	{
		min_capacity = r.capacity;
		for (size_t i = capacity; i < r.capacity; ++i)
			if (r.integer[i] != 0)
				return true;
	}

	for (size_t i = min_capacity - 1; i != -1; --i)
	{
		if (integer[i] < r.integer[i])
			return true;
		else if (integer[i] > r.integer[i])
			return false;
	}

	return true;
}
bool BigUInt::operator==(uint64_t r)const
{
	const uint8_t* b = reinterpret_cast<uint8_t*>(&r);
	size_t size = sizeof(uint64_t);
	if (size > capacity)
		size = capacity;
	for (size_t i = 0; i < size; ++i)
		if (integer[i] != b[i])
			return false;
	for (size_t i = sizeof(uint64_t); i < capacity; ++i)
		if (integer[i] != 0)
			return false;

	return true;
}
bool BigUInt::operator!=(uint64_t r)const
{
	const uint8_t* b = reinterpret_cast<uint8_t*>(&r);
	size_t size = sizeof(uint64_t);
	if (size > capacity)
		size = capacity;
	for (size_t i = 0; i < size; ++i)
		if (integer[i] != b[i])
			return true;
	for (size_t i = sizeof(uint64_t); i < capacity; ++i)
		if (integer[i] != 0)
			return true;

	return false;
}

BigUInt& BigUInt::operator=(uint64_t num)
{
	if (capacity < sizeof(num))
	{
		delete[] integer;
		integer = new uint8_t[sizeof(num)];
		capacity = sizeof(num);
		memcpy(integer, &num, sizeof(num));
		return *this;
	}

	memset(integer, 0, capacity);
	memcpy(integer, &num, sizeof(num));
	return *this;
}

void BigUInt::loadFromByteReversArray(const uint8_t* arr, size_t size)
{
	delete[] integer;
	integer = new uint8_t[size];
	capacity = size;
	memset(integer, 0, size);

	for (size_t i = 0; i < size; ++i)
	{
		integer[i] = arr[size - i - 1];
	}
}
BigUInt& BigUInt::operator=(const BigUInt& r)
{
	if (this == &r)
		return *this;

	delete[]integer;
	integer = new uint8_t[r.capacity];
	capacity = r.capacity;
	memcpy(integer, r.integer, capacity);
	return *this;
}
BigUInt& BigUInt::operator=(BigUInt&& r) noexcept
{
	if (this == &r)
		return *this;

	delete[] integer;
	integer = r.integer;
	r.integer = nullptr;
	capacity = r.capacity;
	return *this;
}
uint8_t BigUInt::operator[](size_t i) const
{
	if (i >= capacity)
		return 0;
	return integer[i];
}

void BigUInt::reserve(size_t capacity)
{
	if (this->capacity >= capacity)
		return;

	uint8_t* temp = new uint8_t[capacity];
	//memset(temp, 0, capacity);
	for (size_t i = 0; i < capacity; ++i)
		temp[i] = 0;
	memcpy(temp, integer, this->capacity);
	delete[] integer;
	integer = temp;
	this->capacity = capacity;
}

std::ostream& operator<<(std::ostream& out, const BigUInt& point)
{
	auto flags = out.flags();

	out << std::hex;
	for (size_t i = point.capacity - 1; i != -1; --i)
		out << std::setfill('0') << std::setw(2) << (uint16_t)point[i];
	out.flags(flags);
	return out;
}

//openssl x509 -inform der -in D:\Files\Tima\MyWinCert\GolovlevTimofeyTim_dev.cer -text -noout