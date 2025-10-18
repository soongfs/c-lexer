// 目前只支持测评单文件程序，所以需要将代码都写到这个文件中。

#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdio.h>
#include <string>
#include <vector>

// using namespace std;
// #define BUF_SIZE 4096
// char *buffer[BUF_SIZE];

enum {
    KEYWORD,
    IDENTIFIER,
    OPERATOR,
    DELIMITER,
    CHARCON,
    STRING,
    NUMBER,
    ERROR
};

std::vector<std::string> kind_name{"KEYWORD",   "IDENTIFIER", "OPERATOR",
                                   "DELIMITER", "CHARCON",    "STRING",
                                   "NUMBER",    "ERROR"};

enum {
    AUTO,
    DOUBLE,
    INT,
    STRUCT,
    BREAK,
    ELSE,
    STATIC,
    LONG,
    SWITCH,
    CASE,
    ENUM,
    REGISTER,
    TYPEDEF,
    CHAR,
    EXTERN,
    RETURN,
    UNION,
    CONST,
    FLOAT,
    SHORT,
    UNSIGNED,
    CONTINUE,
    FOR,
    SIGNED,
    VOID,
    DEFAULT,
    GOTO,
    SIZEOF,
    VOLATILE,
    DO,
    IF,
    WHILE
};

std::vector<std::string> keyword_table{
    "auto",     "double", "int",    "struct", "break",    "else",    "static",
    "long",     "switch", "case",   "enum",   "register", "typedef", "char",
    "extern",   "return", "union",  "const",  "float",    "short",   "unsigned",
    "continue", "for",    "signed", "void",   "default",  "goto",    "sizeof",
    "volatile", "do",     "if",     "while"};

int state = 0;
char C = '\0';
std::vector<char> buffer;
char *forward = nullptr;
int line_no = 1;
int counter[8];
std::string token;
int iskey;
std::vector<std::string> symbol_table;

bool load_file(std::string file_name);
void get_char();
void get_nbc();
bool letter(char C);
bool digit(char C);
void ret(int kind, std::string tok);
void cat(char C);
void retract();
int reserve(std::string token);
int table_insert(std::string token);
void error();

int main(int argc, char *argv[]) {

    std::string file_name = argv[1]; // 该字符串变量为识别的C语言程序文件名
    // std::string file_name = "sample.c";

    /*程序每次运行只需要考虑识别一个C语言程序;
      需要使用读取文件的操作,可以在自己的本地环境里创建样例文件进行测试；
      在自己的IDE里测试时可以将file_name赋其他的值，
      但注意，最终测评时由平台提供的main参数来传入文件名*/

    load_file(file_name);

    do {
        switch (state) {
        case 0:
            get_char();
            get_nbc();
            // std::cout << C << std::endl;         // [DEBUG]
            // std::cout << letter(C) << std::endl; // [DEBUG]
            // std::cout << digit(C) << std::endl;  // [DEBUG]
            if (letter(C))
                state = 1;
            else if (digit(C))
                state = 2;
            else {
                switch (C) {
                case '<':
                    state = 8;
                    break;
                case '>':
                    state = 9;
                    break;
                case ':':
                    state = 10;
                    break;
                case '/':
                    state = 11;
                    break;
                case '=':
                    state = 0;
                    ret(OPERATOR, "=");
                    break;
                case '+':
                    state = 0;
                    ret(OPERATOR, "+");
                    break;
                case '-':
                    state = 0;
                    ret(OPERATOR, "-");
                    break;
                case '*':
                    state = 0;
                    ret(OPERATOR, "*");
                    break;
                case '(':
                    state = 0;
                    ret(DELIMITER, "(");
                    break;
                case ')':
                    state = 0;
                    ret(DELIMITER, ")");
                    break;
                case ';':
                    state = 0;
                    ret(DELIMITER, ";");
                    break;
                case '\'':
                    state = 0;
                    ret(OPERATOR, "'");
                    break;
                default:
                    state = 13;
                    break;
                }
            }
            break;
        case 1:
            cat(C);
            get_char();
            if (letter(C) || digit(C))
                state = 1;
            else {
                retract();
                state = 0;
                iskey = reserve(token);
                if (iskey != -1)
                    ret(KEYWORD, keyword_table[iskey]);
                else {
                    int identry = table_insert(token);
                    ret(IDENTIFIER, symbol_table[identry]);
                }
            }
            break;
        case 2:
            cat(C);
            get_char();
            if (digit(C)) {
                state = 2;
            } else {
                switch (C) {
                case '.':
                    state = 3;
                    break;
                case 'E':
                    state = 5;
                    break;
                // [TODO] 需要处理'e'？
                case 'e':
                    state = 5;
                    break;
                default:
                    retract();
                    state = 0;
                    ret(NUMBER, token);
                    break;
                }
            }
            break;
        case 3:
            cat(C);
            get_char();
            if (digit(C))
                state = 4;
            else {
                error();
                state = 0;
            }
            break;
        case 4:
            cat(C);
            get_char();
            if (digit(C))
                state = 4;
            else if (C == 'E' || C == 'e')
                state = 5;
            else {
                retract();
                state = 0;
                ret(NUMBER, token);
            }
            break;
        case 5:
            cat(C);
            get_char();
            if (digit(C))
                state = 7;
            else {
                switch (C) {
                case '+':
                    state = 6;
                    break;
                case '-':
                    state = 6;
                    break;
                default:
                    retract();
                    error();
                    state = 0;
                    break;
                }
            }
            break;
        case 6:
            cat(C);
            get_char();
            if (digit(C))
                state = 7;
            else {
                retract();
                error();
                state = 0;
            }
            break;
        case 7:
            cat(C);
            get_char();
            if (digit(C))
                state = 7;
            else {
                retract();
                state = 0;
                ret(NUMBER, token);
            }
            break;
        case 8:
            cat(C);
            get_char();
            switch (C) {
            case '=':
                cat(C);
                state = 0;
                ret(OPERATOR, "<=");
                break;
            case '<':
                cat(C);
                state = 0;
                ret(OPERATOR, "<<");
                break;
            // [TODO] 支持"<<="
            default:
                retract();
                state = 0;
                ret(OPERATOR, "<");
                break;
            }
            break;
        case 9:
            cat(C);
            get_char();
            switch (C) {
            case '=':
                cat(C);
                state = 0;
                ret(OPERATOR, ">=");
                break;
            case '>':
                cat(C);
                state = 0;
                ret(OPERATOR, ">>");
                break;
            }
            break;
        case 10:
            // [TODO] C语言没有这个操作符，重做
            break;
        case 11:
            cat(C);
            get_char();
            switch (C) {
            case '*':
                state = 12;
            // [TODO] 支持//注释
            // [TODO] 支持/=操作
            default:
                retract();
                state = 0;
                ret(OPERATOR, "/");
                break;
            }
            break;
        case 12:
            get_char();
            while (C != '*')
                get_char();
            get_char();
            if (C == '/')
                state = 0;
            else
                state = 12;
            break;
        case 13:
            error();
            state = 0;
            break;
        }
    } while (C != '\0');

    std::cout<<line_no-1<<std::endl;

    for (int i=KEYWORD;i<=STRING;++i) {
        std::cout<<counter[i]<<" ";
    }
    std::cout<<counter[NUMBER]<<std::endl;

    std::cout<<counter[ERROR]<<std::endl;

    return 0;
}

bool load_file(std::string file_name) {
    std::ifstream fin(file_name, std::ios::binary);
    if (!fin)
        return false;
    buffer.assign(std::istreambuf_iterator<char>(fin),
                  std::istreambuf_iterator<char>());
    buffer.push_back('\0');
    forward = buffer.data(); // 指向首字符
    return true;
}

void get_char() {
    C = *forward;
    if (C != '\0') {
        ++forward;
        if (C == '\n')
            ++line_no;
    }
}

void get_nbc() {
    while (C != '\0' && (C == ' ' || C == '\t' || C == '\r' || C == '\n')) {
        get_char();
    }
}

bool letter(char C) {
    if (('a' <= C && C <= 'z') || ('A' <= C && C <= 'Z'))
        return true;
    else
        return false;
}

bool digit(char C) {
    if ('0' <= C && C <= '9')
        return true;
    else
        return false;
}

void ret(int kind, std::string tok) {
    ++counter[kind];
    std::cout << line_no << " <" << kind_name[kind];
    std::cout << "," << tok << ">" << std::endl;
    token.clear();
    return;
}

void cat(char C) {
    token += C;
    return;
}

void retract() {
    if (!forward || forward == buffer.data()) return;  // 边界保护
    --forward;                         // 回退到上一个字符
    if (*forward == '\n' && line_no > 1) --line_no;    // ← 关键：跨回换行要减行号
    // C = *forward;                      // 可选：让 C 与 forward 同步
    return;
}

int reserve(std::string token) {
    for (int i = 0; i < keyword_table.size(); ++i)
        if (token == keyword_table[i])
            return i;
    return -1;
}

int table_insert(std::string token) {
    for (int i = 0; i < symbol_table.size(); ++i)
        if (token == symbol_table[i])
            return i;
    symbol_table.push_back(token);
    return symbol_table.size() - 1;
}

void error() {
    // [TODO] 错误分析
    std::cout << "An error." << std::endl;
    return;
}
