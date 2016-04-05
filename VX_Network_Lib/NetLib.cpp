#include "stdafx.h"
#include "NetLib.h"
#include <sstream>
#include <string.h>

//#include "stdlib.h"
//#include "stdio.h"

using namespace System::Runtime::Serialization;
using namespace System::Configuration;
//using namespace std;

/*auto main() -> int {
}*/

void NetLib::ZistiVelkostSerializovanehoObjektu() {

	System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream();
	IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
	formater->Serialize(ms, os);

	SizeOfSerializedObjekt = ms->Length;

}

void NetLib::Init() {
	IPAddress^ address;

	//	os =  gcnew Objekt();/
	//	or =  gcnew Objekt(0);

	//address = IPAddress::Parse("127.0.0.1");
	//address = IPAddress::Parse("127.0.0.1");
	address = IPAddress::Parse(ConfigurationSettings::AppSettings["IPAddr"]); //address = IPAddress::Parse("10.62.12.251");
	IPEndPoint^ endPoint = gcnew IPEndPoint(address, Int64::Parse(ConfigurationSettings::AppSettings["Port"]));

	//IPEndPoint^ endPoint = gcnew IPEndPoint(address, 8888);
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

	//address = IPAddress::Parse("127.0.0.1");
	//address = IPAddress::Parse(ConfigurationSettings::AppSettings["IPAddr"]); //address = IPAddress::Parse("10.62.12.251");
	IPEndPoint^ endpoint = gcnew IPEndPoint(address, Int64::Parse(ConfigurationSettings::AppSettings["Port"]));
	ssocket = gcnew Socket(endpoint->AddressFamily, SocketType::Stream, ProtocolType::Tcp);
	ssocket->Bind(endpoint);
	ssocket->Listen(205);
	csocket = ssocket->Accept();
	csocket->ReceiveBufferSize = SizeOfSerializedObjekt;

	while (1 && !ExitThread) {
		if (csocket->Connected && csocket->Available >= SizeOfSerializedObjekt) {

			size = csocket->Receive(recieve_buffer, 0, SizeOfSerializedObjekt, SocketFlags::None);
			printf("Buffer >> %d\n", size);//or->HeadTilt);
			//csocket->

				IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
				System::IO::MemoryStream^ mms = gcnew System::IO::MemoryStream();
				mms->Write(recieve_buffer, 0, size);
				mms->Position = 0;

				//

				or = (Objekt^)formater->Deserialize(mms);
			

			//printf("Velkost prijatych dat[b]: %d\n", size);
			printf("Prijate >> %d\n\n", size);//or->HeadTilt);
		}
	}
}

NetLib::NetLib() {
	ExitThread = false;
	//ip = nullptr;
	ssocket = nullptr;
	csocket = nullptr;

	os = gcnew Objekt();
	or = gcnew Objekt(0);

	ZistiVelkostSerializovanehoObjektu();

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

	//!!!!Unhandled Exception: System.Net.Sockets.SocketException: An existing connection was forcibly closed by the rem


	//os = gcnew Objekt();
	//SERIAL sss;
	//IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
	//formater->Serialize(System::IO::Stream::,os);


	//std::basic_ofstream;
	//std::basic_ostringstream<unsigned char> ss;

	//ss.write((unsigned char *)&sss, sizeof(sss));

	/*std::ostringstream os;
	os << "dec: " << 15 << " hex: " << std::hex << 15 << std::endl;
	std::cout << os.str() << std::endl;*/

	//printf("%d \n",sizeof(os->IR));

	//lsocket->Send(send_buffer);
	//lsocket->Send();

	System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream();
	IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
	formater->Serialize(ms, os);




	array<unsigned char>^ buff = gcnew array<unsigned char>(ms->Length);
	ms->Position = 0;
	ms->Read(buff, 0, ms->Length);
	
	/*System::IO::MemoryStream^ mms = gcnew System::IO::MemoryStream();
	mms->Write(buff, 0, buff->Length);
	ms->Position = 0;
	mms->Position = 0;
	or = (Objekt^)formater->Deserialize(mms);*/
	
	
	
	lsocket->Send(buff);
	printf("Odoslane >> %d\n", ms->Length);//or->HeadTilt);


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

int NetLib::Get() {
	//int var;
	//var = size;
	//size = 0;

	return size;
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