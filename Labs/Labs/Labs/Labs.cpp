﻿#include "pch.h"
#include <iostream>

void check_file(FILE *file, const char *filename)
{
	if (!file)
	{
		printf("Can't open file %s!", filename);
		exit(1);
	}
}

char* read_file(const char* filename)
{
	FILE *file = fopen(filename, "rb");
	check_file(file, filename);

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	size_t newSize = size;
	while (newSize % 10 != 0) newSize++;

	char *source_text = (char*)malloc(newSize * sizeof(char) + 1);

	fread(source_text, 1, size, file);
	for (int i = size; i < newSize; i++) source_text[i] = ' ';
	source_text[newSize] = '\0';

	fclose(file);

	return source_text;
}

char* encrypt_str(const char* text)
{
	const int key[10] = { 3, 9, 8, 1, 6, 10, 5, 2, 7, 4 };

	size_t size = strlen(text);
	size_t blocks = size / 10;

	char* encrypted_text = (char *)malloc(sizeof(char) * size + 1);

	for (int i = 0; i < blocks; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			encrypted_text[10 * i + j] = text[10 * i + key[j] - 1];
		}
	}
	encrypted_text[size] = '\0';

	return encrypted_text;
}

char* decrypt_str(const char* text)
{
	const int key[10] = { 3, 9, 8, 1, 6, 10, 5, 2, 7, 4 };
	//const int key[10] = { 4, 8, 1, 10, 7, 5, 9, 3, 2, 6 };

	size_t size = strlen(text);
	size_t blocks = size / 10;

	char* decrypted_text = (char *)malloc(sizeof(char) * size + 1);

	for (int i = 0; i < blocks; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			decrypted_text[10 * i + key[j] - 1] = text[10 * i + j];
		}
	}
	decrypted_text[size] = '\0';

	return decrypted_text;
}

void encrypt_file(const char* source_fname, const char* output_fname)
{
	FILE *fsource = fopen(source_fname, "rb");
	check_file(fsource, source_fname);

	char *source_text = read_file(source_fname);
	printf("Source text for encryption: [%s] with length %d\n", source_text, strlen(source_text));

	char *encrypted_text = encrypt_str(source_text);
	printf("Encrypted text: [%s] with length %d\n", encrypted_text, strlen(encrypted_text));

	FILE *foutput = fopen(output_fname, "wb");
	check_file(foutput, output_fname);

	fwrite(encrypted_text, 1, strlen(encrypted_text), foutput);

	fclose(fsource);
	fclose(foutput);
	free(source_text);
	free(encrypted_text);
}

void decrypt_file(const char* source_fname, const char* output_fname)
{
	FILE *fsource = fopen(source_fname, "rb");
	if (!fsource)
	{
		printf("Can't open file %s!\n", source_fname);
		exit(1);
	}

	char *source_text = read_file(source_fname);
	printf("Source text for decryption: [%s] with length %d\n", source_text, strlen(source_text));

	char *decrypted_text = decrypt_str(source_text);
	printf("Decrypted text: [%s] with length %d\n", decrypted_text, strlen(decrypted_text));

	FILE *foutput = fopen(output_fname, "wb");
	if (!foutput)
	{
		printf("Can't open file %s!\n", output_fname);
		exit(1);
	}
	fwrite(decrypted_text, 1, strlen(decrypted_text), foutput);

	fclose(fsource);
	fclose(foutput);
	free(source_text);
	free(decrypted_text);
}

bool file_equals(const char* file1_name, const char* file2_name)
{
	bool files_are_equal = true;
	FILE *file1 = fopen(file1_name, "rb");
	check_file(file1, file1_name);

	FILE *file2 = fopen(file2_name, "rb");
	check_file(file2, file2_name);

	int c1, c2;
	while ((c1 = fgetc(file1)) != EOF && (c2 = fgetc(file2)) != EOF)
	{
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
	const char *source_fname = "input.cpp";
	const char *encrypted_fname = "encrypted.txt";
	const char *decrypted_fname = "decrypted.txt";

	encrypt_file(source_fname, encrypted_fname);

	decrypt_file(encrypted_fname, decrypted_fname);

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