//
//  main.cpp
//  Calculatrice
//

//#include "stdafx.h"  //visual studio...

#include "main.h"

using namespace std;

#define regex_digits "([\\d]*)"
#define regex_number "((!(\\*|\\/||\\+|-)\\+|-)?" regex_digits "\\.?" regex_digits "?)"
#define regex_operators "(\\+|-|\\*|\\/|\\%)"
#define regex_bloc "\\((" regex_number regex_operators regex_number "?)*\\)"
#define regex_function "([a-zA-Z]+\\((" regex_number "(" regex_operators regex_number ")?)*\\))"

#define free_vector(type, vec) //vec.clear(); vector<type>().swap(vec)


#ifdef _WIN32
    #define ok "ok"
    #define bad ":("
#else
    #define ok "👍🏼"
    #define bad "👎🏼"
#endif

enum LastParsed {
	kLastParsedNone,
	kLastParsedNumber,
	kLastParsedOperator
};

enum Operator : int {
    kAddOperator       = 0,
    kSubstractOperator = 1,
    kMultiplyOperator  = 2,
    kDivideOperator    = 3,
    kModulusOperator   = 4
};
enum Sign : int {
    kNotSigned,
    kSignPositive,
    kSignNegative
};

string getInput() {
	string input;

	getline(cin, input);
    
    regex reg("[^a-zA-Z0-9`!@#$%^&*()_+|\\-=\\{}\\[\\]:"";'<>?,./]");
    
    string fixed = regex_replace(input, reg, "");

    transform(fixed.begin(), fixed.end(), fixed.begin(), ::tolower);

	return fixed;
}

int vectorIndex(vector<string> vec, string str) {
	int index = -1,
        i = -1;

	while(++i < (int)vec.size()) {
		if(vec[i] == str) {
			index = i;
		}
	}

	return index;
}

int operatorIndex(string str) {
	vector<string> operations = {
		"+",
		"-",
		"*",
		"/",
        "%"
	};

	int index = vectorIndex(operations, str);
    
    free_vector(string, operations);
    
    return index;
}

bool isOperator(string input) {
	int index = operatorIndex(input);

	return index != -1;
}
bool isNumber(string input) {
	vector<string> numbers = {
		"0", "1", "2",
        "3", "4", "5",
        "6", "7", "8",
        "9", ".", "-"
	};

	int index = vectorIndex(numbers, input);
    
    free_vector(string, numbers);
    
	return index != -1;
}
bool isBrackets(string input) {
    return input == "(" || input == ")";
}


long double compute(vector<Operator> operators, vector<long double> numbers) {
	if (numbers.size() == 0) {
		return 0;
	}

	long double value = numbers[0];
    
    if(operators.size() != numbers.size() - 1) {
        throw invalid_argument("number/operators mismatch");
    }
    
    int i = 0;
    
    while(++i < (int)numbers.size()) {
		long double number = numbers[i];
		Operator op = operators[i - 1];

		switch(op) {
            case kAddOperator:
                value += number;
                break;
            case kSubstractOperator:
                value -= number;
                break;
            case kMultiplyOperator:
                value *= number;
                break;
            case kDivideOperator:
                if (number == 0) {
                    throw invalid_argument("Division par zéro");
                }

                value /= number;
                break;
            case kModulusOperator:
                value = (int)value % (int)number;
                break;
            default:
                cout << "WTF unknown op : " << op << endl;
                break;
		}
	}
    
    free_vector(Operator, operators);
    free_vector(long double, numbers);
    
	return value;
}

string getStringChar(string input, int position) {
    return input.substr(position, 1);
}


long double parseString(string input) {
	vector<Operator> operators;
	vector<long double> numbers;

	LastParsed lastParsed = kLastParsedNone;
	unsigned int i = 0;
    Sign sign = kNotSigned;
    
    string character, nextCharacter;
    bool nextCharacterIsNumber, characterIsOperator;

	do {
        character = i == 0 ? getStringChar(input, i) : nextCharacter;
        nextCharacter = getStringChar(input, i + 1);
        characterIsOperator = isOperator(character);
        nextCharacterIsNumber = isNumber(nextCharacter) && !isOperator(nextCharacter);
        
		if(isNumber(character) && !characterIsOperator) {
            long double value = 0;
            bool ignore = false;
            
			if(lastParsed == kLastParsedNumber && !nextCharacterIsNumber) {
                string number, prevChar = character;
                int position = i;
                
                while(isNumber(prevChar) && !isOperator(prevChar)) {
                    number = prevChar + number;
                    
                    if(position != 0) {
                        prevChar = getStringChar(input, --position);
                    }
                    else {
                        break;
                    }
                }
                
                value = stold(number);
			}
            else if(!nextCharacterIsNumber) {
                value = stold(character);
            }
            else {
                ignore = true;
            }
            
            if(!ignore) {
                numbers.push_back(value * (sign == kSignNegative ? -1 : 1));
            }

			lastParsed = kLastParsedNumber;
		}
        else if(characterIsOperator) {
			if(i == 0) {
                free_vector(Operator, operators);
                free_vector(long double, numbers);
                
				return parseString("0" + input);
			}
            string prevChar = getStringChar(input, i - 1);
            
            if((character == "-" || character == "+") && (isOperator(prevChar) || isBrackets(prevChar))) {
                sign = character == "-" ? kSignNegative : kSignPositive;
                lastParsed = kLastParsedNumber;
            }
            else {
                operators.push_back((Operator)operatorIndex(character));
                
                sign = kNotSigned;
                lastParsed = kLastParsedOperator;
            }
		}
        else if(!isBrackets(character)) {
            throw invalid_argument("Bad formula format");
        }
    } while(++i < input.length());

	return compute(operators, numbers);
}

long double evalFn(string input) {
    int i = -1,
        length = (int)input.length();
    
    long double value = 0;
    string fnName, bloc;
    
    while(++i < length) {
        string character = getStringChar(input, i);
        
        if(character == "(") {
            bloc = input.substr(i + 1, length - (i + 1) - 1);
            
            break;
        }
        else {
            fnName = fnName + character;
        }
    }
    
    if(bloc.length()) {
        long double blocVal = calc(bloc);
        
        if(fnName == "sqrt") {
            value = sqrt(blocVal);
        }
        else if(fnName == "cos") {
            value = cos(blocVal);
        }
        else if(fnName == "sin") {
            value = sin(blocVal);
        }
        else if(fnName == "log") {
            value = log(blocVal);
        }
        else {
            cout << "Unknow function called '" << fnName << "'" << endl;
        }
    }
    
    return value;
}


long double calc(string input) {
    regex fn_regex(regex_function);
    
	smatch results;
    
	regex_search(input, results, fn_regex);

    if(results.size()) {
        string operation = results[0];
        
        long double value = evalFn(operation);
        
        string computedString = input.replace(results.position(), operation.length(), to_string(value));
        
        return calc(computedString);
    }
    
    regex bloc_regex(regex_bloc);
        
    regex_search(input, results, bloc_regex);
        
    if(results.size()) {
        string operation = results[0],
               bloc = operation.substr(1, operation.length() - 2),
               computedOperation = to_string(calc(bloc)),
               computedString = input.replace(results.position(), operation.length(), computedOperation);
        
        return calc(computedString);
    }
        
    regex priority_regex(regex_number "(\\*|\\/|\\%)" regex_number);
            
    regex_search(input, results, priority_regex);
            
    if(results.size()) {
        string operation = results[0],
                computedOperation = to_string(parseString(operation)),
                computedString = input.replace(results.position(), operation.length(), computedOperation);
                
        return calc(computedString);
    }
    
    return parseString(input);
}


void test() {
	vector<string> operations = {
        "1*-1",
        "-1*1",
        "0-1",
        "-1",
        "1+2%3",
        "(1+2)%3",
		"1+2*3",
		"(1+2)*3",
		"1+2*3/4",
        "(1+2*3)/4",
        "(12*34)/(12*34)",
        "((12*34)/(12*34))%5",
        "((1+2)*(2*1.5)/5.000)*(12-4.5)",
        "((1+2)*(2*1.5)/5.000)*(12-4.5*3)",
        "(1*2/4+5)/((1*-2)+3)+(4*5)",
        "12%34",
        "sqrt(4)",
        "sqrt(2*2)",
        "sqrt(2)*sqrt(2)",
        "sqrt(2*2)*2"
    };
    vector<long double> values = {
        -1,
        -1,
        -1,
        -1,
        3,
        0,
		7,
		9,
        2.5,
        1.75,
        1,
        1,
        13.5,
        -2.7,
        25.5,
        10,
        2,
        2,
        2,
        4
	};
    unsigned int i = 0;
    
	do {
		string operation = operations[i];
        
        try {
            long double value = calc(operation),
                        okValue = values[i];
            
            int a = value * 100,
                b = value * 100;
            
            if(a == b) {
                cout << "OK : " << operation << " = " << value << endl;
            }
            else {
                cout << "Erreur : " << operation << " = " << value << " != " << okValue << endl;
            }
        }
        catch(...) {
            cout << "Crash lors du test de l'operation \"" << operation << "\"" << endl;
        }
    } while(++i < operations.size());

	try {
        double value = calc("1/0");
        
        cout << "Erreur : 1/0 = " << value << " != crash " << endl;
	}
	catch (...) {
		cout << "OK : 1/0 = crash" << endl;
	}
}

void help() {
    cout << "####- Calculatrice " << endl;
    cout << "Commandes:" << endl;
    cout << "\t t ou test => lancer la procédure de test" << endl;
    cout << "\t q ou quit => quitter le programme" << endl;
    cout << "\t h ou help => afficher cette page" << endl;
    cout << "Syntaxe:" << endl;
    cout << "\t Opérateurs:" << endl;
    cout << "\t\t + => addition" << endl;
    cout << "\t\t - => soustraction" << endl;
    cout << "\t\t * => multiplication" << endl;
    cout << "\t\t / => division" << endl;
    cout << "\t\t % => modulo" << endl << endl;
    cout << "\t Fonctions:" << endl;
    cout << "\t\t sqrt => racine carée" << endl;
    cout << "\t\t log  => logarithme" << endl;
    cout << "\t\t cos  => cosinus" << endl;
    cout << "\t\t sin  => sinus" << endl << endl;
    cout << "\t Exemple:" << endl;
    cout << "\t\t <= (1+2)*4/(5-6+7)" << endl;
    cout << "\t\t => 2" << endl;
    cout << "\t\t <= sqrt(2*2)*2" << endl;
    cout << "\t\t => 4" << endl;
}

int main(int argc, const char * argv[]) {
    cout << "Entrez une opération (q pour quitter, h pour aide) : " << endl;
    
	do {
        cout << "<= ";
        
		string input = getInput();
        
		if(input == "q" || input == "quit") {
			break;
		}
        if(input == "h" || input == "help") {
            help();
        }
		else if(input == "test" || input == "t") {
			test();
		}
        else if(input == "") {
            cout << "=> " << ok << endl;
        }
        else {
            cout << "=> ";
            
            try {
                long double value = calc(input);
                
                cout << input << " = " << value << endl;
            }
            catch (...) {
                cout << bad << " Mouaah y'a comme un problème là" << endl;
            }
        }

	} while(true);

	return 0;
}
