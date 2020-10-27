#include "pch.h"
#include <iostream>

const size_t BUFFER_SIZE = 10;
const int key_to_encrypt[10] = { 3, 9, 8, 1, 6, 10, 5, 2, 7, 4 };
//const int key[10] =          { 4, 8, 1, 10, 7, 5, 9, 3, 2, 6 };

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

void add_spaces(char **text, const size_t size)
{
	size_t newSize = size;
	while (newSize < BUFFER_SIZE)
	{
		(*text)[newSize++] = ' ';
	}
}

void calculate_key(int* key, TranspositionCipherMode mode)
{
	if (mode == ENCRYPT)
	{
		for (int i = 0; i < BUFFER_SIZE; i++)
		{
			key[i] = key_to_encrypt[i];
		}
	}
	else if (mode == DECRYPT)
	{
		for (int i = 0; i < BUFFER_SIZE; i++)
		{
			int move_to = key_to_encrypt[i] - 1;
			key[key_to_encrypt[move_to] - 1] = key_to_encrypt[i];
		}
	}
}

char* transposition_cipher_str(const char* text, const int key[10])
{
	char *text_after_transposition = (char*)malloc(BUFFER_SIZE + 1);

	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		text_after_transposition[i] = text[key[i] - 1];
	}
	text_after_transposition[BUFFER_SIZE] = '\0';

	return text_after_transposition;
}

void transposition_cipher(const char* source_fname, const char* output_fname, TranspositionCipherMode mode)
{
	FILE *fsource = open_file(source_fname, "rb");
	FILE *foutput = open_file(output_fname, "wb");

	int *key = new int[10];
	calculate_key(key, mode);

	char* buffer = (char*)malloc(BUFFER_SIZE + 1);
	char* buffer_after_transposition = NULL;
	while (!feof(fsource))
	{
		size_t element_count = fread_s(buffer, BUFFER_SIZE, sizeof(char), BUFFER_SIZE, fsource);
		if (element_count == 0) break;
		add_spaces(&buffer, element_count);
		buffer_after_transposition = transposition_cipher_str(buffer, key);
		fwrite(buffer_after_transposition, sizeof(char), BUFFER_SIZE, foutput);
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