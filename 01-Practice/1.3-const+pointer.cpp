#include<iostream>
#include<string.h>
using namespace std;

int main(){
	
	const char *p="abc";
	cout<<p<<endl;
	

	const char *q=new char[5]{'d','e','f',' '};
	cout<<q<<strlen(q)+1<<endl;  //strlen������'\0'������+1
	
  //char *const q=new char[5]{'d','e','f'};  //��������q��q�Ͳ��ܸı���
  //cout<<q<<strlen(q)+2<<endl;

	char x[2];
	gets(x);
    strcpy((char*)q,(char*)p); //����ǿ��ת�����ͣ�ʵ���ַ������ݵĸı�

	//strcpy(q,p);
	//cin>>q[strlen(q)];
    strcat((char*)q,x);

  
    cout<<q<<endl;;




	delete[]q;
	//system("pause");
	return 0;
} 