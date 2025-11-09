#include <iostream>
#include "omp.h"
using namespace std;
int main(int argc, char **argv) {
	//设置线程数，一般设置的线程数不超过CPU核心数，这里开4个线程执行并行代码段
	omp_set_num_threads(4);
#pragma omp parallel
	{
		cout << "Hello" << ", I am Thread " << omp_get_thread_num() << endl;
	}
}