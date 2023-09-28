#include "DES.hpp"

int main()
{
	ndes::DES des;	
	if (des.open_data_file("data.txt"))
	{
		des.set_keyword("new_keyword");
		des.encode();
		des.decode();
	}
}