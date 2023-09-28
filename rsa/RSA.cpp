#include "RSA.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

nrsa::RSA::RSA()
{
	_init();
}

nrsa::RSA::RSA(uint64_t seed) : _seed(seed)
{
	_init();
}

void nrsa::RSA::generate_keys()
{
	uint64_t p{}, q{}, totient{};

	p = _get_prime_num();
	q = _get_prime_num();
	std::cout << "p - " << p << std::endl;
	std::cout << "q - " << q << std::endl;

	while (q == p)
		q = _get_prime_num();

	_modulus = p * q;
	
	totient = (p - 1) * (q - 1);
	std::cout << "phi(n) - " << totient << std::endl;

	_open_exp = _calc_e(totient, p, q);
	_secret_exp = _calc_d(totient, _open_exp);
}

void nrsa::RSA::show_keys()
{
	std::cout << "Public key [e n]: (" << _open_exp << " " << _modulus << ")" << std::endl;
	std::cout << "Private key [d n]: (" << _secret_exp << " " << _modulus << ")\n" << std::endl;
}

bool nrsa::RSA::save_keys(const char* filename)
{
	if (not _save_key(filename, false))
		return false;

	if (not _save_key(filename, true))
		return false;

	return true;
}

bool nrsa::RSA::load_keys(const char* filename_pub, const char* filename_priv)
{
	if (not _load_key(filename_pub, false))
		return false;

	if (not _load_key(filename_priv, true))
		return false;

	show_keys();

	return true;
}

void nrsa::RSA::_init()
{
	if (_seed)
		_mt64.seed(_seed);
	else
	{
		std::random_device rd;
		_mt64.seed(rd());
	}
}

uint64_t nrsa::RSA::_get_prime_num(uint64_t _mod)
{
	uint64_t val{};
	
	do
	{
		val = _rand() % _mod;
	} while (not is_prime_num(val));

	return val;
}

bool nrsa::RSA::is_prime_num(uint64_t val)
{
	return _miller_rabin_prime(val);
}

bool nrsa::RSA::encode(const char* filename)
{
	std::string source_data;

	if (_secret_exp == 0 or _modulus == 0)
	{
		generate_keys();
		save_keys();
	}

	if (not _read_file_data(filename, source_data))
		return false;

	return _encode_data(_remove_non_ascii(source_data));
}

bool nrsa::RSA::decode(const char* filename)
{
	std::string encoded_data;

	if (_open_exp == 0 or _modulus == 0)
	{
		std::cout << "Firstly, you need to load your keys!\n" << std::endl;
		return false;
	}

	if (not _read_file_data(filename, encoded_data, true))
		return false;

	return _decode_data(encoded_data);
}

bool nrsa::RSA::_miller_rabin_prime(uint64_t val, int16_t iterations)
{
	if (val < 2) return false;
	if (val == 2) return true;
	if (not (val & 1)) return false;
	
	// Now our num is odd number greater than 2
	uint64_t s = 0;
	uint64_t d = val - 1;
	
	while (not (d & 1)) 
	{
		++s;
		d >>= 1;
	}
	
	// Now take a random integer between [2, val) as a
	for (int16_t i = 0; i <= iterations; ++i) 
	{
		uint64_t a = _rand(2, val - 1);
		uint64_t x = _pow_mod(a, d, val);
		
		if (x == 1 || x == (val - 1)) 
			continue;
		
		int16_t j{};
		for (j = 0; j < s; ++j) 
		{
			x = _multiply_mod1(x, x, val);
			if (x == (val - 1))
				break;
		}

		// val is composite
		if (j == s)
			return false;
	}

	// val is probable prime
	return true;
}

uint64_t nrsa::RSA::_pow_mod(uint64_t base, uint64_t exp, uint64_t modulus)
{
	// https://stackoverflow.com/a/8498251
	base %= modulus;

	uint64_t result = 1;
	while (exp > 0) 
	{
		if (exp & 1) result = _multiply_mod1(result, base, modulus);
			base = _multiply_mod1(base, base, modulus);
		exp >>= 1;
	}
	return result;
}

uint64_t nrsa::RSA::_multiply_mod1(uint64_t val1, uint64_t val2, uint64_t modulus)
{
	// https://stackoverflow.com/a/18680280

	uint64_t res = 0;
	uint64_t temp_val2;

	/* Only needed if val2 may be >= modulus */
	if (val2 >= modulus) 
	{
		if (modulus > UINT64_MAX / 2u)
			val2 -= modulus;
		else
			val2 %= modulus;
	}

	while (val1 != 0) 
	{
		if (val1 & 1) 
		{
			/* Add val2 to res, modulus, without overflow */
			/* Equiv to if (res + val2 >= modulus), without overflow */
			if (val2 >= modulus - res)
				res -= modulus;
			res += val2;
		}
		val1 >>= 1;

		/* Double val2, modulo modulus */
		temp_val2 = val2;

		/* Equiv to if (2 * val2 >= modulus), without overflow */
		if (val2 >= modulus - val2)
			temp_val2 -= modulus;
		val2 += temp_val2;
	}
	return res;
}

uint64_t nrsa::RSA::_multiply_mod2(uint64_t val1, uint64_t val2, uint64_t modulus)
{
	// https://www.rosettacode.org/wiki/Miller%E2%80%93Rabin_primality_test#C++

	uint64_t am = (val1 < modulus) ? val1 : val1 % modulus;
	uint64_t bm = (val2 < modulus) ? val2 : val2 % modulus;
	if (bm > am) {
		std::swap(am, bm);
	}
	uint64_t result{};
	while (bm > 0) {
		if ((bm & 1) == 1) {
			result = _add_mod(result, am, modulus);
		}
		am = (am << 1) - (am >= (modulus - am) ? modulus : 0);
		bm >>= 1;
	}
	return result;
}

uint64_t nrsa::RSA::_add_mod(uint64_t val1, uint64_t val2, uint64_t modulus)
{
	uint64_t am = (val1 < modulus) ? val1 : val1 % modulus;
	if (val2 == 0) {
		return am;
	}
	uint64_t bm = (val2 < modulus) ? val2 : val2 % modulus;
	uint64_t b_from_m = modulus - bm;
	if (am >= b_from_m) {
		return am - b_from_m;
	}
	return am + bm;
}

uint64_t nrsa::RSA::_rand()
{
	return _mt64();
}

uint64_t nrsa::RSA::_rand(uint64_t _left, uint64_t _right)
{
	std::uniform_int_distribution<uint64_t> dist(_left, _right);
	return dist(_mt64);
}

uint64_t nrsa::RSA::_calc_e(uint64_t totient, uint64_t p, uint64_t q)
{
	uint64_t e = _rand(std::max(p, q), totient - 1);

	while (_gcd(e, totient) != 1)
		e = _rand(std::max(p, q), totient - 1);

	//for (; e < totient; ++e)
	//	if (_gcd(e, totient) == 1)
	//		break;
	return e;
}

uint64_t nrsa::RSA::_calc_d(uint64_t totient, uint64_t e)
{
	return _ext_gcd(e, totient);
}

uint64_t nrsa::RSA::_gcd(uint64_t a, uint64_t b)
{
	while (b) 
	{
		a %= b;
		std::swap(a, b);
	}
	return a;
}

uint64_t nrsa::RSA::_ext_gcd(uint64_t u, uint64_t v)
{
	// https://stackoverflow.com/a/27736785

	uint64_t inv, u1, u3, v1, v3, t1, t3, q;
	int64_t iter;
	u1 = 1;
	u3 = u;
	v1 = 0;
	v3 = v;
	iter = 1;
	while (v3 != 0)
	{
		q = u3 / v3;
		t3 = u3 % v3;
		t1 = u1 + q * v1;
		u1 = v1;
		v1 = t1;
		u3 = v3;
		v3 = t3;
		iter = -iter;
	}
	if (u3 != 1)
		return  0;
	if (iter < 0)
		inv = v - u1;
	else
		inv = u1;
	return inv;
}

bool nrsa::RSA::_save_key(std::string filename, bool is_private)
{
	std::ofstream fout;

	if (not is_private)
		filename += "_pub";

	fout.open(filename + ".txt");
	if (not fout.is_open())
	{
		std::cout << "Cannot open file [" << filename << "] to save!\n" << std::endl;
		return false;
	}
	fout << (is_private ? _secret_exp : _open_exp) << " " << _modulus;
	fout.close();

	return true;
}

bool nrsa::RSA::_load_key(const std::string& filename, bool is_private)
{
	std::ifstream fin;

	fin.open(filename);
	if (not fin.is_open())
	{
		std::cout << "Cannot open file [" << filename << "] to load!\n" << std::endl;
		return false;
	}
	fin >> (is_private ? _secret_exp : _open_exp) >> _modulus;
	fin.close();

	return true;
}

bool nrsa::RSA::_read_file_data(const std::string& filename, std::string& read_data, bool is_decode)
{
	std::ifstream fin;

	fin.open(filename);
	if (not fin.is_open())
	{
		std::cout << "Cannot open file [" << filename << "] to " << (is_decode ? "decode" : "encode") << "!\n" << std::endl;
		return false;
	}

	std::stringstream ss;
	ss << fin.rdbuf();
	fin.close();

	read_data = ss.str();

	return true;
}

std::string nrsa::RSA::_remove_non_ascii(const std::string& source_data)
{
	std::string ascii_string;
	ascii_string.reserve(source_data.size());

	for (auto chr : source_data)
		if (chr >= 0 and chr < 128)
			ascii_string += chr;

	ascii_string.shrink_to_fit();
	return ascii_string;
}

bool nrsa::RSA::_encode_data(const std::string& source_data)
{
	std::vector<uint64_t> encoded_data(source_data.size());

	for (size_t i = 0; i < source_data.size(); ++i)
		encoded_data[i] = _pow_mod(source_data[i], _secret_exp, _modulus);

	std::ofstream fout;
	
	fout.open(RSA_ENCODED_DATA_FILENAME);
	if (not fout.is_open())
	{
		std::cout << "Cannot open file [" << RSA_ENCODED_DATA_FILENAME << "] to write!\n" << std::endl;

		for (size_t i = 0; i < source_data.size(); ++i)
			std::cout << encoded_data[i] << " ";
		std::cout << std::endl;
		return false;
	}
	std::copy(encoded_data.begin(), encoded_data.end(), std::ostream_iterator<uint64_t>(fout, " "));
	fout.close();

	std::cout << "Source data was encoded and written to file [" << RSA_ENCODED_DATA_FILENAME << "].\n" << std::endl;
}

bool nrsa::RSA::_decode_data(const std::string& encoded_data)
{
	std::stringstream ss(encoded_data);
	std::istream_iterator<std::string> begin(ss);
	std::istream_iterator<std::string> end;

	std::vector<std::string> _encoded_data(begin, end);
	std::string decoded_data;

	decoded_data.reserve(_encoded_data.size());
	for (auto& encoded_symbol : _encoded_data)
		decoded_data += static_cast<char>(_pow_mod(std::stoull(encoded_symbol), _open_exp, _modulus));

	std::ofstream fout;

	fout.open(RSA_DECODED_DATA_FILENAME);
	if (not fout.is_open())
	{
		std::cout << "Cannot open file [" << RSA_DECODED_DATA_FILENAME << "] to write!\n" << std::endl;

		std::cout << decoded_data << "\n" << std::endl;
		return false;
	}
	fout << decoded_data;
	fout.close();

	std::cout << "Encoded data was decoded and written to file [" << RSA_DECODED_DATA_FILENAME << "].\n" << std::endl;
	return true;
}
