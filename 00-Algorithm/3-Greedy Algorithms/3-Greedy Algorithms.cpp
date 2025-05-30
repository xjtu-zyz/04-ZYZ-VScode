#include<iostream>
#include<cstring>
using namespace std;
typedef struct  node 
{
    int father;   //父节点
    int distance;   //到父节点的路长
    int out;        //出度
    int maxroad;    //到叶节点的路畅
    int cut;       //切掉标志
}node, *pnode;
		
class dTree {
	private:
    pnode tree;
    int BiggestDistance, numberofnode, sum, qhead, qend;
    int que[1000];
	public:
		
    dTree(int n, int d)  //构建与初始化树
    {       
        sum = 0;
        qhead = 0;
        qend = 0;
        BiggestDistance = d;
        numberofnode = n;
        tree = new node[n];
        tree[0].father = -1;
        tree[0].distance = 0;
        for (int i = 0; i < n; i++) 
        {
            int NumOfSon, WeightOfRoad, num;
            cin >> NumOfSon ;
            tree[i].cut = 0;
            tree[i].out = NumOfSon ;
            tree[i].maxroad = 0;
            for (int j = 0; j < NumOfSon; j++) 
            {
                cin >>  num >> WeightOfRoad ;
                tree[num].father = i;
                tree[num].distance =  WeightOfRoad;
            }
        }
    }
    void push(int i) //入队列
    {              
        que[qend] = i;
        qend++;
    }
    int pop() //出队列
    {                  
        int temp = que[qhead++];
        return temp;
    }
    void solution()
    {          
        for (int i = numberofnode - 1; i >= 0; i--) 
        {
            if (tree[i].out == 0) 
            {
                push(i);
            }
        }
        while (qhead != qend) 
        {
            int temp = pop();
            int len = tree[temp].distance;
            int par = tree[temp].father;
            if (tree[par].cut == 0 && tree[temp].maxroad + len > BiggestDistance) 
            {
                tree[par].cut = 1;
                par = tree[par].father;
                sum++;
            } 
            else if (tree[temp].cut == 0 && tree[par].maxroad < tree[temp].maxroad + len) 
            {
                tree[par].maxroad = tree[temp].maxroad + len;
            }
            if (--tree[par].out == 0)
                push(par);
        }
        cout<<sum<<endl;
    }
	
};

int main(){
    int n, d;
    cin >> n >> d;
    dTree tree(n, d);
    tree.solution();
 
    system("pause");
    
    return 0;
}