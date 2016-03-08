#pragma once

#include <memory>

#include <OVR_CAPI.h>

typedef int OVRHMDHandle;

namespace vx_ovr_namespace_ {

	class OVRWrapper
	{
	public:
		~OVRWrapper();

		void initialize();
		bool isInitialized();

		static std::shared_ptr<OVRWrapper> getInstance();
		static std::shared_ptr<OVRHMDHandle> getOVRDeviceHandle();
	private:
		OVRWrapper();

		bool initialized_;
	};

};

