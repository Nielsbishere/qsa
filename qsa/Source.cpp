/* QSA - made by Osomi Inc (Niels Brunekreef)
* Quick String Analyze
* Determines how likely a character is to appear within a string and also measures lengths
* It will then allow you to generate a new string with the same patterns
* The more strings you use for inputs, the more accurate the string will be */

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <ctime>

struct QSAChar {

	int count;
	char c;
	float occurence = 0;

	//std::sort(std::vector<QSAChar>)
	bool operator<(const QSAChar &c) {
		return count > c.count || (count == c.count && this->c < c.c);
	}

};

struct QSALength {

	int count;
	int length;
	float occurence = 0;

	//std::sort(std::vector<QSALength>)
	bool operator<(const QSALength &l) {
		return count > l.count || (count == l.count && this->length > l.length);
	}

};

struct QSAStruct {

	std::vector<QSALength> lengths;
	std::vector<QSAChar> characters;
	
	int lineCount = 0;
	int characterCount = 0;

};

bool analyze(QSAStruct &qsa, std::string &str) {

	std::ifstream input(str);

	if (!input.good())
		return false;

	qsa.lengths.reserve(1024);
	qsa.characters.reserve(256);

	for (std::string line; getline(input, line); ) {

		int lsize = (int) line.size();

		++qsa.lineCount;
		qsa.characterCount += lsize;

		bool contains = false;

		for (QSALength &l : qsa.lengths)
			if (l.length == lsize) {
				contains = true;
				++l.count;
				break;
			}

		if (!contains)
			qsa.lengths.push_back({ 1, lsize });

		for (char &c : line) {

			contains = false;

			for (QSAChar &qc : qsa.characters)
				if (qc.c == c) {
					contains = true;
					++qc.count;
					break;
				}

			if (!contains)
				qsa.characters.push_back({ 1, c });

		}
	}

	std::sort(qsa.lengths.begin(), qsa.lengths.end());
	std::sort(qsa.characters.begin(), qsa.characters.end());

	for (QSALength &length : qsa.lengths)
		length.occurence = (float) length.count / qsa.lineCount;

	for (QSAChar &c : qsa.characters)
		c.occurence = (float)c.count / qsa.characterCount;

	return true;
}

void invalidSyntax(char *c) {
	printf("Please use correct syntax:\n");
	printf("qse.exe <someFile>.txt\n");
	printf("Where someFile is the path to a file with a number of strings seperated by \\n or \\r\\n.\n");
	printf("Error %s\n", c);
	system("pause");
}

float frand() {
	return (float) rand() / RAND_MAX;
}

int pickLength(QSAStruct &qsa) {

	float r = frand();
	float val = 0;

	for (QSALength &len : qsa.lengths) {

		val += len.occurence;

		if (r <= val)
			return len.length;

	}

	return qsa.lengths[0].length;
}

char pickChar(QSAStruct &qsa) {

	float r = frand();
	float val = 0;

	for (QSAChar &c : qsa.characters) {

		val += c.occurence;

		if (r <= val)
			return c.c;

	}

	return qsa.characters[0].c;
}

void generate(QSAStruct &qsa, std::string &s) {

	int length = pickLength(qsa);
	s.resize(length, ' ');

	for (char &c : s)
		c = pickChar(qsa);

}

void smartGenerate(QSAStruct &qsa, std::vector<std::string> &s, int count) {

	s.resize(count);

	int i = 0;
	
	while (i < count) {

		generate(qsa, s[i]);

		bool contains = false;

		for (int j = 0; j < i; ++j)
			if (s[j] == s[i]) {
				contains = true;
				break;
			}

		if (contains) continue;

		++i;
	}

}

bool write(std::vector<std::string> &s, std::string path) {

	std::ofstream output(path);

	if (!output.good())
		return false;

	for (std::string &str : s)
		output << str << std::endl;

	return true;
}

//Main entry point; please run it as
//qse.exe test.txt
//Where test.txt is a file containing strings seperated by \n or \r\n
int main(int argc, char *argv[]) {
	
	std::string str = argc == 2 ? argv[1] : "keys.txt";

	if (str.size() < 4 || str.substr(str.size() - 4, 4) != ".txt") {
		invalidSyntax("0x0");
		return 0;
	}

	printf("Analyzing strings...\n");

	QSAStruct qsa;
	if (!analyze(qsa, str)) {
		printf("Couldn't analyze the file\n");
		system("pause");
		return 0;
	}

	printf("Analyzed the strings!\n");
	printf("Contains %u different lengths and %u different characters\n", qsa.lengths.size(), qsa.characters.size());
	printf("From %u lines and %u characters\n", qsa.lineCount, qsa.characterCount);

	if (qsa.lengths.size() == 1)
		printf("Determined the length of a string to be %u exactly\n", qsa.lengths[0].length);
	else {

		printf("Length couldn't be fully determined; but here are possible results based on likeliness\n");

		for (QSALength &l : qsa.lengths)
			printf("%u: %f%%\n", l.length, l.occurence * 100);
	}

	if(qsa.characters.size() == 1)
		printf("Determined the character to be '%c' exactly\n", qsa.characters[0].c);
	else {

		printf("Here are possible character results based on likeliness\n");

		for (QSAChar &c : qsa.characters)
			printf("'%c': %f%%\n", c.c, c.occurence * 100);
	}

	printf("Enter the number of words you want to generate\n");

	int count;
	std::cin >> count;

	srand(time(0));

	std::vector<std::string> keys;
	smartGenerate(qsa, keys, count);

	printf("Output to console? y/n\n");

	char c;
	std::cin >> c;

	if(c == 'y')
		for (std::string &s : keys)
			printf("%s\n", s.c_str());

	printf("Output to file? y/n\n");

	std::cin >> c;

	if (c == 'y' && !write(keys, str + ".gen"))
		printf("Failed to write to file\n");

	system("pause");
	return 1;
}