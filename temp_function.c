#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "temp_functions.h"

void printError(ErrorCode error)
{
	if(error == THERE_IS_NO_FILENAME)
	{
		fprintf(stderr, "error: there is no name of file after -f\n");
	}
	else if(error == FILE_FORMAT_IS_NOT_CSV)
	{
		fprintf(stderr, "error: format of file after -f is not CSV\n");
	}
	else if(error == THERE_IS_NO_MONTHNUMBER)
	{
		fprintf(stderr, "error: there is no number of month after -m\n");
	}
	else if(error == UNKNOWN_TOKEN)
	{
		fprintf(stderr, "error: unknown token\n");
	}
	else if(error == INCORRECT_MONTHNUMBER)
	{
		fprintf(stderr, "error: incorrect number of month\n");
	}
}

bool isNumber(const char *string)
{
	for(int i = 0; string[i] != '\0'; ++i)
	{
		if(!isdigit(string[i]))
		{
			return false;
		}
	}
	return true;
}

Options handleKeys(int size, char **options, ErrorCode *errorCode)
{
	Options result = {0, 0, 0}; // заполняем объект, хранящий информацию о том, какие ключи и значения были поданы в командную строку
	int i;
	for(i = 0; i < size; ++i)
	{
		if(strcmp(options[i], "-h") == 0) // если аргумент командной строки - ключ -h, то поле printHelp выставится в true
		{
			result.printHelp = true;
		}
		else if(strcmp(options[i], "-f") == 0) // если аргумент командной строки - ключ -f, то
		{
			if(i + 1 < size) // если следующая ячейка существует,
			{
				result.fileName = options[++i]; // имя файла хранится в ней;
				int str_size = strlen(result.fileName);
				if(str_size < 4 || strcmp(&result.fileName[str_size - 4], ".csv") != 0)
				{
					*errorCode = FILE_FORMAT_IS_NOT_CSV;
					return result;
				}
			}
			else
			{
				*errorCode = THERE_IS_NO_FILENAME; // иначе ошибка
				return result;
			}
		}
		else if(!strcmp(options[i], "-m")) // если аргумент командной строки - ключ -m, то
		{
			if(i + 1 < size && isNumber(options[i + 1])) // если следующая ячейка существует, в ней хранится число
			{
				result.numberOfMonth = atoi(options[++i]); 
				if(result.numberOfMonth <= 0 || result.numberOfMonth > 12) // и оно не в диапазоне от [1, 12]
				{
					*errorCode =  INCORRECT_MONTHNUMBER; // то возникает ошибка
					return result;
				}
			}
			else
			{
				*errorCode = THERE_IS_NO_MONTHNUMBER; // если же числа нет, то тоже возникает ошибка, но другая
				return result;
			}
		}
		else
		{
			*errorCode = UNKNOWN_TOKEN; // если аргумент командной строки не является ни одним из перечисленных ключей, то ошибка
			return result;
		}
	}
	return result;
}

DynamicArray* CreateDynamicArray()
{
	DynamicArray *res = (DynamicArray*)malloc(sizeof(DynamicArray) * 1);
	res->wdPtr = NULL;
	res->size = 0;
	return res;
}

void DeleteDynamicArray(DynamicArray* da)
{
	if (da != NULL)
	{
		free(da->wdPtr);
		free(da);
	}
}

void Add(DynamicArray* da, const WeatherData* element)
{
	da->wdPtr = (WeatherData*)realloc(da->wdPtr, sizeof(WeatherData) * (da->size + 1));
	da->wdPtr[da->size] = *element;
	++da->size;
}

static bool isYearLeap(int year) // статическая функция, использующаяся только в этом файле. вычисляет, чётный ли год по его номеру
{
	if(year % 400 == 0)
	{
		return true;
	}
	if(year % 100 == 0)
	{
		return false;
	}
	if(year % 4 == 0)
	{
		return true;
	}
	return false;
}

static void clean_buffer(FILE * file) // считывает все символы из файла, пока не встретит \n. функция используется для собственной функции my_fgets
{
	char c;
	while ((c = fgetc(file)) != EOF)
	{
		if (c == '\n')
		{
			break;
		}
	}
}

static char* my_fgets(char *buf, size_t size, FILE *file) // собственная реализация fgetc - посимвольного чтения из файла.
{
	bool stringWasFullyRead = false; // строка считается полностью прочтённой, если былы встречены или \n, или \t, или EOF
	int i;
	for(i = 0; i < size - 1; ++i) // читает до \n, \t, EOF или size - 1 символов
	{
		char symbol = fgetc(file);
		if(symbol == '\n')
		{
			stringWasFullyRead = true;
			break;
		}
		if (symbol == '\r')
		{
			fgetc(file);
			stringWasFullyRead = true;
			break;
		}
		if (symbol == EOF)
		{
			stringWasFullyRead = true;
			if (i == 0)
			{
				return NULL;
			}
			break;
		}
		buf[i] = symbol;
	}
	buf[i] = '\0'; // в конце ставит символ окончания строки
	if (!stringWasFullyRead) // если строка прочтена не полностью, то выполнить очистку буфера
	{
		clean_buffer(file);
	}
	return buf;
}

void readFromFile(DynamicArray* da, FILE* file)
{
	int number_of_args = 0, number_of_string = 1;
	WeatherData wd = {0, 0, 0, 0, 0, 0};
	
	char str[22]; // строка, заданная в задаче имеет длину 20 (если температура отрицательная) или 19 символов. 21 и 22 символ отдаются под перевод строки и \0
	int year = -1; // первый считанный год запомнится в эту переменную. 
	// если года, считанные позже, не будут соответствовать этому значению - эта запись не будет учтена в статистике
	
	while (my_fgets(str, 22, file) != 0)
	{
		// используем форматный ввод, чтобы проверить, что строка имеет данный вид: число;число;число;число;число;число\n
		number_of_args = sscanf(str, "%d;%d;%d;%d;%d;%d\n", &wd.year, &wd.month, &wd.day, &wd.hour, &wd.minute, &wd.temperature);
		if(number_of_args != 6) // sscanf возвращает кол-во удачно считанных аргументов. нам нужно считать 6 аргументов
		{
			printf("%d: contains an error.\n", number_of_string);
		}
		// строки с 212 по 250 проверяют введённые числа на корректность
		else if(wd.year <= 0)
		{
			printf("%d: Incorrect year %d.\n", number_of_string, wd.year);
		}
		else if(wd.month <= 0 || wd.month > 12)
		{
			printf("%d: Incorrect month %d.\n", number_of_string, wd.month);
		}
		else if(!checkDay(wd.year, wd.month, wd.day))
		{
			printf("%d: Incorrect day %d.\n", number_of_string, wd.day);
		}
		else if(wd.hour < 0 || wd.hour >= 24)
		{
			printf("%d: Incorrect hour %d.\n", number_of_string, wd.hour);
		}
		else if(wd.minute < 0 || wd.minute >= 60)
		{
			printf("%d: Incorrect minute  %d.\n", number_of_string, wd.minute);
		}
		else if(wd.temperature < -99 || wd.minute > 99)
		{
			printf("%d: Incorrect temperature  %d.\n", number_of_string, wd.temperature);
		}
		else
		{
			if(year == -1)
			{
				year = wd.year;
			}
			if(wd.year != year)
			{
				printf("%d: Incorrect year %d, year should be %d.\n", number_of_string, wd.year, year);
			}
			else
			{
				Add(da, &wd); // если все значения корректны, до добавляем новую запись в массив
			}
		}		
		++number_of_string;		
	}	
}

bool checkDay(int year, int month, int day)
{
	switch (month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		return day > 0 && day <= 31;
	case 2:
		return day > 0 && day <= (isYearLeap(year) ? 29:28);
	default:
		return day > 0 && day <= 30;
	}
}

int numberOfDaysInMonth(int year, int month)
{
	switch (month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		return 31;
	case 2:
		return (isYearLeap(year) ? 29:28);
	default:
		return 30;
	}
}

float averageYearTemp(DynamicArray* da)
{
	float sum = 0;
	int	count = 0;
	for(int i = 1; i < 13; ++i)
	{		
		bool isThereSuchMonth = false;
		float temp = averageMonthTemp(da, i, &isThereSuchMonth);
		if(isThereSuchMonth)
		{
			sum += temp;
			++count;
		}
	}
	if(count == 0)
	{
		return 0;
	}
	return sum / count;
}

float averageMonthTemp(DynamicArray* da, int month, bool* isThereSuchMonth)
{
	float sum = 0;
	int count = 0;
	if(da->wdPtr != NULL)
	{
		int year = da->wdPtr[0].year;	
		for(int i = 1; i <= numberOfDaysInMonth(year, month); ++i)
		{
			bool isThereSuchDay = false;
			float temp = averageDayTemp(da, month, i, &isThereSuchDay);
			if(isThereSuchDay)
			{
				sum += temp;
				++count;
			}
		}
	}
	if(count == 0)
	{
		*isThereSuchMonth = false;
		return 0;
	}
	*isThereSuchMonth = true;
	return sum / count;
}

float averageDayTemp(DynamicArray* da, int month, int day, bool *isThereSuchDay)
{
	float sum = 0;
	int	count = 0;
	for(int i = 0; i < 24; ++i)
	{
		bool isThereSuchHour = false;
		float temp = averageHourTemp(da, month, day, i, &isThereSuchHour);
		if(isThereSuchHour)
		{
			sum += temp;
			++count;
		}
	}
	if(count == 0)
	{
		*isThereSuchDay = false;
		return 0;
	}
	*isThereSuchDay = true;
	return sum / count;
}

float averageHourTemp(DynamicArray* da, int month, int day, int hour, bool *isThereSuchHour)
{
	long sum = 0;
	int count = 0;
	for(int i = 0; i < da->size; ++i)
	{
		WeatherData current = da->wdPtr[i];
		if(current.month ==  month && current.day == day && current.hour ==  hour)
		{
			sum += current.temperature;
			++count;
		}
	}
	if(count == 0)
	{
		*isThereSuchHour = false;
		return 0;
	}
	*isThereSuchHour = true;
	return (float)sum / count;
}

int minYearTemp(DynamicArray* da)
{
	int min = 99;
	for(int i = 0; i < da->size; ++i)
	{
		WeatherData current = da->wdPtr[i];
		if(current.temperature < min)
		{
			min = current.temperature;
		}
	}
	return min;
}
int maxYearTemp(DynamicArray* da)
{
	int max = -99;
	for(int i = 0; i < da->size; ++i)
	{
		WeatherData current = da->wdPtr[i];
		if(current.temperature > max)
		{
			max = current.temperature;
		}
	}
	return max;
}

int minMonthTemp(DynamicArray* da, int month, bool* isThereSuchMonth)
{
	int min = 99;
	*isThereSuchMonth = false;
	for(int i = 0; i < da->size; ++i)
	{
		WeatherData current = da->wdPtr[i];
		if(current.month ==  month)
		{
			*isThereSuchMonth = true;
			if(current.temperature < min)
			{
				min = current.temperature;				
			}
		}
	}
	return min;
}

int maxMonthTemp(DynamicArray* da, int month, bool* isThereSuchMonth)
{
	int max = -99;
	*isThereSuchMonth = false;
	for(int i = 0; i < da->size; ++i)
	{
		WeatherData current = da->wdPtr[i];
		if(current.month ==  month)
		{
			*isThereSuchMonth = true;
			if(current.temperature > max)
			{
				max = current.temperature;				
			}
		}
	}
	return max;
}

void printStatPerYear(DynamicArray* da)
{
	if(da->wdPtr != NULL)
	{
		float avg = averageYearTemp(da);
		printf("%d год:\nСреднегодовая температура: %f\n", da->wdPtr[0].year, avg);
		int min = minYearTemp(da);
		printf("Минимальная температура: %d\n", min);
		int max = maxYearTemp(da);
		printf("Максимальная температура: %d\n", max);
	}
	else
	{
		printf("the file doesn't contain strings of the appropriate format\n");
	}
}

void printStatPerMonth(DynamicArray* da, int month)
{
	bool isThereSuchMonth;
	float avg = averageMonthTemp(da, month, &isThereSuchMonth);
	if(isThereSuchMonth)
	{
		printf("%d месяц:\nСреднемесячная температура: %f\n", month, avg);
	}
	
	int min = minMonthTemp(da, month, &isThereSuchMonth);
	if(isThereSuchMonth)
	{
		printf("Минимальная температура: %d\n", min);
	}
	
	int max = maxMonthTemp(da, month, &isThereSuchMonth);
	if(isThereSuchMonth)
	{
		printf("Максимальная температура: %d\n", max);
	}	
}
