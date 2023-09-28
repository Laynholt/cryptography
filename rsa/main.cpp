#include "RSA.hpp"

int main()
{
	nrsa::RSA rsa;

	if (not rsa.load_keys())
	{
		rsa.generate_keys();
		rsa.save_keys();
		rsa.show_keys();
	}
	
	rsa.encode("data.txt");
	rsa.decode();
}