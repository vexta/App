// This is the main DLL file.

#include "NuiKinectFusionApi.h"
#include "stdafx.h"
#include "NetLib.h"
#include "VX_Network_Lib.h"

#define DEBUG
#define ERROR_OUTPUT (stdout)
#define WRITE_ERROR fprintf(ERROR_OUTPUT, "%s\n\n", ex->ToString());


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
	try {
		Singleton^ a = Singleton::Instance;  //!!!!TypeInitialization was unhandeled The type initializer for 'Singleton' threw an exception.
		a->abc->Send();
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}

void VX_Network_Lib::KniznicaDLL::Send(INuiFusionMesh *meshData) {
	try {
		Singleton^ a = Singleton::Instance;  //!!!!TypeInitialization was unhandeled The type initializer for 'Singleton' threw an exception.
		a->abc->Send(meshData);
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}

void VX_Network_Lib::KniznicaDLL::Send(int cislo) {
	try {
		Singleton^ a = Singleton::Instance;
		a->abc->Send(cislo);
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}

int VX_Network_Lib::KniznicaDLL::Get() {
	try {
		Singleton^ a = Singleton::Instance;
		return a->abc->Get();
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}

Vector3* VX_Network_Lib::KniznicaDLL::GetVrcholy(int *pocet) {
	try {
		Singleton^ a = Singleton::Instance;
		return a->abc->GetVrcholy(pocet);
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}

int VX_Network_Lib::KniznicaDLL::newDataAvailable() {
	Singleton^ a = Singleton::Instance;
	return a->abc->newDataAvailable();
}


void VX_Network_Lib::KniznicaDLL::Send(int Kocka,
	float lhandx, float lhandy, float lhandz,
	float rhandx, float rhandy, float rhandz,
	float k0x, float k0y, float k0z,
	float k1x, float k1y, float k1z,
	float k2x, float k2y, float k2z
	) {
	try {
		Singleton^ a = Singleton::Instance;
		return a->abc->Send(Kocka,
			lhandx, lhandy, lhandz,
			rhandx, rhandy, rhandz,
			k0x, k0y, k0z,
			k1x, k1y, k1z,
			k2x, k2y, k2z);
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}

void VX_Network_Lib::KniznicaDLL::GetKocky(
	int *Kocka,
	float *lhandx, float *lhandy, float *lhandz,
	float *rhandx, float *rhandy, float *rhandz,
	float *k0x, float *k0y, float *k0z,
	float *k1x, float *k1y, float *k1z,
	float *k2x, float *k2y, float *k2z) {
	try {
		Singleton^ a = Singleton::Instance;
		return a->abc->GetKocky(
			Kocka,
			lhandx, lhandy, lhandz,
			rhandx, rhandy, rhandz,
			k0x, k0y, k0z,
			k1x, k1y, k1z,
			k2x, k2y, k2z);
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}




//ukazka pouzitia kodu
//#include "VX_Network_Lib.h"
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
