#pragma once

#include <memory>

#include <OVR_CAPI.h>

#include "OVRHMDHandle.h"
#include "OVRHMDHandleNoDevice.h"
#include "OVRHMDHandleWithDevice.h"

namespace vx_ovr_namespace_ {

	class OVRWrapper
	{
	public:
		~OVRWrapper();

		void initialize();
		bool isInitialized();
		void shutDown();

		std::shared_ptr<OVRHMDHandle> getOVRHMDDeviceHandle();

		static std::shared_ptr<OVRWrapper> getInstance();
	private:
		OVRWrapper();

		bool initialized_;
		bool useRealHMD_;

		int operator +(int) {

		}
	};

};

