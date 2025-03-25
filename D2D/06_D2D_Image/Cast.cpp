
/*
#include <iostream>

int main()
{
	double db = 10.1234f;
	int a = db;							// 묵시적 형변환	     (10)
	int b = (int)db;					// 명시적 형변환	     (10)
	int c = static_cast<int>(db);		// static_cast 형변환 (10)

	// 10, 10, 10 이 출력된다.
	// 일반적인 변수는 부담없이 처리된다.
	printf("%d\n%d\n%d\n", a, b, c);

	return 0;
}

// ------------------------------

int main()
{
	double* db = new double(10.1234f);
	int* a = db;						// 묵시적 형변환	     (NO) > 데이터 타입이 맞지 않기 때문.
	int* b = (int*)db;					// 명시적 형변환	     (OK)
	int* c = static_cast<int>(db);		// static_cast 형변환 (NO) > 데이터 타입이 맞지 않기 때문.

	// 10, 10, 10 이 출력된다.
	// 일반적인 변수는 부담없이 처리된다.
	printf("%d\n%d\n%d\n", a, b, c);

	return 0;
}

// ------------------------------

int main()
{
	unsigned int num = 20;
	int SignedNum = (int)num;

	return 0;
}

// static_cast<변환할 타입>(변환할 대상); : 값과 포인터에 대해 캐스팅을 한다.
// 정적 캐스팅 : 컴파일 타임에 캐스팅이 완료된다. -> 런타임 결정되는 타입은 안될 수 있다.

class Foo
{
	// ...
};

class A : public Foo
{
	// ...
};

class B : public Foo
{
	// ...
	void FuncOfB();
};

Foo* f = new A();
A* a = static_cast<A*>(f);		// 가능! (F 와 a 는 상속 관계이므로)
B* b = static_cast<B*>(f);		// 가능! (F 와 a 는 상속 관계이므로)

b->FunOfB();					// A 에 없는 함수를 호출할 경우, 크래쉬가 날 수 있다.

// reinterpret_cast<변환할 타입>(변환할 대상); : 포인터에 대한 캐스팅만 한다.
// 값형식에 대한 캐스팅을 제외하면 관계없는 타입끼리도 캐스팅이 되기에 위험성이 높다.
*/
