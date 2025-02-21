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
	//����� ��������� (condition)?(expression):(expression);
	//���� (condition)?(expression);
	//������� ������� ��� �� #condition?expression:expression%;
	prchar* tmp = start; //������ ����������� ������
	prchar* closebracket = NULL;//������ ����������� ������
	prchar* question = NULL; //���� ������� ����� ������ ���� ������
	prchar* openbr2 = NULL; //����������� ������ ����� ����� ����
	prchar* closebr2 = NULL; //������ ����������� ������
	prchar* semic = NULL; //���������
	prchar* openbr3 = NULL; //����������� ������ ����� ���������
	prchar* closebr3 = NULL; //������ ����������� ������
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
	if (tmp->next == NULL) //�������� ����� �����
		return tmp;
	else closebracket = tmp->next; //���������� ������ ���� ������
	//��������� ������� ����� ������
	if (closebracket->next == NULL)
		return closebracket;
	else if (closebracket->next->c != '?')
		return closebracket;
	//����� �� ����� �������������� IF
	//����� ������� ������ ������ �� ������ ������ ����� #
	start->c = '#';
	question = closebracket->next; //���������� ���� �������
	//������ ������ ���� ����� ������
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
	//����� �� ������
	openbr2 = tmp->next; //���������� ������ ���� ������
	//������ ������� a)?(b �� a?b
	closebracket->prev->next = question;
	question->prev = closebracket->prev;
	question->next = openbr2->next;
	if (openbr2->next != NULL) openbr2->next->prev = question;
	//���� ������
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
	if (tmp->next == NULL) //�������� ����� �����
	{
		prError("unexpected end of file");
		return tmp;
	}
	else closebr2 = tmp->next; //���������� ������ ���� ������
	tmp = closebr2;
	//������ ����� ���� : (ELSE) ���� ��� ������, �� �� ����� �����
	if (tmp->next == NULL)
	{
		prError("expected at least ; after condition expression");
		return tmp;
	}
	else if (tmp->next->c != ':') //������ �������� ���� ��������
	{
		//������ ) �� %�
		closebr2->c = '%';
		return tmp;
	}
	//������� ���� ELSE
	semic = tmp->next; //���������� ���������
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
	//����� ������ ���� ������
	openbr3 = tmp->next; //���������� ������ ���� ������
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
	if (tmp->next == NULL) //�������� ����� �����
	{
		prError("unexpected end of file");
		return tmp;
	}
	else closebr3 = tmp->next; //���������� ������ ���� ������
	tmp = closebr3;
	//������ ����� ����������� ������ ��������� a):(expression)b �� a@expression%b
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
	//precompiler, ����� � ����
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

	//������ � ������������
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
	//��������
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