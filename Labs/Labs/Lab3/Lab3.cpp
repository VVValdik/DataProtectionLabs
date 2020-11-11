#include <bitset>
#include <iostream>
#include <string>
#include <time.h>
#include <map>

using namespace std;

unsigned char R0, R1;

const size_t TRIM_SIZE = 8;
const size_t BITSET_SIZE = 16;

void timer()
{
	time_t timer;
	time(&timer);

	R0 = timer % 100;
	R1 = timer % 100;
}

float get_random()
{
	short R2 = R1 * R0;
	float random;

	string R2str, trimed;
	bitset <BITSET_SIZE> bs(R2);
	R2str = bs.to_string();
	trimed = R2str.substr(4, 8);
	bitset <TRIM_SIZE> bs2(trimed);
	random = stof("0." + to_string(bs2.to_ulong()));

	R0 = R1;
	R1 = stoi(trimed);

	return random;
}

void statistics()
{
	map <int, int> rang;
	for (int i = 0; i < 200; i++)
	{
		float rand = get_random();
		int r_rand = rand * 10;
		rang[r_rand]++;
	}

	for (int i = 0; i < 10; i++)
	{
		cout << i << " " << rang[i] << endl;
	}
}

int main()
{
	timer();
	get_random();
	statistics();

	return 0;
}