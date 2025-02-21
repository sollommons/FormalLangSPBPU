#include "parser.h"
#include <iostream>
#include <cstdlib>
bool prErrorFlag = false;
using namespace std;
struct prchar
{
	char c;
	prchar* next;
	prchar* prev;
};
void prError(const char* msg)
{
	printf("Precompiler error: %s\n", msg);
	prErrorFlag = true;
}
void printHelp()
{
	cout << "Usage: cmilan input_file" << endl;
}
prchar* parseBrackets(prchar* start)
{
	//»меем выражение (condition)?(expression):(expression);
	//либо (condition)?(expression);
	//функци€ заменит его на #condition?expression:expression%;
	prchar* tmp = start; //перва€ открывающа€ скобка
	prchar* closebracket = NULL;//перва€ закрывающа€ скобка
	prchar* question = NULL; //знак вопроса после первой закр скобки
	prchar* openbr2 = NULL; //открывающа€ скобка после знака вопр
	prchar* closebr2 = NULL; //втора€ закрывающа€ скобка
	prchar* semic = NULL; //двоеточие
	prchar* openbr3 = NULL; //открывающа€ скобка после двоеточи€
	prchar* closebr3 = NULL; //трет€€ закрывающа€ скобка
	while (tmp->next != NULL && tmp->next->c != ')')
	{
		if (tmp->next->c == '(')
		{
			tmp = parseBrackets(tmp->next);
		}
		else
		{
			tmp = tmp->next;
		}
	}
	if (tmp->next == NULL) //проверка конца файла
		return tmp;
	else closebracket = tmp->next; //запоминаем первую закр скобку
	//провер€ем элемент после скобки
	if (closebracket->next == NULL)
		return closebracket;
	else if (closebracket->next->c != '?')
		return closebracket;
	//инача мы нашли арифметический IF
	//сразу заменим первую скобку на символ начала блока #
	start->c = '#';
	question = closebracket->next; //запоминаем знак вопроса
	//дальше должна быть снова скобка
	tmp = question;
	if (tmp->next == NULL)
	{
		prError("expected open bracket after ? in (condition)?(expression) conditional expression");
		return tmp;
	}
	else if (tmp->next->c != '(')
	{
		prError("expected open bracket after ? in (condition)?(expression) conditional expression");
		return tmp;
	}
	//иначе всЄ хорошо
	openbr2 = tmp->next; //запоминаем вторую откр скобку
	//теперь заменим a)?(b на a?b
	closebracket->prev->next = question;
	question->prev = closebracket->prev;
	question->next = openbr2->next;
	if (openbr2->next != NULL) openbr2->next->prev = question;
	//идем дальше
	tmp = question;
	while (tmp->next != NULL && tmp->next->c != ')')
	{
		if (tmp->next->c == '(')
		{
			tmp = parseBrackets(tmp->next);
		}
		else
		{
			tmp = tmp->next;
		}
	}
	if (tmp->next == NULL) //проверка конца файла
	{
		prError("unexpected end of file");
		return tmp;
	}
	else closebr2 = tmp->next; //запоминаем вторую закр скобку
	tmp = closebr2;
	//дальше может быть : (ELSE) либо что угодно, но не конец файла
	if (tmp->next == NULL)
	{
		prError("expected at least ; after condition expression");
		return tmp;
	}
	else if (tmp->next->c != ':') //значит условный блок завершен
	{
		//мен€ем ) на %ж
		closebr2->c = '%';
		return tmp;
	}
	//имеетс€ блок ELSE
	semic = tmp->next; //запоминаем двоеточие
	tmp = tmp->next;
	if (tmp->next == NULL)
	{
		prError("expected open bracket after : in (condition)?(expression):(expression) conditional expression");
		return tmp;
	}
	else if (tmp->next->c != '(')
	{
		prError("expected open bracket after : in (condition)?(expression):(expression) conditional expression");
		return tmp;
	}
	//нашли третюю закр скобку
	openbr3 = tmp->next; //запоминаем третью откр скобку
	tmp = openbr3;
	while (tmp->next != NULL && tmp->next->c != ')')
	{
		if (tmp->next->c == '(')
		{
			tmp = parseBrackets(tmp->next);
		}
		else
		{
			tmp = tmp->next;
		}
	}
	if (tmp->next == NULL) //проверка конца файла
	{
		prError("unexpected end of file");
		return tmp;
	}
	else closebr3 = tmp->next; //запоминаем третью закр скобку
	tmp = closebr3;
	//теперь нужно осуществить замену элементов a):(expression)b на a@expression%b
	closebr2->prev->next = semic;
	semic->prev = closebr2->prev;
	semic->next = openbr3->next;
	openbr3->next->prev = semic;
	semic->c = '@';
	closebr3->c = '%';
	return closebr3;
	//#cond?
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		printHelp();
		getchar();
		return 1;
	}
	FILE* src;
	src = fopen(argv[1], "rt");//argv[1]
	FILE* tmp = fopen("tmp.prmil", "wt");
	if (src == NULL) printf("no source found\n");
	//precompiler, пишем в стек
	prchar* pr_head = new prchar;
	pr_head->c = fgetc(src);
	prchar* prtmpprev = pr_head;
	prchar* prtmp = pr_head;

	while (prtmp->c != EOF)
	{
		prtmp = new prchar;
		prtmp->c = fgetc(src);
		prtmp->prev = prtmpprev;
		prtmp->next = NULL;
		prtmpprev->next = prtmp;
		prtmpprev = prtmp;
	}
	prchar* pr_tail = prtmp->prev;

	//читаем и обрабатываем
	prtmp = pr_head;
	while (prtmp->next != NULL)
	{
		if (prtmp->c == '(')
		{
			prtmp = parseBrackets(prtmp);
		}
		else
		{
			prtmp = prtmp->next;
		}
	}
	//печатаем
	prtmp = pr_head;
	while (prtmp->next != NULL)
	{
		fprintf(tmp, "%c", prtmp->c);
		prtmp = prtmp->next;
	}
	if (prErrorFlag == true)
	{
		printf("Precompiler found errors, compiler will not run\n");
		return 0;
	}
	fclose(src);
	fclose(tmp);
	Parser p("tmp.prmil");
	p.parse();
	return 0;
}