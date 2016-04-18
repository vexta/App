#pragma once

#include "NuiKinectFusionApi.h"
#include <string>
#include <iostream>
#using   "system.dll"

#define BUF_LEN 10000000
#define VELKOSTPOLA 300000

using namespace System;
using namespace System::Text;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Threading;
using namespace System::Collections::Generic;

typedef double ind;

[Serializable]
public ref class AppData {
public:
	AppData();
	AppData(int i);
	~AppData();

	ind HandX;
	ind HandY;
	ind HandZ;

	ind HeadX;
	ind HeadY;
	ind HeadZ;

	float HeadTilt;
	INuiFusionMesh *meshData;

	int VertexCount;

	array<float>^ vertices = gcnew array<float>(3 * VELKOSTPOLA);

	//array<unsigned char>^ IR = gcnew array<unsigned char>(640 * 480);
	//array<unsigned char>^ RGB = gcnew array<unsigned char>(640 * 480 * 3);
	//Celkova velkost objektu 1 228 852
};

public ref class NetLib {
public:
	NetLib();
	~NetLib();
	void Send();
	void Send(int cislo); //uint8_t cislo[]
						  //int Get();
	void Send(INuiFusionMesh *meshData);
	int Get();
	Vector3* GetVrcholy(int *velkostpola);
	int newDataAvailable();
private:
	int _newData = 0;
	int _isConnected = 0;

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

		AppData^ os;
		AppData^ or;
};