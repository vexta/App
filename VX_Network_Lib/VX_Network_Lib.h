// VX_Network_Lib.h

#pragma once

//using namespace System;

namespace VX_Network_Lib {
	class __declspec(dllexport) KniznicaDLL {
	public:
		void Send();
		void Send(int cislo);
		int Get();
	};
}
