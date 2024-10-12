#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<stdexcept>
#include<cstdio>
#include<cstdlib>
#include "solver.h"
#include "Diagdoku.h"
#include <ctime> 
#include <vector>
#include <algorithm>
#include <time.h>
#include <numeric>
#include<string>
using namespace std;
extern int random;
#define N 9  // 9x9 ����
int inputOrder(int stt, int end, const char* const text = NULL)
//���������������ѡ�ѡ��Χ��[stt,end]
//���˵������óɺ��������Ա����ظ��ļ�������Ƿ���ȷ
//�Է���ֵ����ʽ������ 
//����ַ���text�������Ļ�ϣ�����ʾ�û����� 
{

    if (end < stt)throw runtime_error("in inputOrder():end<stt\n");//������������ʱ�������汨���϶��Ǵ������������������ 
    int i, ans = -1;
    /*
    i:ѭ������
    ans:�����ص�ֵ

    */
    char s[1000];//�����һ�У�Ӧ��û���˻�һ�д�1000���ַ��� 
    bool ok = false;//���ڱ�������Ƿ�Ϸ��� 
    if (text)printf("%s", text);//�����ʾ���� 
    while (1)//��ѭ����������Ϸ����������룬�����ٴ�ѭ�� 
    {
        fgets(s, sizeof(s), stdin);//����ȫ�ء�����Ļ�϶���һ���ַ��� 
        ok = true;

        for (i = 0; s[i] != '\n'; i++)//��������Ƿ��з������ַ� 
        {
            if (s[i] < '0' || s[i]>'9')
            {
                printf("���뺬�з������ַ�(%c,%d)��������\n", s[i], s[i]);
                ok = false;
                break;//����forѭ�� 
            }
        }
        s[i] = '\0';//����ַ�����ĩβ 
        if (ok == false)continue;
        if (strlen(s) == 0)continue;//����Ϊ��Ӧ���û�����һ�����е���� 

        ans = atoi(s);// atoi:���ַ�����ʽ������ת��Ϊint 
        if (ans<stt || ans>end)// �ж��Ƿ񳬳���Χ 
        {
            printf("���볬����Χ��������\n");
            ok = false;
            continue;
        }
        return ans;//��δ������Χ������
    }



}

void useDPLL()
//�˺���������ʾdpllģ��
{
    srand(clock());

    char filename[500];//cnf�ļ��� 
    char filename2[500];//res�ļ��� 
    crossCNF* cnf = NULL;//������cnf���� 
    int choice;//ѡ�� 
    FILE* fp = NULL;//�����ļ�ָ�� 
    bool solved = false;//��ʾ��ǰcnf�Ƿ񱻽�����������cnf�������ݽṹ�ᱻ�ƻ��������ٽ⣬�ٽ���Ҫ���´��ļ��ж�ȡ 
    while (1)//��ѭ�� 
    {
        system("cls");//���� 
        choice = inputOrder(0, 4,
            "||^^^^^^^^^^^^^^^^^^^^^^^||\n"
            "|| ѡ�������            ||\n"
            "|| ��1��.cnf�Ķ�ȡ��洢 || \n"
            "|| ��2��.����ָ����cnf   ||\n"
            "|| ��3��.������        ||\n"
            "||  (0) .�˳�            ||\n"
            "||^^^^^^^^^^^^^^^^^^^^^^^||\n");

        switch (choice)
        {
        case 0: {//�˳� 
            if (cnf != NULL)delete cnf;//�ͷ�cnf 
            return;
        }
        case 1: {//ָ��cnf 
            //����ָ��cnf�ļ���֮�⣬����黹��Ҫ���³�ʼ����������
            if (cnf != NULL)
            {
                printf("cnf�Ѿ����壬�����ͷž�cnf\n");
                delete cnf;//�ͷ�cnf 
                cnf = NULL;
                fclose(fp);
                fp = NULL;
            }


            while (fp == NULL)
            {
                printf("ָ����Ҫ����cnf�ļ�����\n");
                scanf("%s", filename);

                fp = fopen(filename, "r");
                if (fp == NULL)printf("���ļ�ʧ�ܣ����ԣ�\n");
            }
            //�����Ѿ����ļ��ɹ�
            cnf = new crossCNF(fp);//���³�ʼ��
            solved = false;
            //Ȼ��Ҫ���¸�ֵfilename2
            strcpy(filename2, filename);
            int i = 0;
            while (filename2[i] != '.')i++;
            filename2[i + 1] = 'r';
            filename2[i + 2] = 'e';
            filename2[i + 3] = 's';
            filename2[i + 4] = '\0';

            printf("����ļ���ָ��Ϊ%s\n", filename2);
            break;
        }

        case 2: {//�����ָ����cnf 
            if (cnf == NULL)
            {
                printf("��δָ��cnf!\n");
                break;
            }
            if (solved)
            {
                printf("���cnf�Ѿ�������� ���ݽṹ���ƻ��������³�ʼ����\n");
                break;
            }

            cnf->solve(filename2, filename, true);
            solved = true;
            printf("������Ѿ���д��%s\n", filename2);
            break;
        }

        case 3: {//��֤����ʾ  
            if (cnf == NULL)
            {
                printf("��δָ��cnf!\n");
                break;
            }
            if (solved == false)
            {
                printf("���cnf��δ��������\n");
                break;
            }
            cnf->calculate(filename2);
            break;
        }


        }
        system("pause");
    }



}


// �� (i, j, k) ת��ΪΨһ����������
int var(int i, int j, int k) {
    return (i - 1) * N * N + (j - 1) * N + k;
}
void inverse_var(int v, int& i, int& j, int& k) {
    v = v - 1;  // ת������ 0 ��ʼ������
    i = v / (N * N) + 1;
    j = (v % (N * N)) / N + 1;
    k = v % N + 1;
}
void toCnf(vector<vector<int>>& sudoku, int holes) {
    FILE* fp = fopen("Diagdoku.cnf", "w");

    if (!fp) {
        printf("Cannot open file for writing!\n");
        return;
    }
    fprintf(fp, "c\nc\n");
    fprintf(fp, "p cnf 729 %d\n", 9558 - holes);     //9477+81-holes �Խ��������������� 81-holes �������

    // ���Ӿ�
    for (int row = 0; row < sudoku.size(); ++row) {
        for (int col = 0; col < sudoku[row].size(); ++col) {
            int num = sudoku[row][col];
            if (num != 0)
            {
                fprintf(fp, "%d %d\n", var(row + 1, col + 1, num), 0);
            }
        }
    }
    // ÿ��ÿ��ÿ���ÿ������ֻ�ܳ���һ��
    for (int x = 1; x <= 9; x++) {
        for (int y = 1; y <= 9; y++) {
            for (int z = 1; z <= 9; z++) {
                fprintf(fp, "%d ", (x - 1) * 81 + (y - 1) * 9 + z);
            }
            fprintf(fp, "0\n");
        }
    }
    // ��
    for (int y = 1; y <= 9; y++) {
        for (int z = 1; z <= 9; z++) {
            for (int x = 1; x <= 8; x++) {
                for (int i = x + 1; i <= 9; i++) {
                    fprintf(fp, "-%d -%d 0\n", (x - 1) * 81 + (y - 1) * 9 + z, (i - 1) * 81 + (y - 1) * 9 + z);
                }
            }
        }
    }
    // ��
    for (int x = 1; x <= 9; x++) {
        for (int z = 1; z <= 9; z++) {
            for (int y = 1; y <= 8; y++) {
                for (int i = y + 1; i <= 9; i++) {
                    fprintf(fp, "-%d -%d 0\n", (x - 1) * 81 + (y - 1) * 9 + z, (x - 1) * 81 + (i - 1) * 9 + z);
                }
            }
        }
    }
    // 3x3������
    for (int z = 1; z <= 9; z++) {
        for (int i = 0; i <= 2; i++) {
            for (int j = 0; j <= 2; j++) {
                for (int x = 1; x <= 3; x++) {
                    for (int y = 1; y <= 3; y++) {
                        for (int k = y + 1; k <= 3; k++) {
                            fprintf(fp, "-%d -%d 0\n", (3 * i + x - 1) * 81 + (3 * j + y - 1) * 9 + z, (3 * i + x - 1) * 81 + (3 * j + k - 1) * 9 + z);
                        }
                    }
                }
            }
        }
    }
    for (int z = 1; z <= 9; z++) {
        for (int i = 0; i <= 2; i++) {
            for (int j = 0; j <= 2; j++) {
                for (int x = 1; x <= 3; x++) {
                    for (int y = 1; y <= 3; y++) {
                        for (int k = x + 1; k <= 3; k++) {
                            for (int l = 1; l <= 3; l++) {
                                fprintf(fp, "-%d -%d 0\n", (3 * i + x - 1) * 81 + (3 * j + y - 1) * 9 + z, (3 * i + k - 1) * 81 + (3 * j + l - 1) * 9 + z);
                            }
                        }
                    }
                }
            }
        }
    }
    // ���Խ���
    for (int z = 1; z <= 9; z++) {
        for (int i = 1; i <= 8; i++) {
            for (int j = i + 1; j <= 9; j++) {
                fprintf(fp, "-%d -%d 0\n", (i - 1) * 81 + (i - 1) * 9 + z, (j - 1) * 81 + (j - 1) * 9 + z);
            }
        }
    }
    // ���Խ���
    for (int z = 1; z <= 9; z++) {
        for (int i = 1; i <= 8; i++) {
            for (int j = i + 1; j <= 9; j++) {
                fprintf(fp, "-%d -%d 0\n", (i - 1) * 81 + (9 - i) * 9 + z, (j - 1) * 81 + (9 - j) * 9 + z);
            }
        }
    }
    fclose(fp);
}



// ��ӡ����
void print_sudoku(const vector<vector<int>>& sudoku) {
    for (const auto& row : sudoku) {
        for (int num : row) {
            cout << (num == 0 ? "." : to_string(num)) << " ";
        }
        cout << endl;
    }
}

// �������num�Ƿ���Է���(row, col)λ����
bool is_safe(const vector<vector<int>>& sudoku, int row, int col, int num) {
    // �����
    for (int x = 0; x < N; x++) {
        if (sudoku[row][x] == num) return false;
    }

    // �����
    for (int x = 0; x < N; x++) {
        if (sudoku[x][col] == num) return false;
    }

    // ���3x3�ӷ���
    int start_row = row - row % 3;
    int start_col = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (sudoku[i + start_row][j + start_col] == num) return false;
        }
    }

    // ���Խ���
    if (row == col) {  // ���Խ���
        for (int i = 0; i < N; i++) {
            if (sudoku[i][i] == num) return false;
        }
    }
    if (row + col == N - 1) {  // ���Խ���
        for (int i = 0; i < N; i++) {
            if (sudoku[i][N - 1 - i] == num) return false;
        }
    }

    return true;
}

// ʹ�õݹ�����㷨�������
bool fill_sudoku(vector<vector<int>>& sudoku, int row, int col) {
    // ����Ѿ�����������true
    if (row == N - 1 && col == N) return true;

    // �������������������һ��
    if (col == N) {
        row++;
        col = 0;
    }

    // �����ǰλ���Ѿ���ֵ���ݹ���һ��λ��
    if (sudoku[row][col] != 0) return fill_sudoku(sudoku, row, col + 1);

    // �������1��9������
    vector<int> numbers(N);
    iota(numbers.begin(), numbers.end(), 1);  // ����1��9������
    random_shuffle(numbers.begin(), numbers.end());  // �����������˳��

    for (int num : numbers) {
        if (is_safe(sudoku, row, col, num)) {
            sudoku[row][col] = num;

            // �ݹ������һ��λ��
            if (fill_sudoku(sudoku, row, col + 1)) return true;

            // ���ݣ�������ʧ�ܣ��ָ�0
            sudoku[row][col] = 0;
        }
    }

    return false;
}

// ����һ��������9x9�Խ�������
void generate_sudoku(vector<vector<int>>& sudoku) {
    fill_sudoku(sudoku, 0, 0);
}

// �ڶ���
void remove_digits(const vector<vector<int>>& original_sudoku, vector<vector<int>>& modified_sudoku, int holes) {
    // ����ԭʼ�������޸ĺ������
    modified_sudoku = original_sudoku;

    // ��������λ�õ��б�
    vector<int> positions;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            positions.push_back(i * N + j);
        }
    }

    // ����λ���б�
    random_shuffle(positions.begin(), positions.end());

    // �ڶ�
    for (int k = 0; k < holes; ++k) {
        int pos = positions[k];
        int row = pos / N;
        int col = pos % N;

        modified_sudoku[row][col] = 0; // ��ָ��λ����Ϊ0����ʾ�ڶ�
    }
}
void useDiagdoku()
//�˺�������ʹ�÷�������ģ��
{
    int choice;
    choice = inputOrder(0, 4,
        "||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
        "|| ѡ���ܣ�               ||\n"
        "|| 1.��������               ||\n"
        "|| 2.����SAT������������  ||\n"
        "|| 3.�����������           ||\n"
        "|| 4.�����               ||\n"
        "|| 0.�˳�                   ||\n"
        "||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
    );
    // ��ʼ��һ��9x9�յ�����
    vector<vector<int>> sudoku(N, vector<int>(N, 0));
    vector<vector<int>> modified_sudoku(N, vector<int>(N, 0));
    vector<vector<int>> newsudoku;
    int row, col, num;
    crossCNF* cnf = NULL;//������cnf����
    FILE* file = fopen("Diagdoku.cnf", "r");
    FILE* file2 = NULL;
    char line1[10], s[2];
    int temp1, i, j, k;
    int ans[N][N], cnt = 1;
    while (choice != 0)
    {
        system("cls");
        switch (choice)
        {
        case 1:
            srand(time(0)); // ʹ�õ�ǰʱ����Ϊ���������
            // ���ɶԽ�������
            generate_sudoku(sudoku);

            // // �ڶ�ǰ������
            // cout << "����������:\n";
            // print_sudoku(sudoku);

            // �ڶ���������������Ҫ��20����
            int holes;
            cout << "��������Ҫ�ڵĶ�����Ŀ\n";
            cin >> holes;
            remove_digits(sudoku, modified_sudoku, holes);
            newsudoku = modified_sudoku;
            // �ڶ��������
            cout << "\n���ɵ�����Ϊ:\n";
            print_sudoku(modified_sudoku);

            // �������� CNF
            toCnf(modified_sudoku, holes);

            printf("��������Ӧ��cnf�ļ���\n");
            system("pause");
            break;

        case 2:
            cnf = new crossCNF(file);//���³�ʼ��
            cnf->solve("Diagdoku.res", "Diagdoku.cnf", true);
            file2 = fopen("Diagdoku.res", "r");
            // ��ȡ��һ��
            fgets(line1, sizeof(line1), file2);
            // ��ȡ�ڶ���ͷ�����ַ�
            fgets(s, sizeof(s), file2);
            while (cnt <= 81)
            {
                fscanf(file2, "%d ", &temp1);
                if (temp1 > 0)
                {
                    inverse_var(temp1, i, j, k);
                    ans[i - 1][j - 1] = k;
                    cnt++;
                }
            }
            // �ر��ļ�
            fclose(file2);
            cout << "��׼��:\n";
            print_sudoku(sudoku);
            printf("SAT���𰸣�\n");
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < N; j++)
                    printf("%d ", ans[i][j]);
                printf("\n");
            }
            printf("�������");
            break;
        case 3:
            newsudoku = modified_sudoku;
            cout << "��ǰ��������" << endl;
            print_sudoku(newsudoku);
            cout << "��������������������λ�ã�i��j�����Լ����������num��ֻ����1��0�˳���д��" << endl;
            cin >> row;
            while (row != 0)
            {
                cin >> col >> num;
                newsudoku[row - 1][col - 1] = num;
                cout << "��ǰ��������" << endl;
                print_sudoku(newsudoku);
                cout << "������д������(i,j,num),����д������0" << endl;
                cin >> row;
            }
            break;
        case 4:
            if (newsudoku == sudoku)
                cout << "�����Ǹ���ţ�" << endl;
            else
                cout << "��д����Ŷ��������һ�ΰɣ�";
            cout << "����д�Ľ��" << endl;
            print_sudoku(newsudoku);
            cout << "��׼��" << endl;
            print_sudoku(sudoku);
            break;

        default:
            break;
        }
        choice = inputOrder(0, 4,
            "||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
            "|| ѡ���ܣ�                ||\n"
            "|| 1.��������                ||\n"
            "|| 2.����SAT������������    ||\n"
            "|| 3.�����������             ||\n"
            "|| 4.�����                 ||\n"
            "|| 0.�˳�                    ||\n"
            "||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
        );
    }
}




int main()
{
    int choice;
    while (1)
    {
        system("cls");
        choice = inputOrder(0, 2,
            "||^^^^^^^^^^^^^^^^^^^^^^^||\n"
            "|| ѡ���ܣ�            ||\n"
            "|| 1.SAT���  ģ��       ||\n"
            "|| 2.Diagdoku ģ��       ||\n"
            "|| 0.�˳�                ||\n"
            "||^^^^^^^^^^^^^^^^^^^^^^^||\n"
        );

        switch (choice)
        {
        case 0: {
            return 0;//��ʱ�˳����ǽ����������� 
        }
        case 1: {
            useDPLL();
            break;
        }

        case 2: {
            useDiagdoku();
            break;
        }
        }

    }


}