#include "stdafx.h"
#include "NetLib.h"

using namespace System::Runtime::Serialization;

/*auto main() -> int {
}*/

void NetLib::Init() {
	IPAddress^ address;

	//	os =  gcnew Objekt();/
	//	or =  gcnew Objekt(0);

	address = IPAddress::Parse("127.0.0.1");

	IPEndPoint^ endPoint = gcnew IPEndPoint(address, 8888);
	lsocket = gcnew Socket(endPoint->AddressFamily, SocketType::Stream, ProtocolType::Tcp);
	lsocket->Connect(endPoint);
	if (lsocket->Connected) {
		printf("Conncetion succesful.\n");
	}
	else {
		printf("Conncetion UNSUCCESFULL.\n");
	}
}

void NetLib::Listen() {
	IPAddress^ address;
	address = IPAddress::Any;

	//address = IPAddress::Parse("127.0.0.1"); //System::Configuration::ConfigurationSettings::AppSettings["IPAddr"]); //address = IPAddress::Parse("10.62.12.251");

	IPEndPoint^ endpoint = gcnew IPEndPoint(address, 8888);
	ssocket = gcnew Socket(endpoint->AddressFamily, SocketType::Stream, ProtocolType::Tcp);
	ssocket->Bind(endpoint);
	ssocket->Listen(5);
	csocket = ssocket->Accept();

	while (1 && !ExitThread) {
		if (csocket->Connected) {
			size = csocket->Receive(recieve_buffer);
			//printf("Velkost prijatych dat[b]: %d\n", size);
		}
	}
}

NetLib::NetLib() {
	ExitThread = false;
	//ip = nullptr;
	ssocket = nullptr;
	csocket = nullptr;

	thread = gcnew Thread(gcnew ThreadStart(this, &NetLib::Listen));
	thread->Start();

	Init();

	//while (1 == 1) {
	//printf("Main\n");
	//	Send();
	//	thread->Sleep(1000);
	//}
}

NetLib::~NetLib() {
	ExitThread = true;
	thread->Join();
}

void NetLib::Send() {
	//os = gcnew Object();
	//IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
	//formater->Serialize(System::IO::Stream::,os);

	//lsocket->Send(send_buffer);
	//lsocket->Send();
}

void NetLib::Send(int cislo) { //uint8_t cislo[]
	int i = 0;
	for (i = 0; i < counter - 1; i++)
		send_buffer[i] = 0;
	send_buffer[i] = 0;
	send_buffer[50000] = counter;

	counter++;
	//uint8 *lReceivedBytes = new uint8[count];
	//lReceivedBytes = arrByte->Data;

	//buffer->

	lsocket->Send(send_buffer);
}

//int NetLib::Get() {
//int var;
//var = size;
//size = 0;

//	return size;
//}

unsigned char NetLib::Get() {
	//int var;
	//var = size;
	//size = 0;

	return recieve_buffer[50000];
}



Objekt::Objekt() {

	HeadTilt = 1;

	HandX = 2;
	HandY = 3;
	HandZ = 4;

	HeadX = 5;
	HeadY = 6;
	HeadZ = 7;

	for (int i = 0; i < 640 * 480; i++)
		IR[i] = 127;

	for (int i = 0; i < 640 * 480 * 3; i++)
		RGB[i] = 255;
}

Objekt::Objekt(int i) {

	HeadTilt = i;

	HandX = i;
	HandY = i;
	HandZ = i;

	HeadX = i;
	HeadY = i;
	HeadZ = i;

	for (int i = 0; i < 640 * 480; i++)
		IR[i] = i;

	for (int i = 0; i < 640 * 480 * 3; i++)
		RGB[i] = i;
}

Objekt::~Objekt() {

}