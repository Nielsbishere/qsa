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
#include <unordered_map>
#include <future>
#include <chrono>

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

struct QSAIndex {
	std::vector<QSAChar> c;
	int totalCount = 0;
};

struct QSAStruct {

	std::vector<QSALength> lengths;
	std::unordered_map<int, QSAIndex> indices;
	
	int lineCount = 0;

};

bool analyze(QSAStruct &qsa, std::string &str) {

	std::ifstream input(str);

	if (!input.good())
		return false;

	qsa.lengths.reserve(128);
	qsa.indices.reserve(128 * 16);

	for (std::string line; getline(input, line); ) {

		int lsize = (int) line.size();

		++qsa.lineCount;

		bool contains = false;

		for (QSALength &l : qsa.lengths)
			if (l.length == lsize) {
				contains = true;
				++l.count;
				break;
			}

		if (!contains)
			qsa.lengths.push_back({ 1, lsize });

		int i = 0;

		for (char &c : line) {

			contains = false;

			QSAIndex &index = qsa.indices[i];
			++index.totalCount;

			for (QSAChar &qc : index.c)
				if (qc.c == c) {
					contains = true;
					++qc.count;
					break;
				}

			if (!contains)
				index.c.push_back({ 1, c });


			++i;
		}
	}

	std::sort(qsa.lengths.begin(), qsa.lengths.end());

	for(auto &ind : qsa.indices)
		std::sort(ind.second.c.begin(), ind.second.c.end());

	for (QSALength &length : qsa.lengths)
		length.occurence = (float) length.count / qsa.lineCount;

	for (auto &ind : qsa.indices)
		for(QSAChar &c : ind.second.c)
			c.occurence = (float)c.count / ind.second.totalCount;

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

char pickChar(QSAStruct &qsa, int i) {

	float r = frand();
	float val = 0;

	QSAIndex &index = qsa.indices[i];

	for (QSAChar &c : index.c) {

		val += c.occurence;

		if (r <= val)
			return c.c;

	}

	return index.c[0].c;
}

void generate(QSAStruct &qsa, std::string &s) {

	int length = pickLength(qsa);
	s.resize(length, ' ');

	int i = 0;

	for (char &c : s) {
		c = pickChar(qsa, i);
		++i;
	}

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

void batchGenerate(QSAStruct &qsa, std::vector<std::string> &s, int count) {

	s.resize(count);

	std::vector<std::string> *sptr = &s;
	QSAStruct *qsaptr = &qsa;

	unsigned int thrs = std::thread::hardware_concurrency();
	unsigned int perThread = (unsigned int) count / thrs;
	unsigned int leftOver = perThread;

	std::vector<std::future<void>> fs(thrs);

	for (unsigned int i = 0; i < thrs; ++i)
		fs[i] = std::move(std::async([perThread, leftOver, sptr, qsaptr, count](unsigned int j) -> void {

				unsigned int c = (j == 0 ? leftOver : 0) + perThread;
				unsigned int off = j == 0 ? 0 : leftOver + perThread * j;

				for (unsigned int k = off; k < off + c && k < count; ++k)
					generate(*qsaptr, (*sptr)[k]);

		}, i));

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
	printf("Contains %u different lengths from %u lines\n", (unsigned int) qsa.lengths.size(), qsa.lineCount);

	if (qsa.lengths.size() == 1)
		printf("Determined the length of a string to be %u exactly\n", qsa.lengths[0].length);
	else {

		printf("Length couldn't be fully determined; but here are possible results based on likeliness\n");

		for (QSALength &l : qsa.lengths)
			printf("%u: %f%%\n", l.length, l.occurence * 100);
	}

	printf("Here are possible character results based on likeliness\n");

	size_t possible = 1;

	for (auto &ind : qsa.indices) {
		printf("%u: analyzed %u chars; %u unique\n", ind.first, ind.second.totalCount, (unsigned int) ind.second.c.size());
		for (QSAChar &c : ind.second.c)
			printf("%u; '%c': %f%%\n", ind.first, c.c, c.occurence);
		possible *= ind.second.c.size();
	}

	printf("This keysequence has %I64u different outcomes\n", possible);

	printf("Enter the number of sequences you want to generate\n");

	int count;
	std::cin >> count;

	srand((unsigned int)time(0));

	auto timePoint0 = std::chrono::high_resolution_clock::now();

	std::vector<std::string> keys;
	//smartGenerate(qsa, keys, count);
	batchGenerate(qsa, keys, count);

	auto timePoint1 = std::chrono::high_resolution_clock::now();
	printf("BatchGenerate took %fs\n", std::chrono::duration<float>(timePoint1 - timePoint0).count());

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