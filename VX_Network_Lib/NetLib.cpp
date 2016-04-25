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

//#define DEBUG
#define ERROR_OUTPUT (stdout)
#define WRITE_ERROR fprintf(ERROR_OUTPUT, "%s\n\n", ex->ToString());

//OK
void NetLib::ZistiVelkostSerializovanehoObjektu() {
	try {
		System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream();
		IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
		formater->Serialize(ms, os);

		SizeOfSerializedObjekt_Mesh = ms->Length;

		
		ms = gcnew System::IO::MemoryStream();
		formater = gcnew Formatters::Binary::BinaryFormatter();
		formater->Serialize(ms, ol_s);

		SizeOfSerializedObjekt_Logika = ms->Length;
		
	}
	catch (Exception^ ex) {
	#ifdef DEBUG
		WRITE_ERROR
	#endif
	}
}

//ok
NetLib::NetLib() {
	try {
		printf("Pokus o pripajanie sa.\n");
		ExitThread = false;
		//ip = nullptr;
		ssocket = nullptr;
		csocket = nullptr;

		os = gcnew AppData_Mesh();
		or = gcnew AppData_Mesh();

		ol_s = gcnew AppData_Logika();
		ol_r = gcnew AppData_Logika();

		ZistiVelkostSerializovanehoObjektu();

		_isOculus = INT16::Parse(ConfigurationSettings::AppSettings["isOculus"]);
		
		if (_isOculus)
			thread = gcnew Thread(gcnew ThreadStart(this, &NetLib::Listen_Oculus));
		else
			thread = gcnew Thread(gcnew ThreadStart(this, &NetLib::Listen_Kinect));

		thread->Start();

		Init();
	}
	catch (Exception^ ex) {
		WRITE_ERROR
	}
}

int NetLib::isOculus() {
	return _isOculus;
}

//OK
void NetLib::Init() {
	IPAddress^ address;
	IPEndPoint^ endPoint;

	try {
		address = IPAddress::Parse(ConfigurationSettings::AppSettings["IPAddr"]);
		endPoint = gcnew IPEndPoint(address, Int64::Parse(ConfigurationSettings::AppSettings["Port"]));
		_isOculus = INT16::Parse(ConfigurationSettings::AppSettings["isOculus"]);
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		WRITE_ERROR
#endif
	}
	
	char c = 'y';
	lsocket = gcnew Socket(endPoint->AddressFamily, SocketType::Stream, ProtocolType::Tcp);

	printf("Chcete pripojit vzdialeneho pouzivatela? [y]/[n]\n");
	c = getchar();
	getchar();

	while (c == 'y') {
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
void NetLib::Listen_Oculus() {
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
		csocket->ReceiveBufferSize = SizeOfSerializedObjekt_Mesh;

		while (1 && !ExitThread) {
			try {
				if (csocket->Connected && csocket->Available >= SizeOfSerializedObjekt_Mesh) { 

					size = csocket->Receive(recieve_buffer, 0, SizeOfSerializedObjekt_Mesh, SocketFlags::None);

					IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
					System::IO::MemoryStream^ mms = gcnew System::IO::MemoryStream();
					mms->Write(recieve_buffer, 0, size);
					mms->Position = 0;

					or = (AppData_Mesh^)formater->Deserialize(mms);

#ifdef DEBUG
					fprintf(ERROR_OUTPUT, "Velkost prijatych dat Oculus %d \n", or->VertexCount);
#endif
					
					_newData = 1;		//nastav ze ma nove data
#ifdef DEBUG
					fprintf(ERROR_OUTPUT, "Prijate >> Oculus %d\n\n", size);
#endif
				}
			}
				catch (Exception^ ex) {
#ifdef DEBUG
					WRITE_ERROR
					fprintf(ERROR_OUTPUT, "Oculus\n");
#endif
		
		}
	}
}

void NetLib::Listen_Kinect() { //pre Kinect
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
	csocket->ReceiveBufferSize = SizeOfSerializedObjekt_Logika;

	while (1 && !ExitThread) {
		try {
			if (csocket->Connected && csocket->Available >= SizeOfSerializedObjekt_Logika) {

				size = csocket->Receive(recieve_buffer, 0, SizeOfSerializedObjekt_Logika, SocketFlags::None);

				IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
				System::IO::MemoryStream^ mms = gcnew System::IO::MemoryStream();
				mms->Write(recieve_buffer, 0, size);
				mms->Position = 0;

				ol_r = (AppData_Logika^)formater->Deserialize(mms);

#ifdef DEBUG
				fprintf(ERROR_OUTPUT, "Velkost prijatych dat %d \n", size);
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
			fprintf(ERROR_OUTPUT, "Kinect\n");
#endif

		}
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

			const Vector3* tempverticies;
			const Vector3* tempnormals;
			
			meshData->GetVertices(&tempverticies);
			os->VertexCount = meshData->VertexCount();
			for (int i = 0, j = 0; i < os->VertexCount; i++, j += 3) {
				os->vertices[j] =	  tempverticies[i].x;
				os->vertices[j + 1] = tempverticies[i].y;
				os->vertices[j + 2] = tempverticies[i].z;
			}
			
			meshData->GetNormals(&tempnormals);
			os->NormalCount = meshData->NormalCount();
			for (int i = 0, j = 0; i < os->NormalCount; i++, j += 3) {
				os->normals[j] =	 tempnormals[i].x;
				os->normals[j + 1] = tempnormals[i].y;
				os->normals[j + 2] = tempnormals[i].z;
			}

			//formater->Serialize(ms, os);
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
Vector3* NetLib::GetVrcholy(int *vertexcount, int* normalcount, const Vector3** normals) {
	static Vector3 tempVertices[VELKOSTPOLA];
	static Vector3 tempNormals[VELKOSTPOLA];
	*vertexcount = 0;
	try {
		if (_isConnected) {
			*vertexcount = or ->VertexCount;
			int i = 0, j = 0;
			for (i = 0, j = 0; ((i < or ->VertexCount) && (i < VELKOSTPOLA)); i++, j += 3) {
				tempVertices[i].x = or ->vertices[j];
				tempVertices[i].y = or ->vertices[j + 1];
				tempVertices[i].z = or ->vertices[j + 2];
			}
			*vertexcount = i;

			*normalcount = or ->NormalCount;
			for (i = 0, j = 0; ((i < or ->NormalCount) && (i < VELKOSTPOLA)); i++, j += 3) {
				tempNormals[i].x = or ->normals[j];
				tempNormals[i].y = or ->normals[j + 1];
				tempNormals[i].z = or ->normals[j + 2];
			}
			*normalcount = i;

			*normals = tempNormals;

			//printf("Ahoj %d \n", i);
			_newData = 0;
		}
	}
	catch (Exception^ ex) {
#ifdef DEBUG
		fprintf(ERROR_OUTPUT, "%s\n", ex->ToString());
#endif
	}
	return tempVertices;
}

void NetLib::Send(int Kocka,
	float lhandx, float lhandy, float lhandz,
	float rhandx, float rhandy, float rhandz,
	float k0x, float k0y, float k0z,
	float k1x, float k1y, float k1z,
	float k2x, float k2y, float k2z
	) {
	try {
		if (_isConnected) {
			System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream();
			IFormatter^ formater = gcnew Formatters::Binary::BinaryFormatter();
			
			ol_s->random = Kocka;

			ol_s->lhx = lhandx;
			ol_s->lhy = lhandy;
			ol_s->lhz = lhandz;

			ol_s->rhx = rhandx;
			ol_s->rhy = rhandy;
			ol_s->rhz = rhandz;

			ol_s->k0x = k0x;
			ol_s->k0y = k0y;
			ol_s->k0z = k0z;

			ol_s->k1x = k1x;
			ol_s->k1y = k1y;
			ol_s->k1z = k1z;

			ol_s->k2x = k2x;
			ol_s->k2y = k2y;
			ol_s->k2z = k2z;

			formater->Serialize(ms, ol_s);
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
void NetLib::GetKocky(
	int *Kocka,
	float *lhandx, float *lhandy, float *lhandz,
	float *rhandx, float *rhandy, float *rhandz,
	float *k0x, float *k0y, float *k0z,
	float *k1x, float *k1y, float *k1z,
	float *k2x, float *k2y, float *k2z) {

	*Kocka = ol_r->random;

	*lhandx = ol_r->lhx;
	*lhandy = ol_r->lhy;
	*lhandz = ol_r->lhz;

	*rhandx = ol_r->rhx;
	*rhandy = ol_r->rhy;
	*rhandz = ol_r->rhz;

	*k0x = ol_r->k0x;
	*k0y = ol_r->k0y;
	*k0z = ol_r->k0z;

	*k1x = ol_r->k1x;
	*k1y = ol_r->k1y;
	*k1z = ol_r->k1z;

	*k2x = ol_r->k2x;
	*k2y = ol_r->k2y;
	*k2z = ol_r->k2z;
}


AppData_Mesh::AppData_Mesh() {

}

//Funkcia Objekt(int i) sluzi IBA na DEBUG-ovacie ucely
AppData_Mesh::AppData_Mesh(int i) {
	HeadTilt = i;

	HandX = i;
	HandY = i;
	HandZ = i;

	HeadX = i;
	HeadY = i;
	HeadZ = i;

	meshData = nullptr;
}

AppData_Mesh::~AppData_Mesh() {

}
