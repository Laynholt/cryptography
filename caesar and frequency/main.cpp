#include "utils.hpp"

#include <sstream>

constexpr int16_t CAESAR_SHIFT = 1;

std::string caesar_encode(const std::string& source_data, int16_t shift)
{
	std::string encoded_data = source_data;

	// Encode source data
	for (auto& symbol : encoded_data)
	{
		if (is_letter(symbol))
		{
			char start_symbol = get_start_symbol_for(symbol);
			symbol = ((symbol - start_symbol + CAESAR_SHIFT) % ALPHABET_SIZE) + start_symbol;
		}
	}
	
	std::ofstream fout;

	fout.open("caesar_encoded.txt", std::ios_base::out);
	if (!fout.is_open())
	{
		std::cout << "Caesa\'s cipher with shift [" << CAESAR_SHIFT << "]:\n\n" << encoded_data << std::endl;
		return encoded_data;
	}

	fout << encoded_data;
	fout.close();

	return encoded_data;
}

void brute_force_decoding(const std::string& encoded_data)
{
	std::string decoded_data = encoded_data;

	// Try to decode for every shift
	for (int16_t shift = 0; shift < ALPHABET_SIZE; ++shift)
	{
		for (int16_t j = 0; j < decoded_data.size(); ++j)
		{
			if (is_letter(encoded_data[j]))
			{
				char start_symbol = get_start_symbol_for(encoded_data[j]);
				decoded_data[j] = ((encoded_data[j] - start_symbol - shift + ALPHABET_SIZE) % ALPHABET_SIZE) + start_symbol;
			}
		}

		std::string preview_data;
		
		// Create preview string
		if (decoded_data.size() > 30)
			preview_data = decoded_data.substr(0, 30);
		else
			preview_data = decoded_data;

		int16_t is_correct{ 0 };
		std::cout << "Its correct decoded data? [1/0]:\n\n" << preview_data << "\n\n>> ";
		std::cin >> is_correct;

		// If is it correct, then print and return
		if (is_correct)
		{
			std::ofstream fout;

			fout.open("brute_force_decoded_data.txt", std::ios_base::out);
			if (!fout.is_open())
			{
				std::cout << "Brute force decoded Caesa\'s cipher with shift [" << CAESAR_SHIFT << "]:\n\n" << decoded_data << std::endl;
				return;
			}

			fout << decoded_data;
			fout.close();
			return;
		}
	}
}

std::map<char, char> frequency_analysis_decoding(const std::string& encoded_data)
{
	// https://www3.nd.edu/~busiforc/handouts/cryptography/letterfrequencies.html
	std::map<double, char> standart_frequency = {
		{0.1300, 'e'}, {0.0910, 't'},
		{0.0812, 'a'}, {0.0768, 'o'},
		{0.0731, 'i'}, {0.0695, 'n'},
		{0.0628, 's'}, {0.0609, 'h'},
		{0.0599, 'r'}, {0.0425, 'd'},
		{0.0403, 'l'}, {0.0276, 'u'},
		{0.0251, 'c'}, {0.0223, 'm'},
		{0.0209, 'w'}, {0.0203, 'f'},
		{0.0182, 'g'}, {0.0178, 'y'},
		{0.0149, 'p'}, {0.0129, 'b'},
		{0.0098, 'v'}, {0.0077, 'k'},
		{0.00151, 'j'}, {0.0015, 'x'},
		{0.0010, 'q'}, {0.0007, 'z'}
	};

	bool is_frequency_auto_decode = false;
	bool is_frequency_caesar_decode = true;

	std::cout << "Do you want to decode yourself(0) or automatically(1)\n>> ";
	std::cin >> is_frequency_auto_decode;

	std::cout << "Will be decoding the Caesar code? [no - 0 / yes - 1]\n>> ";
	std::cin >> is_frequency_caesar_decode;
	std::cout << "\n" << std::endl;

	std::map<char, double> encoded_data_frequency;
	std::map<char, char> comparison_table;

	calculate_frequency(encoded_data_frequency, encoded_data);

	std::cout << "Mean frequency for English alphabet:" << std::endl;
	print_sorted_map_by_values(_reverse_map(standart_frequency));

	std::cout << "\nFrequency for encoded data:" << std::endl;
	print_sorted_map_by_values(encoded_data_frequency);

	// Decoding
	std::string decoded_data = encoded_data;
	
	if (is_frequency_auto_decode)
	{
		std::map<char, char> precomparison_table;

		// Create precomparison table
		for (auto& pair : encoded_data_frequency)
		{
			auto iterator = standart_frequency.lower_bound(pair.second);

			if (iterator == standart_frequency.end())
				iterator = std::prev(standart_frequency.end());

			char decoded_symbol = iterator->second;
			precomparison_table[pair.first] = decoded_symbol;
		}

		// If we decode caesar cipher
		if (is_frequency_caesar_decode)
		{
			std::map<uint16_t, uint16_t> letters_shifts;

			for (auto& pair : precomparison_table)
			{
				// Calculate shift encoded -> decoded;
				uint16_t shift = (pair.first - pair.second + ALPHABET_SIZE) % ALPHABET_SIZE;
				letters_shifts[shift] += 1;
			}

			// Decode with the shift that has the most letters
			auto supposed_shift = std::max_element(
				letters_shifts.begin(), letters_shifts.end(), [](const auto& pair1, const auto& pair2)
				{
					return pair1.second < pair2.second;
				}
			);
			std::cout << "\nSupposed shift: [" << supposed_shift->first << "]" << std::endl;

			// Remake comparison table
			for (auto& pair : precomparison_table)
				comparison_table[pair.first] = (pair.first - 'a' - supposed_shift->first + ALPHABET_SIZE) % ALPHABET_SIZE + 'a';
		}
		else
			comparison_table = precomparison_table;

		// Auto decoding
		for (auto& symbol : decoded_data)
		{
			if (is_letter(symbol))
			{
				char decoded_symbol = comparison_table[to_lower(symbol)];

				if (is_upper(symbol))
					decoded_symbol = to_upper(decoded_symbol);

				symbol = decoded_symbol;
			}
		}
	}

	// Self decoding
	else
	{
		std::stack<std::pair<char, char>> snapshots;
		bool _exit = false;
		char choice{};
		
		std::string show_data = (decoded_data.size() > 50) ? decoded_data.substr(0, SYMBOLS_IN_SHOW_DATA) : decoded_data;
		std::cout << "\nEncoded data:\n\n" << show_data << "\n" << std::endl;

		while (not _exit)
		{
			print_controls();
			std::cin >> choice;
			
			switch (choice)
			{
			case '0':
				_exit = true;
				std::cout << "Exit with " << snapshots.size() << " changes!\n" << std::endl;
				break;

			case 'c':
				{
					char symbol1{}, symbol2{};
					
					do
					{
						std::cout << "Enter which character you want to change [from]: ";
						std::cin >> symbol1;
					} while (not is_letter(symbol1));

					do
					{
						std::cout << "Enter what character you want to change [to]: ";
						std::cin >> symbol2;
					} while (not is_letter(symbol2));

					symbol1 = to_lower(symbol1);
					symbol2 = to_lower(symbol2);

					if (not is_frequency_caesar_decode)
					{ 
						for (auto& chr : decoded_data)
							if (to_lower(chr) == symbol1)
								chr = is_upper(chr) ? to_upper(symbol2) : symbol2;

						snapshots.push(std::make_pair(symbol1, symbol2));
						comparison_table[symbol1] = symbol2;

						show_data = (decoded_data.size() > 50) ? decoded_data.substr(0, SYMBOLS_IN_SHOW_DATA) : decoded_data;
						std::cout << "\nAfter [" << symbol1 << "] -> [" << symbol2 << "]:\n\n" << show_data << "\n" << std::endl;
					}
					else
					{
						uint16_t shift = (symbol1 - symbol2 + ALPHABET_SIZE) % ALPHABET_SIZE;
						for (auto& chr : decoded_data)
						{
							if (is_letter(chr))
							{
								char start_symbol = get_start_symbol_for(chr);
								char decoded_symbol = ((chr - start_symbol - shift + ALPHABET_SIZE) % ALPHABET_SIZE) + start_symbol;

								comparison_table[to_lower(chr)] = to_lower(decoded_symbol);
								chr = decoded_symbol;
							}
						}
						snapshots.push(std::make_pair(symbol1, symbol2));

						show_data = (decoded_data.size() > 50) ? decoded_data.substr(0, SYMBOLS_IN_SHOW_DATA) : decoded_data;
						std::cout << "\nIf we decode [" << symbol1 << "] -> [" << symbol2 << "] shift will be equal ["
							<< shift << "]:\n\n" << show_data << "\n" << std::endl;
					}
				}
				break;

			case 'b':
				if (not snapshots.empty())
				{
					roll_back_snapshot(snapshots, decoded_data, comparison_table, is_frequency_caesar_decode);
				}
				else
					std::cout << "You dont have a snapshot yet!\n" << std::endl;
				break;

			case 'r':
				decoded_data = encoded_data;
				comparison_table.clear();

				while (not snapshots.empty())
					snapshots.pop();
				
				show_data = (decoded_data.size() > 50) ? decoded_data.substr(0, SYMBOLS_IN_SHOW_DATA) : decoded_data;
				std::cout << "Data has been reset!\n\n" << show_data << std::endl;
				break;

			default:
				std::cout << "Unknown command entered!\n" << std::endl;
				break;
			}
		}
	}

	int16_t i = 0;
	std::cout << "\nComparison table [enc/dec]:" << std::endl;
	for (auto& pair : comparison_table)
	{
		std::cout << pair.first << " - " << pair.second << "\t";
		if ((i++ + 1) % 3 == 0)
			std::cout << std::endl;
	}
	std::cout << "\n";

	std::ofstream fout;

	fout.open("freq_decoded_data.txt", std::ios_base::out);
	if (!fout.is_open())
	{
		std::cout << "Frequency analysis decoded Caesa\'s cipher:\n\n" << decoded_data << std::endl;
		return comparison_table;
	}

	fout << decoded_data;
	fout.close();
	return comparison_table;
}

int main()
{
	std::ifstream fin;

	// Open source file
	fin.open("data.txt", std::ios_base::in);
	if (!fin.is_open())
	{
		std::cout << "Cant open source data file [data.txt]!" << std::endl;
		return -1;
	}

	// Read data from file
	std::string source_data;
	std::stringstream data;

	data << fin.rdbuf();
	source_data = data.str();

	// Close source file
	fin.close();

	// Work only with ASCII
	_remove_non_ascii(source_data);

	// Encode source data with Caesar's cipher 
	std::string encoded_data = caesar_encode(source_data, CAESAR_SHIFT);
	
	// Brute force decoding
	std::cout << ".--[Brute Force Decoding]--." << std::endl;
	brute_force_decoding(encoded_data);

	std::cout << "\n\n\n";

	// Frequency analysis decoding
	std::cout << ".--[Frequency analysis]--." << std::endl;
	std::map<char, char> comparison_table = frequency_analysis_decoding(encoded_data);

	print_correct_comparison_table(source_data, encoded_data, comparison_table);

	return 0;
}