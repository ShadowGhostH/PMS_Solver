# PMS_Solver

Program to implement a Patrial Max SAT solver

## Cat

用 $enum$ 存放函数的返回状态，其中

**satisfied**  - 用于表示 PMS 中 hard clauses 全部满足，以及 soft clauses 全部验证完毕的状态

**unsatisfied** - 用于表示在求解过程中，出现 hard clause 为 false 的情况，此时求解不应继续进行下去

**normal** - 用于表示

## Formula

用于存放 PMS 实例的类，其中

```C++
vector<int> literals;
```

用于表示变量的取值，-1表示未赋值，0-ture，1-false

```C++
vector<int> literal_frequency; 
```

用于表示变量出现的频率，贪心的选出出现次数最多的变量，这样赋值后影响的语句更多，有利于剪枝

```C++
vector<int> literal_polarity;
```

用于记录当前变量以 $x_i$ 状态出现的多，还是以 $\neg x_i$ 状态出现的多，我们考虑优先给 $x_i$ 赋值 true or false

```C++
vector<vector<int> > clauses[2];
```

用于存放 hard clauses - clauses[0] 和 soft clauses - clauses[1]，通过下标关联方便遍历

```C++
int remove_count;
```

用于记录当前状态的 Formula 中，移除的、不满足条件的 soft clause 的数量，方便之后计算满足条件的 soft clause

```C++
void initialize(int literal_count, int hard_clause_count, int soft_clause_count{}
```

用于初始化 Formula 的大小和初值

```C++
void input(int hard_clause_count, int soft_clause_count){}
```

用于读入 hard clauses 和 soft clauses

## PMSATSolver

求解 PMS 问题所用的相关变量和函数

```C++
int literal_count;
Formula formula;
int hard_clause_count;
int soft_clause_count;
```

用于存放相关变量的数值

```C++
int unit_propagate(Formula &);
```

对一个 Formula 中的 **hard clauses** 部分进行单元传播，即对于子句中仅有一个变量的句子，我们可以确定它的取值。由于 soft clauses 可以不被满足，我们只求满足的最大值，所以 soft clauses 不适用于此。（$Unit\;Propagate\;rule$）

unit_propagate 的返回值如 Cat 中定义的那样，用来表示 当前实例状态，包括 satisfied、unsatisfied、normal

```C++
int apply_transform(Formula &, int);
```

对于我们确定下某个变量的取值后，我们应将这个取值应用在 Formula 中，应用时，对于包括当前变量的子句，有以下两种状态

1. 满足当前子句为真 - 已满足该子句，从 clauses 中删除，即为 empty_clause 状态
2. 不满足当前子句 - 从该子句中删除当前变量，若此时该子句为空，又考虑：
   - 若该子句为 hard clause，则立即返回 unsatisfied
   - 若该子句为 soft clause，则应使 remove_count++;

```C++
void display(Formula &, int, int);
```

打印实例是否被满足，如果满足则打印 Assignment

在当前程序版本中，在解答树的叶子节点上，打印了当前可行解，用于判断剪枝是否成功以及当前程序的正确性。最终版本需要删除


```C++
int PMSAT(Formula, int);
```

递归求解的程序，使用 Branch and Bound method，在解答树上进行 DFS，算法的结构如下

> **Algorithm** Branch and Bound Procedure for PMSAT
>
> $PMSAT(f,\; lower\_bound)$
>
> **Input: ** f - PMSAT 实例
> 			 lower_bound - 当前计算得到的最优解的值，初始化为 $-\infty$ 
>
> **Algorithm：**
>
> ​	upper_bound - 仍有满足的 soft clause 个数
>
> ​	**if**  upper_bound <= lower_bound **return** lower_bound （剪枝）
>
> ​	$unit\_propagate(f)$
>
> ​	**if**  satisfied  **return**  empty_clause in soft clauses 
>
> ​	**else if** unsatisfied **return** $- \infty$ 
>
> ​	l = select_variable  选出一个变量 l
>
> ​	$PMSAT(f_l,lower\_bound)$   $PMSAT(f_{\bar{l}}, lower_bound)$   (分支)
>
> ​	**return** $\max$ 

```C++
void initialize();
```

用于初始化

```C++
void solve();
```

封装的解答程序
