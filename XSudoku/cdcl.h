#pragma once
#ifndef cdcl_HPP
#define cdcl_HPP
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace cdcl {

    // һЩ���Ͷ��壬�����������ʹ��
    using Var = int;       // ��������
    using CRef = int;      // �Ӿ���������
    using lbool = int;     // ����ֵ����

    // �������壬CRef_Undef��ʾδ������Ӿ�����
    const CRef CRef_Undef = -1;

    class Solver2 {
    private:
        // ���岼��ֵ״̬ l_True, l_False, l_Undef
        const lbool l_True = 0;    // ��ʾ��
        const lbool l_False = 1;   // ��ʾ��
        const lbool l_Undef = 2;   // ��ʾδ����

        const int var_Undef = -1;  // ��ʾδ����ı���

        // Literal�����֣��Ľṹ�嶨��
        struct Lit {
            int x; // ���ִ洢��ֵ

            // ���������������ȽϺͲ�������
            inline bool operator==(Lit p) const { return x == p.x; }
            inline bool operator!=(Lit p) const { return x != p.x; }
            inline bool operator<(Lit p) const { return x < p.x; }

            // ȡ������������ת���ֵķ���
            inline Lit operator~() {
                Lit q;
                q.x = x ^ 1; // ͨ��λ���㷭ת����
                return q;
            }
        };

        // ����һ�����֣���������ͷ���
        inline Lit mkLit(Var var, bool sign) {
            Lit p;
            p.x = var + var + sign;  // ���ݱ����ͷ��Ŵ�������
            return p;
        }

        // ��ȡ���ֵķ���
        inline bool sign(Lit p) const { return p.x & 1; }

        // ��ȡ���ֵı�������
        inline int var(Lit p) const { return p.x >> 1; }

        // ������ת��Ϊ����
        inline int toInt(Var v) { return v; }

        // ������ת��Ϊ����
        inline int toInt(Lit p) { return p.x; }

        // ������ת��Ϊ����
        inline Lit toLit(int x) {
            Lit p;
            p.x = x;
            return p;
        }

        // �����������������
        const Lit lit_Undef = { -2 }; // δ���������
        const Lit lit_Error = { -1 }; // ���������

        // VarData���������ݣ��Ľṹ�嶨��
        struct VarData {
            CRef reason;  // �Ӿ����ã���ʾ���¸ñ�����������Դ
            int level;    // ���������ľ��߲��
        };

        // �����������ݵĺ���
        inline VarData mkVarData(CRef cr, int l) {
            VarData d = { cr, l };
            return d;
        }

        // Watcher�����������Ľṹ�嶨�壬�����Ӿ�������ּ��ӻ���
        struct Watcher {
            CRef cref;    // �Ӿ�����
            Lit blocker;  // ���赲������
            Watcher() {}
            Watcher(CRef cr, Lit p) : cref(cr), blocker(p) {}

            // ���رȽ������
            bool operator==(const Watcher& w) const { return cref == w.cref; }
            bool operator!=(const Watcher& w) const { return cref != w.cref; }
        };

        // Clause���Ӿ䣩���ඨ�壬��ʾһ���Ӿ�
        class Clause {
        public:
            // �Ӿ�ͷ��������ѧϰ״̬���Ӿ��С
            struct {
                bool learnt;  // �Ƿ�Ϊѧϰ�Ӿ�
                int size;     // �Ӿ��е���������
            } header;

            // �洢�Ӿ��е�����
            std::vector<Lit> data;

            Clause() {}

            // ���캯�������������б��ѧϰ״̬�����Ӿ�
            Clause(const std::vector<Lit>& ps, bool learnt) {
                header.learnt = learnt;    // �����Ƿ�Ϊѧϰ�Ӿ�
                header.size = ps.size();   // �����Ӿ��С

                data.resize(header.size);  // �����洢�ռ��С
                for (int i = 0; i < ps.size(); i++) {
                    data[i] = ps[i];         // �������ֵ�������
                }
            }

            // ��ȡ�Ӿ��С
            int size() const { return header.size; }

            // ����Ƿ�Ϊѧϰ�Ӿ�
            bool learnt() const { return header.learnt; }

            // �Ӿ���±����������ȡ����������
            Lit& operator[](int i) { return data[i]; }
            Lit operator[](int i) const { return data[i]; }
        };

        // �����Ӿ�ĺ������������б�ת��Ϊ�Ӿ䲢�������ڴ�
        CRef allocClause(std::vector<Lit>& ps, bool learnt = false) {
            static CRef res = 0;         // ��̬���������Ӿ�����
            ca[res] = std::move(Clause(ps, learnt));  // �����Ӿ�
            return res++;                // �����Ӿ����ò�����
        }

        // �����±�������ӵ��������ݽṹ��
        Var newVar(bool sign = true, bool dvar = true) {
            int v = nVars(); // ��ȡ��ǰ����������

            // ���δ���岼��ֵ�����������б�
            assigns.emplace_back(l_Undef);

            // �����������ݣ�������Դδ���壬���Ϊ0
            vardata.emplace_back(mkVarData(CRef_Undef, 0));

            // ����ԡ����Ե���Ϣ��ʼ��
            activity.emplace_back(0.0);
            seen.push_back(false);
            polarity.push_back(sign);
            decision.push_back(0);

            // ���ñ���Ϊ���߱���
            setDecisionVar(v, dvar);
            return v;
        }

        // ���һ���Ӿ�
        bool addClause_(std::vector<Lit>& ps) {
            // ����Ӿ�Ϊ�գ�����false
            if (ps.size() == 0) {
                return false;
            }
            else if (ps.size() == 1) {  // ����Ӿ�ֻ��һ�����֣�ֱ�����
                uncheckedEnqueue(ps[0]);
            }
            else {  // �����Ӿ���䲢���ӵ������б�
                CRef cr = allocClause(ps, false);
                attachClause(cr);
            }
            return true;
        }

        // ���Ӿ丽�ӵ������ּ����б�
        void attachClause(CRef cr) {
            const Clause& c = ca[cr]; // ��ȡ�Ӿ�

            assert(c.size() > 1);     // ȷ���Ӿ䳤�ȴ���1

            // ���Ӿ��ǰ����������ӵ������б�
            watches[(~c[0]).x].emplace_back(Watcher(cr, c[1]));
            watches[(~c[1]).x].emplace_back(Watcher(cr, c[0]));
        }

        // �����Ӿ�ĺ��������ַ����ж�ȡ�Ӿ䲢ת��Ϊ�����б�
        void readClause(const std::string& line, std::vector<Lit>& lits) {
            lits.clear();  // ��������б�
            int parsed_lit, var;
            parsed_lit = var = 0;  // ��ʼ��

            bool neg = false;      // ��¼�Ƿ�Ϊ������
            std::stringstream ss(line);  // �����ַ�������ȡÿ��

            while (ss) {  // �����ȡ����
                int val;
                ss >> val;  // ���ַ����ж�ȡһ��ֵ
                if (val == 0) break;  // �����ȡ��0�������
                var = abs(val) - 1;   // ��ȡ�������

                while (var >= nVars()) {  // ���������ų�����Χ�������±���
                    newVar();
                }
                // �������������Ŵ������֣�����ӵ��б�
                lits.emplace_back(val > 0 ? mkLit(var, false) : mkLit(var, true));
            }
        }


        // ά���Ӿ伯��
        std::unordered_map<CRef, Clause> ca;  // �洢�����Ӿ䣬CRefΪ�Ӿ����ã�ClauseΪ�Ӿ����
        std::unordered_set<CRef> clauses;     // ԭʼ�����е��Ӿ伯��
        std::unordered_set<CRef> learnts;     // ѧϰ�õ����Ӿ伯��

        // �������б�ÿ�����ֶ�����������������
        std::unordered_map<int, std::vector<Watcher>> watches;

        // �����������
        std::vector<VarData> vardata;  // �洢ÿ��������������Դ�;��߲��
        std::vector<bool> polarity;    // ÿ����������ѡ���ԣ�����ֵ��
        std::vector<bool> decision;    // ����Ƿ�Ϊ���߱���
        std::vector<bool> seen;        // ���ڱ���Ƿ���ʹ�ĳ������

        // ׷���ѷ�������֣���ֵ���Լ����ǵľ��߲��
        int qhead;                     // trail���еĵ�ǰͷ��ָ��
        std::vector<Lit> trail;        // �洢�ѷ��������
        std::vector<int> trail_lim;    // �洢ÿ�����߲�ε�trail�߽�

        // ���߱��������ȼ����У�ʹ�û������
        std::set<std::pair<double, Var>> order_heap;  // ʹ�ñ�����Ժͱ�����Ž�������
        std::vector<double> activity;  // ÿ�������Ļ��
        double var_inc;                // ��Ե����Ӳ���

        // ģ�ͺͳ�ͻ���
        std::vector<Lit> model;        // �洢���յĽ⣨ģ�ͣ�
        std::vector<Lit> conflict;     // �洢��ǰ��ͻ���Ӿ�

        // ��ȡ��ǰ�ı���������nVars��
        int nVars() const { return vardata.size(); }  // ���ر�������

        // ��ȡ��ǰ���߲��
        int decisionLevel() const { return trail_lim.size(); }  // ���߲����trail_lim��С��ͬ

        // ����һ�����߲��
        void newDecisionLevel() { trail_lim.emplace_back(trail.size()); }  // �ڵ�ǰtrail�߽��¼�µľ��߲��

        // ��ȡĳ��������������Դ�Ӿ�
        inline CRef reason(Var x) const { return vardata[x].reason; }

        // ��ȡĳ�������ľ��߲��
        inline int level(Var x) const { return vardata[x].level; }

        // ����������Ӻ���������ĳ�������Ļ��
        inline void varBumpActivity(Var v) {
            std::pair<double, Var> p = std::make_pair(activity[v], v);
            activity[v] += var_inc;  // ���ӱ����Ļ��
            // ����ñ�����order_heap�У��Ƴ��ɵ�ֵ�������µĻ��
            if (order_heap.erase(p) == 1) {
                order_heap.emplace(std::make_pair(activity[v], v));
            }

            // �����Թ���������л�Խ������ţ���ֹ�����
            if (activity[v] > 1e100) {
                std::set<std::pair<double, Var>> tmp_order;
                tmp_order = std::move(order_heap);
                order_heap.clear();
                for (int i = 0; i < nVars(); i++) {
                    activity[i] *= 1e-100;  // ��С���б����Ļ��
                }
                for (auto& val : tmp_order) {
                    order_heap.emplace(std::make_pair(activity[val.second], val.second));  // ���²��������ź�ı���
                }
                var_inc *= 1e-100;  // ��С������Ӳ���
            }
        }

        // �ж��Ӿ��Ƿ��Ѿ�����
        bool satisfied(const Clause& c) const {
            for (int i = 0; i < c.size(); i++) {
                if (value(c[i]) == l_True) {  // ���ĳ������Ϊ�棬���Ӿ��Ѿ�����
                    return true;
                }
            }
            return false;  // �����Ӿ�δ����
        }

        // ��ȡĳ�������ĵ�ǰ��ֵ������ֵ��
        lbool value(Var p) const { return assigns[p]; }

        // ��ȡĳ�����ֵĵ�ǰ��ֵ
        lbool value(Lit p) const {
            if (assigns[var(p)] == l_Undef) {
                return l_Undef;  // �������δ��ֵ������δ����
            }
            return assigns[var(p)] ^ sign(p);  // �������ֵ�ֵ�����Ƿ���
        }

        // ����ĳ������Ϊ���߱���
        void setDecisionVar(Var v, bool b) {
            decision[v] = b;  // ���øñ���Ϊ���߱���
            order_heap.emplace(std::make_pair(0.0, v));  // ��������ӵ����߶�����
        }

        // ��ĳ������δ�����ӣ���ֵ��
        void uncheckedEnqueue(Lit p, CRef from = CRef_Undef) {
            assert(value(p) == l_Undef);  // ȷ������δ����ֵ
            assigns[var(p)] = sign(p);    // ��ֵ
            vardata[var(p)] = std::move(mkVarData(from, decisionLevel()));  // ����������Դ�;��߲��
            trail.emplace_back(p);  // ��������ӵ�trail��
        }

        // ѡȡ��֧���֣����߱�����
        Lit pickBranchLit() {
            Var next = var_Undef;
            // ѭ��ѡ����һ��δ��ֵ�ı���
            while (next == var_Undef or value(next) != l_Undef) {
                if (order_heap.empty()) {  // ������߶���Ϊ�գ��޷�ѡ�����
                    next = var_Undef;
                    break;
                }
                else {
                    auto p = *order_heap.rbegin();  // ѡ������ߵı���
                    next = p.second;
                    order_heap.erase(p);  // �Ƴ��ñ���
                }
            }
            return next == var_Undef ? lit_Undef : mkLit(next, polarity[next]);  // ����ѡ�������
        }

        // �Ӿ�ѧϰ�ͳ�ͻ����
        void analyze(CRef confl, std::vector<Lit>& out_learnt, int& out_btlevel) {
            int pathC = 0;  // ��¼·���е���������
            Lit p = lit_Undef;  // ��ǰ����
            int index = trail.size() - 1;  // ��trail��ĩβ��ʼ����
            out_learnt.emplace_back(mkLit(0, false));  // ��ʼ��ѧϰ�Ӿ�

            do {
                assert(confl != CRef_Undef);  // ȷ����ͻ�Ӿ����
                Clause& c = ca[confl];  // ��ȡ��ͻ�Ӿ�
                for (int j = (p == lit_Undef) ? 0 : 1; j < c.size(); j++) {
                    Lit q = c[j];
                    // �������δ����ǲ������ھ��߲�δ���0�ı���
                    if (not seen[var(q)] and level(var(q)) > 0) {
                        varBumpActivity(var(q));  // ���Ӹñ����Ļ��
                        seen[var(q)] = 1;  // ��Ǹñ���
                        if (level(var(q)) >= decisionLevel()) {
                            pathC++;  // ����ñ������ڵ�ǰ���߲�Σ�����·������
                        }
                        else {
                            out_learnt.emplace_back(q);  // ���򣬽�����ӵ�ѧϰ�Ӿ���
                        }
                    }
                }
                // ��������ǰһ�����֣�ֱ���ҵ���ǰ���߲�ε�����
                while (not seen[var(trail[index--])])
                    ;
                p = trail[index + 1];
                confl = reason(var(p));  // ��ȡǰһ�����ֵ�������Դ
                seen[var(p)] = 0;  // ȡ�����
                pathC--;  // ����·������
            } while (pathC > 0);

            out_learnt[0] = ~p;  // ��ת���Ⱦ��ߵ����֣���Ϊѧϰ�Ӿ�ĵ�һ������

            // ���ѧϰ�Ӿ�ֻ��һ�����֣������ݲ����Ϊ0����Ԫ�Ӿ䣩
            if (out_learnt.size() == 1) {
                out_btlevel = 0;
            }
            else {
                int max_i = 1;
                // �ҵ�ѧϰ�Ӿ��о��߲����ߵ�����
                for (int i = 2; i < out_learnt.size(); i++) {
                    if (level(var(out_learnt[i])) > level(var(out_learnt[max_i]))) {
                        max_i = i;
                    }
                }
                // ����������Ϊѧϰ�Ӿ�ĵڶ������֣����»��ݲ��
                Lit p = out_learnt[max_i];
                out_learnt[max_i] = out_learnt[1];
                out_learnt[1] = p;
                out_btlevel = level(var(p));
            }

            // ��ձ��
            for (int i = 0; i < out_learnt.size(); i++) {
                seen[var(out_learnt[i])] = false;
            }
        }

        // ���ݵ�ָ���ľ��߲�Σ���ȡ�����߲�ε����и�ֵ
        void cancelUntil(int level) {
            // ֻ�е�ǰ�ľ��߲�δ���Ŀ����ʱ�Ž��л���
            if (decisionLevel() > level) {
                // ����trail�еĸ�ֵ���ӵ�ǰ���߲�ε�ָ�����
                for (int c = trail.size() - 1; c >= trail_lim[level]; c--) {
                    Var x = var(trail[c]);   // ��ȡ����
                    assigns[x] = l_Undef;    // ȡ���ñ����ĸ�ֵ
                    polarity[x] = sign(trail[c]);  // �ָ��ñ����ļ��ԣ����򸺣�
                    // ���������¼�����߶��У��Ա�����������½��о���
                    order_heap.emplace(std::make_pair(activity[x], x));
                }
                qhead = trail_lim[level];  // ���´��������
                // ���trail��trail_lim�дӵ�ǰ������ϵ�������Ŀ
                trail.erase(trail.end() - (trail.size() - trail_lim[level]), trail.end());
                trail_lim.erase(trail_lim.end() - (trail_lim.size() - level), trail_lim.end());
            }
        }

        // ���������Ѹ�ֵ�����ֲ�����ͻ
        CRef propagate() {
            CRef confl = CRef_Undef;  // ��ʼ����ͻ����Ϊδ����
            int num_props = 0;        // ��¼��������

            // ����trail�е�ÿ�����֣����д���
            while (qhead < trail.size()) {
                Lit p = trail[qhead++];  // ��trail��ȡ��һ����Ҫ����������
                std::vector<Watcher>& ws = watches[p.x];  // ��ȡ�����ֵļ����б�
                std::vector<Watcher>::iterator i, j, end;
                num_props++;  // ���Ӵ�������

                // ���������б��е������Ӿ�
                for (i = j = ws.begin(), end = i + ws.size(); i != end;) {
                    // ��ȡ�赲���֣�������Ӿ���в���Ҫ�ļ��
                    Lit blocker = i->blocker;
                    if (value(blocker) == l_True) {  // ����赲����Ϊ�棬������ǰ�Ӿ�
                        *j++ = *i++;
                        continue;
                    }

                    CRef cr = i->cref;  // ��ȡ��ǰ���ӵ��Ӿ�����
                    Clause& c = ca[cr];  // ��ȡ�Ӿ����
                    Lit false_lit = ~p;  // ��ȡ��ǰ�������ֵķ�ת
                    if (c[0] == false_lit) c[0] = c[1], c[1] = false_lit;  // �����Ӿ��еĵ�һ���͵ڶ�������
                    assert(c[1] == false_lit);  // ȷ���ڶ���������false_lit
                    i++;

                    Lit first = c[0];  // ��ȡ�Ӿ�ĵ�һ������
                    Watcher w = Watcher(cr, first);  // �����µļ�����
                    if (first != blocker && value(first) == l_True) {  // �����һ������Ϊ�棬������һ��
                        *j++ = w;
                        continue;
                    }

                    // �����ҵ��µļ�������
                    for (int k = 2; k < c.size(); k++) {
                        if (value(c[k]) != l_False) {  // ����ҵ�һ���Ǽ�����
                            c[1] = c[k];  // ���µڶ�����������
                            c[k] = false_lit;  // ��false_lit�Ż��Ӿ�
                            watches[(~c[1]).x].emplace_back(w);  // ���µļ�������������б�
                            goto NextClause;  // ������ǰ�Ӿ䣬������һ���Ӿ�
                        }
                    }
                    *j++ = w;  // ���û���ҵ��µļ������֣�������ǰ������

                    // �����һ�����ֵ�ֵΪ�٣�������ͻ
                    if (value(first) == l_False) {
                        confl = cr;  // ��¼��ͻ���Ӿ�
                        qhead = trail.size();  // ֹͣ����
                        while (i < end) *j++ = *i++;  // ��ʣ��ļ���������
                    }
                    else {
                        uncheckedEnqueue(first, cr);  // ����һ��������ӣ���������
                    }
                NextClause:;
                }
                int size = i - j;  // ������Ҫ�Ƴ�����Ч����������
                ws.erase(ws.end() - size, ws.end());  // �Ƴ���Ч�ļ�����
            }
            return confl;  // ���س�ͻ���ã�����У�
        }

        // Luby�������Ժ���������luby����ֵ
        static double luby(double y, int x) {
            int size, seq;
            // �ҵ���������'x'�������������Լ��������еĴ�С
            for (size = 1, seq = 0; size < x + 1; seq++, size = 2 * size + 1)
                ;

            while (size - 1 != x) {
                size = (size - 1) >> 1;  // ����������
                seq--;
                x = x % size;  // ����'x'Ϊ�������е��������
            }

            return pow(y, seq);  // ����Luby���е�ֵ
        }

        // ��Ҫ������������������������ͻ�������ݡ����ߡ�����������
        lbool search(int nof_conflicts) {
            int backtrack_level;
            std::vector<Lit> learnt_clause;  // ���ڴ洢ѧϰ�Ӿ�
            learnt_clause.emplace_back(mkLit(-1, false));  // ��ʼ��ѧϰ�Ӿ�
            int conflictC = 0;  // ��ͻ����

            while (true) {
                CRef confl = propagate();  // ���д�������ȡ��ͻ�Ӿ�

                if (confl != CRef_Undef) {  // ������ͻ
                    conflictC++;  // ���ӳ�ͻ����
                    if (decisionLevel() == 0) return l_False;  // ����ھ��߲��0������ͻ�������޽�
                    learnt_clause.clear();
                    analyze(confl, learnt_clause, backtrack_level);  // ���г�ͻ����������ѧϰ�Ӿ�
                    cancelUntil(backtrack_level);  // ���ݵ��ʵ��Ĳ��

                    // ���ѧϰ�Ӿ��ǵ��Ӿ䣬����ֱ�Ӽ��뵽���߶���
                    if (learnt_clause.size() == 1) {
                        uncheckedEnqueue(learnt_clause[0]);
                    }
                    else {
                        // ���򴴽�ѧϰ�Ӿ䣬�����뵽�Ӿ伯��
                        CRef cr = allocClause(learnt_clause, true);
                        attachClause(cr);  // ����ѧϰ�Ӿ�
                        uncheckedEnqueue(learnt_clause[0], cr);  // ��ѧϰ�Ӿ�ĵ�һ���������
                    }
                    var_inc *= 1.05;  // ���ӱ�����ԵĲ���������˥����
                }
                else {
                    // û�г�ͻ�����
                    if ((nof_conflicts >= 0 and conflictC >= nof_conflicts)) {
                        cancelUntil(0);  // �ﵽ��ͻ���ƺ󣬻��ݵ�������
                        return l_Undef;  // ����δ���壬��ʾ��Ҫ����
                    }
                    Lit next = pickBranchLit();  // ѡ����һ�����߱���

                    if (next == lit_Undef) {
                        return l_True;  // ���û�и��������ѡ�������н⣨SAT��
                    }
                    newDecisionLevel();  // �����µľ��߲��
                    uncheckedEnqueue(next);  // �����߱������
                }
            }
        };


    public:
        // ��ǰ�ı�����ֵ��assigns[0] = 0 ��ʾ X1 = True��assigns[1] = 1 ��ʾ X2 = False
        std::vector<lbool> assigns;
        // �洢�������մ𰸣�0 ��ʾ������ (SATISFIABLE)��1 ��ʾ�������� (UNSATISFIABLE)��2 ��ʾδ֪ (UNKNOWN)
        lbool answer;

        // ���캯������ʼ������ͷ��qhead��Ϊ 0
        Solver2() { qhead = 0; }

        // ����DIMACS��ʽ��CNF�����ļ�����ȡ�������Ӿ���Ϣ
        void parseDimacsProblem(std::string problem_name) {
            std::vector<Lit> lits;  // �洢��ʱ��ȡ���Ӿ�����
            int vars = 0;           // ��������
            int clauses = 0;        // �Ӿ����
            std::string line;       // ��ǰ��ȡ����
            std::ifstream ifs(problem_name, std::ios_base::in);  // �������ļ�
            while (ifs.good()) {
                getline(ifs, line);  // ��ȡÿһ��
                if (line.size() > 0) {
                    if (line[0] == 'p') {  // ���������������
                        sscanf(line.c_str(), "p cnf %d %d", &vars, &clauses);  // �����������Ӿ�����
                    }
                    else if (line[0] == 'c' or line[0] == 'p') {
                        continue;  // ����ע���к�����������
                    }
                    else {
                        // ��ȡ�Ӿ���Ϣ������Ӿ䵽�����
                        readClause(line, lits);
                        if (lits.size() > 0) addClause_(lits);  // ����Ӿ䲻Ϊ�գ���ӵ��Ӿ伯
                    }
                }
            }
            ifs.close();  // �ر��ļ�
        }

        // ��Ҫ����⺯��������Luby�������Խ���DPLL���������ؽ��״̬
        lbool solve() {
            model.clear();     // ���ģ��
            conflict.clear();  // ��ճ�ͻ��
            lbool status = l_Undef;  // ��ʼ��״̬Ϊδ֪
            answer = l_Undef;  // ��ʼ�����մ�Ϊδ֪
            var_inc = 1.01;    // ���ó�ʼ��������
            int curr_restarts = 0;  // ��ǰ��������
            double restart_inc = 2;  // Luby���е���������
            double restart_first = 100;  // �״������Ļ���
            while (status == l_Undef) {
                // ʹ��Luby���Լ����������������ƺ�ʱ��������
                double rest_base = luby(restart_inc, curr_restarts);
                status = search(rest_base * restart_first);  // �������������Խ����ǰ����
                curr_restarts++;  // ������������
            }
            answer = status;  // ��¼���մ�
            return status;    // ���ؽ����SAT, UNSAT, �� UNKNOWN
        }

        // ����Ӿ䵽�����
        void addClause(std::vector<int>& clause) {
            std::vector<Lit> lits;  // �洢����
            lits.resize(clause.size());  // �����Ӿ��С������ʱ�洢������С
            for (int i = 0; i < clause.size(); i++) {
                int var = abs(clause[i]) - 1;  // ��DIMACS����ת��Ϊ0�����ı������
                while (var >= nVars()) newVar();  // ���������δ���䣬�����±���
                // �����Ӿ��е������Ŵ�������
                lits[i] = std::move((clause[i] > 0 ? mkLit(var, false) : mkLit(var, true)));
            }
            addClause_(lits);  // ��Ӵ���õ��Ӿ䵽�����
        }

        // ��ӡ���Ľ��
        void printAnswer() {
            if (answer == 0) {  // ����ǿ������(SAT)
                std::cout << "SAT" << std::endl;
                for (int i = 0; i < assigns.size(); i++) {
                    // ��ӡÿ�������ĸ�ֵ�������0��ʾTrue������ΪFalse
                    if (assigns[i] == 0) {
                        std::cout << (i + 1) << " ";  // ��������ֵ��ʾTrue
                    }
                    else {
                        std::cout << -(i + 1) << " ";  // ��ֵ��ʾFalse
                    }
                }
                std::cout << "0" << std::endl;  // ��ӡ0��ʾ����
            }
            else {
                std::cout << "UNSAT" << std::endl;  // ����ǲ��������(UNSAT)�����UNSAT
            }
        }

    };
}  // namespace cdcl
#endif  // cdcl_HPP
