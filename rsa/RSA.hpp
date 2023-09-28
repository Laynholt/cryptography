#pragma once

#include <random>

namespace nrsa
{
	const char* const RSA_PUBLIC_KEY_FILENAME = "key_pub.txt";
	const char* const RSA_PRIVATE_KEY_FILENAME = "key.txt";

	const char* const RSA_ENCODED_DATA_FILENAME = "encoded_data.txt";
	const char* const RSA_DECODED_DATA_FILENAME = "decoded_data.txt";


	class RSA
	{
	public:
		RSA();
		RSA(uint64_t seed);

		void set_seed(uint64_t seed) { _seed = seed; _mt64.seed(seed); }
		
		void generate_keys();
		void show_keys();
		bool save_keys(const char* filename = "key");
		bool load_keys(const char* filename_pub = RSA_PUBLIC_KEY_FILENAME, const char* filename_priv = RSA_PRIVATE_KEY_FILENAME);

		bool is_prime_num(uint64_t val);

		bool encode(const char* filename);
		bool decode(const char* filename = RSA_ENCODED_DATA_FILENAME);

	private:
		void _init();
		uint64_t _get_prime_num(uint64_t _mod = 65536);
		
		bool _miller_rabin_prime(uint64_t val, int16_t iterations = 5);
		
		uint64_t _pow_mod(uint64_t base, uint64_t exp, uint64_t modulus);
		uint64_t _multiply_mod1(uint64_t val1, uint64_t val2, uint64_t modulus);
		uint64_t _multiply_mod2(uint64_t val1, uint64_t val2, uint64_t modulus);
		uint64_t _add_mod(uint64_t val1, uint64_t val2, uint64_t modulus);

		uint64_t _rand();
		uint64_t _rand(uint64_t _left, uint64_t _right);

		uint64_t _calc_e(uint64_t totient, uint64_t p, uint64_t q);
		uint64_t _calc_d(uint64_t totient, uint64_t e);

		uint64_t _gcd(uint64_t a, uint64_t b);
		uint64_t _ext_gcd(uint64_t u, uint64_t v);

	private:
		bool _save_key(std::string filename, bool is_private);
		bool _load_key(const std::string& filename, bool is_private);

		bool _read_file_data(const std::string& filename, std::string& read_data, bool is_decode = false);
		std::string _remove_non_ascii(const std::string& source_data);

		bool _encode_data(const std::string& source_data);
		bool _decode_data(const std::string& encoded_data);

	private:
		uint64_t _modulus{};
		uint64_t _open_exp{};
		uint64_t _secret_exp{};

	private:
		uint64_t _seed{};
		std::mt19937_64 _mt64;
	};
}