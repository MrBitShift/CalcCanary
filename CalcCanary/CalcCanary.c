#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#else

#include <stdlib.h>

#endif

#include <stdio.h>
#include <string.h>
#include <LibMath.h>
#include <LibInput.h>

#pragma region Macros

#define check(A, B, ...) if (!(A))\
{\
	printf(B "\n", ##__VA_ARGS__);\
	goto error;\
}

#pragma endregion

#pragma region Warning and Error Strings

#define INVALID "Invalid command. Type \"help\" for a list of commands."
#define INVALID_USAGE "Invalid usage. Type \"help <commandName>\" for more info."
#define INVALID_EQUATION "Equation was invalid. Type \"help <commandName> for more info.\""

#pragma endregion

#pragma region Command Strings

// Command to show to logo
#define CANARY_CMD "canary"
// Command to find the y value at x of an equation
#define Y_CMD "y"
// Command to find the derivative of an equation
#define DIFFERENTIATE_CMD "dxdy"
// Command to find the indefinite integral of an equation
#define INTEGRATE_CMD "S"
// Command to find the riemann sum of an equation in a range
#define RIEMANN_CMD	"riemann"
// Command to find the one sided limit of a three part equation
#define LIMIT_CMD "lim"
// Command to show help for commands
#define HELP_CMD "help"
// Command to exit the program
#define EXIT_CMD "exit"

#pragma endregion

#pragma region Help Strings

#define HELP_STR \
"y <x> <equation>\n"\
	"\tFinds the y value of <equation> at <x>\n"\
"dxdy <equation>\n"\
	"\tFinds the derivative of <equation>.\n"\
"S <equation>\n"\
	"\tFinds the indefinite integral of <equation>.\n"\
"riemann <type> <start> <end> <numberOfRectangles> <equation>\n"\
	"\tApproximates the definite integral of <equation> using a riemann sum\n"\
"lim <x> <lessThanEquation>,<equalToEquation>,<greaterThanEquation>\n"\
	"\tFinds the one sided limit of <x> using the three part equation entered.\n"

char *functionHelp[] = {
	"dxdy", "dxdy <equation>\n"
		"\tFinds the derivative of <equation>.\n"
		"\t<equation>:\n"
		"	\t\tA simple equation composed of terms \"ax^b\" seperated by \" + \" or \" - \"\n",
	"S", "S <equation>\n"
		"\tFinds the integral of <equation>.\n"
		"\t<equation>:\n"
			"\t\tA simple equation composed of terms \"ax^b\" seperated by \" + \" or \" - \"\n",
	"riemann", "riemann <type> <start> <end> <numberOfRectangles> <equation>\n"
		"\tApproximates the definite integral of <equation> using a riemann sum\n"
		"\t<type>:\n"
			"\t\tThe type of riemann sum to use. It can be \"right\" \"left\" \"midpoint\"\n"
			"\t\tor \"trapezoidal\"\n"
		"\t<start>:\n"
			"\t\tA number indicating the start of the range to integrate.\n"
		"\t<end>:\n"
			"\t\tA number indicating the end of the range to integrate.\n"
		"\t<numberOfRectangles>:\n"
			"\t\tA number indicating the number of rectangles to use in the sum.\n"
		"\t<equation>:\n"
			"\t\tA simple equation composed of terms \"ax^b\" seperated by \" + \" or \" - \"\n",
	"lim", "lim <x> <lessThanEquation>,<equalToEquation>,<greaterThanEquation>\n"
		"\tFinds the one sided limit of <x> using the three part equation entered.\n"
		"\t<x>:\n"
			"\t\tThe value of x to take the limit of.\n"
		"\t<lessThanEquation>:\n"
			"\t\tThe equation when x is less than <x>\n"
		"\t<equalToEquation>:\n"
			"\t\tThe equation when x is equal to <x>\n"
		"\t<greaterThanEquation>:\n"
			"\t\tThe equation when x is greater then <x>\n"
		"\tNote: All equations must be composed of terms \"ax^b\" seperated by \" + \"\n"
		"\t\tor \" + \"\n",
	"y", "y <x> <equation>\n"
		"\tFinds the y value of <equation> at <x>\n"
		"\t<x>:\n"
			"\t\tThe x value to plug into <equation>\n"
		"\t<equation>:\n"
			"\t\tA simple equation composed of terms \"ax^b\" seperated by \" + \" or \" - \"\n"
};

#pragma endregion

#pragma region Structs and Misc. Globals

typedef enum RiemannTypes
{
	Right = 0, Left = 1, Midpoint = 2, Trapezoidal = 3, Other = 4
} RiemannTypes;

char *RiemannTypesStr[] = { "right", "left", "midpoint", "trapezoidal" };

#define CANARY_STR \
"  (o o)" "\n"\
" (  V  )" "\n"\
"/--m-m-" // no newline on purpose


#pragma endregion

#pragma region Command Functions

int Canary()
{
	printf(CANARY_STR);

	return 0;
}

int Y(double x, char *equationStr)
{
	Term *equation = NULL;

	check(equationStr != NULL, INVALID_USAGE);

	size_t equationLen;
	equation = StringToEquation(equationStr, &equationLen);
	check(equation != NULL, INVALID_EQUATION);

	double result = 0;
	check(FindY(equation, equationLen, x, &result) == 0, "Error in FindY: %s", LibMathErr());

	printf("f(%lf) = %lf\n", x, result);

	free(equation);
	return 0;

error:
	free(equation);
	return 1;
}

int Differentiate(char *equationStr)
{
	Term *equation = NULL;
	Term *result = NULL;

	size_t equationLen = 5;
	equation = StringToEquation(equationStr, &equationLen);
	check(equation != NULL, INVALID_EQUATION);

	result = DifferentiateEquation(equation, equationLen);
	PrintEquation(result, equationLen); printf("\n"); // PrintEquation doesn't add newline

	free(result);
	free(equation);
	return 0;

error:
	free(result);
	free(equation);
	return 1;
}

int IndefiniteIntegral(char *equationStr)
{
	Term *equation = NULL;
	Term *result = NULL;

	size_t equationLen;
	equation = StringToEquation(equationStr, &equationLen);
	check(equation != NULL, INVALID_EQUATION);

	result = IntegrateEquation(equation, equationLen);
	PrintEquation(result, equationLen); printf(" + C\n"); // PrintEquation doesn't add newl

	free(equation);
	free(result);
	return 0;

error:
	free(equation);
	free(result);
	return 1;
}

int RiemannSum(RiemannTypes type, char *equationStr, double start, double end, unsigned long numIntervals)
{
	Term *equation = NULL;

	check(equationStr != NULL, "equationStr can't be null.");
	check(type < Other && type >= 0, "type was invalid.");

	size_t equationLen;
	equation = StringToEquation(equationStr, &equationLen);
	check(equation != NULL, INVALID_EQUATION);

	int makeNegative = 0;
	if (start > end)
	{
		double tmp;
		tmp = start;
		start = end;
		end = tmp;
		makeNegative = 1;
	}

	double result = 0;
	double x;
	int isTrapezoidal = 0;
	switch (type)
	{
		case Left:
			x = start;
			break;
		case Right:
			x = start + ((end - start) / (double)numIntervals);
			break;
		case Midpoint:
			x = start + 0.5 * ((end - start) / (double)numIntervals);
			break;
		case Trapezoidal:
			x = start;
			isTrapezoidal = 1;
			break;
		default:
			check(0, "type was invalid.");
	}

	for (unsigned long i = 0; i < numIntervals; i++)
	{
		if (isTrapezoidal)
		{
			double tmpLeft;
			double tmpRight;
			check(FindY(equation, equationLen, x, &tmpLeft) == 0, "Error in FindY: %s", LibMathErr());
			x += (end - start) / (double)numIntervals;
			check(FindY(equation, equationLen, x, &tmpRight) == 0, "Error in FindY: %s", LibMathErr());
			result += (tmpLeft + tmpRight) / 2;
		}
		else
		{
			double tmp;
			check(FindY(equation, equationLen, x, &tmp) == 0, "Error in FindY: %s", LibMathErr());
			result += tmp;
			x += (end - start) / (double)numIntervals;
		}
	}

	result *= (end - start) / (double)numIntervals;

	if (makeNegative)
	{
		result *= -1;
	}

	printf("%lf\n", result);

	free(equation);
	return 0;

error:
	free(equation);
	return 1;
}

int OneSidedLimit(double x, char *equationStr)
{
	char *nextToken = NULL;
	char *equationSep = ",";
	
	char *lessThanStr = NULL;
	char *equalToStr = NULL;
	char *greaterThanStr = NULL;

	Term *lessThan = NULL;
	size_t lessThanLen = 0;
	Term *equalTo = NULL;
	size_t equalToLen = 0;
	Term *greaterThan = NULL;
	size_t greaterThanLen = 0;

	lessThanStr = strtok_s(equationStr, equationSep, &nextToken);
	check(lessThanStr != NULL, INVALID_USAGE);
	lessThan = StringToEquation(lessThanStr, &lessThanLen);
	check(lessThan != NULL, INVALID_EQUATION);

	equalToStr = strtok_s(NULL, equationSep, &nextToken);
	check(equalToStr != NULL, INVALID_USAGE);
	equalTo = StringToEquation(equalToStr, &equalToLen);
	check(equalTo != NULL, INVALID_EQUATION);

	greaterThanStr = nextToken;
	check(*greaterThanStr != '\0', INVALID_USAGE);
	greaterThan = StringToEquation(greaterThanStr, &greaterThanLen);
	check(greaterThan != NULL, INVALID_EQUATION);

	double lessResult;
	double greaterResult;
	double atResult;
	check(FindY(lessThan, lessThanLen, x, &lessResult) == 0, "Error in FindY: %s", LibMathErr());
	printf("x->%lf- = %lf\n", x, lessResult);
	check(FindY(greaterThan, greaterThanLen, x, &greaterResult) == 0, "Error in FindY: %s", LibMathErr());
	printf("x->%lf+ = %lf\n", x, greaterResult);

	switch (lessResult == greaterResult)
	{
		case 0:
			printf("x->%lf = DNE\n", x);
			break;
		default:
			printf("x->%lf = %lf\n", x, lessResult);
			break;
	}

	check(FindY(equalTo, equalToLen, x, &atResult) == 0, "Error in FindY: %s", LibMathErr());
	printf("f(%lf) = %lf\n", x, atResult);

	free(lessThan);
	free(equalTo);
	free(greaterThan);
	return 0;

error:
	free(lessThan);
	free(equalTo);
	free(greaterThan);
	return 1;
}


int ShowHelp(char *functionName)
{
	for (size_t i = 0; i < sizeof(functionHelp) / sizeof(char*); i += 2)
	{
		if (strcmp(functionName, functionHelp[i]) == 0)
		{
			printf(functionHelp[i + 1]);
			return 0;
		}
	}

	printf(INVALID "\n");

	return 1;
}

#pragma endregion

#pragma region Command handling

int RunCommand(char *string)
{
	check(string != NULL, "string can't be null.");

	char *seperators = " \n";
	char* nextToken = NULL;
	char *command = strtok_s(string, seperators, &nextToken);
	check(command != NULL, INVALID);
	if (strcmp(command, CANARY_CMD) == 0)
	{
		check(*nextToken == '\0', INVALID_USAGE);
		Canary();
	}
	else if (strcmp(Y_CMD, command) == 0)
	{
		char *lastReadChar = NULL;
		char *xStr = strtok_s(NULL, seperators, &nextToken);
		check(xStr != NULL, INVALID_USAGE);

		double x = strtod(xStr, &lastReadChar);
		check(*lastReadChar == '\0', INVALID_USAGE);
		check(*nextToken != '\0', INVALID_USAGE);

		Y(x, nextToken);
	}
	else if (strcmp(DIFFERENTIATE_CMD, command) == 0)
	{
		check(*nextToken != '\0', INVALID_USAGE);
		Differentiate(nextToken);
	}
	else if (strcmp(INTEGRATE_CMD, command) == 0)
	{
		check(*nextToken != '\0', INVALID_USAGE);
		IndefiniteIntegral(nextToken);
	}
	else if (strcmp(RIEMANN_CMD, command) == 0)
	{
		char *typeStr = strtok_s(NULL, seperators, &nextToken);
		check(*typeStr != '\0', INVALID_USAGE);
		RiemannTypes type = Other;
		for (int i = 0; i < sizeof(RiemannTypesStr) / sizeof(char*); i++)
		{
			if (strcmp(typeStr, RiemannTypesStr[i]) == 0)
			{
				type = i;
			}
		}
		check(type != -1, INVALID);

		char *lastReadChar;

		double start;
		char *startStr = strtok_s(NULL, seperators, &nextToken);
		check(startStr != NULL, INVALID_USAGE);
		start = strtod(startStr, &lastReadChar);
		check(*lastReadChar == '\0', INVALID_USAGE);

		double end;
		char *endStr = strtok_s(NULL, seperators, &nextToken);
		check(endStr != NULL, INVALID_USAGE);
		end = strtod(endStr, &lastReadChar);
		check(*lastReadChar == '\0', INVALID_USAGE);

		long long numIntervals;
		char *intervalStr = strtok_s(NULL, seperators, &nextToken);
		check(intervalStr != NULL, INVALID_USAGE);
		numIntervals = strtoll(intervalStr, &lastReadChar, 10);
		check(*lastReadChar == '\0', INVALID_USAGE);
		check(numIntervals >= 0, INVALID_USAGE);

		char *equationStr = nextToken;
		check(equationStr != NULL, INVALID_USAGE);

		RiemannSum(type, equationStr, start, end, (unsigned long)numIntervals);
	}
	else if (strcmp(LIMIT_CMD, command) == 0)
	{
		char *lastReadChar;

		double x;
		char *xString;
		xString = strtok_s(NULL, seperators, &nextToken);
		check(xString != NULL, INVALID_USAGE);
		x = strtod(xString, &lastReadChar);
		check(*lastReadChar == '\0', INVALID_USAGE);

		char *equationStr = nextToken;
		check(*equationStr != '\0', INVALID_USAGE);
		OneSidedLimit(x, equationStr);
	}
	else if (strcmp(HELP_CMD, command) == 0)
	{
		if (*nextToken != '\0')
		{
			ShowHelp(nextToken);
		}
		else
		{
			printf(HELP_STR);
		}
	}
	else if (strcmp(EXIT_CMD, command) == 0)
	{
		return 2; // means safe exit
	}
	else
	{
		check(0, INVALID);
	}

	return 0;

error:
	return 1;
}

#pragma endregion

#pragma region Main

int main(int argc, char *argv[])
{

	printf("CalcCanary v1.1.0\nCreated by MrBitShift\nSee LICENSE for legal info.\n");

	printf(CANARY_STR);

	char *command;
	while (1)
	{
		printf(":> ");
		command = ReadLine();
		check(command != NULL, "Could not read user input. Exiting...");
		int rc = RunCommand(command);
		free(command);
		if (rc == 2) // means safe exit
		{
			break;
		}
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return 0;

error:
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return 1;
}

#pragma endregion