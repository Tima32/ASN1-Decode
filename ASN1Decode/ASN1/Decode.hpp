#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include "DataType.hpp"

namespace ASN1
{
	struct ObjectIdentifierConst
	{
		ObjectIdentifierConst() = default;
		ObjectIdentifierConst(const uint8_t* data, size_t size);

		size_t size;
		const uint8_t* data;

		bool operator==(const ObjectIdentifierConst& r) const;

		static const ObjectIdentifierConst sha256WithRSAEncryption;
		static const ObjectIdentifierConst sha256;
		static const ObjectIdentifierConst CommonName;
		static const ObjectIdentifierConst rsaEncryption;
	};

	class DecodeException : public std::exception
	{
	public:
		enum class ErrorCode : uint8_t
		{
			NA,
			BlockIDNOtSupported,
			BadPermittedConstruction,
			ThisObjectIsFinal,
			HotSutableTypeForConvertation,
			ErrorTypeConvertation,
			OutOfRange
		};

		DecodeException(const char* message, ErrorCode ec, size_t error_pos);

		ErrorCode error;
		size_t error_pos;
	};

	class DecodeBlock
	{
	public:
		DecodeBlock();


		Class getClass() const;  //Класс
		Type getType() const;    //Тип
		Tag getTag() const;      //Тег блока

		size_t getLength() const;//Длинна данных в блоке
		size_t getSize() const;  //Колличество элементов в блоке (только для структур)
		DecodeBlock operator[](size_t num) const;//Получить елемент (только для структур)

		const uint8_t* getBlock() const;
		size_t getBlockSize()const;//Размер блока

		//data tools
		int64_t getInt64() const;
		std::string getString() const;
		std::wstring getWstring()const;
		const uint8_t* getByteArray(size_t& size) const; // OUT, IN

	protected:
		void init();
		const uint8_t* data_begin;
		const uint8_t* block_begin;
		size_t length;
		size_t data_offset;

	private:
		size_t decodeLength();
	};

	class Decode: public DecodeBlock
	{
	public:
		void setData(const uint8_t* data);
	private:
	};
}