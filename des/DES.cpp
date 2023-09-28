#include <iostream>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <random>

#include <bitset>

#include "DES.hpp"

bool ndes::DES::open_data_file(const char* const filepath)
{
	std::ifstream fin;

	fin.open(filepath);
	if (!fin.is_open())
	{
		std::cout << "Cannot open data file [" << filepath << "]!\n" << std::endl;
		return false;
	}

	// Read data from file
	std::stringstream data;
	data << fin.rdbuf();

	_source_data = data.str();
	
	fin.close();
	return true;
}

void ndes::DES::set_keyword(const std::string& keyword)
{
	_keyword = keyword;

	if (keyword.size() == DES_KEY_SIZE)
		return;
	else if (keyword.size() > DES_KEY_SIZE)
		_keyword = _keyword.substr(0, DES_KEY_SIZE);
	else
		_keyword += create_random_key().substr(0, DES_KEY_SIZE - _keyword.size());
}

std::string ndes::DES::create_random_key()
{
	std::string random_key;
	random_key.reserve(DES_KEY_SIZE);

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	std::mt19937 dist(DES_KEY_RANDOM_SEED);

	for (int16_t i = 0; i < DES_KEY_SIZE; ++i) 
		random_key += alphanum[dist() % (sizeof(alphanum) - 1)];

	_keyword = random_key;
	return random_key;
}

void ndes::DES::encode()
{
	_padding_counter = 0;
	_encrypt(DES_ENCODE);
	_write_result(DES_ENCODE);
	_reset_data();
}

void ndes::DES::decode()
{
	_encrypt(DES_DECODE);
	_write_result(DES_DECODE);
	_reset_data();
}

std::string ndes::DES::_ascii_to_bin(const std::string& str)
{
	std::string bin;
	bin.reserve(str.size() * 8);

	for (auto chr : str)
		bin += std::bitset<8>(chr).to_string();
	return bin;
}

std::string ndes::DES::_bin_to_ascii(const std::string& bin)
{
	std::string str;
	str.reserve(bin.size() / 8);

	for (int32_t i = 0; i < bin.size(); i += 8)
		str += static_cast<char>(std::bitset<8>(bin.substr(i, 8)).to_ulong());
	return str;
}

void ndes::DES::_remove_non_ascii(std::string& source_data)
{
	std::string ascii_string;
	ascii_string.reserve(source_data.size());

	for (auto chr : source_data)
		if (chr >= 0 and chr < 128)
			ascii_string += chr;

	ascii_string.shrink_to_fit();
	source_data = ascii_string;
}

std::string ndes::DES::_int_to_bin(int16_t value)
{
	return std::bitset<4>(value).to_string();
}

int16_t ndes::DES::_bin_to_int(const std::string& bin)
{
	return std::stoi(bin);
}

std::string ndes::DES::_bin_xor(const std::string& bin1, const std::string& bin2)
{
	std::string bin(bin1.size(), '0');

	for (int16_t i = 0; i < bin1.size(); ++i)
		bin[i] = (bin1[i] == bin2[i]) ? '0' : '1';
	return bin;
}

void ndes::DES::_add_padding(std::string& str)
{
	while ((str.size() % 8) != 0)
	{
		str += _padding_symbol;
		++_padding_counter;
	}
}

void ndes::DES::_remove_padding(std::string& str)
{
	if (!str.size())
		return;


	str = str.substr(0, str.size() - _padding_counter);
}

std::string ndes::DES::_permutate(const std::string& binary_str, std::vector<data_type> permutation_table)
{
	std::string permuted_str;
	permuted_str.reserve(permutation_table.size());

	for (int16_t i = 0; i < permutation_table.size(); ++i)
		permuted_str += binary_str[permutation_table[i]];
	return permuted_str;
}

void ndes::DES::_make_cyclic_shift(std::string& binary_str, int16_t shift_size)
{
	std::rotate(binary_str.begin(), binary_str.begin() + shift_size, binary_str.end());
}

void ndes::DES::_reset_data()
{
	_source_data.clear();
	_result_data.clear();
	_keys_n.clear();
}

void ndes::DES::_write_result(int16_t crypt_type)
{
	std::ofstream fout;
	std::string filepath = (crypt_type == DES_ENCODE) ? DES_ENCODED_OUTPUT : DES_DECODED_OUTPUT;
	std::string type = (crypt_type == DES_ENCODE) ? "encrypting" : "decrypting";

	fout.open(filepath);
	if (!fout.is_open())
	{
		std::cout << "Cannot open file [" << filepath << "] to write [" << type << "] results!\nOutput here:\n" << std::endl;
		std::cout << _result_data << std::endl;
		return;
	}
	fout << _result_data;
	fout.close();
	std::cout << "End " << type << " using key [" << _keyword << "].\nInput size is [" << _result_data.size() << "] byte(-es)." << std::endl;
	std::cout << "Output file is [" << filepath << "]\n" << std::endl;
}

void ndes::DES::_print_init(int16_t crypt_type)
{
	std::string type = (crypt_type == DES_ENCODE) ? "encrypting" : "decrypting";
	int32_t size = (crypt_type == DES_ENCODE) ? _padding_counter : 0;
	std::cout << "Start " << type << " using key [" << _keyword << "].\nInput size is [" << _source_data.size() - size << "] byte(-es).\n" << std::endl;
}

void ndes::DES::_encrypt(int16_t crypt_type)
{
	if (_keyword.empty())
		create_random_key();
	
	if (!_source_data.size())
	{
		if (crypt_type == DES_ENCODE)
		{
			std::cout << "You need to specify source data file!\n" << std::endl;
			return;
		}

		if (!open_data_file(DES_ENCODED_OUTPUT))
			return;

		if (!_source_data.size())
			return;
	}

	if ((_source_data.size() % 8) != 0)
	{
		if (crypt_type == DES_DECODE)
		{
			std::cout << "Cannot decode data, because, invalid data length, data must be a multiple of 8 bytes!" << std::endl;
			return;
		}

		// Else is encoder, so we need to add paddings
		_add_padding(_source_data);
	}

	// Work ONLY with ASCII
	if (crypt_type == DES_ENCODE)
		_remove_non_ascii(_source_data);

	_print_init(crypt_type);
	_create_sub_keys();

	int16_t block_size = (crypt_type == DES_ENCODE) ? 8 : 64;
	std::string left_subblock, right_subblock;

	// Main part of the encryption algorithm
	for (int32_t k = 0; k < _source_data.size(); k += block_size)
	{
		std::string block_of_data = (crypt_type == DES_ENCODE) ? _ascii_to_bin(_source_data.substr(k, block_size)) : _source_data.substr(k, block_size);
		block_of_data = _permutate(block_of_data, _initial_permutation);

		left_subblock = block_of_data.substr(0, 32);
		right_subblock = block_of_data.substr(32, 32);

		// Encryption starts from Kn[1] through to Kn[16]
		int16_t iteration{ 0 }, iteration_adjustment{ 1 };

		// Decryption starts from Kn[16] down to Kn[1]
		if (crypt_type == DES_DECODE)
		{
			iteration = 15;
			iteration_adjustment = -1;
		}

		std::string temp_right_subkey;
		std::vector<std::string> mini_blocks;
		std::string bn;

		for (int16_t i = 0; i < 16; ++i, iteration += iteration_adjustment)
		{
			bn.clear();
			mini_blocks.clear();

			// Make a copy of R[i - 1], this will later become L[i]
			temp_right_subkey = right_subblock;

			// Permutate R[i - 1] to start creating R[i]
			right_subblock = _permutate(right_subblock, _expansion_table);

			right_subblock = _bin_xor(right_subblock, _keys_n[iteration]);

			// Create Bi
			for (int16_t j = 0; j < 8; ++j)
				mini_blocks.push_back(right_subblock.substr(6 * j, 6));

			// Permutate B[1] to B[8] using the S - Boxes
			for (int16_t j = 0; j < 8; ++j)
			{

				// Work out the offsets
				int16_t row{}, col{};

				for (int16_t ind = 0, sh = 1; ind < 6; ind += 5, --sh)
					row += (static_cast<int16_t>(mini_blocks[j][ind] - '0') << sh);

				for (int16_t ind = 1, sh = 3; ind < 5; ++ind, --sh)
					col += (static_cast<int16_t>(mini_blocks[j][ind] - '0') << sh);

				// Find the permutation value
				int16_t value = _sbox[j][row * 16 + col];

				bn += _int_to_bin(value);
			}

			// Permutate the concatination of B[1] to B[8] (Bn)
			right_subblock = _permutate(bn, _permutation2);

			// Xor with L[i - 1]
			right_subblock = _bin_xor(right_subblock, left_subblock);

			// L[i] becomes R[i - 1]
			left_subblock = temp_right_subkey;
		}

		// Final permutation of R[16]L[16]
		_result_data += _permutate(right_subblock + left_subblock, _final_permutation);
	}

	if (crypt_type == DES_DECODE)
	{
		_result_data = _bin_to_ascii(_result_data);
		_remove_padding(_result_data);
	}
}

void ndes::DES::_create_sub_keys()
{
	std::string temp_key = _permutate(_ascii_to_bin(_keyword), _permuted_choice_key1);

	std::string left_subkey = temp_key.substr(0, DES_KEY_BINSIZE / 2);
	std::string right_subkey = temp_key.substr(DES_KEY_BINSIZE / 2, DES_KEY_BINSIZE / 2);

	for (int16_t i = 0; i < 16; ++i)
	{
		_make_cyclic_shift(left_subkey, _cyclical_shifts[i]);
		_make_cyclic_shift(right_subkey, _cyclical_shifts[i]);

		_keys_n.push_back(_permutate(left_subkey + right_subkey, _permuted_choice_key2));
	}
}


