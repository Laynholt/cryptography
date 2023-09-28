#pragma once

#include <iostream>
#include <fstream>

#include <algorithm>

#include <string>
#include <map>
#include <stack>
#include <vector>

constexpr int16_t ALPHABET_SIZE = 26;
constexpr int32_t SYMBOLS_IN_SHOW_DATA = 200;

bool is_letter(char symbol);
bool is_upper(char symbol);
char to_lower(char symbol);
char to_upper(char symbol);
char get_start_symbol_for(char symbol);

void calculate_frequency(std::map<char, double>& freq_data, const std::string& data);
void print_sorted_map_by_values(const std::map<char, double>& data);
void print_correct_comparison_table(const std::string& source_data, const std::string& encoded_data,
	std::map<char, char> resulting_comparison_table);

void print_controls();
void roll_back_snapshot(std::stack<std::pair<char, char>>& snapshot, std::string& data,
	std::map<char, char>& comparison_table, bool is_caesar_decode);

void _remove_non_ascii(std::string& data);

template<typename Type1, typename Type2>
std::map<Type2, Type1> _reverse_map(const std::map<Type1, Type2>& source_map)
{
	std::map<Type2, Type1> reversed_map;

	for (auto& pair : source_map)
		reversed_map[pair.second] = pair.first;
	return reversed_map;
}