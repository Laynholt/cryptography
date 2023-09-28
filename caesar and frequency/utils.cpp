#include "utils.hpp"

bool is_letter(char symbol)
{
	if ((symbol >= 'A' and symbol <= 'Z') or (symbol >= 'a' and symbol <= 'z'))
		return 1;
	return 0;
}

bool is_upper(char symbol)
{
	if (symbol >= 'A' and symbol <= 'Z')
		return 1;
	return 0;
}

char to_lower(char symbol)
{
	if (symbol >= 'A' and symbol <= 'Z')
		return symbol + 32;
	return symbol;
}

char to_upper(char symbol)
{
	if (symbol >= 'a' and symbol <= 'z')
		return symbol - 32;
	return symbol;
}

char get_start_symbol_for(char symbol)
{
	return is_upper(symbol) ? 'A' : 'a';
}

void calculate_frequency(std::map<char, double>& freq_data, const std::string& data)
{
	// Counting symbols
	uint32_t letters_size{};
	for (auto symbol : data)
	{
		if (is_letter(symbol))
		{
			freq_data[to_lower(symbol)] += 1;
			++letters_size;
		}
	}

	for (auto& pair : freq_data)
	{
		pair.second /= letters_size;
	}
}

void print_sorted_map_by_values(const std::map<char, double>& data)
{
	std::vector<std::pair<char, double>> temp_vector;
	temp_vector.reserve(data.size());

	for (auto& pair : data)
	{
		temp_vector.push_back(pair);
	}
	std::sort(temp_vector.begin(), temp_vector.end(),
		[](const std::pair<char, double>& pair1, const std::pair<char, double>& pair2)
		{
			return pair1.second > pair2.second;
		}
	);

	int16_t i = 0;
	for (auto& pair : temp_vector)
	{
		std::cout << pair.first << " - " << pair.second << "\t";
		if ((i++ + 1) % 3 == 0)
			std::cout << std::endl;
	}
	std::cout << "\n";
}

void print_correct_comparison_table(const std::string& source_data, const std::string& encoded_data,
	std::map<char, char> resulting_comparison_table)
{
	std::map<char, double> source_data_frequency;
	std::map<char, double> encoded_data_frequency;
	std::map<char, char> comparison_table;

	calculate_frequency(source_data_frequency, source_data);
	calculate_frequency(encoded_data_frequency, encoded_data);

	std::cout << "\n\nFrequency for source data:" << std::endl;
	print_sorted_map_by_values(source_data_frequency);

	for (auto& pair1 : encoded_data_frequency)
	{
		for (auto& pair2 : source_data_frequency)
		{
			if (pair1.second == pair2.second)
			{
				comparison_table[pair1.first] = pair2.first;
				break;
			}
		}
	}

	int16_t i = 0;
	std::cout << "\nCorrect comparison table [enc/dec]:" << std::endl;
	for (auto& pair : comparison_table)
	{
		std::cout << pair.first << " - " << pair.second << "\t";
		if ((i++ + 1) % 3 == 0)
			std::cout << std::endl;
	}
	std::cout << "\n";

	int16_t correct_decoded_counter = 0;
	std::vector<char> correct_decoded;
	for (auto& pair : resulting_comparison_table)
	{
		if (pair.second == comparison_table[pair.first])
		{
			++correct_decoded_counter;
			correct_decoded.push_back(pair.first);
		}
	}

	std::cout << "\n\nNumber of correctly decoded symbols: " << correct_decoded_counter << std::endl;
	for (int16_t i = 0; i < correct_decoded.size(); ++i)
	{
		std::cout << correct_decoded[i] << " ";
	}
	std::cout << std::endl;
}

void print_controls()
{
	std::cout << "\nSelect action:\n1] c - change symbol to another one\n2] r - reset all changes\n3] b - undo previous change\n4] 0 - exit\n>>";
}

void roll_back_snapshot(std::stack<std::pair<char, char>>& snapshot, std::string& data,
	std::map<char, char>& comparison_table, bool is_caesar_decode)
{
	char symbol1 = snapshot.top().first;
	char symbol2 = snapshot.top().second;

	if (not is_caesar_decode)
	{
		for (auto& chr : data)
			if (to_lower(chr) == symbol2)
				chr = is_upper(chr) ? to_upper(symbol1) : symbol1;
		comparison_table.erase(snapshot.top().first);
	}
	else
	{
		uint16_t shift = (symbol2 - symbol1 + ALPHABET_SIZE) % ALPHABET_SIZE;
		for (auto& chr : data)
		{
			if (is_letter(chr))
			{
				char start_symbol = get_start_symbol_for(chr);
				char decoded_symbol = ((chr - start_symbol - shift + ALPHABET_SIZE) % ALPHABET_SIZE) + start_symbol;

				comparison_table[to_lower(chr)] = to_lower(decoded_symbol);
				chr = decoded_symbol;
			}
		}
	}
	snapshot.pop();

	std::string show_data = (data.size() > 50) ? data.substr(0, SYMBOLS_IN_SHOW_DATA) : data;
	std::cout << "\nAfter roll back [" << symbol2 << "] -> [" << symbol1 << "]:\n\n" << show_data << "\n" << std::endl;
}

void _remove_non_ascii(std::string& data)
{
	std::string ascii_string;
	ascii_string.reserve(data.size());

	for (auto chr : data)
		if (chr >= 0 and chr < 128)
			ascii_string += chr;

	ascii_string.shrink_to_fit();
	data = ascii_string;
}
