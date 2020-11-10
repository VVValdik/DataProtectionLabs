#include "pch.h"
#include <iostream>
#include <bitset>

using namespace std;

const size_t BUFFER_SIZE = 1;
const size_t KEY_SIZE = 32;
const size_t BITS_IN_BYTE = 8;
const int key_to_encrypt[KEY_SIZE] = { 2, 28, 13, 15, 20, 12, 5, 24, 25, 27, 1, 
									3, 14, 9, 8, 21, 22, 4, 26, 23,
									19, 30, 18, 6, 11, 17, 31, 16, 10, 7, 29, 0 };

typedef int bytes_block;

enum TranspositionCipherMode {
	ENCRYPT,
	DECRYPT
};

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

void calculate_key(int* key, TranspositionCipherMode mode)
{
	if (mode == ENCRYPT)
	{
		for (int i = 0; i < KEY_SIZE; i++)
		{
			key[i] = key_to_encrypt[i];
		}
	}
	else if (mode == DECRYPT)
	{
		for (int i = 0; i < KEY_SIZE; i++)
		{
			int move_to = key_to_encrypt[i];
			key[key_to_encrypt[move_to]] = key_to_encrypt[i];
		}
	}
}

bytes_block* transposition_cipher_str(const bytes_block* buffer, const int key[KEY_SIZE], size_t readed_bytes)
{
	bytes_block *buffer_after_transposition = (bytes_block*)malloc(BUFFER_SIZE * sizeof(bytes_block));
	bitset<KEY_SIZE> buffer_bitset(*buffer);
	bitset<KEY_SIZE> buffer_after_bitset;

	for (int i = 0; i < KEY_SIZE; i++)
	{
		buffer_after_bitset[i] = buffer_bitset[key[i]];
	}

	*buffer_after_transposition = (unsigned int)buffer_after_bitset.to_ulong();

	return buffer_after_transposition;
}

void transposition_cipher(const char* source_fname, const char* output_fname, TranspositionCipherMode mode)
{
	FILE *fsource = open_file(source_fname, "rb");
	FILE *foutput = open_file(output_fname, "wb");

	int *key = new int[KEY_SIZE];
	calculate_key(key, mode);

	bytes_block* buffer = (bytes_block*)malloc(BUFFER_SIZE * sizeof(bytes_block));
	bytes_block* buffer_after_transposition = NULL;

	while (!feof(fsource))
	{
		size_t element_count = fread_s(buffer, sizeof(bytes_block), sizeof(bytes_block), BUFFER_SIZE, fsource);
		if (element_count == 0) break;
		buffer_after_transposition = transposition_cipher_str(buffer, key, element_count);
		fwrite(buffer_after_transposition, sizeof(bytes_block), BUFFER_SIZE, foutput);
		free(buffer_after_transposition);
	}

	free(buffer);
	free(key);
	fclose(fsource);
	fclose(foutput);
}

bool file_equals(const char* file1_name, const char* file2_name)
{
	bool files_are_equal = true;

	FILE *file1 = open_file(file1_name, "rb");
	FILE *file2 = open_file(file2_name, "rb");

	int c1, c2;
	while (!feof(file1) && !feof(file2))
	{
		c1 = fgetc(file1);
		c2 = fgetc(file2);
		if (c1 == EOF || c2 == EOF) break;
		if (c1 != c2)
		{
			files_are_equal = false;
			break;
		}
	}
	fclose(file1);
	fclose(file2);

	return files_are_equal;
}

int main()
{
	const char *source_fname = "some.exe";
	const char *encrypted_fname = "encrypted.txt";
	const char *decrypted_fname = "decrypted.txt";

	transposition_cipher(source_fname, encrypted_fname, ENCRYPT);

	transposition_cipher(encrypted_fname, decrypted_fname, DECRYPT);

	if (file_equals(source_fname, decrypted_fname))
	{
		printf("Files are equal!\n");
	}
	else
	{
		printf("Files are not equal!\n");
	}

	return 0;
}