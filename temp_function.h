#include <stdbool.h>

#define INFO(message) fprintf(stderr, "%s:%s:%d:%s\n", __FILE__, __FUNCTION__, __LINE__, message) // отладочный макрос напечатает файл, функцию, строку, message

typedef struct WeatherData // структура для хранения записей о погоде
{
	int year; // год
	int month; // месяц
	int day; // день
	int hour; // час
	int minute; // минута
	int temperature; // температура
} WeatherData;

typedef enum ErrorCode // перечисление для хранения кода ошибки
{
	OK = 0, // отсутствие ошибки
	THERE_IS_NO_FILENAME, // отсутствует имя файла после ключа -f
	FILE_FORMAT_IS_NOT_CSV,
	THERE_IS_NO_MONTHNUMBER, // отсутствует номер месяца после ключа -m
	UNKNOWN_TOKEN, // неизвестный ключ
	INCORRECT_MONTHNUMBER // после ключа -m был введён некорректный номер месяца	
} ErrorCode;

typedef struct Options // структура для хранения значений из argv, поданных с ключом
{
	bool printHelp; // был ли подан ключ -h
	const char *fileName; // какая строка была подана после -f
	int numberOfMonth; // номер месяца после -m
} Options;

void printError(enum ErrorCode error); // выводит сообщение в зависимости от значения enum

bool isNumber(const char *string); // проверяет, хранит ли переданная строка число

Options handleKeys(int size, char **options, ErrorCode *errorCode); // 

typedef struct DynamicArray // структура для хранения динамического массива. её поля:
{
	WeatherData* wdPtr; // указатель на массив, хранящий записи о погоде
	int size; // размер массива
} DynamicArray;

DynamicArray* CreateDynamicArray(); // "конструктор" массива; возвращает указатель на созданный внутри функции массив
void DeleteDynamicArray(DynamicArray* da); // очищает память объекта DynamicArray, переданного в качестве параметра
void Add(DynamicArray* da, const WeatherData* element); // функция для добавления в переданный DynamicArray нового элемента (запись о погоде)

void readFromFile(DynamicArray* da, FILE* file); // функция для чтения в динамический массив из файлового потока

bool checkDay(int year, int month, int day); // проверяет корректность номера дня в зависимости от года и месяца

float averageYearTemp(DynamicArray* da); // функция возвращает значение среднегодовой температуры

float averageMonthTemp(DynamicArray* da, int month, bool* isThereSuchMonth); // возвращает значение среднемесячной температуры для указанного месяца. 
//параметр isThereSuchMonth равен true, сли информация о таком месяце имеется, иначе false

float averageDayTemp(DynamicArray* da, int month, int day, bool *isThereSuchDay);// возвращает значение среднедневной температуры для указанного дня. 
//параметр isThereSuchDay равен true, сли информация о таком дне имеется, иначе false

float averageHourTemp(DynamicArray* da, int month, int day, int hour, bool *isThereSuchHour);// возвращает значение среднедневной температуры для указанного часа. 
//параметр isThereSuchHour равен true, сли информация о таком часе имеется, иначе false

int minYearTemp(DynamicArray* da); // возвращает минимальную годовую температуру
int maxYearTemp(DynamicArray* da); // возвращает максимальную годовую температуру

int minMonthTemp(DynamicArray* da, int month, bool* isThereSuchMonth); // возвращает минимальную месячную температуру для указанного месяца.
//параметр isThereSuchMonth равен true, сли информация о таком месяце имеется, иначе false

int maxMonthTemp(DynamicArray* da, int month, bool* isThereSuchMonth); // возвращает максимальную месячную температуру для указанного месяца.
//параметр isThereSuchMonth равен true, сли информация о таком месяце имеется, иначе false

void printStatPerYear(DynamicArray* da); // функция печатает статистику за год
void printStatPerMonth(DynamicArray* da, int month); // функция печатает статистику за указанный месяц
