#include <iostream>
#include <fstream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "inputbuf.h"
#include "lexer.h"


typedef enum {C1, C2, C3, C4, C5} ConstraintType;

typedef enum {INT_TYPE = 1, REAL_TYPE, BOOL_TYPE} Type;

struct sTableEntry
{
    std::string name;
    int scope;
    int pubpriv;
    int binNo;
};

struct sTable
{
    sTableEntry* item;
    sTable* prev;
    sTable* next;
};


class Parser // Define class parser..
{
public:
    void printList();
    int parse_program();
    

private:
     void type_mismatch(ConstraintType c);
    void update_Types(int t1, int t2);
    Token look();
    Token assume(TokenType type);
    void addList(std::string n, int line, int t);
    int searchList(std::string n);
    int parse_globalVars();
    int parse_vardecllist();
    int parse_vardecl();
    int parse_varlist();
    int parse_typename();
    int parse_body();
    int parse_ifstmt();
    int parse_whilestmt();
    int parse_switchstmt();
    int parse_caselist();
    int parse_case();
    int parse_expression();
    int parse_unaryOperator();
    int parse_binaryOperator();
    int parse_primary();
    int parse_stmtlist();
    int parse_stmt();
    int parse_assstmt();  
};

using namespace std;

string reserved[] = { "END_OF_FILE", "INT", "REAL", "BOOL", "TR", "FA", "IF", "WHILE", "SWITCH", "CASE", "PUBLIC", "PRIVATE", "NUM", "REALNUM", "NOT", "PLUS", "MINUS", "MULT", "DIV", "GTEQ", "GREATER", "LTEQ", "NOTEQUAL", "LESS", "LPAREN", "RPAREN", "EQUAL", "COLON", "COMMA", "SEMICOLON", "LBRACE", "RBRACE", "ID", "ERROR" // TODO: Add labels for new token types here (as string)
};

#define KEYWORDS_COUNT 11
string keyword[] = { "int", "real", "bool", "true", "false", "if", "while", "switch", "case", "public", "private" };


int currentPrivPub = 0;

string constraint[] = { "C1", "C2", "C3", "C4", "C5" };

LexicalAnalyzer lexer;
Token token;
TokenType tempTokenType;
int enumType;
int enumCount = 4;


sTable* symbolTable;

void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}



bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::SkipComments()
{
    bool comments = false;
    char c;
    if(input.EndOfInput() ){
        input.UngetChar(c);
        return comments;
    }
    
    input.GetChar(c);
    
    
    if(c == '/'){
        input.GetChar(c);
        if(c == '/'){
            comments = true;
            while(c != '\n'){
                comments = true;
                input.GetChar(c);
            
            
            }
            line_no++;
            
            SkipComments();
        }else{
            comments = false;
            cout << "Syntax Error\n";
            exit(0);
        }
        
        
        
        
        
        
    }else{
           input.UngetChar(c);
           
           return comments;
    }
            
      
         
    

}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanNumber()
{
    char c;
    bool realNUM = false;
    input.GetChar(c);
    if (isdigit(c)) {
        if (c == '0') {
            tmp.lexeme = "0";
            input.GetChar(c);
            if(c == '.'){
                
                //cout << "\n I am here too " << c << " \n";
                input.GetChar(c);
                
                if(!isdigit(c)){
                    input.UngetChar(c);
                }else{
                    while (!input.EndOfInput() && isdigit(c)) {
                        tmp.lexeme += c;
                        input.GetChar(c);
                        realNUM = true;
                        
                    }   
                    input.UngetChar(c);
                }
            }else{
                input.UngetChar(c);
            }
        } else {
            tmp.lexeme = "";
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }
            if(c == '.'){
                
                //cout << "\n I am here too " << c << " \n";
                input.GetChar(c);
                
                if(!isdigit(c)){
                    input.UngetChar(c);
                }else{
                    while (!input.EndOfInput() && isdigit(c)) {
                        tmp.lexeme += c;
                        input.GetChar(c);
                        realNUM = true;
                    }   
                }
            }
            
            if (!input.EndOfInput()) {
                input.UngetChar(c);
            }
        }
        // TODO: You can check for REALNUM, BASE08NUM and BASE16NUM here!
        if(realNUM){
            tmp.token_type = REALNUM;
        }else{
            tmp.token_type = NUM;
        }
        tmp.line_no = line_no;
        return tmp;
    } else { 
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);
    
    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    SkipComments();
    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    //cout << "\n Char obtained " << c << "\n";
    switch (c) {
        case '!':
            tmp.token_type = NOT;
            return tmp;    
        case '+':
            tmp.token_type = PLUS;
            return tmp;    
        case '-':
            tmp.token_type = MINUS;
            return tmp; 
        case '*':
            tmp.token_type = MULT;
            return tmp;       
        case '/':
            tmp.token_type = DIV;
            return tmp;    
        case '>':
            input.GetChar(c);
            if(c == '='){
                tmp.token_type = GTEQ;   
            }else{
                input.UngetChar(c);
                tmp.token_type = GREATER;
            }
            return tmp;    
        case '<':
            input.GetChar(c);
            if(c == '='){
                tmp.token_type = LTEQ;   
            }else if (c == '>'){
                tmp.token_type = NOTEQUAL;    
            }else{
                input.UngetChar(c);
                tmp.token_type = LESS;
            }
            return tmp;            
        case '(':
            //cout << "\n I am here" << c << " \n";
            tmp.token_type = LPAREN;
            return tmp;    
        case ')':
            tmp.token_type = RPAREN;
            return tmp;    
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '{':
            tmp.token_type = LBRACE;
            return tmp;
        case '}':
            tmp.token_type = RBRACE;
            return tmp;
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                //cout << "\n ID scan " << c << " \n"; 
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}


Token Parser::look()
{
    Token token = lexer.GetToken();
    lexer.UngetToken(token);
    return token;
}

Token Parser::assume(TokenType type)
{
    token = lexer.GetToken();
    if (token.token_type != type)
    {
        cout << "\n Syntax Error \n";
    }
    return token;
}



void Parser::type_mismatch(ConstraintType c)
{
    cout << "TYPE MISMATCH " << token.line_no << " " << constraint[c] << endl;
    exit(1);
}

void Parser::addList(std::string name, int line, int type)
{
    if (symbolTable == NULL)
    {
        sTable* newEntry = new sTable();
        sTableEntry* newItem = new sTableEntry();

        newItem->name = name;
        newItem->scope = token.line_no;
        newItem->pubpriv = type;
        newItem->binNo = 0;

        newEntry->item = newItem;
        newEntry->next = NULL;
        newEntry->prev = NULL;

        symbolTable = newEntry;
    }
    else
    {
        sTable* temp = symbolTable;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }

        sTable* newEntry = new sTable();
        sTableEntry* newItem = new sTableEntry();

        newItem->name = name;
        newItem->scope = token.line_no;
        newItem->pubpriv = type;
        newItem->binNo = 0;

        newEntry->item = newItem;
        newEntry->next = NULL;
        newEntry->prev = temp;
        temp->next = newEntry;
    }
}

int Parser::searchList(std::string n)
{
    sTable* temp = symbolTable;
    bool found = false;
    if (symbolTable == NULL)
    {
        addList(n, token.line_no, enumCount);
        enumCount++;
        return (4);
    }
    else
    {
        while (temp->next != NULL)
        {
            if (strcmp(temp->item->name.c_str(), n.c_str()) == 0)
            {
                found = true;
                return(temp->item->pubpriv);
            }
            else
            {
                temp = temp->next;
            }
        }
        if (strcmp(temp->item->name.c_str(), n.c_str()) == 0)
        {
            found = true;
            return(temp->item->pubpriv);
        }
        if (!found)
        {
            addList(n, token.line_no, enumCount);
            enumCount++;
            int t = enumCount - 1;
            return(t);
        }
    }
}


int Parser::parse_varlist()
{
    assume(ID);
    int tempI;
    char* lexeme = (char*)malloc(sizeof(token.lexeme) + 1);
    memcpy(lexeme, (token.lexeme).c_str(), (token.lexeme).size() + 1);
    addList(lexeme, token.line_no, 0);

    Token t1 = look();
    if (t1.token_type == COMMA)
    {
        assume(COMMA);
        cerr << "\n Rule Parsed: var_list -> ID COMMA var_list \n";
        tempI = parse_varlist();
    }
    else if (t1.token_type == COLON)
    {
        cerr << "\n Rule Parsed: var_list -> ID \n";
    }
    else
    {
        cout << "\n Syntax Error \n";

    }

    return(0);
}



// return 0 for error and return 1 for NOT
int Parser::parse_unaryOperator()
{
    assume(NOT);
    cerr << "\n Rule parsed: unary_operator -> NOT";
    return(1);
}

// this returns 0 if lType cant be bool, 1 if lType can be anything, -1 if type error
int Parser::parse_binaryOperator()
{
    token = lexer.GetToken();
    //keep track of the number of bin operations in binNo
    if(token.token_type == PLUS  ){
        return(PLUS);
        cerr << "\n Rule parsed: binary_operator -> PLUS\n";
    }else if(token.token_type == MINUS ){
        //return -1
        return (MINUS);
        cerr << "\n Rule parsed: binary_operator -> MINUS \n";
        
    }else if(token.token_type == MULT){
        cerr << "\n Rule parsed: binary_operator -> MULT\n";
        //return -1
        return (MULT);
    }else if(token.token_type == DIV ){
        //return -1
        return (DIV);
        cerr << "\n Rule parsed: binary_operator -> DIV \n";
        
    }else if(token.token_type == GREATER){
        // return 2
        return (GREATER);
        cerr << "\n Rule parsed: binary_operator -> GREATER \n";
    }else if(token.token_type == LESS  ){
        // return 2
        return (LESS);
        cerr << "\n Rule parsed: binary_operator -> LESS\n";
    }else if(token.token_type == GTEQ ){
        // return 2
        return (GTEQ);
        cerr << "\n Rule parsed: binary_operator -> GTEQ \n";
        
    }else if(token.token_type == LTEQ){
        cerr << "\n Rule parsed: binary_operator -> LTEQ\n";
        // return 2
        return (LTEQ);
    }else if(token.token_type == EQUAL ){
        // return 2
        return (EQUAL);
        cerr << "\n Rule parsed: binary_operator -> EQUAL \n";
        
    }else if(token.token_type == NOTEQUAL){
        // return 2
        return (NOTEQUAL);
        cerr << "\n Rule parsed: binary_operator -> NOTEQUAL \n";
    }else{
        cerr << "\n Syntax Error \n";
        return(-1);
    }
}

int Parser::parse_primary()
{
    token = lexer.GetToken();
    
    if(token.token_type == ID  ){
        // search list for the token. If token available then return the type of the token. if not then add the token to the list
        // make its scope = "h" and make its type = -1;
        //char *lex1
        //strcpy(lex1, token.lexeme.c_str());
        //sTableEntry *t2 = searchList(lex1, 0);
        //return (t2->pubpriv)
        cerr << "\n Rule parsed: primary -> ID\n";
         return(searchList(token.lexeme));
    }else if(token.token_type == NUM ){
    
        cerr << "\n Rule parsed: primary -> NUM \n";
        return(INT_TYPE);
    }else if(token.token_type == REALNUM){
        cerr << "\n Rule parsed: primary -> REALNUM\n";
        return(REAL_TYPE);
    }else if(token.token_type == TR ){
    
        cerr << "\n Rule parsed: primary -> TRUE \n";
        return(BOOL_TYPE);
    }else if(token.token_type == FA){
        cerr << "\n Rule parsed: primary -> FALSE \n";
        return(BOOL_TYPE);
    }else{
        cerr << "\n Syntax Error \n";
        return(0);
    }
}

int Parser::parse_expression()
{
    int tempI;
    token = look();
    if (token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR || token.token_type == FA)
    {
        cerr << "\n Rule parsed: expression -> primary \n";
        tempI = parse_primary();
    }
    else if (token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV || 
        token.token_type == GREATER || token.token_type == LESS || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL)
    {
        cerr << "\n Rule parsed: expression -> binary_operator expression expression \n";
        int tempI1;
        int tempI2;
        tempI = parse_binaryOperator(); 
        tempI1 = parse_expression();
        tempI2 = parse_expression();
                //if(tempI1 != tempI2) type mismatch token.lineno C2 (this is not true if tempI1 == -1 or tempI2 == -1 )

        if ((tempI1 != tempI2) || (tempI != PLUS && tempI != MINUS && tempI != MULT && tempI != DIV && tempI != GTEQ && tempI != GREATER && tempI != LTEQ && tempI != NOTEQUAL && tempI != LESS && tempI != EQUAL))
        {

            if (tempI == PLUS || tempI == MINUS || tempI == MULT || tempI == DIV)
            {
                if (tempI1 <= 2 && tempI2 > 3)
                {
                    update_Types(tempI2, tempI1);
                    tempI2 = tempI1;
                }
                else if (tempI1 > 3 && tempI2 <= 2)
                {
                    update_Types(tempI2, tempI1);
                    tempI1 = tempI2;
                }
                else if (tempI1 > 3 && tempI2 > 3)
                {
                    update_Types(tempI2, tempI1);
                    tempI2 = tempI1;
                }
                else
                {
                    type_mismatch(C2);
                }
            }
            else if (tempI == GREATER || tempI == LESS || tempI == GTEQ || tempI == LTEQ || tempI == EQUAL || tempI == NOTEQUAL)
            {
                if (tempI1 <= 2 && tempI2 > 3)
                {
                    update_Types(tempI2, tempI1);
                    tempI2 = tempI1;
                    return(3);
                }
                else if (tempI1 > 3 && tempI2 <= 2)
                {
                    update_Types(tempI2, tempI1);
                    tempI1 = tempI2;
                    return(3);
                }
                else if (tempI2 > 3 && tempI1 > 3)
                {
                    update_Types(tempI2, tempI1);
                    tempI2 = tempI1;
                    return(3);
                }
                else
                {
                    type_mismatch(C2);
                }
            }
            else
            {
                type_mismatch(C2);
            }
        }
        if (tempI == GTEQ || tempI == GREATER || tempI == LTEQ || tempI == LESS || tempI == EQUAL || tempI == NOTEQUAL)
        {
            tempI = 3;
        }
        else
        {
            tempI = tempI2;
        }
    }
    else if (token.token_type == NOT)
    {
        cerr << "\n Rule parsed: expression -> unary_operator expression \n";
        tempI = parse_unaryOperator(); 
        tempI = parse_expression();
        if (tempI != 3)
                //if parse expression returns an ID and type of that ID is -1 then make it 2 by using search list
        // if tempI2 != 2 and != -1 then Type mismatch token.line_no C3????
        {
            type_mismatch(C3);
        }
    }
    else
    {
        cout << "\n Syntax Error \n";
    }
    return tempI;
}



void Parser::update_Types(int currentType, int newType)
{
    sTable* temp = symbolTable;

    while (temp->next != NULL)
    {
        if (temp->item->pubpriv == currentType)
        {
            temp->item->pubpriv = newType;
        }
        temp = temp->next;
    }
    if (temp->item->pubpriv == currentType)
    {
        temp->item->pubpriv = newType;
    }
}

int Parser::parse_assstmt()
{
    int tempI;
    string name;
    int t1;
    assume(ID);

    t1 = searchList(token.lexeme);
    assume(EQUAL);


    token = look();
    if (token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR || token.token_type == FA ||
        token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV ||
        token.token_type == LESS || token.token_type == GREATER || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL ||
        token.token_type == NOT)
    {
        tempI = parse_expression();
        if (t1 == INT_TYPE || t1 == REAL_TYPE || t1 == BOOL_TYPE)
        {
            if (t1 != tempI)
            {
                if (t1 < 3)
                {
                    type_mismatch(C1);
                }
                else
                {
                    update_Types(tempI, t1);
                    tempI = t1;
                }
            }
        }
        else
        {
            update_Types(t1, tempI);
            t1 = tempI;
        }
        assume(SEMICOLON);
        cerr << "\n Rule parsed: assignment_stmt -> ID EQUAL expression SEMICOLON"<<endl;
    }
    else
    {
        cout << "\n Syntax Error \n";
    }
    return(0);
}

int Parser::parse_case()
{
    int tempI;
    token = lexer.GetToken();
    if(token.token_type == CASE ){
        token = lexer.GetToken();
        if(token.token_type == NUM){
            token = lexer.GetToken();
            if(token.token_type == COLON){
                cerr << "\n Rule parsed: case -> CASE NUM COLON body";
                tempI = parse_body();
            }else{
                cout << "\n Syntax Error \n";
            }
        
        }else{
            cout << "\n Syntax Error \n";
        }
    
    }else{
        cout << "\n Syntax Error \n";
    }
}

int Parser::parse_caselist()
{
    int tempI;
    token = lexer.GetToken();
    if(token.token_type == CASE){
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_case();
        token = lexer.GetToken();
        if(token.token_type == CASE){
            tempTokenType = lexer.UngetToken(token);
            cerr << "\n Rule parsed: case_list -> case case_list \n";
            tempI = parse_caselist();
        }else if(token.token_type == RBRACE){
            tempTokenType = lexer.UngetToken(token);
            cerr << "\n Rule parsed: case_list -> case  \n";
        }
    }
    return(0);
}

int Parser::parse_switchstmt()
{
    int tempI;
    
    token = lexer.GetToken();
    if(token.token_type == SWITCH){
        token = lexer.GetToken();
        if(token.token_type == LPAREN){
            tempI = parse_expression();
            // if tempI != INT then throw type error
            // else if tempI = -1 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 1.
                if (tempI <= 3 && tempI != 1)
                {
                    type_mismatch(C5);
                }
            token = lexer.GetToken();
            if(token.token_type == RPAREN){
                token = lexer.GetToken();
                if(token.token_type == LBRACE){
                    tempI = parse_caselist();
                    token = lexer.GetToken();
                    if(token.token_type == RBRACE){
                        cerr << "\n Rule parsed: switch_stmt -> SWITCH LPAREN expression RPAREN LBRACE case_list RBRACE \n";        
                    }else{
                        cout << "\n Syntax Error \n";
                    }   
                }else{
                    cout << "\n Syntax Error \n";
                }
                
            }else{
                cout << "\n Syntax Error \n";
            }
        }else{
            cout << "\n Syntax Error \n";
        }    
    }else{
        cout << "\n Syntax Error \n";
    }
    return(0);
}

int Parser::parse_whilestmt()
{
    int tempI;
    
    token = lexer.GetToken();
    if(token.token_type == WHILE){
        token = lexer.GetToken();
        if(token.token_type == LPAREN){
            tempI = parse_expression();
            // if tempI != bool then throw type error
            // else if tempI = -1 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 2.
            if (tempI != BOOL_TYPE)
            {
                type_mismatch(C4);
            }
            token = lexer.GetToken();
            if(token.token_type == RPAREN){
                cerr << "\n Rule parsed: whilestmt -> WHILE LPAREN expression RPAREN body \n";
                tempI = parse_body();
                
            }else{
                cout << "\n Syntax Error \n";
            }
        }else{
            cout << "\n Syntax Error \n";
        }    
    }else{
        cout << "\n Syntax Error \n";
    }
    return(0);
}

int Parser::parse_ifstmt()
{
     int tempI;
    
    token = lexer.GetToken();
    if(token.token_type == IF){
        token = lexer.GetToken();
        if(token.token_type == LPAREN){
            tempI = parse_expression();

            if (tempI != BOOL_TYPE)
            {
                type_mismatch(C4);
            }
            // if tempI != bool then throw type error
            // else if tempI = -1 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 2.

            token = lexer.GetToken();
            if(token.token_type == RPAREN){
                cerr << "\n Rule parsed: ifstmt -> IF LPAREN expression RPAREN body \n";
                tempI = parse_body();
                
            }else{
                cout << "\n Syntax Error \n";
            }
        }else{
            cout << "\n Syntax Error \n";
        }    
    }else{
        cout << "\n Syntax Error \n";
    }
    return(0);
}

int Parser::parse_stmt()
{
    int tempI;
    token = lexer.GetToken();
    if(token.token_type == ID){
        tempTokenType = lexer.UngetToken(token);
        cerr << "\n Rule parsed: stmt -> assignment_stmt \n";
        tempI = parse_assstmt();        
                
    }else if(token.token_type == IF){
        tempTokenType = lexer.UngetToken(token);
        cerr << "\n Rule parsed: stmt -> if_stmt";
        tempI = parse_ifstmt();
    }else if(token.token_type == WHILE){
        tempTokenType = lexer.UngetToken(token);
        cerr << "\n Rule parsed: stmt -> while_stmt";
        tempI = parse_whilestmt();
    }else if(token.token_type == SWITCH){
        tempTokenType = lexer.UngetToken(token);
        cerr << "\n Rule parsed: stmt -> switch_stmt";
        tempI = parse_switchstmt();
    }else{
        cerr << "\n Syntax Error \n";
    }
    return(0);
}

int Parser::parse_stmtlist()
{
    token = look();
    int tempI;
    if (token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH)
    {
        tempI = parse_stmt();
        token = look();
        if (token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH)
        {
            cerr << "\n Rule Parsed: stmt_list -> stmt stmt_list \n";
            tempI = parse_stmtlist();

        }
        else if (token.token_type == RBRACE)
        {
            cerr << "\n Rule parsed: stmt_list -> stmt \n";
        }
    }
    else
    {
        cout << "\n Syntax Error \n";
    }
    return(0);
}

int Parser::parse_body()
{
    token = lexer.GetToken();
    int tempI;
    if (token.token_type == LBRACE)
    {
        cerr << "\n Rule Parsed: scope -> ID LBRACE public_vars private_vars stmt_list RBRACE \n";
        tempI = parse_stmtlist();
        assume(RBRACE);
        cerr << "\n Rule parsed: body -> LBRACE stmt_list RBRACE \n";
    }
    else if (token.token_type == END_OF_FILE)
    {
        tempTokenType = lexer.UngetToken(token);
    }
    else
    {
        cout << "\n Syntax Error \n";
    }
    return(0);
}

int Parser::parse_typename()
{
    sTable* temp = symbolTable;

    token = lexer.GetToken();
    if(token.token_type == INT || token.token_type == REAL || token.token_type == BOOL){
        while (temp->next != NULL)
        {
            if (temp->item->scope == token.line_no)
            {
                temp->item->pubpriv = token.token_type;
            }
        temp = temp->next;
        }
        if (temp->item->scope == token.line_no)
        {
            temp->item->pubpriv = token.token_type;
        }
        cerr  << "\n Rule parse: type_name -> " << token.token_type << "\n"; 
        
    }else{
        cout << "\n Syntax Error \n";
    }   
    return(0);// if Int ret 0 if float ret 1 if bool ret 2
}


int Parser::parse_vardecl()
{
    int tempI;
    token = look();
    if (token.token_type == ID)
    {
        tempI = parse_varlist();
        assume(COLON);
        tempI = parse_typename();
        assume(SEMICOLON);
        cerr << "\n Rule parsed: var_decl -> var_list COLON type_name SEMICOLON" << endl;
    }
    return(0);
}

int Parser::parse_vardecllist()
{
    int tempI;
    token = look();
    while (token.token_type == ID)
    {
        tempI = parse_vardecl();
        token = look();
        if (token.token_type != ID)
        {
            cerr << "\n Rule Parsed: var_decl_list -> var_decl \n";
        }
        else
        {
            cerr << "\n Rule Parsed: var_decl_list -> var_decl var_decl_list \n";
        }
    }
    return(0);
}

int Parser::parse_globalVars()
{
    token = look();
    int tempI;
    if (token.token_type == ID)
    {
        cerr << "\n Rule parsed: global_vars -> var_decl_list \n";
        tempI = parse_vardecllist();
    }
    else
    {
        cout << "\n Syntax Error \n";
    }
    return(0);
}


int Parser::parse_program()
{
  token = look();
    int tempI;
    while (token.token_type != END_OF_FILE)
    {
        if (token.token_type == ID)
        {
            cerr << "\n Rule parsed: program -> global_vars scope \n";
            tempI = parse_globalVars();
            tempI = parse_body();
        }
        else if (token.token_type == LBRACE)
        {
            cerr << "\n Rule parsed: global_vars -> epsilon \n";
            tempI = parse_body();
        }
        else if (token.token_type == END_OF_FILE)
        {
            return(0);
        }
        else
        {
            cout << "\n Syntax Error \n";
            return(0);
        }
        token = look();
    }
  
}



string output = "";

void Parser::printList()
{

    sTable* temp = symbolTable;
    int temp1;

    while (temp->next != NULL)
    {
        if (temp->item->pubpriv > 3 && temp->item->binNo == 0)
        {
            temp1 = temp->item->pubpriv;
            output += temp->item->name;
            temp->item->binNo = 1;
            while (temp->next != NULL)
            {
                temp = temp->next;
                if (temp->item->pubpriv == temp1)
                {
                    output += ", " + temp->item->name;
                    temp->item->binNo = 1;
                }
            }
            output += ": ? #";
            cout << output << endl;
            temp->item->binNo = 1;
            output = "";
            temp = symbolTable;
        }
        else if (temp->item->pubpriv < 4 && temp->item->binNo == 0)
        {
            string lt = keyword[(temp->item->pubpriv) - 1];
            int temp1 = temp->item->pubpriv;
            output = temp->item->name + ": " + lt + " #";
            cout << output << endl;
            output = "";
            temp->item->binNo = 1;

            while (temp->next != NULL && temp->next->item->pubpriv == temp1)
            {
                temp = temp->next;
                string lt2 = keyword[(temp->item->pubpriv) - 1];
                output = temp->item->name + ": " + lt2 + " #";
                cout << output << endl;
                temp->item->binNo = 1;
                output = "";
            }
        }
        else
        {
            temp = temp->next;
        }
    }
    if (temp->item->pubpriv <= 3 && temp->item->binNo == 0)
    {
        string lt3 = keyword[(temp->item->pubpriv) - 1];
        output += temp->item->name + ": " + lt3 + " #";
        cout << output << endl;
        output = "";
    }
    else if (temp->item->pubpriv > 3 && temp->item->binNo == 0)
    {
        output += temp->item->name + ":" + " ? " + "#";
        cout << output << endl;
        output = "";
    }
}
int main()
{
	Parser parser;
	int i;
	i = parser.parse_program();
	parser.printList();
}
