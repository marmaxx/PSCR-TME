#include <iostream>
#include <fstream>
#include <regex>
#include <chrono>
#include <string>
#include <algorithm>
#include <vector>
#include "HashMap.h"

// helper to clean a token (keep original comments near the logic)
static std::string cleanWord(const std::string& raw) {
	// une regex qui reconnait les caractères anormaux (négation des lettres)
	static const std::regex re( R"([^a-zA-Z])");
	// élimine la ponctuation et les caractères spéciaux
	std::string w = std::regex_replace(raw, re, "");
	// passe en lowercase
	std::transform(w.begin(), w.end(), w.begin(), ::tolower);
	return w;
}

int main(int argc, char** argv) {
	using namespace std;
	using namespace std::chrono;

	// Allow filename as optional first argument, default to project-root/WarAndPeace.txt
	// Optional second argument is mode (e.g. "count" or "unique").
	string filename = "../WarAndPeace.txt";
	string mode = "count";
	if (argc > 1) filename = argv[1];
	if (argc > 2) mode = argv[2];

	ifstream input(filename);
	if (!input.is_open()) {
		cerr << "Could not open '" << filename << "'. Please provide a readable text file as the first argument." << endl;
		cerr << "Usage: " << (argc>0?argv[0]:"TME2") << " [path/to/textfile]" << endl;
		return 2;
	}
	cout << "Parsing " << filename << " (mode=" << mode << ")" << endl;
	
	auto start = steady_clock::now();
	
	// prochain mot lu
	string word;

	if (mode == "count") {
		size_t nombre_lu = 0;
	
		// default counting mode: count total words
		while (input >> word) {
			// élimine la ponctuation et les caractères spéciaux
			word = cleanWord(word);

			// word est maintenant "tout propre"
			/*if (nombre_lu % 100 == 0)
				// on affiche un mot "propre" sur 100
				cout << nombre_lu << ": "<< word << endl;*/
			nombre_lu++;
		}
	input.close();
	cout << "Finished parsing." << endl;
	cout << "Found a total of " << nombre_lu << " words." << endl;

	} else if (mode == "unique") {
		// skeleton for unique mode
		// before the loop: declare a vector "seen"
		// TODO
		vector<string> seen;

		while (input >> word) {
			// élimine la ponctuation et les caractères spéciaux
			word = cleanWord(word);

			// add to seen if it is new
			// TODO
			size_t found_index = -1;
			for (size_t i = 0; i < seen.size(); i++) {
				if (seen[i] == word) {
					found_index = i;
					break;
				}
			}
			if (found_index == -1) {
				seen.push_back(word);
			}
		}
	input.close();
	// TODO
	cout << "Found " << seen.size() << " unique words." << endl;

	} else if (mode == "freq") {
		vector<pair<string, int>> freqs;

		while (input >> word) {
			word = cleanWord(word);

			size_t found_index = -1;
			for (size_t i = 0; i < freqs.size(); i++) {
				if (freqs[i].first == word) {
					found_index = i;
					freqs[i].second++;
					break;
				}
			}
			if (found_index == -1) {
				auto new_word = make_pair(word, 1);
				freqs.push_back(new_word);
			}
		}
	input.close();
	for (const auto& p : freqs) {
		if (p.first == "war" || p.first == "peace" || p.first == "toto"){
			cout << p.first << ": " << p.second << endl;
		}
	}
	/*std::sort(freqs.begin(), freqs.end(), [](const auto& p1, const auto& p2) { return p1.second > p2.second; });
	cout << "Top 10 most frequent words:" << endl;
	for (size_t i = 0; i < 10 && i < freqs.size(); i++) {
		cout << freqs[i].first << endl;	
	}*/
	} else if (mode == "freqhash") {
		HashMap<string, int> freqs(100);

		while(input >> word) {
			word = cleanWord(word);

			int* count = freqs.get(word);
			if (count) {
				(*count)++;
			} else {
				freqs.put(word, 1);
			}
		}
	input.close();
	//cout << "Found " << freqs.size() << " unique words." << endl;
	cout << "war: " << (freqs.get("war") ? *freqs.get("war") : 0) << endl;
	cout << "peace: " << (freqs.get("peace") ? *freqs.get("peace") : 0) << endl;
	cout << "toto: " << (freqs.get("toto") ? *freqs.get("toto") : 0) << endl;
	
	} else {
		// unknown mode: print usage and exit
		cerr << "Unknown mode '" << mode << "'. Supported modes: count, unique" << endl;
		input.close();
		return 1;
	}

	// print a single total runtime for successful runs
	auto end = steady_clock::now();
	cout << "Total runtime (wall clock) : " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

	return 0;
}


