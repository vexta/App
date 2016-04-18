//posielat maximalnu dlzku zakazdym
//poslat predobjet
//binary data + serializovany objekt

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

#define DEBUG
#define ERROR_OUTPUT (stdout)
#define WRITE_ERROR fprintf(ERROR_OUTPUT, "%s\n\n", ex->ToString());

//OK
void NetLib::ZistiVelkostSerializovanehoObjektu() {
	try {
		System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream();
		IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
		formater->Serialize(ms, os);

		SizeOfSerializedObjekt = ms->Length;
	}
	catch (Exception^ ex) {
	#ifdef DEBUG
		WRITE_ERROR
	#endif
	}
}

//OK
void NetLib::Init() {
	IPAddress^ address;
	IPEndPoint^ endPoint;

	try {
		address = IPAddress::Parse(ConfigurationSettings::AppSettings["IPAddr"]);
		endPoint = gcnew IPEndPoint(address, Int64::Parse(ConfigurationSettings::AppSettings["Port"]));
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
	
	char c = 'y';
	lsocket = gcnew Socket(endPoint->AddressFamily, SocketType::Stream, ProtocolType::Tcp);

	while (c == 'y')
	{
		try {
			lsocket->Connect(endPoint);
		}
		catch (Exception^ ex) {
#ifdef DEBUG
			WRITE_ERROR
#endif
		}

		if (lsocket->Connected) {
			printf("Conncetion succesful.\n");
			_isConnected = 1;
			break;
		}
		else {
			printf("Nepodarilo sa pripoji. Skúsi znovu? [y]/[n]\n");
			c = getchar();
			getchar();
			//throw gcnew Exception("Unable to connect to remote host. (c)DoDou");
		}
	}
}

//OK
void NetLib::Listen() {
	IPAddress^ address;
	address = IPAddress::Any;

	IPEndPoint^ endpoint;
	try {
		endpoint = gcnew IPEndPoint(address, Int64::Parse(ConfigurationSettings::AppSettings["Port"]));
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}


		ssocket = gcnew Socket(endpoint->AddressFamily, SocketType::Stream, ProtocolType::Tcp);
		ssocket->Bind(endpoint);
		ssocket->Listen(205);
		csocket = ssocket->Accept();
		csocket->ReceiveBufferSize = SizeOfSerializedObjekt;

		while (1 && !ExitThread) {
			try {
				if (csocket->Connected && csocket->Available >= SizeOfSerializedObjekt) { 

					size = csocket->Receive(recieve_buffer, 0, SizeOfSerializedObjekt, SocketFlags::None);

					IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
					System::IO::MemoryStream^ mms = gcnew System::IO::MemoryStream();
					mms->Write(recieve_buffer, 0, size);
					mms->Position = 0;

					or = (AppData^)formater->Deserialize(mms);

#ifdef DEBUG
					fprintf(ERROR_OUTPUT, "Velkost prijatych dat %d \n", or->VertexCount);
#endif
					
					_newData = 1;		//nastav ze ma nove data
#ifdef DEBUG
					fprintf(ERROR_OUTPUT, "Prijate >> %d\n\n", size);
#endif
				}
			}
				catch (Exception^ ex) {
#ifdef DEBUG
					WRITE_ERROR
#endif
		
		}
	}
}

//ok
NetLib::NetLib() {
	try {
		ExitThread = false;
		//ip = nullptr;
		ssocket = nullptr;
		csocket = nullptr;

		os = gcnew AppData();
		or = gcnew AppData();

		ZistiVelkostSerializovanehoObjektu();

		thread = gcnew Thread(gcnew ThreadStart(this, &NetLib::Listen));
		thread->Start();

		Init();
	}
	catch (Exception^ ex) {
		WRITE_ERROR
	}
}

//ok
NetLib::~NetLib() {
	ExitThread = true;
	thread->Join();
	lsocket->Close();
	ssocket->Close();
	csocket->Close();
}

//Funkcia SEND() sluzi IBA na DEBUG-ovacie ucely
void NetLib::Send() {
	try
	{
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
#ifdef DEBUG
		printf("Odoslane >> %d\n", ms->Length);
#endif

		//lsocket->Send();
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}

//Funkcia SEND(int cislo) sluzi IBA na DEBUG-ovacie ucely
void NetLib::Send(int cislo) { //uint8_t cislo[]
	try {
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
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}

void NetLib::Send(INuiFusionMesh *meshData) {
	try {
		if (_isConnected) {
			System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream();
			IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
			formater->Serialize(ms, os);

			const Vector3* temp;
			meshData->GetVertices(&temp);

			os->VertexCount = meshData->VertexCount();
			for (int i = 0, j = 0; i < os->VertexCount; i++, j += 3) {
				os->vertices[j] = temp[i].x;
				os->vertices[j + 1] = temp[i].y;
				os->vertices[j + 2] = temp[i].z;
			}
			array<unsigned char>^ buff = gcnew array<unsigned char>(ms->Length);
			ms->Position = 0;
			ms->Read(buff, 0, ms->Length);

			lsocket->Send(buff);
#ifdef DEBUG
			printf("Odoslane >> %d\n", ms->Length);
#endif
		}
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}

//ok
int NetLib::newDataAvailable() {
	try {
		return _newData;
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
}

//Funkcia GET() sluzi IBA na DEBUG-ovacie ucely
int NetLib::Get() {
	_newData = 0;	//zmaz ze nie su aktualne data
	return size;
}

//ok
Vector3* NetLib::GetVrcholy(int *velkostpola) {
	static Vector3 temp[VELKOSTPOLA];
	*velkostpola = 0;
	try {
		if (_isConnected) {
			*velkostpola = or ->VertexCount;
			int i = 0, j = 0;
			for (i = 0, j = 0; ((i < or ->VertexCount) && (i < VELKOSTPOLA)); i++, j += 3) {
				temp[i].x = or ->vertices[j];
				temp[i].y = or ->vertices[j + 1];
				temp[i].z = or ->vertices[j + 2];
			}
			*velkostpola = i;
			//printf("Ahoj %d \n", i);
			_newData = 0;
		}
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		fprintf(ERROR_OUTPUT, "%s\n", ex->ToString());
#endif
	}
	return temp;
}



AppData::AppData() {

}

//Funkcia Objekt(int i) sluzi IBA na DEBUG-ovacie ucely
AppData::AppData(int i) {
	HeadTilt = i;

	HandX = i;
	HandY = i;
	HandZ = i;

	HeadX = i;
	HeadY = i;
	HeadZ = i;

	meshData = nullptr;
}

AppData::~AppData() {

}