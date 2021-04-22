#include <iostream>

#include "Decode.hpp"

using std::cout;
using std::wcout;
using std::endl;

namespace ASN1
{
	//----------------[ObjectIdentifier]---------------//
	ObjectIdentifierConst::ObjectIdentifierConst(const uint8_t* data, size_t size):
		size(size), data(data)
	{}
	bool ObjectIdentifierConst::operator==(const ObjectIdentifierConst& r) const
	{
		if (size != r.size)
			return false;
		for (size_t i = 0; i < size; i++)
		{
			if (data[i] != r.data[i])
				return false;
		}
		return true;
	}

	const uint8_t sha256WithRSAEncryption_data[] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B };
	const ObjectIdentifierConst ObjectIdentifierConst::sha256WithRSAEncryption(sha256WithRSAEncryption_data, 9);
	const uint8_t sha256_data[] = { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01 };
	const ObjectIdentifierConst ObjectIdentifierConst::sha256(sha256_data, 9);
	const uint8_t common_name_data[] = {0x55, 0x04, 0x03};
	const ObjectIdentifierConst ObjectIdentifierConst::CommonName(common_name_data, 3);
	const uint8_t rsa_encryption_data[] = { 0x2A, 0x86, 0x48, 0x86,  0xF7, 0x0D, 0x01, 0x01, 0x01 };
	const ObjectIdentifierConst ObjectIdentifierConst::rsaEncryption(rsa_encryption_data, 9);



	//DecodeException
	DecodeException::DecodeException(const char* message, ErrorCode ec, size_t error_pos):
		std::exception(message), error(ec), error_pos(error_pos)
	{}


	//DecodeBlock
	DecodeBlock::DecodeBlock():
		data_begin(nullptr), block_begin(nullptr), length(0), data_offset(0)
	{}

	Class DecodeBlock::getClass() const
	{
		return static_cast<Class>(block_begin[0] & 0b11000000);
	}
	Type DecodeBlock::getType() const
	{
		return static_cast<Type>(block_begin[0] & 0b00100000);
	}
	Tag DecodeBlock::getTag() const
	{
		return static_cast<Tag>(block_begin[0] & 0b00011111);
	}

	size_t DecodeBlock::getLength() const
	{
		return length;
	}
	size_t DecodeBlock::getSize() const
	{
		return 0;
	}
	DecodeBlock DecodeBlock::operator[](size_t num) const
	{
		const uint8_t* block_begin = this->block_begin + data_offset;

		Type type = getType();
		if (type != Type::Constructed)
			throw DecodeException("The object contains no other objects.", DecodeException::ErrorCode::ThisObjectIsFinal, block_begin - data_begin);

		while(1)
		{
			if (block_begin > this->block_begin + getBlockSize())
				throw DecodeException("Out of range.", DecodeException::ErrorCode::OutOfRange, block_begin - data_begin);

			DecodeBlock d_obj;
			d_obj.data_begin = data_begin;
			d_obj.block_begin = block_begin;
			d_obj.init();

			if (num == 0)
				return d_obj;

			num--;
			block_begin += d_obj.getBlockSize();
		}
	}

	const uint8_t* DecodeBlock::getBlock() const
	{
		return block_begin;
	}
	size_t DecodeBlock::getBlockSize()const
	{
		return data_offset + length;
	}

	//data tools
	int64_t DecodeBlock::getInt64() const
	{
		constexpr size_t type_size = sizeof(int64_t);

		//Checking the correctness of the type.
		if (getClass() != Class::Universal || getType() != Type::Primitive || getTag() != Tag::INTEGER)
			throw DecodeException("Not a suitable type for conversion.", DecodeException::ErrorCode::HotSutableTypeForConvertation, block_begin - data_begin);

		if (length > type_size)
			throw DecodeException("Error type convertation.", DecodeException::ErrorCode::ErrorTypeConvertation, block_begin - data_begin);

		int64_t integer = 0;
		if (*(block_begin + data_offset) & 1 << 7)
			integer = -1;

		uint8_t* integer_byte = reinterpret_cast<uint8_t*>(&integer);
		for (size_t i = 0; i < length; i++)
			integer_byte[type_size - 1 - i - (type_size - length)] = (block_begin + data_offset)[i];

		return integer;
	}
	std::string DecodeBlock::getString() const
	{
		if (getClass() != Class::Universal || getType() != Type::Primitive || getTag() != Tag::UTCTime)
			throw DecodeException("Not a suitable type for conversion.", DecodeException::ErrorCode::HotSutableTypeForConvertation, block_begin - data_begin);

		std::string str;
		for (size_t i = 0; i < length; i++)
		{
			str += static_cast<char>((block_begin + data_offset)[i]);
		}

		return str;
	}
	std::wstring DecodeBlock::getWstring()const
	{
		if (getClass() != Class::Universal || getType() != Type::Primitive || !(getTag() == Tag::BMPString || getTag() == Tag::PrintableString || getTag() == Tag::UTCTime))
			throw DecodeException("Not a suitable type for conversion.", DecodeException::ErrorCode::HotSutableTypeForConvertation, block_begin - data_begin);

		if (getTag() == Tag::BMPString && length % 2)
			throw DecodeException("Error type convertation.", DecodeException::ErrorCode::ErrorTypeConvertation, block_begin - data_begin);

		std::wstring str;
		if (getTag() == Tag::BMPString)
		{
			for (size_t i = 0; i < length; i += 2)
			{
				wchar_t c = 0;
				c += (block_begin + data_offset)[i] << 8;
				c += (block_begin + data_offset)[i + 1];
				str += c;
			}
		}
		else if (getTag() == Tag::PrintableString || getTag() == Tag::UTCTime)
		{
			for (size_t i = 0; i < length; ++i)
			{
				wchar_t c = 0;
				c = (block_begin + data_offset)[i];
				str += c;
			}
		}

		return str;
	}
	const uint8_t* DecodeBlock::getByteArray(size_t& size) const
	{
		size = length;
		return block_begin + data_offset;
	}

	//protected
	void DecodeBlock::init()
	{
		size_t length_size = decodeLength();
		data_offset = 1 + length_size;
	}

	//private
	size_t DecodeBlock::decodeLength()
	{
		if ((block_begin[1] & 1 << 7) == 0)
		{
			length = block_begin[1];
			data_offset = 2;
			return 1;
		}

		rsize_t length_size = (block_begin[1] & 0b01111111);
		for (size_t i = 0; i < length_size; i++)
		{
			length <<= 8;
			length += block_begin[2 + i];
		}
		return 1 + length_size;
	}

	//Decode
	void Decode::setData(const uint8_t* data)
	{
		this->data_begin = data;
		this->block_begin = data;
		init();
	}
}