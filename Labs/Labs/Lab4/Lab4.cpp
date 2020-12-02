#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <bitset>
#include <string>
#include <vector>
#include <queue>
#include <conio.h>

using namespace std;

const size_t BITS_IN_BYTE = 8;
const size_t WORD_SIZE = 16;
const size_t DWORD_SIZE = 32;

const size_t BUFFER_SIZE = 3;

ostream& operator<<(ostream& os, const RGBTRIPLE &color)
{
	os << static_cast<int>(color.rgbtRed) <<
		" " << static_cast<int>(color.rgbtGreen) <<
		" " << static_cast<int>(color.rgbtBlue);

	return os;
}

ostream& operator<<(ostream& os, const vector<bool> &vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		os << vec[i];
	}

	return os;
}

ostream& operator<<(ostream& os, const BITMAPFILEHEADER bfh)
{
	cout << "bfh bfType " << bfh.bfType << endl;
	cout << "bfh bfOffBits " << bfh.bfOffBits << endl;
	cout << "bfh bfReserved1 " << bfh.bfReserved1 << endl;
	cout << "bfh bfReserved2 " << bfh.bfReserved2 << endl;
	cout << "bfh bfSize " << bfh.bfSize << endl << endl;

	return os;
}

ostream& operator<<(ostream& os, const BITMAPINFOHEADER bih)
{
	cout << "bih biBitCount " << bih.biBitCount << endl;
	cout << "bih biClrImportant " << bih.biClrImportant << endl;
	cout << "bih biClrUsed " << bih.biClrUsed << endl;
	cout << "bih biCompression " << bih.biCompression << endl;
	cout << "bih biWidth " << bih.biWidth << endl;
	cout << "bih biHeight " << bih.biHeight << endl;
	cout << "bih biSize " << bih.biSize << endl;
	cout << "bih biPlanes " << bih.biPlanes << endl;
	cout << "bih biSizeImage " << bih.biSizeImage << endl << endl;

	return os;
}

void check_handle(HANDLE &handle)
{
	if (INVALID_HANDLE_VALUE == handle)
	{
		cout << "Invalid handle value " << GetLastError() << endl;
		exit(1);
	}
}

void str_to_bits(string &str, queue<bool> &bits)
{
	for (int i = 0; i < str.length(); i++)
	{
		bitset<BITS_IN_BYTE> sym_bits(str[i]);
		for (int j = 0; j < BITS_IN_BYTE; j++)
		{
			bits.push(sym_bits[j]);
		}
	}
}

void change_pixel(RGBTRIPLE &pixel, queue<bool> &bits)
{
	bitset<BITS_IN_BYTE> rgb_bs[3]{
		bitset<BITS_IN_BYTE>(pixel.rgbtRed),
		bitset<BITS_IN_BYTE>(pixel.rgbtGreen),
		bitset<BITS_IN_BYTE>(pixel.rgbtBlue)
	};
	BYTE *color_byte[3]{ &pixel.rgbtRed, &pixel.rgbtGreen, &pixel.rgbtBlue };

	for (int i = 0; i < 3 && !bits.empty(); i++)
	{
		rgb_bs[i][0] = bits.front(); // change last bit
		*(color_byte[i]) = (BYTE)(rgb_bs[i].to_ulong());
		bits.pop();
	}
}

void get_secret_bits(RGBTRIPLE &pixel, vector<bool> &secret_bits)
{
	bitset<BITS_IN_BYTE> rgb_bs[3]{
		bitset<BITS_IN_BYTE>(pixel.rgbtRed),
		bitset<BITS_IN_BYTE>(pixel.rgbtGreen),
		bitset<BITS_IN_BYTE>(pixel.rgbtBlue)
	};

	for (int i = 0; i < 3; i++)
	{
		secret_bits.push_back(rgb_bs[i][0]); // last bit
	}
}

FILE* open_file(const char* filename, const char* mode)
{
	FILE *file = fopen(filename, mode);
	if (!file)
	{
		printf("Can't open file %s!", filename);
		exit(1);
	}
	return file;
}

void add_byte(queue<bool> &bits, byte *buffer)
{
	bitset<BITS_IN_BYTE> bs(*buffer);
	for (int i = 0; i < BITS_IN_BYTE; i++)
	{
		bits.push(bs[i]);
	}
}

queue<bool> get_str_from_file(string &filename)
{
	FILE* file = open_file(filename.c_str(), "rb");

	queue<bool> bits;

	byte *buffer = new byte[BUFFER_SIZE * sizeof(byte)];
	while (!feof(file))
	{
		size_t element_count = fread_s(buffer, sizeof(byte) * BUFFER_SIZE, sizeof(byte), BUFFER_SIZE, file);
		if (element_count == 0) break;

		for (size_t i = 0; i < element_count; i++)
		{
			add_byte(bits, &(buffer[i]));
		}
	}
	
	free(buffer);
	fclose(file);

	return bits;
}

bool is_bmp(BITMAPFILEHEADER bfh)
{
	return (bfh.bfType == 0x4D42);
}

void add_secret_msg(string secret_fname, string image_fname)
{
	queue<bool> secret_bits = get_str_from_file(secret_fname);
	size_t msg_len = secret_bits.size() / 8; // length in bytes
	if (msg_len > UINT_MAX)
	{
		cout << "Message is too big!\n";
		exit(1);
	}

	HANDLE hFile = CreateFileA(image_fname.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	check_handle(hFile);

	OVERLAPPED olf = { 0 };
	DWORD bytes = 0;

	// read structure BITMAPFILEHEADER
	BITMAPFILEHEADER bfh;
	ReadFile(hFile, &bfh, sizeof(bfh), &bytes, &olf);

	if (!is_bmp(bfh))
	{
		cout << "Not BMP image!\n";
		CloseHandle(hFile);
		exit(1);
	}

	bitset<DWORD_SIZE> msg_len_bs(msg_len);
	bitset<WORD_SIZE> bs_high(msg_len_bs.to_string().substr(0, WORD_SIZE));
	bitset<WORD_SIZE> bs_low(msg_len_bs.to_string().substr(WORD_SIZE, WORD_SIZE));
	bfh.bfReserved1 = (WORD)bs_high.to_ulong();
	bfh.bfReserved2 = (WORD)bs_low.to_ulong();

	WriteFile(hFile, &bfh, sizeof(bfh), &bytes, &olf);
	olf.Offset += bytes;

	// read structure BITMAPINFOHEADER
	BITMAPINFOHEADER bih;
	ReadFile(hFile, &bih, sizeof(bih), &bytes, &olf);

	// check messag size
	LARGE_INTEGER image_max_bits{ 0 };
	image_max_bits.QuadPart = bih.biWidth;
	image_max_bits.QuadPart *= bih.biHeight;
	image_max_bits.QuadPart *= 3;
	if (image_max_bits.QuadPart <= secret_bits.size())
	{
		cout << "Size of message is too big!\n";

		olf.Offset = 0;
		bfh.bfReserved1 = 0;
		bfh.bfReserved2 = 0;
		WriteFile(hFile, &bfh, sizeof(bfh), &bytes, &olf);

		CloseHandle(hFile);
		exit(1);
	}

	olf.Offset += bytes;

	RGBTRIPLE pixel;

	while (!secret_bits.empty())
	{
		ReadFile(hFile, &pixel, sizeof(pixel), &bytes, &olf);
		change_pixel(pixel, secret_bits);
		WriteFile(hFile, &pixel, sizeof(pixel), &bytes, &olf);

		olf.Offset += bytes;
	}

	cout << "Success!\n";

	CloseHandle(hFile);
}

bool has_secret_msg(string image_fname)
{
	HANDLE hFile = CreateFileA(image_fname.c_str(), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, 0, NULL);
	check_handle(hFile);

	DWORD bytes = 0;
	BITMAPFILEHEADER bfh;
	ReadFile(hFile, &bfh, sizeof(bfh), &bytes, NULL);

	if (!is_bmp(bfh))
	{
		cout << "Not BMP image!\n";
		exit(1);
	}

	CloseHandle(hFile);

	return (bfh.bfReserved1 != 0 || bfh.bfReserved2 != 0);
}

void get_secret_msg(string output, string image_fname)
{
	if (!has_secret_msg(image_fname))
	{
		cout << "No secret message!\n";
		exit(1);
	}

	vector<bool> secret_bits;

	HANDLE hFile = CreateFileA(image_fname.c_str(), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, 0, NULL);
	check_handle(hFile);

	DWORD bytes = 0;

	// read structure BITMAPFILEHEADER
	BITMAPFILEHEADER bfh;
	ReadFile(hFile, &bfh, sizeof(bfh), &bytes, NULL);

	// get size of secret msg
	WORD high = bfh.bfReserved1;
	WORD low = bfh.bfReserved2;
	bitset<DWORD_SIZE> bs;
	bitset<WORD_SIZE> high_bs(high);
	bitset<WORD_SIZE> low_bs(low);
	for (int i = 0; i < WORD_SIZE; i++)
	{
		bs[WORD_SIZE + i] = high_bs[i];
		bs[i] = low_bs[i];
	}

	size_t msg_bits_len = (size_t)bs.to_ulong() * BITS_IN_BYTE;

	// read structure BITMAPINFOHEADER
	BITMAPINFOHEADER bih;
	ReadFile(hFile, &bih, sizeof(bih), &bytes, NULL);

	RGBTRIPLE pixel;

	// read secret message
	for (int i = 0; i < msg_bits_len; i += 3)
	{
		ReadFile(hFile, &pixel, sizeof(pixel), &bytes, NULL);
		get_secret_bits(pixel, secret_bits);
	}

	ofstream secret(output);
	// write secret msg to file
	size_t len = secret_bits.size() / BITS_IN_BYTE;
	for (int i = 0; i < len; i++)
	{
		bitset<BITS_IN_BYTE> sym_bits;
		for (int j = 0; j < BITS_IN_BYTE; j++)
		{
			sym_bits[j] = secret_bits[i*BITS_IN_BYTE + j];
		}
		//cout << "Get symbits: " << (char)sym_bits.to_ulong() << endl;
		secret << (char)sym_bits.to_ulong();
	}

	cout << "Success!\n";

	CloseHandle(hFile);
	secret.close();
}

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		cout << "No arguments\n";
		exit(1);
	}
	
	if (strcmp(argv[1], "-embed") == 0)
	{
		if (argc == 4) add_secret_msg(argv[2], argv[3]);
		else cout << "Wrong arguments!\n";
	}
	else if (strcmp(argv[1], "-retrieve") == 0)
	{
		if (argc == 4) get_secret_msg(argv[2], argv[3]);
		else cout << "Wrong arguments!\n";
	}
	else if (strcmp(argv[1], "-check") == 0)
	{
		if (argc == 3)
		{
			if (has_secret_msg(argv[2])) cout << "Has secret message!\n";
			else cout << "No secret message!\n";
		}
		else cout << "Wrong arguments! " << argc << "!\n";
	}
	else
	{
		cout << "Wrong command!\n";
		exit(1);
	}

	/*add_secret_msg(
		"C:\\Users\\Rin\\Desktop\\secret.txt",
		"C:\\Users\\Rin\\Desktop\\doggo.bmp"
	);

	get_secret_msg(
		"C:\\Users\\Rin\\Desktop\\output.txt",
		"C:\\Users\\Rin\\Desktop\\doggo.bmp"
	);

	if (has_secret_msg("C:\\Users\\Rin\\Desktop\\doggo.bmp")) cout << "Doggo secret\n";*/

	return 0;
}