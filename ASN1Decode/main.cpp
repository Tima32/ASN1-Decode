#include <Windows.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include "sha1_git/sha1.hpp"
#include "sha1_git2/sha1 002.h"
#include "sha256/SHA256.h"

#include "ASN1/Decode.hpp"
#include "BigUInt/BigUInt.hpp"

using namespace std;

extern void RSATest(const uint8_t* ab, const uint8_t* eb, const uint8_t* mb);

uint8_t* LoadFileToMemory(const wchar_t* name, size_t& data_size)
{
	FILE* file = 0;
	auto err = _wfopen_s(&file, name, L"rb");
	if (err != 0 || file == 0)
		return nullptr;

	fseek(file, 0, SEEK_END);

	fpos_t size;
	fgetpos(file, &size);
	data_size = size;
	fseek(file, 0, SEEK_SET);

	uint8_t* buff = new uint8_t[size];

	fread(buff, 1, size, file);
	fclose(file);
	return buff;
}
void PrintByteArray(const void* arr, size_t size)
{
	const uint8_t* byte = reinterpret_cast<const uint8_t*>(arr);

	for (size_t i = 0; i < size; i++)
		cout << std::hex << std::setfill('0') << std::setw(2) << (uint16_t)byte[i];
}

bool RSASignatureDecode(
	uint8_t* out, size_t out_len, 
	const uint8_t* signature, size_t signature_len,
	const uint8_t* exponent, size_t exponent_len,
	const uint8_t* modulus, size_t modulus_len)
{
	cout << "---------RSASignatureVerification----------" << endl;

	if (signature_len != modulus_len)
	{
		cout << "<RSASignatureVerification>Error: s_len != modulus_len" << endl;
		return false;
	}
	if (out_len < modulus_len)
	{
		cout << "<RSASignatureVerification>Error: Output buffer less than modulus" << endl;
		return false;
	}

	BigUInt s, e, m;

	s.loadFromByteReversArray(signature, signature_len);
	e.loadFromByteReversArray(exponent, exponent_len);
	//m.loadFromByteReversArray(--modulus, ++modulus_len);
	m.loadFromByteReversArray(modulus, modulus_len);

	//cout << "a: " << s << endl;
	//cout << "e: " << e << endl;
	//cout << "m: " << m << endl;

	BigUInt r = s.modPow(e, m);
	for (size_t i = 0; i < modulus_len; ++i)
	{
		out[i] = r.integer[modulus_len - 1 - i];
	}

	return true;
}

const uint8_t* LoadCertificate(const uint8_t** p_exponent, size_t& p_exponent_len, const uint8_t** p_modulus, size_t& p_modulus_len)
{
	//1. загрузка сертификата
	size_t data_size;
	uint8_t* data = LoadFileToMemory(L"GolovlevTimofeyTim_dev.cer", data_size);
	//uint8_t* data = LoadFileToMemory(L"OEM_Root_CA_2017.cer", data_size);
	//uint8_t* data = LoadFileToMemory(L"Private Trade Unitary Enterprise LST.cer", data_size);
	if (data == nullptr)
	{
		cout << "Error load file" << endl;
		delete[] data;
		return nullptr;
	}

	//2. Получение данных
	size_t digest_data_len = 0;
	const uint8_t* digest_data = nullptr;

	int64_t version = -1;

	size_t serial_number_len = 0;
	const uint8_t *serial_number = nullptr;

	size_t signature_algoritm_len = 0;
	const uint8_t* signature_algoritm = nullptr;

	constexpr size_t issuer_max_size = 256;
	wchar_t issuer[issuer_max_size] = {0};

	constexpr size_t not_before_max_size = 256;
	constexpr size_t not_after_max_size = 256;
	wchar_t not_before[not_before_max_size] = { 0 };
	wchar_t not_after[not_after_max_size] = { 0 };

	constexpr size_t subject_max_size = 256;
	wchar_t subject[subject_max_size] = { 0 };

	ASN1::ObjectIdentifierConst public_key_algorithm;

	size_t modulus_len = 0;
	const uint8_t* modulus = nullptr;

	size_t exponent_len = 0;
	const uint8_t* exponent = nullptr;

	ASN1::ObjectIdentifierConst digital_signature_algorithm;

	size_t digital_signature_len = 0;
	const uint8_t* digital_signature = nullptr;

	try
	{
		ASN1::Decode d;
		d.setData(data);
		digest_data = d[0].getBlock();
		digest_data_len = d[0].getBlockSize();
		auto sert_info = d[0];

		//2.1 Получение версии
		version = sert_info[0][0].getInt64();
		cout << "Sert version: " << version << endl;

		//2.2 Серийный номер
		serial_number = sert_info[1].getByteArray(serial_number_len);
		cout << "Serial number: ";
		PrintByteArray(serial_number, serial_number_len);
		cout << endl;

		//2.3 Алгоритм подписи
		signature_algoritm = sert_info[2][0].getByteArray(signature_algoritm_len);
		cout << "Signature algoritm: ";
		PrintByteArray(signature_algoritm, signature_algoritm_len);
		if (ASN1::ObjectIdentifierConst(signature_algoritm, signature_algoritm_len) == ASN1::ObjectIdentifierConst::sha256WithRSAEncryption)
			cout << " " << "sha256WithRSAEncryption";
		cout << endl;

		//2.4 Эмитент имя - Поиск только этого поля все остальные не интересны
		try{
			size_t i = 0;
			while (1)
			{
				size_t len;
				const uint8_t* object = sert_info[3][i][0][0].getByteArray(len);
				ASN1::ObjectIdentifierConst type(object, len);
				if (type == ASN1::ObjectIdentifierConst::CommonName)
				{
					wstring s = sert_info[3][i][0][1].getWstring();
					if (s.size() >= issuer_max_size)
						throw;
					if (wcscpy_s(issuer, s.c_str()))
						throw;
					break;
				}

				++i;
			}

			wcout << L"Issuer: " << issuer << endl;
		}
		catch (...)
		{
			cout << "Error find issuer" << endl;
		}

		//2.5 Время начала и окончания действия
		{
			wstring s = sert_info[4][0].getWstring();
			if (s.size() >= not_before_max_size)
				throw;
			if (wcscpy_s(not_before, s.c_str()))
				throw;

			s = sert_info[4][1].getWstring();
			if (s.size() >= not_after_max_size)
				throw;
			if (wcscpy_s(not_after, s.c_str()))
				throw;

			wcout << L"Not before: " << not_before << endl;
			wcout << L"Not after: " << not_after << endl;
		}

		//2.6 Получение имени субъекта
		try {
			size_t i = 0;
			while (1)
			{
				size_t len;
				const uint8_t* object = sert_info[5][i][0][0].getByteArray(len);
				ASN1::ObjectIdentifierConst type(object, len);
				if (type == ASN1::ObjectIdentifierConst::CommonName)
				{
					wstring s = sert_info[5][i][0][1].getWstring();
					if (s.size() >= subject_max_size)
						throw;
					if (wcscpy_s(subject, s.c_str()))
						throw;
					break;
				}

				++i;
			}

			wcout << L"Subject: " << subject << endl;
		}
		catch (...)
		{
			cout << "Error find issuer" << endl;
		}

		//2.7 Получение цифровой подписи
		{
			//2.7.1 получение алгоритма
			size_t len;
			const uint8_t* object = sert_info[6][0][0].getByteArray(len);
			public_key_algorithm.data = object;
			public_key_algorithm.size = len;

			cout << "Public key algorithm: ";
			PrintByteArray(object, len);
			if (public_key_algorithm == ASN1::ObjectIdentifierConst::rsaEncryption)
				cout << " " << "rsaEncryption";
			cout << endl;

			{
				size_t key_size;
				const uint8_t* key_b = sert_info[6][1].getByteArray(key_size);
				++key_b;
				--key_size;

				ASN1::Decode key;
				key.setData(key_b);

				//2.7.2 Получение модуля
				modulus = key[0].getByteArray(modulus_len);
				cout << "Modulus: ";
				PrintByteArray(++modulus, --modulus_len);
				cout << endl;

				//2.7.3 Получение степени
				exponent = key[1].getByteArray(exponent_len);
				cout << "Exponent ";
				PrintByteArray(exponent, exponent_len);
				cout << endl;
			}
		}

		//2.8 Получить подпись сертификатаы
		{
			//2.8.1 Алгоритм
			size_t len;
			const uint8_t* data = d[1][0].getByteArray(len);
			digital_signature_algorithm.data = data;
			digital_signature_algorithm.size = len;

			cout << "Digital signature algorithm: ";
			PrintByteArray(data, len);
			if (digital_signature_algorithm == ASN1::ObjectIdentifierConst::sha256WithRSAEncryption)
				cout << " sha256WithRSAEncryption";
			cout << endl;

			//2.8.2 Подпись
			digital_signature = d[2].getByteArray(digital_signature_len);
			cout << "Digital signature: ";
			PrintByteArray(++digital_signature, --digital_signature_len);
			cout << endl;
		}
	}
	catch (const ASN1::DecodeException& ex)
	{
		cout << ex.what() << " Error pars cert pos: " << ex.error_pos << endl;
		delete[] data;
		return nullptr;
	}
	catch (const std::exception& ex)
	{
		cout << ex.what() << endl;
		delete[] data;
		return nullptr;
	}

	//3 Проверка сертификата
	if (!(digital_signature_algorithm == ASN1::ObjectIdentifierConst::sha256WithRSAEncryption))
	{
		cout << "Only sha256WithRSAEncryption supported" << endl;
		delete[] data;
		return nullptr;
	}

	uint8_t* decode = new uint8_t[modulus_len];
	if (!RSASignatureDecode(decode, modulus_len, digital_signature, digital_signature_len, exponent, exponent_len, modulus, modulus_len))
	{
		cout << "RSA decode error" << endl;
		delete[] data;
		return nullptr;
	}

	cout << "RSA decoded: ";
	PrintByteArray(decode, modulus_len);
	cout << endl;

	SHA256 sha;
	sha.update(digest_data, digest_data_len);
	uint8_t* digest = sha.digest();
	std::cout << "sha256: " << SHA256::toString(digest) << std::endl;

	bool verify = true;
	for (size_t i = 0; i < 32; ++i)
	{
		if (decode[modulus_len - 1 - i] != digest[31 - i])
		{
			cout << "Digital signature mismatch." << endl;
			delete[] data;
			delete[] digest;
			return nullptr;
		}
	}

	cout << "The digital signature has been verified." << endl;

	delete[] digest; // Don't forget to free the digest!

	return data;
}
const uint8_t* LoadPECertificate()
{
	cout << "-------LoadPECertificate--------" << endl;

	return nullptr;
}

int main()
{
	const uint8_t* exponent, *modulus;
	size_t exponent_len, modulus_len;
	const uint8_t* data = LoadCertificate(&exponent, exponent_len, &modulus, modulus_len);
	if (data == nullptr)
	{
		cout << "Error in LoadCertificate" << endl;
		return -1;
	}

	LoadPECertificate();

	return 0;
}