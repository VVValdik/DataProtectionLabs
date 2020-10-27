#include "pch.h"
#include <iostream>

const size_t BUFFER_SIZE = 10;
const int key[10] = { 3, 9, 8, 1, 6, 10, 5, 2, 7, 4 };

void check_file(FILE *file, const char *filename)
{
	if (!file)
	{
		printf("Can't open file %s!", filename);
		exit(1);
	}
}

void add_spaces(char **text, const size_t size)
{
	size_t newSize = size;
	while (newSize < BUFFER_SIZE)
	{
		(*text)[newSize++] = ' ';
	}
}

char* encrypt_str(const char *text, const size_t size)
{
	char* encrypted_text = (char*)malloc(BUFFER_SIZE + 1);
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		encrypted_text[i] = text[key[i] - 1];
	}
	encrypted_text[BUFFER_SIZE] = '\0';

	return encrypted_text;
}

char* decrypt_str(const char *text, const size_t size)
{
	char* decrypted = (char*)malloc(BUFFER_SIZE + 1);
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		decrypted[key[i] - 1] = text[i];
	}
	decrypted[BUFFER_SIZE] = '\0';

	return decrypted;
}

void encrypt_file(const char* source_fname, const char* output_fname)
{
	FILE *fsource = fopen(source_fname, "rb");
	check_file(fsource, source_fname);

	FILE *foutput = fopen(output_fname, "wb");
	check_file(foutput, output_fname);

	char* buffer = (char*)malloc(BUFFER_SIZE + 1);
	char* encrypted_text;
	while (!feof(fsource))
	{
		size_t element_count = fread_s(buffer, BUFFER_SIZE,sizeof(char), BUFFER_SIZE, fsource);
		add_spaces(&buffer, element_count);
		encrypted_text = encrypt_str(buffer, element_count);
		fwrite(encrypted_text, sizeof(char), BUFFER_SIZE, foutput);
		free(encrypted_text);
	}

	if(buffer) free(buffer);
	if(fsource) fclose(fsource);
	if(foutput) fclose(foutput);
}

void decrypt_file(const char* source_fname, const char* output_fname)
{
	FILE *fsource = fopen(source_fname, "rb");
	check_file(fsource, source_fname);
	
	FILE *foutput = fopen(output_fname, "wb");
	check_file(foutput, output_fname);

	char* buffer = (char*)malloc(BUFFER_SIZE + 1);
	char* decrypted;
	while (!feof(fsource))
	{
		size_t element_count = fread_s(buffer, BUFFER_SIZE, sizeof(char), BUFFER_SIZE, fsource);
		if (element_count == 0) break;
		add_spaces(&buffer, element_count);
		decrypted = decrypt_str(buffer, element_count);
		fwrite(decrypted, sizeof(char), element_count, foutput);
		free(decrypted);
	}

	if(buffer) free(buffer);
	if(fsource) fclose(fsource);
	if(foutput) fclose(foutput);
}

bool file_equals(const char* file1_name, const char* file2_name)
{
	bool files_are_equal = true;
	FILE *file1 = fopen(file1_name, "rb");
	check_file(file1, file1_name);

	FILE *file2 = fopen(file2_name, "rb");
	check_file(file2, file2_name);

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