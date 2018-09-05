#include <iostream>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <tchar.h>
#include <string>       
#include <sstream> 

using namespace std;

vector<char> identifiers;
vector<char> numbers;
vector<int> FollowE;
vector<int> FollowT;
vector<int> FollowL;
vector<int> SINGLE;
vector<int> DOUBLE;
vector<int> LITERACY;
int MAX_TOKENS = 1000;
int MAX_SINGLE = 23;
int MAX_DOUBLE = 4;
int MAX_KEYWORDS = 30;

string tmp;

struct Token {
	char a;
	int n;
};

struct Identifier {
	string name;
	int identifierType;
	int arrayType;
	int arraySize;
};

class LexicalParser {
public:
	LexicalParser();
	~LexicalParser();

	vector<Identifier> identifiersToken;
	vector<int> numbersToken;
	vector<char> singleDelimitersToken;
	vector<string> literalsToken,
		keywordsToken,	
		doubleDelimitersToken;
	
	int totalNumbers,
		totalLiterals,	
		totalIdentifiers;	
	
	FILE *lex_file, *lex_tokens;
	fpos_t lex_filePosition;

	Token lex(bool changePosition);

	int addIdentifierToTable(string currenIdentifier, char &token);
	int addLiteralToTable(string currentLiteral);
	int addNumberToTable(int currentNumber);
	int singleDelimiter(string sDelimiter);
	int doubleDelimiter(string dDelimiter);
	bool in(char x, vector<char> array);
	void initializeTokens();
	void error();
};

LexicalParser::LexicalParser() {	
	lex_file = fopen("sourceCode.txt", "r");
	lex_tokens = fopen("tokens.txt", "w+");
	
	totalIdentifiers = 0;
	totalLiterals = 0;
	totalNumbers = 0;
	lex_filePosition = 0;

	initializeTokens();
}

LexicalParser::~LexicalParser() {
	fclose(lex_file);
}

Token LexicalParser::lex(bool changePosition) {	
	Token result;
	int state = 0,
		lexemeNumber;
	char x, T;
	string currentLexeme;

	fsetpos(lex_file, &lex_filePosition);

	result.a = 'G';
	result.n = -1;

	while (true) {
		switch (state) {
			case 0 :
				x = fgetc(lex_file);
				// перевод строки (\xA, \xD) или пробел (\x20) или символ табуляции (\x9)
				if ((x == '\xA') || (x == '\xD') || (x == '\x20') || (x == '\x9'))
					x = fgetc(lex_file);
				if (in(x, identifiers)) // char
					state = 1;
				if (in(x, numbers)) // number
					state = 2;
				if (x == '\x27') // \x27 = '
					state = 3;
				if (x == '\x2F') // \x2F = /
					state = 5;				
				if (in(x, singleDelimitersToken) && (state == 0)) // single delimiter и не ' (для literal)
					state = 4;
				if (feof(lex_file)) // Конец файла
					state = 6;	
			break;
			case 1 : // identifier
				while (in(x, identifiers) || in(x, numbers)) {
					currentLexeme += x;
					if (feof(lex_file))
						break;
					x = fgetc(lex_file);
				}
				fseek(lex_file, -1, SEEK_CUR);
				lexemeNumber = addIdentifierToTable(currentLexeme, T);
				result.a = T;
				result.n = lexemeNumber;
				state = 7;
			break;
			case 2 : // number
				do {
					currentLexeme += x;
					if (feof(lex_file))
						break;
					x = fgetc(lex_file);
				} while (in(x, numbers));
				fseek(lex_file, -1, SEEK_CUR);
				lexemeNumber = addNumberToTable(atoi(currentLexeme.c_str()));
				result.a = 'C';
				result.n = lexemeNumber;
				state = 7;
			break;
			case 3 : // literal
				currentLexeme = x;
				do {
					if (feof(lex_file))
						break;
					x = fgetc(lex_file);
					currentLexeme += x;
				} while (x != '\x27'); // \x27 = '
				currentLexeme = currentLexeme.substr(1, currentLexeme.length() - 2);
				lexemeNumber = addLiteralToTable(currentLexeme);
				result.a = 'L';
				result.n = lexemeNumber;
				state = 7;
			break;
			case 4 : // delimiters
				currentLexeme = x;
				if ((x == '>') || (x == '<') || (x == '=') || (x == '!')) { // double delimiter
					x = fgetc(lex_file);
					if (x == '=') { // double
						currentLexeme += x;
						lexemeNumber = doubleDelimiter(currentLexeme);
						if (lexemeNumber != -1) {
							result.a = 'D';
							result.n = lexemeNumber;
						}
						else
							error();
					}
					else { // single
						lexemeNumber = singleDelimiter(currentLexeme);
						if (lexemeNumber != -1) {
							fseek(lex_file, -1, SEEK_CUR);
							result.a = 'R';
							result.n = lexemeNumber;
						}
						else
							error();
					}
				}
				else { // only single
					lexemeNumber = singleDelimiter(currentLexeme);
					if (lexemeNumber != -1) {
						result.a = 'R';
						result.n = lexemeNumber;
					}
					else
						error();
				}
				state = 7;
			break;
			case 5 : // comment
				currentLexeme = x;
				x = fgetc(lex_file);
				switch (x) {
					case '\x2F' : // \x2F = /
						do {
							currentLexeme += x;
							x = fgetc(lex_file);
						} while (!((x == '\xA') || feof(lex_file)));
						currentLexeme = "";
						state = 0;
					break;
					case '\x2A': // \x2A = *
						do {
							currentLexeme += x;
							x = fgetc(lex_file);
						} while (!((x == '\x2F') && (currentLexeme[currentLexeme.length() - 1] == '\x2A')));
						currentLexeme += x;
						state = 0;
					break;
					default :
						result.a = 'R';
						result.n = 4;
						fseek(lex_file, -1, SEEK_CUR);
						state = 7;
					break;
				};
			break;
			case 6 : // end of file
				result.a = 'R';
				result.n = singleDelimiter(tmp = x);
				state = 7;
			break;
			case 7 : // result
				if (changePosition == true)
					fgetpos(lex_file, &lex_filePosition);
				fseek(lex_tokens, 0, SEEK_SET);
				fputc(result.a, lex_tokens);
				fputc(result.n, lex_tokens);
				fclose(lex_tokens);
				return result;
			break;
		}
	}
}

int LexicalParser::addIdentifierToTable(string currenIdentifier, char &token) {
	int result;
	bool isAdded = false,
		isKeyword = false;
	// search in keywords
	for (int i = 0; i < MAX_KEYWORDS; i++)
		if (currenIdentifier == keywordsToken[i]) {
			isKeyword = true;
			result = i;
			token = 'K';
			break;
		}
	// not found in keywords
	if (isKeyword == false) {
		token = 'I';
		for (int i = 0; i < totalIdentifiers; i++)
			if (identifiersToken[i].name == currenIdentifier) {
				isAdded = true;
				result = i;
				break;
			}
		// add new identifier
		if (isAdded == false) {
			result = totalIdentifiers;
			Identifier newIdentifier;
			newIdentifier.name = currenIdentifier;
			identifiersToken.push_back(newIdentifier);
			totalIdentifiers++;
		}
	}
	return result;
}

int LexicalParser::addLiteralToTable(string currentLiteral) {
	int result;
	bool isAdded = false;
	// search in literals
	for (int i = 0; i < totalLiterals; i++)
		if (literalsToken[i] == currentLiteral) {
			isAdded = true;
			result = i;
			break;
		}
	// add new literal
	if (isAdded == false) { // not found in literals
		result = totalLiterals;
		literalsToken.push_back(currentLiteral);
		totalLiterals++;
	}
	return result;
}

int LexicalParser::addNumberToTable(int currentNumber) {
	int result;
	bool isAdded = false;
	// search in numbers
	for (int i = 0; i < totalNumbers; i++)
		if (numbersToken[i] == currentNumber) {
			isAdded = true;
			result = i;
			break;
		}
	// add new number
	if (isAdded == false) { // not found 
		result = totalNumbers;
		numbersToken.push_back(currentNumber);
		totalNumbers++;
	}
	return result;
}

int LexicalParser::singleDelimiter(string sDelimiter) {
	for (int i = 0; i < MAX_SINGLE; i++)
		if (singleDelimitersToken[i] == sDelimiter[0])
			return i;
	return -1;
}

int LexicalParser::doubleDelimiter(string dDelimiter) {
	for (int i = 0; i < MAX_DOUBLE; i++)
		if (doubleDelimitersToken[i] == dDelimiter)
			return i;
	return -1;
}

bool LexicalParser::in(char x, vector<char> array) {
	return (find(array.begin(), array.end(), x) != array.end());
}

void LexicalParser::initializeTokens() {
	char* line;
	FILE* strm;
	// request a change in capacity
	keywordsToken.reserve(MAX_KEYWORDS);
	singleDelimitersToken.reserve(MAX_SINGLE);
	doubleDelimitersToken.reserve(MAX_DOUBLE);
	// load from file double delimiters 
	strm = fopen("doubleDelimiters.txt", "r");
	while (fgets(line, 1024, strm)) {
		string tmp = line; 
		doubleDelimitersToken.push_back(tmp.substr(0, tmp.length() - 1));
	}
	fclose(strm);
	// load from file single delimiters
	strm = fopen("singleDelimiters.txt", "r");	
	while (fgets(line, 1024, strm)) {
		char tmp = line[0];		
		singleDelimitersToken.push_back(tmp);			
	}
	fclose(strm);
	// load from file keywords
	strm = fopen("keywords.txt", "r");
	while (fgets(line, 1024, strm)) {
		string tmp = line; 
		keywordsToken.push_back(tmp.substr(0, tmp.length() - 1));
	}
	fclose(strm);	
	// init elements identifiers and numbers
	identifiers.push_back('_');
	for (char i = 'a'; i <= 'z'; i++)
		identifiers.push_back(i);
	for (char i = 'A'; i <= 'Z'; i++)
		identifiers.push_back(i);
	for (char i = '0'; i <= '9'; i++)
		numbers.push_back(i);
	// follow E arithmetic
	FollowE.push_back(6); 
	FollowE.push_back(7); 
	FollowE.push_back(8); 
	FollowE.push_back(9); 
	FollowE.push_back(11); 
	FollowE.push_back(13); 
	FollowE.push_back(14); 
	FollowE.push_back(15); 
	FollowE.push_back(16);
	// follow T arithmetic
	FollowT.push_back(1); 
	FollowT.push_back(2); 
	FollowT.push_back(6); 
	FollowT.push_back(7); 
	FollowT.push_back(8); 
	FollowT.push_back(9); 
	FollowT.push_back(11); 
	FollowT.push_back(13); 
	FollowT.push_back(14);
	FollowT.push_back(15); 
	FollowT.push_back(16);
	// follow logic
	FollowL.push_back(6); 
	FollowL.push_back(11);
	FollowL.push_back(13); 
	FollowL.push_back(15);
	//
	SINGLE.push_back(7); 
	SINGLE.push_back(8);
	SINGLE.push_back(9);
	SINGLE.push_back(18);
	//
	DOUBLE.push_back(0); 
	DOUBLE.push_back(1);
	DOUBLE.push_back(2);
	DOUBLE.push_back(3);
	//
	LITERACY.push_back(2);
	LITERACY.push_back(4);
	LITERACY.push_back(8);
	LITERACY.push_back(9);
	LITERACY.push_back(10);
	LITERACY.push_back(12);
	LITERACY.push_back(16);
	LITERACY.push_back(19);
	LITERACY.push_back(20);
	LITERACY.push_back(21);
	LITERACY.push_back(22);
	LITERACY.push_back(23);
	LITERACY.push_back(25);
	LITERACY.push_back(26);
	LITERACY.push_back(27);
	//
	for (int i = 0; i < 0; i++) {}
}

void LexicalParser::error() {
	cout << "Incorrect symbol" << endl;
	exit(1);
}

class SyntaxParser {
public:
	SyntaxParser();
	~SyntaxParser();

	char t_a;
	int t_n,
		labelCount,
		cmpType,
		currenIdentifier,
		baseType,
		arraySize;
	FILE *asm_code, *asm_variables;

	void scan(bool arg);
	
	bool inFollow(int x, vector<int> array);
	string generateAsmLabel();

	void E();
	void T();
	void F();
	void EL();
	void TL();
	void FL();
	void Z();

	void f_program();
	void f_variables();
	void f_type();
	void f_operatorsBlock();
	void f_operator();
	void f_if();
	void f_let();
	void f_while();
	void f_read(bool ln);
	void f_readInt(int idenNum);
	void f_readBool(int idenNum);
	void f_readChar(int idenNum);
	void f_write(bool ln);
	void f_writeInt();
	void f_writeLiteral();
	void f_writeChar(int idenNum);
	void f_writeBool();
	void error(int errorCode, string oper);
	void f_repeat(); 
	
	void compile();

	LexicalParser lexicalParser;
};

SyntaxParser::SyntaxParser() {
	labelCount = 0;
	asm_code = fopen("asmKod.asm", "w+");
	asm_variables = fopen("asmData.asm", "w+");
}

SyntaxParser::~SyntaxParser() {
	fclose(asm_code);
	fclose(asm_variables);
}

void SyntaxParser::scan(bool arg) {
	Token scannedToken = lexicalParser.lex(arg);
	t_a = scannedToken.a;
	t_n = scannedToken.n;
}

bool SyntaxParser::inFollow(int x, vector<int> array) {
	return (find(array.begin(), array.end(), x) != array.end());
}

string SyntaxParser::generateAsmLabel() {
	stringstream ss; 
	ss << labelCount++;
	return "@" + ss.str();
}

// E -> T {+T | -T}
void SyntaxParser::E() {
	T(); // T
	scan(false);
	while (!((t_a == 'R') && inFollow(t_n, FollowE)))
		if ((t_a == 'R') && (t_n == 1)) { // +
			scan(true);
			T(); // T
			fprintf(asm_code, "     pop   bx \n");
			fprintf(asm_code, "     pop   ax \n");
			fprintf(asm_code, "     add   ax, bx \n");
			fprintf(asm_code, "     push  ax \n");
		} else if ((t_a == 'R') && (t_n == 2)) { // -
			scan(true);
			T(); // T
			fprintf(asm_code, "     pop   bx \n");
			fprintf(asm_code, "     pop   ax \n");
			fprintf(asm_code, "     sub   ax, bx \n");
			fprintf(asm_code, "     push  ax \n");
		} else if (!( ((t_a == 'R') && inFollow(t_n, SINGLE)) || ((t_a == 'D') && inFollow(t_n, DOUBLE)) || ((t_a == 'K') && inFollow(t_n, LITERACY)) ))
			error(2, "E arithmetic"); 
		else
			break;
}

// T -> F {*F | /F}
void SyntaxParser::T() {
	F(); // F
	scan(false);
	while (!((t_a == 'R') && inFollow(t_n, FollowT)))
		if ((t_a == 'R') && (t_n == 3)) { // *
			scan(true);
			F(); // F
			scan(false);
			fprintf(asm_code, "     pop   bx \n");
			fprintf(asm_code, "     pop   ax \n");
			fprintf(asm_code, "     mul   bx \n");
			fprintf(asm_code, "     push  ax \n");
		} else if ((t_a == 'R') && (t_n == 4)) { // /
			scan(true);
			F(); // F
			scan(false);
			fprintf(asm_code, "     pop   bx \n");
			fprintf(asm_code, "     pop   ax \n");
			fprintf(asm_code, "     xor   dx, dx \n");
			fprintf(asm_code, "     div   bx \n");
			fprintf(asm_code, "     push  ax \n");
		} else if (!( ((t_a == 'R') && inFollow(t_n, SINGLE)) || ((t_a == 'D') && inFollow(t_n, DOUBLE)) || ((t_a == 'K') && inFollow(t_n, LITERACY)) ))
			error(3, "T arithmetic");
		else
			break;
}

// F -> '(' E ')' | I ('[' E ']' | eps) | C
void SyntaxParser::F() {
	int idenNum;
	scan(false);
	if (t_a == 'C') { // C
		scan(true);
		fprintf(asm_code, "     mov   ax, %d \n", lexicalParser.numbersToken[t_n]);		
		fprintf(asm_code, "     push  ax \n");
	} else if (t_a == 'I') { // I ('[' E ']')
			fprintf(asm_code, "     mov   di, 0 \n");
			idenNum = t_n;
			scan(true);
			scan(false);
			if ((t_a == 'R') && (t_n == 10)) { // '[' E ']'
				scan(true);
				E();
				fprintf(asm_code, "     pop   di \n");
				fprintf(asm_code, "     shl   di, 1 \n");
				scan(true);
				if (!((t_a == 'R') && (t_n == 11))) // ']'
					error(4, "F arithmetic");
			}
			if ((lexicalParser.identifiersToken[idenNum].identifierType == 3) || (lexicalParser.identifiersToken[idenNum].arrayType == 3)) { // bool
				fprintf(asm_code, "     xor   ax, ax \n");
				fprintf(asm_code, "     mov   al, %s[di] \n", lexicalParser.identifiersToken[idenNum].name.c_str());
			} else
				fprintf(asm_code, "     mov   ax, %s[di] \n", lexicalParser.identifiersToken[idenNum].name.c_str());
			fprintf(asm_code, "     push  ax \n");
		} else if ((t_a == 'R') && (t_n == 5)) { // '(' E ')'
			scan(true);
			E();
			scan(true);
			if (!((t_a == 'R') && (t_n == 6))) // ')'
				error(5, "F");
		}
}

// El -> Tl {'or' Tl}
void SyntaxParser::EL() {
	TL();
	scan(false);
	while (!(((t_a == 'R') && inFollow(t_n, FollowL)) || ((t_a == 'K') && ((t_n == 8) || (t_n == 22))))) // K8 = else; K22 = then
		if ((t_a == 'K') && (t_n == 17)) { // K17 = or
			scan(true);
			TL();
			fprintf(asm_code, "     pop   bx \n");
			fprintf(asm_code, "     pop   ax \n");
			fprintf(asm_code, "     or    ax, bx \n");
			fprintf(asm_code, "     push  ax \n");
		} else
			error(6, "E logic");
}

// Tl -> Fl {'and' Fl}
void SyntaxParser::TL() {
	FL();
	scan(false);
	while (!(((t_a == 'R') && inFollow(t_n, FollowL)) || ((t_a == 'K') && ((t_n == 8) || (t_n == 17) || (t_n == 22))))) // K8 = else; K17 = or; K22 = then
		if ((t_a == 'K') && (t_n == 0)) { // K1 = and
			scan(true);
			FL();
			fprintf(asm_code, "     pop   bx \n");
			fprintf(asm_code, "     pop   ax \n");
			fprintf(asm_code, "     and   ax, bx \n");
			fprintf(asm_code, "     push  ax \n");
		} 
}

// Fl -> 'true' | 'false' | 'not' Fl | '(' El ')' | '[' E Zn E ']' | I ('[' E ']' | eps)
void SyntaxParser::FL() {
	int idenNum;
	string trueLabel, falseLabel;
	scan(false);
	if (t_a == 'C')
		error(30, "F logic");	
	// 'true' | 'false' | I ('[' E ']' | eps)
	if (((t_a == 'K') && ((t_n == 11) || (t_n == 23))) || (t_a == 'I')) { // K11 = false; K23 = true
		scan(true);
		if (t_a == 'K') // 'true' | 'false'
			switch (t_n) {
				case 11 : fprintf(asm_code, "     push  0 \n"); break; // 'false'
				case 23 : fprintf(asm_code, "     push  1 \n"); break; // 'true'
			}
		if (t_a == 'I') { // I ('[' E ']' | eps)
			fprintf(asm_code, "     mov   di, 0 \n");
			idenNum = t_n;
			scan(false);
			if ((t_a == 'R') && (t_n == 10)) { // '[' E ']'
				scan(true);
				E();
				fprintf(asm_code, "     pop   di \n");
				scan(true);
				if (!((t_a == 'R') && (t_n == 11))) // ']'
					error(4, "F logic");
			}
			fprintf(asm_code, "     xor   ah, ah \n");
			fprintf(asm_code, "     mov   al, %s[di] \n", lexicalParser.identifiersToken[idenNum].name.c_str());
			fprintf(asm_code, "     push  ax \n");
		}
	} else {
		if ((t_a == 'K') && (t_n == 15)) { // 'not' Fl
			scan(true);
			FL();
			fprintf(asm_code, "     pop   ax \n");
			fprintf(asm_code, "     not   ax \n");
			fprintf(asm_code, "     push  ax \n");
		} else { 
			if ((t_a == 'R') && (t_n == 10)) { // '[' E Zn E ']'
				scan(true);
				E();
				Z();
				E();

				trueLabel = generateAsmLabel();
				falseLabel = generateAsmLabel();

				fprintf(asm_code, "     pop   bx \n");
				fprintf(asm_code, "     pop   ax \n");
				fprintf(asm_code, "     cmp   ax, bx \n");

				switch (cmpType) {
					case 0 : fprintf(asm_code, "     je    %s \n", trueLabel.c_str()); break; // ==
					case 1 : fprintf(asm_code, "     jne   %s \n", trueLabel.c_str()); break; // !=
					case 2 : fprintf(asm_code, "     jbe   %s \n", trueLabel.c_str()); break; // <=
					case 3 : fprintf(asm_code, "     jae   %s \n", trueLabel.c_str()); break; // >=
					case 7 : fprintf(asm_code, "     ja    %s \n", trueLabel.c_str()); break; // >
					case 8 : fprintf(asm_code, "     jb    %s \n", trueLabel.c_str()); break; // <
				}

				fprintf(asm_code, "     push  0 \n");
				fprintf(asm_code, "     jmp   %s \n", falseLabel.c_str());
				fprintf(asm_code, "%s: \n", trueLabel.c_str());
				fprintf(asm_code, "     push  1 \n");
				fprintf(asm_code, "%s: \n", falseLabel.c_str());

				scan(true);
				if (!((t_a == 'R') && (t_n == 11))) // ']'
					error(4, "F logic");
			} else if ((t_a == 'R') && (t_n == 5)) { // '(' El ')'
				scan(true);
				EL();
				scan(true);
				if (!((t_a == 'R') && (t_n == 6))) // ')'
					error(5, "F logic");
			}
		}		 
	}	
}

// Zn -> '>' | '<' | '==' | '!=' | '>=' | '<='
void SyntaxParser::Z() {
	scan(false);
	if (((t_a == 'R') && inFollow(t_n, SINGLE)) || ((t_a == 'D') && inFollow(t_n, DOUBLE))) {  // '>' | '<' | '==' | '!=' | '>=' | '<=' 
		cmpType = t_n;
		scan(true);
	} else
		error(8, "Z logic");
}

// <program> -> 'program' I ';' <variables> <operators_block> '.'
void SyntaxParser::f_program() {
	fprintf(asm_code, "     .model small \n");
	fprintf(asm_code, "     include asmData.asm \n");
	scan(true);
	if (!((t_a == 'K') && (t_n == 18))) // 'program'
		error(9, "program");
	scan(true);
	if (!(t_a == 'I')) // I
		error(10, "program");
	scan(true);
	if (!((t_a == 'R') && (t_n == 15))) // ';'
		error(16, "program");		
	
	f_variables(); // <variables>
	
	fprintf(asm_code, "     .stack 100h \n");
	fprintf(asm_code, "     .code \n");
	fprintf(asm_code, "     .386 \n");
	fprintf(asm_code, "start: \n");
	fprintf(asm_code, "     mov   ax, @data \n");
	fprintf(asm_code, "     mov   ds, ax \n");
	
	f_operatorsBlock();

	scan(true);
	if (!((t_a == 'R') && (t_n == 12))) // '.'
		error(11, "program");

	fprintf(asm_code, "error: \n");
	fprintf(asm_code, "     mov   ax, 4C00h \n");
	fprintf(asm_code, "     int   21h \n");
	fprintf(asm_code, "     end   start \n");
}

// <variables> -> 'var' { <type> I ('[' C ']' | eps) { ',' I ('[' C ']' | eps) } ';' } 'endvar' ';'
void SyntaxParser::f_variables() {
	int u;
	currenIdentifier = 0;
	int varListBegin = 0;
	int varListEnd = 0;
	
	fprintf(asm_variables, "		.data \n");
	fprintf(asm_variables, "; SUPPORTING VARIABLES \n");
	fprintf(asm_variables, "		@buffer   db		6 \n");
	fprintf(asm_variables, "		blength   db      (?) \n");
	fprintf(asm_variables, "		@buf      db      256 DUP (?) \n");
	fprintf(asm_variables, "		clrf      db      0Dh, 0Ah, \"$\" \n");
	fprintf(asm_variables, "		output    db      6 DUP (?), \"$\" \n");
	fprintf(asm_variables, "		err_msg   db      \"Input error, try again\", 0Dh, 0Ah, \"$\" \n");
	fprintf(asm_variables, "		@true     db      \"true\" \n");
	fprintf(asm_variables, "		@@true    db      \"true$\" \n");
	fprintf(asm_variables, "		@false    db      \"false\" \n");
	fprintf(asm_variables, "		@@false   db      \"false$\" \n");
	fprintf(asm_variables, "; USING VARIABLES \n");
	
	scan(true);
	if (!((t_a == 'K') && (t_n == 24))) // var
		error(12, "var");
	
	// { <type> I ('[' C ']' | eps) { ',' I ('[' C ']' | eps) } ';' }
	scan(false);
	while (!((t_a == 'K') && (t_n == 10))) { // 'endvar'

		f_type(); 

		scan(true); 
		if (t_a != 'I') // I
			error(13, "var");
		currenIdentifier++;
		if (t_n != currenIdentifier)
			error(1, "var 1");

		varListBegin = t_n;
		varListEnd = t_n;

		scan(false);
		if ((t_a == 'R') && (t_n == 10)) { // ('[' C ']' | eps)
			scan(true); // '['
			scan(true);
			if (t_a != 'C') // 'C'
					error(19, "arrayType");
			arraySize = lexicalParser.numbersToken[t_n];
			scan(true);
			if (!((t_a == 'R') && (t_n == 11))) // ']'
					error(4, "arrayType");
			lexicalParser.identifiersToken[currenIdentifier].identifierType = 1;
			lexicalParser.identifiersToken[currenIdentifier].arrayType = baseType;
			lexicalParser.identifiersToken[currenIdentifier].arraySize = arraySize;
		} else
			lexicalParser.identifiersToken[currenIdentifier].identifierType = baseType;


		scan(false);
		while (!((t_a == 'R') && (t_n == 15))) { // { ',' I ('[' C ']' | eps) }
			scan(true); // ','
			scan(true);
			if (t_a != 'I') // I
				error(13, "var 2");
			currenIdentifier++;
			if (t_n != currenIdentifier)
				error(1, "var");

			varListEnd = t_n;

			scan(false);
			if ((t_a == 'R') && (t_n == 10)) { // ('[' C ']' | eps)
				scan(true); // '['

				scan(true);
				if (t_a != 'C') // 'C'
						error(19, "arrayType");
				arraySize = lexicalParser.numbersToken[t_n];
				scan(true);
				if (!((t_a == 'R') && (t_n == 11))) // ']'
						error(4, "arrayType");
				lexicalParser.identifiersToken[currenIdentifier].identifierType = 1;
				lexicalParser.identifiersToken[currenIdentifier].arrayType = baseType;
				lexicalParser.identifiersToken[currenIdentifier].arraySize = arraySize;
			} else
				lexicalParser.identifiersToken[currenIdentifier].identifierType = baseType;

			scan(false);
		}

		scan(true);
		if (!((t_a == 'R') && (t_n == 15))) // ;
			error(16, "var");
			
		for (int i = varListBegin; i <= varListEnd; i++) {
			fprintf(asm_variables, "%s", lexicalParser.identifiersToken[i].name.c_str());
			
			if (lexicalParser.identifiersToken[i].identifierType == 1)
				u = lexicalParser.identifiersToken[i].arrayType;
			else
				u = lexicalParser.identifiersToken[i].identifierType;
			
			fprintf(asm_variables, (u == 13) ? " dw " : " db ");
			
			if (lexicalParser.identifiersToken[i].identifierType == 1)
				fprintf(asm_variables, "%d DUP (?) \n", lexicalParser.identifiersToken[i].arraySize);
			else
				fprintf(asm_variables, "%s", "(?) \n");
	
			if (varListEnd != lexicalParser.totalIdentifiers - 1)
				for (int i = varListEnd+1; i < lexicalParser.totalIdentifiers; i++)
					error(23, "var");
		}

		scan(false);
	}
	
	scan(true); // 'endvar' 
	scan(true);
	if (!((t_a == 'R') && (t_n == 15))) // ';'
		error(16, "var");
}

// <type> -> 'bool' | 'char' | 'int'
void SyntaxParser::f_type() {
	scan(true);
	if (t_a != 'K')
		error(18, "baseType");
	else {
		if ((t_n == 3) || (t_n == 6) || (t_n == 13)) // | 'bool' | 'char' | 'int'
			baseType = t_n;
		else 
			error(18, "baseType");
	}
}

// <operators_block> -> 'begin' { <operator> ';' } 'end'
void SyntaxParser::f_operatorsBlock() {
	scan(true);
	if (!((t_a == 'K') && (t_n == 2))) // 'begin'
			error(24, "begin...end");
	scan(false);
	while (!((t_a == 'K') && (t_n == 9))) { // 'end'
		f_operator();
		scan(true);
		if (!((t_a == 'R') && (t_n == 15))) // ';'
			error(16, "begin...end");
		scan(false);
	}
	scan(true); // 'end'

}

// <operators> -> <operatorsBlock> | <if> | <let> | <read> | <readln> | <while> | <write> | <writeln> | <repeat> | eps
void SyntaxParser::f_operator() {
	scan(false);
	if (!((t_a == 'R') && (t_n == 15))) // ';'
		if (t_a == 'K')
			switch (t_n) {
				case 2 : f_operatorsBlock(); break; // <operatos_block>
				case 12 : f_if(); break; // <if>
				case 14 : f_let(); break; // <let>
				case 19 : f_read(false); break; // <read>
				case 20 : f_read(true); break; // <readln>
				case 25 : f_while(); break; // <while>
				case 26 : f_write(false); break; // <write>
				case 27 : f_write(true); break; // <writeln>
				case 28 : f_repeat(); break; // <repeat>
         		default : f_let(); break;
			}
		else
			f_let();
}

// <if> -> 'if' El 'then' <operator> ('else' <operator> | eps)
void SyntaxParser::f_if() {
	fprintf(asm_code, "; IF() \n");
	scan(true); // 'if'
	EL();
	string thenLabel = generateAsmLabel();
	string elseLabel = generateAsmLabel();
	fprintf(asm_code, "     pop   ax \n");
	fprintf(asm_code, "     cmp   ax, 0 \n");
	fprintf(asm_code, "     jz    %s \n", thenLabel.c_str());
	scan(true);
	if (!((t_a == 'K') && (t_n == 22))) // 'then'
		error(26, "if");
	fprintf(asm_code, "; IF THEN \n");
	f_operator();
	fprintf(asm_code, "     jmp   %s \n", elseLabel.c_str());
	fprintf(asm_code, "%s: \n", thenLabel.c_str());	
	scan(false);
	if ((t_a == 'K') && ( t_n == 8)) { // 'else' <operator> | eps
		fprintf(asm_code, "; IF ELSE \n");	
		scan(true);
		f_operator();
	}
	fprintf(asm_code, "%s: \n", elseLabel.c_str());
}

// <let> -> ('let' | eps) I ('[' E ']' | eps) '=' (E | El | L | I ('[' E ']' | eps))
void SyntaxParser::f_let() {
	fprintf(asm_code, "; LET() \n");
	scan(false);
	if ((t_a == 'K') && (t_n == 14)) // 'let'
		scan(true);

	scan(true); 
	if (t_a != 'I')
		error(13, "let");

	int typeCode, idenNum = t_n;
	if (lexicalParser.identifiersToken[t_n].identifierType == 1) // array
		typeCode = lexicalParser.identifiersToken[t_n].arrayType;
	else
		typeCode = lexicalParser.identifiersToken[t_n].identifierType;
	fprintf(asm_code, "     mov   di, 0 \n");

	scan(false); 
	if ((t_a == 'R') && (t_n == 10)) { // '[' E ']' | eps)
		if (lexicalParser.identifiersToken[idenNum].identifierType != 1)
			error(27, "let");	
		scan(true); // '['
		E();
		fprintf(asm_code, "     pop   di \n");
		scan(true);
		if (!((t_a == 'R') && (t_n == 11))) // ']'
			error(4, "let");		
	} 
	fprintf(asm_code, "     push  di \n");
	
	scan(true);
	if (!((t_a == 'R') && (t_n == 9))) // '='
		error(28, "let");	

	switch (typeCode) {
		case 3 : // El
			EL();
			fprintf(asm_code, "     pop   ax \n");
			fprintf(asm_code, "     pop   di \n");
			fprintf(asm_code, "     mov   %s[di], al \n", lexicalParser.identifiersToken[idenNum].name.c_str());
		break;
		case 6 : // L | I ('[' E ']' | eps))
			scan(true);
			if (t_a == 'L') {
				fprintf(asm_code, "     mov   al, \"%s\" \n", lexicalParser.literalsToken[t_n][0]);
				fprintf(asm_code, "     pop   di \n");
				fprintf(asm_code, "     mov   %s[di], al \n", lexicalParser.identifiersToken[idenNum].name.c_str());
			} else { // I
				int sIdenNum = t_n;
				fprintf(asm_code, "     mov   di, 0 \n");
				scan(false);
				if ((t_a == 'R') && (t_n == 10)) { // '[' E ']'
					scan(true); // '['
					E();
					fprintf(asm_code, "     pop   di \n");
					scan(true);
					if (!((t_a == 'R') && (t_n == 11))) // ']'
						error(4, "let");
				}
				scan(false);
				fprintf(asm_code, "     mov   al, %s[di] \n", lexicalParser.identifiersToken[sIdenNum].name.c_str());
				fprintf(asm_code, "     pop   di \n");
				fprintf(asm_code, "     mov   %s[di], al \n", lexicalParser.identifiersToken[idenNum].name.c_str());				
			}		
			scan(false);
		break;
		case 13 : 
			E();
			fprintf(asm_code, "     pop   ax \n");
			fprintf(asm_code, "     pop   di \n");
			if (lexicalParser.identifiersToken[idenNum].identifierType == 1) // array
				fprintf(asm_code, "     shl   di, 1 \n");
			fprintf(asm_code, "     mov   %s[di], ax \n", lexicalParser.identifiersToken[idenNum].name.c_str());
		break;		
	}
}

// <while> -> 'while' '(' El ')' <operator>
void SyntaxParser::f_while() {
	fprintf(asm_code, "; WHILE() \n");
	scan(true); // 'while'
	scan(true);
	if (!((t_a == 'R') && (t_n == 5))) // '('
		error(25, "while");
	string repeatLabel = generateAsmLabel();
	fprintf(asm_code, "%s: \n", repeatLabel.c_str());
	EL();
	fprintf(asm_code, "     pop   ax \n");
	fprintf(asm_code, "     cmp   ax, 0 \n");
	string endLabel = generateAsmLabel();
	fprintf(asm_code, "     jz    %s \n", endLabel.c_str());
	scan(true);
	if (!(t_a == 'R') && (t_n == 6)) // ')'
		error(5, "while");
	f_operator();
	fprintf(asm_code, "     jmp   %s \n", repeatLabel.c_str());
	fprintf(asm_code, "%s: \n", endLabel.c_str());
}

// <read> -> 'read' '(' ((I ('[' E ']' | eps) {',' I ('[' E ']' | eps)}) | eps) | eps ')'
void SyntaxParser::f_read(bool ln) {
	int typeCode, idenNum;
	fprintf(asm_code, (ln) ? "; READLN() \n" : "; READ() \n");
	scan(true); // 'read'
	scan(true);
	if (!((t_a == 'R') && (t_n == 5))) // '('
		error(25, "read(ln)");
	
	scan(false);
	if ((t_a == 'R') && (t_n == 6)) // ')'
		scan(true);	
	else { // ((I ('[' E ']' | eps) {',' I ('[' E ']' | eps)}) | eps)
		if (t_a != 'I')
			error(13, "read(ln)");
		idenNum = t_n;
		if (lexicalParser.identifiersToken[t_n].identifierType == 1) // array
			typeCode = lexicalParser.identifiersToken[t_n].arrayType;
		else
			typeCode = lexicalParser.identifiersToken[t_n].identifierType;
		scan(true);
		
		fprintf(asm_code, "     push  0 \n"); // mov di, 0
		scan(false);
		if ((t_a == 'R') && (t_n == 10)) { // ('[' E ']' | eps)
			scan(true);
			E();
			scan(true);
			if (!((t_a == 'R') && (t_n == 11))) // ']'
				error(5, "read(ln)");
		}
		
		switch (typeCode) {
			case 3 : f_readBool(idenNum);  break;// bool
			case 6 : f_readChar(idenNum);  break; // char
			case 13 : f_readInt(idenNum); break; // int
		}
		
		scan(false);
		while (!((t_a == 'R') && (t_n == 6))) { // {',' I ('[' E ']' | eps)}
			scan(true);
			if (!((t_a == 'R') && (t_n == 13))) // ','
				error(14, "read(ln)");
			scan(false);
			
			if (t_a != 'I')
				error(13, "read(ln)");
			idenNum = t_n;
			if (lexicalParser.identifiersToken[t_n].identifierType == 1) // array
				typeCode = lexicalParser.identifiersToken[t_n].arrayType;
			else
				typeCode = lexicalParser.identifiersToken[t_n].identifierType;
			scan(true);
			
			fprintf(asm_code, "     push  0 \n"); // mov di, 0
			scan(false);
			if ((t_a == 'R') && (t_n == 10)) { // ('[' E ']' | eps)
				scan(true);
				E();
				scan(true);
				if (!((t_a == 'R') && (t_n == 11))) // ']'
					error(5, "read(ln)");
			}
			
			switch (typeCode) {
				case 3 : f_readBool(idenNum);  break;// bool
				case 6 : f_readChar(idenNum);  break; // char
				case 13 : f_readInt(idenNum); break; // int
			}
			
			scan(false);
		}
	}
	scan(true);
	if (ln) {
		fprintf(asm_code, "     lea   dx, clrf \n");
		fprintf(asm_code, "     mov   ah, 9 \n");
		fprintf(asm_code, "     int   21h \n");
	}
}

void SyntaxParser::f_readInt(int idenNum) {
	fprintf(asm_code, "; READ INTEGER \n");
	string sLabel = generateAsmLabel();
	string lerror = generateAsmLabel();
	string lstart = generateAsmLabel();
	string lend = generateAsmLabel();
	fprintf(asm_code, "%s: \n", lstart.c_str());
	fprintf(asm_code, "     mov   ah, 0Ah \n");
	fprintf(asm_code, "     lea   dx, @buffer \n");
	fprintf(asm_code, "     int   21h \n");
	fprintf(asm_code, "     mov   ax, 0 \n");
	fprintf(asm_code, "     mov   cx, 0 \n");
	fprintf(asm_code, "     mov   cl, byte ptr[blength] \n");
	fprintf(asm_code, "     mov   bx, cx \n");
	fprintf(asm_code, "%s: \n", sLabel.c_str());
	fprintf(asm_code, "     dec   bx \n");
	fprintf(asm_code, "     mov   al, @buf[bx] \n");
	fprintf(asm_code, "     cmp   al, \"9\" \n");
	fprintf(asm_code, "     ja    %s \n", lerror.c_str());
	fprintf(asm_code, "     cmp   al, \"0\" \n");
	fprintf(asm_code, "     jb    %s \n", lerror.c_str());
	fprintf(asm_code, "     loop  %s \n", sLabel.c_str());
	sLabel = generateAsmLabel();
	fprintf(asm_code, "     mov   cl, byte ptr[blength] \n");
	fprintf(asm_code, "     mov   di, 0 \n");
	fprintf(asm_code, "     mov   ax, 0 \n");
	fprintf(asm_code, "%s: \n", sLabel.c_str());
	fprintf(asm_code, "     mov   bl, @buf[di] \n");
	fprintf(asm_code, "     inc   di \n");
	fprintf(asm_code, "     sub   bl, 30h \n");
	fprintf(asm_code, "     add   ax, bx \n");
	fprintf(asm_code, "     mov   si, ax \n");
	fprintf(asm_code, "     mov   bx, 10 \n");
	fprintf(asm_code, "     mul   bx \n");
	fprintf(asm_code, "     loop  %s \n", sLabel.c_str());
	fprintf(asm_code, "     mov   ax, si \n");
	fprintf(asm_code, "     pop   di \n");
	fprintf(asm_code, "     shl   di, 1 \n");
	fprintf(asm_code, "     mov   %s[di],ax \n", lexicalParser.identifiersToken[idenNum].name.c_str());
	fprintf(asm_code, "     jmp   %s \n", lend.c_str());
	fprintf(asm_code, "%s: \n", lerror.c_str());
	fprintf(asm_code, "     lea   dx, err_msg \n");
	fprintf(asm_code, "     mov   ah, 9 \n");
	fprintf(asm_code, "     int   21h \n");
	fprintf(asm_code, "     jmp   %s \n", lstart.c_str());
	fprintf(asm_code, "%s: \n", lend.c_str());
}

void SyntaxParser::f_readBool(int idenNum) {
	fprintf(asm_code, "; READ BOOLEAN \n");
	string start = generateAsmLabel();
	string l4 = generateAsmLabel();
	string l5 = generateAsmLabel();
	string le = generateAsmLabel();
	string lt = generateAsmLabel();
	string lf = generateAsmLabel();
	string lend = generateAsmLabel();
	string lerror = generateAsmLabel();
	fprintf(asm_code, "%s: \n", start.c_str());
	fprintf(asm_code, "     mov   ah, 0Ah \n");
	fprintf(asm_code, "     lea   dx, @buffer \n");
	fprintf(asm_code, "     int   21h \n");

	fprintf(asm_code, "     cmp   blength, 4 \n");
	fprintf(asm_code, "     je    %s \n", l4.c_str());
	fprintf(asm_code, "     cmp   blength, 5 \n");
	fprintf(asm_code, "     je    '%s \n", l5.c_str());
	fprintf(asm_code, "     jmp   %s \n", lerror.c_str());

	fprintf(asm_code, "%s: \n", l4.c_str());
	fprintf(asm_code, "     lea   si, @true \n");
	fprintf(asm_code, "     lea   di, @buf \n");
	fprintf(asm_code, "     mov   cx, 4 \n");
	fprintf(asm_code, "     repe  cmpsb \n");
	fprintf(asm_code, "     jz  %s \n", le.c_str());
	fprintf(asm_code, "     jmp   %s \n", lerror.c_str());
	
	fprintf(asm_code, "%s: \n", l5.c_str());
	fprintf(asm_code, "     lea   si, @false \n");
	fprintf(asm_code, "     lea   di, @buf \n");
	fprintf(asm_code, "     mov   cx, 5 \n");
	fprintf(asm_code, "     repe  cmpsb \n");
	fprintf(asm_code, "     jz  %s \n", le.c_str());
	fprintf(asm_code, "     jmp   %s \n", lerror.c_str());
	
	fprintf(asm_code, "%s: \n", le.c_str());
	fprintf(asm_code, "     cmp   @buf[0], \"t\" \n");
	fprintf(asm_code, "     je    %s \n", lt.c_str());
	fprintf(asm_code, "     push  0 \n");
	fprintf(asm_code, "     jmp   %s \n", lend.c_str());
	fprintf(asm_code, "%s: \n", lt.c_str());
	fprintf(asm_code, "     push  1 \n");
	fprintf(asm_code, "     jmp   %s \n", lend.c_str());
	fprintf(asm_code, "%s: \n", lerror.c_str());
	fprintf(asm_code, "     lea   dx, err_msg \n");
	fprintf(asm_code, "     mov   ah, 9 \n");
	fprintf(asm_code, "     int   21h \n");
	fprintf(asm_code, "     jmp   %s \n", start.c_str());
	fprintf(asm_code, "%s: \n", lend.c_str());
	fprintf(asm_code, "     pop   ax \n");
	fprintf(asm_code, "     pop   di \n");
	fprintf(asm_code, "     mov   %s[di], al \n", lexicalParser.identifiersToken[idenNum].name.c_str());
}

void SyntaxParser::f_readChar(int idenNum) {
	fprintf(asm_code, "; READ CHAR \n");
	fprintf(asm_code, "     mov   ah, 0Ah \n");
	fprintf(asm_code, "     lea   dx, @buffer \n");
	fprintf(asm_code, "     int   21h \n");
	fprintf(asm_code, "     xor   dx, dx \n");
	fprintf(asm_code, "     mov   dl, @buf[0] \n");
	fprintf(asm_code, "     pop   di \n");
	fprintf(asm_code, "     mov   %s[di], dl \n", lexicalParser.identifiersToken[idenNum].name.c_str());
}

// <write> -> 'write' '(' (((E | L) {',' (E | L)} ) | eps) ')'
void SyntaxParser::f_write(bool ln) {
	int typeCode, idenNum;
	fprintf(asm_code, (ln) ? "; WRITELN() \n" : "; WRITE() \n" );
	scan(true); // 'write'
	scan(true);
	if (!((t_a == 'R') && (t_n == 5))) // '('
		error(25, "write(ln)");

	scan(false);
	if ((t_a == 'R') && (t_n == 6)) // ')'
		scan(true);
	else { // ((E | L) {',' (E | L)} )
		if (t_a == 'L') { // L
			f_writeLiteral();
			scan(true);
		} else {
			if (t_a == 'I') { // E
				idenNum = t_n;
				if (lexicalParser.identifiersToken[t_n].identifierType == 1) // array
					typeCode = lexicalParser.identifiersToken[t_n].arrayType;
				else
					typeCode = lexicalParser.identifiersToken[t_n].identifierType;
				switch (typeCode) {
					case 3 : // bool
						EL();
						f_writeBool();
					break;
					case 6 : // char
						fprintf(asm_code, "     mov   di, 0 \n");
						scan(false);
						if ((t_a == 'R') && (t_n == 10)) {
							scan(true);
							E();
							fprintf(asm_code, "     pop   di \n");
							scan(true);
							if (!((t_a == 'R') && (t_n == 11)))
								error(5, "write(ln)");
						}
						f_writeChar(idenNum);
						scan(true);
					break;
					case 13 : // int
						E();
						f_writeInt();
					break;					
				}					
			}			
		}
		
		// {',' (E | L)}
		scan(false);
		while (!((t_a == 'R') && (t_n == 6))) { // ')'
			scan(true);			
			if (!((t_a == 'R') && (t_n == 13))) // ','
 				error(14, "write(ln)");
			scan(false);
			if (t_a == 'L') { // L
				f_writeLiteral();
				scan(true);
			} else {
				if (t_a == 'I') { // E
					idenNum = t_n;
					if (lexicalParser.identifiersToken[t_n].identifierType == 1) // array
						typeCode = lexicalParser.identifiersToken[t_n].arrayType;
					else
						typeCode = lexicalParser.identifiersToken[t_n].identifierType;
					switch (typeCode) {
						case 3 : // bool
							EL();
							f_writeBool();
						break;
						case 6 : // char
							scan(true);
							fprintf(asm_code, "     mov   di, 0 \n");
							scan(false);
							if ((t_a == 'R') && (t_n == 10)) {
								scan(true);
								E();
								fprintf(asm_code, "     pop   di \n");
								scan(true);
								if (!((t_a == 'R') && (t_n == 11)))
									error(5, "write(ln)");
							}
							f_writeChar(idenNum);
							scan(false);
						break;
						case 13 : // int
							E();
							f_writeInt();
						break;					
					}					
				}			
			}	
			scan(false);
		}
		scan(true);		
	}
	if (ln) {
		fprintf(asm_code, "     lea   dx, clrf \n");
		fprintf(asm_code, "     mov   ah, 9 \n");
		fprintf(asm_code, "     int   21h \n");
	}
}

void SyntaxParser::f_writeInt() {
	fprintf(asm_code, "; WRITE ARIPHMETICS \n");
	string l1 = generateAsmLabel();
	string l2 = generateAsmLabel();
	fprintf(asm_code, "     pop   ax \n");
	fprintf(asm_code, "     mov   bx, 10 \n");
	fprintf(asm_code, "     mov   di, 0 \n");
	fprintf(asm_code, "     mov   si, ax \n");
	fprintf(asm_code, "     cmp   ax, 0 \n");
	string sLabel = generateAsmLabel();
	fprintf(asm_code, "     jns   %s \n", sLabel.c_str());
	fprintf(asm_code, "     neg   si \n");
	fprintf(asm_code, "     mov   ah, 2 \n");
	fprintf(asm_code, "     mov   dl, \"-\" \n");
	fprintf(asm_code, "     int   21h \n");
	fprintf(asm_code, "     mov   ax, si \n");
	fprintf(asm_code, "%s: \n", sLabel.c_str());
	fprintf(asm_code, "     mov   dx, 0 \n");
	fprintf(asm_code, "     div   bx \n");
	fprintf(asm_code, "     add   dl, 30h \n");
	fprintf(asm_code, "     mov   output[di], dl \n");
	fprintf(asm_code, "     inc   di \n");
	fprintf(asm_code, "     cmp   al, 0 \n");
	fprintf(asm_code, "     jnz   %s \n", sLabel.c_str());
	fprintf(asm_code, "     mov   cx, di \n");
	fprintf(asm_code, "     dec   di \n");
	fprintf(asm_code, "     mov   ah, 2 \n");
	sLabel = generateAsmLabel();
	fprintf(asm_code, "%s: \n", sLabel.c_str());
	fprintf(asm_code, "     mov   dl, output[di] \n");
	fprintf(asm_code, "     dec   di \n");
	fprintf(asm_code, "     int   21h \n");
	fprintf(asm_code, "     loop  %s \n", sLabel.c_str());
}

void SyntaxParser::f_writeLiteral() {
	fprintf(asm_code, "; WRITE LITERAL \n");
	string text = generateAsmLabel();
	fprintf(asm_variables, "%sp db \"%s$\" \n", text.c_str(), lexicalParser.literalsToken[t_n].c_str());
	fprintf(asm_code, "     lea   dx, %sp \n", text.c_str());
	fprintf(asm_code, "     mov   ah, 9 \n");
	fprintf(asm_code, "     int   21h \n");
}

void SyntaxParser::f_writeChar(int idenNum) {
	fprintf(asm_code, "; WRITE CHAR \n");
	fprintf(asm_code, "     mov   ax, 0 \n");
	fprintf(asm_code, "     mov   al, %s[di] \n", lexicalParser.identifiersToken[idenNum].name.c_str());
	fprintf(asm_code, "     mov   dl, al \n");
	fprintf(asm_code, "     mov   ah, 2 \n");
	fprintf(asm_code, "     int   21h \n");
}

void SyntaxParser::f_writeBool() {
	string l0 = generateAsmLabel();
	string l1 = generateAsmLabel();
	fprintf(asm_code, "; WRITE BOOLEAN \n");
	fprintf(asm_code, "     pop   ax \n");
	fprintf(asm_code, "     cmp   ax, 0 \n");
	fprintf(asm_code, "     je    %s \n", l0.c_str());
	fprintf(asm_code, "     lea   dx, @@true \n");
	fprintf(asm_code, "     jmp   %s \n", l1.c_str());
	fprintf(asm_code, "%s: \n", l0.c_str());
	fprintf(asm_code, "     lea   dx, @@false \n");
	fprintf(asm_code, "%s: \n", l1.c_str());
	fprintf(asm_code, "     mov   ah, 9 \n");
	fprintf(asm_code, "     int   21h \n");
}


void SyntaxParser::error(int errorCode, string oper) {
	if (errorCode==34){
		printf("ERROR REPEAT KEK. \n");
	}
		else{
		printf("Current lexeme: %c%d . Operator: %s ", t_a, t_n, oper.c_str()); 
		switch (errorCode) {
			case 1: printf("Redefenition of identifier %s. \n", lexicalParser.identifiersToken[t_n].name.c_str()); break;
			case 2: printf("Expected + or -. \n"); break;
			case 3: printf("Expected * or /. \n"); break;
			case 4: printf("Expected ]. \n"); break;
			case 5: printf("Expected ). \n"); break;
			case 6: printf("Expected or. \n"); break;
			case 7: printf("Expected and. \n"); break;
			case 8: printf("Expected compare operation. \n"); break;
			case 9: printf("Expected program \n"); break;
			case 10: printf("Expected %prograt_name% \n"); break;
			case 11: printf("Expected .. \n"); break;
			case 12: printf("Expected var \n"); break;
			case 13: printf("Expected identifier. \n"); break;
			case 14: printf("Expected ,. \n"); break;
			case 15: printf("Expected :. \n"); break;
			case 16: printf("Expected ;. \n"); break;
			case 17: printf("Expected identifier or begin. \n"); break;
			case 18: printf("Expected %type_name%. \n"); break;
			case 19: printf("Expected array. \n"); break;
			case 20: printf("Expected [. \n"); break;
			case 21: printf("Expected number. \n"); break;
			case 22: printf("Expected of. \n"); break;
			case 23: printf("Undeclared identifier %s. \n", lexicalParser.identifiersToken[t_n].name.c_str()); break;
			case 24: printf("Expected begin. \n"); break;
			case 25: printf("Expected (. \n"); break;
			case 26: printf("Expected then. \n"); break;
			case 27: printf("Unexpected [. \n"); break;
			case 28: printf("Expected =. \n"); break;
			case 29: printf("Expected do. \n"); break;
			case 30: printf("Expected true or false in assignment. \n"); break;
			case 31: printf("Expected end. \n"); break;
			case 32: printf("Expected }."); break;
			case 33: printf("Expected {."); break;
		}
	}
	fclose(asm_code);
	fclose(asm_variables);
	exit(1);
}

// <repeat> -> 'repeat' <operator> 'until' El
void SyntaxParser::f_repeat() {
	fprintf(asm_code, "; REPEAT() \n");
	string lstart = generateAsmLabel();
	scan(true);
	fprintf(asm_code, "%s:", lstart.c_str()); 
	scan(false);
	int nam=0;
	while (!((t_a == 'K') && (t_n == 29))) { // 'until'
		f_operator();
		scan(true);
		if (!((t_a == 'R') && (t_n == 15))) // ';'
			error(14, "repeat");
		scan(false);
		nam++;	
	}
	nam=nam%2;
	if (nam==0)
		error(34, "repeat kek");
	scan(true);
	EL();
	fprintf(asm_code, "     pop   ax \n"); 
	fprintf(asm_code, "     cmp   ax, 0 \n");
	fprintf(asm_code, "     jne   %s \n", lstart.c_str());  
}

void SyntaxParser::compile() {
	f_program();
	fclose(asm_code);
	fclose(asm_variables);
	fclose(lexicalParser.lex_file);
	cout << "Build successfully" << endl;
}

int main(int argc, char** argv) {
	SyntaxParser syntaxParser;
	syntaxParser.compile();
	system("pause");
	return 0;
}
