#pragma once

#include <string>
#include <iostream>
#using   "system.dll"

#define BUF_LEN 1000000000

using namespace System;
using namespace System::Text;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Threading;

typedef double ind;

[Serializable]
public ref class Objekt {
public:
	Objekt();
	Objekt(int i);
	~Objekt();

	ind HandX;
	ind HandY;
	ind HandZ;

	ind HeadX;
	ind HeadY;
	ind HeadZ;

	float HeadTilt;

	array<unsigned char>^ IR = gcnew array<unsigned char>(640 * 480);
	array<unsigned char>^ RGB = gcnew array<unsigned char>(640 * 480 * 3);
	//Celkova velkost objektu 1 228 852
};


public ref class NetLib {
public:
	NetLib();
	~NetLib();
	void Send();
	void Send(int cislo); //uint8_t cislo[]
						  //int Get();
	int Get();
private:
	int SizeOfSerializedObjekt;
	
	array<unsigned char>^ send_buffer = gcnew array<unsigned char>(BUF_LEN);
	array<unsigned char>^ recieve_buffer = gcnew array<unsigned char>(BUF_LEN);

	Thread^ thread;
	Boolean ExitThread = false;
	Socket^ ssocket;
	Socket^ csocket;

	Socket^ lsocket;

	int counter;

	Boolean IsServer;
	IPAddress^ ip;
	int size;

	void Listen();
	void Init();
	void ZistiVelkostSerializovanehoObjektu();

		Objekt^ os;
		Objekt^ or;
};