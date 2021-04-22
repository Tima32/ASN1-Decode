#pragma once

namespace ASN1
{
	/*
	* 87   |6   |54321
	* Klass|Type|Teg
	*/
	enum class Class : uint8_t
	{
		Universal =       0,
		Application =     0b01000000,
		ContextSpecific = 0b10000000,
		Private =         0b11000000
	};
	enum class Type : uint8_t
	{
		Primitive = 0,
		Constructed = 0b00100000
	};
	enum class Tag : uint8_t
	{
		INTEGER =  0x2,
		OCTETSTRING = 0x4,
		SEQUENCE = 0x10,
		PrintableString = 0x13,
		BMPString = 0x1E,
		UTCTime = 0x17
	};
}