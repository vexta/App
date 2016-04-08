// This is the main DLL file.

#include "NuiKinectFusionApi.h"
#include "stdafx.h"
#include "NetLib.h"
#include "VX_Network_Lib.h"

public ref class Singleton {
private:
	Singleton() {
	}
	Singleton(const Singleton%) { throw gcnew System::InvalidOperationException("singleton cannot be copy-constructed"); }
	static Singleton m_instance;

public:
	static property Singleton^ Instance { Singleton^ get() { return %m_instance; } }
	NetLib^ abc = gcnew NetLib();
};

void VX_Network_Lib::KniznicaDLL::Send() {
	//try {
	Singleton^ a = Singleton::Instance;  //!!!!TypeInitialization was unhandeled The type initializer for 'Singleton' threw an exception.
	a->abc->Send();
	//}
	//catch (Exception^ e) {
		//throw e;
	//}
}

void VX_Network_Lib::KniznicaDLL::Send(INuiFusionMesh *meshData) {
	Singleton^ a = Singleton::Instance;  //!!!!TypeInitialization was unhandeled The type initializer for 'Singleton' threw an exception.
	a->abc->Send(meshData);
}

void VX_Network_Lib::KniznicaDLL::Send(int cislo) {
	Singleton^ a = Singleton::Instance;
	a->abc->Send(cislo);
}

int VX_Network_Lib::KniznicaDLL::Get() {
	Singleton^ a = Singleton::Instance;
	return a->abc->Get();
}

int VX_Network_Lib::KniznicaDLL::newDataAvailable() {
	Singleton^ a = Singleton::Instance;
	return a->abc->newDataAvailable();
}


//ukazka pouzitia kodu
//#include "../VX_Network_Lib/VX_Network_Lib.h"
int main() {



	int chyba = 0;
	VX_Network_Lib::KniznicaDLL a;
	//a.Send();
	//int ab = a.Get();
	//Thread::Sleep(1500);
	try {
		while (true) {
			//a.Send(1);
			a.Send();
			printf("Main %d\n", a.Get());
			//Thread::Sleep(1000);
			//printf("\n\n\n");
		}
	}
	catch (Exception^ e) {
		chyba++;
		printf("\n\n\n%d\n\n\n", chyba);
	}
	return 0;
}
