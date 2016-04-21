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
public ref class AppData_Mesh {
public:
	AppData_Mesh();
	AppData_Mesh(int i);
	~AppData_Mesh();

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

[Serializable]
public ref class AppData_Logika {
public:
	int random;

	float lhx;
	float lhy;
	float lhz;

	float rhx;
	float rhy;
	float rhz;

	float k0x;
	float k0y;
	float k0z;

	float k1x;
	float k1y;
	float k1z;

	float k2x;
	float k2y;
	float k2z;
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


	void Send(int Kocka,
		float lhandx, float lhandy, float lhandz,
		float rhandx, float rhandy, float rhandz,
		float k0x, float k0y, float k0z,
		float k1x, float k1y, float k1z,
		float k2x, float k2y, float k2z
		);
	void GetKocky(
		int *Kocka,
		float *lhandx, float *lhandy, float *lhandz,
		float *rhandx, float *rhandy, float *rhandz,
		float *k0x, float *k0y, float *k0z,
		float *k1x, float *k1y, float *k1z,
		float *k2x, float *k2y, float *k2z);

	int newDataAvailable();
private:
	int _newData = 0;
	int _isConnected = 0;
	int _isKinect = 0;
	int _isOculus = 0;

	int SizeOfSerializedObjekt_Mesh;
	int SizeOfSerializedObjekt_Logika;
	
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

	void Listen_Kinect();
	void Listen_Oculus();

	void Init();
	void ZistiVelkostSerializovanehoObjektu();

		AppData_Mesh^ os;
		AppData_Mesh^ or;

		AppData_Logika^ ol_s;
		AppData_Logika^ ol_r;
};