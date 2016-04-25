// VX_Network_Lib.h

#pragma once

//using namespace System;

namespace VX_Network_Lib {
	class __declspec(dllexport) KniznicaDLL {
	public:
		int  isOculus();
		void Send();
		void Send(int cislo);
		void Send(INuiFusionMesh *meshData);
		Vector3* GetVrcholy(int *vertexcount, int* normalcount, const Vector3** normals);

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

		int Get();
		
		int newDataAvailable();
	private:
		uint32_t _chyba = 0;
	};
}
