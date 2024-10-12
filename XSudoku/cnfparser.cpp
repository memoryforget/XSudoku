#define _CRT_SECURE_NO_WARNINGS
#include<new>
#include "solver.h"
int crossCNF::changeBool(int x)
//�Ѹ��������һ�ֱȽϴ�������ĺ������Ա������������������� 
{
	return (x > 0) ? x : (2 * boolNum + 1 + x);
}

crossCNF::crossCNF(FILE* fp)//���캯�����ɸ���fp����CNF���� 
{
	char get;

	//����ע�ͣ�		
	while (1)
	{
		get = fgetc(fp);
		if (get == 'c')while (fgetc(fp) != '\n'); // ����ע�� 
		else break;
	}

	//�����ϣ�����get=='p'
	//ȡԪ���� 
	fscanf(fp, "%*s%d%d", &boolNum, &clauseNum);

	remainClauseNum = clauseNum;


	//�������ݿռ� 
	clauses = new crossNode[clauseNum + 1];//ͷ����
	bools = new crossNode[2 * boolNum + 2];//�ڵ��
	hypo = new BOOLVALUE[boolNum + 1];//ÿ���ڵ�ļ���
	sum = new int[clauseNum + 1];//�洢ÿ���Ӿ��ʣ���Ԫ��


	int i, nowClause = 1, temp;
	for (i = 0; i <= clauseNum; i++)clauses[i].Bool = 0;
	for (i = 0; i <= boolNum; i++)hypo[i] = UNSURE;
	for (i = 0; i < 2 * boolNum + 2; i++)bools[i].Bool = 0;
	for (i = 0; i <= clauseNum; i++)sum[i] = 0;
	//����洢
	while (nowClause <= clauseNum)
	{
		fscanf(fp, "%d", &temp);
		if (temp == 0)nowClause++;
		else addNode(nowClause, temp);// �ڴ˴����ʮ�������� 
	}
	fclose(fp);




	//��ʼ�������Ӿ䡱ջ 
	for (i = 1; i <= clauseNum; i++)if (sum[i] == 1)single.push(i);//�������Ӿ䣬�����Ӿ��������ջ��

	//��ʼ�������� 
	tendency = new int[boolNum + 1];//�������в�����Ԫ�ġ���������
	int posi, nega;
	crossNode* p;
	for (i = 1; i <= boolNum; i++)
	{
		posi = 0;
		nega = 0;
		// ��¼�������ֶ໹�Ǹ������ֶ� 
		for (p = bools[i].down; p; p = p->down)posi++;
		for (p = bools[changeBool(-i)].down; p; p = p->down)nega++;
		if (posi >= nega)tendency[i] = 1;
		else tendency[i] = -1;
	}

	return;
}


crossCNF::crossCNF(const char* const filename)//���캯���������ļ�������CNF���� 
{
	FILE* fp = fopen(filename, "r");
	new (this)crossCNF(fp);
	fclose(fp);
}

crossCNF::~crossCNF()//�������� 
{
	crossNode* p, * q;
	int i;
	//����ɾ�� 
	for (i = 1; i <= clauseNum; i++)
	{
		p = clauses[i].right;
		while (p)
		{
			q = p->right;
			delete p;
			p = q;
		}
	}
	delete[] bools, clauses, hypo, sum, tendency;//ɾȥ�ֶ����������ռ� 
}


void crossCNF::print(FILE* fp = stdout)//���δ�ӡĿǰcnfʮ�������е�δ��ɾ��������
{
	crossNode* p;
	for (int i = 1; i <= clauseNum; i++)
	{
		if (clauses[i].del)continue;
		fprintf(fp, "%d:", i);
		for (p = clauses[i].right; p != NULL; p = p->right)
		{
			if (p->del)continue;
			fprintf(fp, "%d ", p->Bool);
		}
		fprintf(fp, "\n");
	}
}



void crossCNF::addNode(int Clause, int Bool)
{
	crossNode* p = new crossNode(), * p1, * p2;
	p->Bool = Bool;
	p->Clause = Clause;
	Bool = changeBool(Bool);//����������ת��Ϊ�������� 
	p1 = &clauses[Clause];
	p2 = &bools[Bool];

	while (p1->right != NULL && changeBool(p1->right->Bool) < Bool)p1 = p1->right;
	while (p2->down != NULL && p2->down->Clause < Clause)p2 = p2->down;
	if (p1->right)p1->right->left = p;
	p->right = p1->right;
	p1->right = p;
	p->left = p1;

	if (p2->down)p2->down->up = p;
	p->down = p2->down;
	p2->down = p;
	p->up = p2;

	sum[Clause]++;

}