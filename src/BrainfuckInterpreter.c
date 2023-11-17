#include <stdio.h>
#include <string.h>
#include <windows.h>

#define DEFAULT_BUFFER_SIZE 32

typedef unsigned char byte;

#pragma region List
typedef struct
{
	int* buffer;
	int size;
	int reserved;
} List;

List CreateList()
{
	List list = { .buffer = malloc(DEFAULT_BUFFER_SIZE * sizeof(int)), .reserved = DEFAULT_BUFFER_SIZE, .size = 0 };
	return list;
}

int AddValueToList(List* list, int value)
{
	if (list->size + 1 > list->reserved)
	{
		list->reserved *= 2;
		int* newBuffer = realloc(list->buffer, list->reserved * sizeof(int));
		if (newBuffer == NULL)
			return 0;

		list->buffer = newBuffer;
	}
	list->buffer[list->size] = value;
	list->size++;

	return 1;
}

int GetLastValueFromList(List* list)
{
	return list->size == 0 ? -1 : list->buffer[list->size - 1];
}
#pragma endregion

#pragma region Memory
typedef struct
{
	byte* buffer;
	int size;
	int address;
} Memory;

Memory CreateMemory(int size)
{
	byte* buffer = malloc(size * sizeof(byte));
	if (buffer == NULL)
		return;
	memset(buffer, 0, size);
	Memory memory = { .buffer = buffer, .size = size, .address = 0 };
	return memory;
}

int IncrementAddressPointer(Memory* memory)
{
	memory->address++;
	if (memory->address >= memory->size)
	{
		int oldSize = memory->size;
		memory->size *= 2;
		byte* newBuffer = realloc(memory->buffer, memory->size);
		if (newBuffer == NULL)
		{
			printf("Error: Failed to reallocate the memory buffer\n");
			return 0;
		}
		memory->buffer = newBuffer;
		memset(memory->buffer + oldSize, 0, oldSize); // set all values to 0 to prevent undefined bahavior
	}
	return 1;
}

int DecrementAddressPointer(Memory* memory)
{
	if (memory->address == 0)
	{
		printf("Error: failed to decrement the address pointer past 0\n");
		return 0;
	}
	memory->address--;
	return 1;
}
#pragma endregion

FILE* sourceFile = NULL;

int ParseInstructions(byte* instructions, int length)
{
	List bracketStartIndices = CreateList();
	Memory memory = CreateMemory(2);
	
	for (int i = 0; i < length; i++)
	{
		switch (instructions[i])
		{
		case '+': // increment value
			memory.buffer[memory.address]++;
			break;
		case '-': // decrement value
			memory.buffer[memory.address]--;
			break;
		case '>': // move one address
			if (!IncrementAddressPointer(&memory))
			{
				printf("Error: failed to increment the address pointer past %i at %i\n", memory.address, i + 1);
				return 0;
			}
			break;
		case '<': // go back one address
			if (!DecrementAddressPointer(&memory))
			{
				printf("Error: failed to decrement the address pointer past %i at %i\n", memory.address, i + 1);
				return 0;
			}
			break;
		case '.': // output
			printf("%c", (unsigned char)memory.buffer[memory.address]); // the (unsigned char) prevents a warning
			break;
		case ',': // input
			printf("Waiting for input: ");
			memory.buffer[memory.address] = getchar();
		case '[': // start while > 0 loop
			if (!AddValueToList(&bracketStartIndices, i))
			{
				printf("Failed to register a '[' instruction\n");
				return 0;
			}
			break;
		case ']': // end loop
			if (bracketStartIndices.size <= 0 || bracketStartIndices.buffer[0] == -1)
			{
				printf("Error: no '[' could be found to match the ']' at %i\n", i + 1);
				return 0;
			}
			if (memory.buffer[memory.address] != 0)
				i = GetLastValueFromList(&bracketStartIndices);
			break;
		default:
			break;
		}
	}
	return 1;
}

void main(int argsCount, char** args)
{
	if (argsCount <= 1)
		args[1] = "helloWorld.bf";

	//open file
	char* fileLocation = args[1];
	if (fopen_s(&sourceFile, fileLocation, "r") != 0)
	{
		printf("Failed to open file\n");
		return 1;
	}
	printf("Opened file %s:\n", fileLocation);
		
	if (!sourceFile)
	{
		printf("source file pointer is invalid\n");
		return 1;
	}

	// get file size
	fseek(sourceFile, 0, SEEK_END);
	int fileSize = ftell(sourceFile);
	rewind(sourceFile);

	byte* sourceCode = malloc(fileSize * sizeof(byte));
	if (sourceCode == NULL)
		return 1;

	fread(sourceCode, fileSize, 1, sourceFile);
	fclose(sourceFile);

	if (!ParseInstructions(sourceCode, fileSize))
		return 1;

	free(sourceCode);
	
	int preventConsoleShutdown = getchar(); // this does not nothing except prevent the console from closing when the file has been read
	return 0;
}