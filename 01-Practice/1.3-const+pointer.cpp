#include<iostream>
#include<string.h>
using namespace std;

int main(){
	
	const char *p="abc";
	cout<<p<<endl;
	

	const char *q=new char[5]{'d','e','f',' '};
	cout<<q<<strlen(q)+1<<endl;  //strlen不包括'\0'，所以+1
	
  
	
	char x[2];
	gets(x);
    strcpy((char*)q,(char*)p); //属于强制转换类型，实现字符串内容的改变
    strcat((char*)q,x);

  
    cout<<q<<endl;;




	delete[]q;
	//system("pause");
	return 0;
} 