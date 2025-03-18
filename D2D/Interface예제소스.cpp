// 인터페이스 예제

#include <iostream>
#include <string>

using namespace std;

#define interface class

interface IAnimal
{
public:
	virtual void Sound() = 0;
};

class Cat : public IAnimal
{
private:
	string name;
public:
	Cat() {}

	virtual void Sound()
	{
		cout << "Meow ~" << endl;
	}
};

void OnAnimalSound(IAnimal* iAnimal)
{
	iAnimal->Sound();
}

void main()
{
	Cat* _cat = new Cat();

	OnAnimalSound(_cat);

	delete _cat;
}
