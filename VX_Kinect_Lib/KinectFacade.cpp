#include "stdafx.h"
#include "Ole2.h"
#include "Kinect.h"
#include "Windows.h"
#include "KinectParameters.h"
#include "KinectTypes.h"
#include "KinectData.h"
#include "KinectFacade.h"
#include "KinectHelper.h"
#include "AssertHelper.h"

#pragma region constants
const int KinectFacade::width = 512;
const int KinectFacade::height = 424;
const int KinectFacade::colorWidth = 1920;
const int KinectFacade::colorHeight = 1080;
const USHORT KinectFacade::minimumReliableDistance = 500;
const float KinectFacade::infraredSourceValueMaximum = static_cast<float>(USHRT_MAX);
const float KinectFacade::infraredOutputValueMinimum = 0.01f;
const float KinectFacade::infraredOutputValueMaximum = 1.0f;
const float KinectFacade::infraredSceneValueAverage = 0.08f;
const float KinectFacade::infraredSceneStandardDeviations = 3.0f;
#pragma endregion

KinectFacade::KinectFacade()
{
	// initialize Kinect sensor
	if (FAILED(GetDefaultKinectSensor(&kinectSensor)) || !kinectSensor) throw std::domain_error("Unable to initialize default sensor!");
	kinectSensor->Open();

	// initialize coordinate mapper and multi-source frame reader
	kinectSensor->get_CoordinateMapper(&coordinateMapper);
	kinectSensor->OpenMultiSourceFrameReader(FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color | FrameSourceTypes::FrameSourceTypes_Infrared | FrameSourceTypes::FrameSourceTypes_Body, &multiSourceFrameReader);

	// default camera parameters
	cameraParameters.focalLengthX = NUI_KINECT_DEPTH_NORM_FOCAL_LENGTH_X;
	cameraParameters.focalLengthY = NUI_KINECT_DEPTH_NORM_FOCAL_LENGTH_Y;
	cameraParameters.principalPointX = NUI_KINECT_DEPTH_NORM_PRINCIPAL_POINT_X;
	cameraParameters.principalPointY = NUI_KINECT_DEPTH_NORM_PRINCIPAL_POINT_Y;
}

KinectFacade::~KinectFacade()
{
	kinectSensor->Close();
	kinectSensor->Release();
}

HRESULT KinectFacade::CreateFrame(NUI_FUSION_IMAGE_TYPE frameType, unsigned int imageWidth, unsigned int imageHeight, NUI_FUSION_IMAGE_FRAME **ppImageFrame)
{
	HRESULT hr = S_OK;

	if (nullptr != *ppImageFrame)
	{
		// if image size or type has changed, release the old one
		if ((*ppImageFrame)->width != imageWidth || (*ppImageFrame)->height != imageHeight || (*ppImageFrame)->imageType != frameType)
		{
			static_cast<void>(NuiFusionReleaseImageFrame(*ppImageFrame));
			*ppImageFrame = nullptr;
		}
	}

	// Create a new frame as needed.
	if (nullptr == *ppImageFrame)
	{
		hr = NuiFusionCreateImageFrame(frameType, imageWidth, imageHeight, &cameraParameters, ppImageFrame);
	}

	return hr;
}

HRESULT KinectFacade::CopyColor(NUI_FUSION_IMAGE_FRAME *colorImage, IColorFrame *colorFrame)
{
	HRESULT hr = S_OK;

	if (nullptr == colorImage)
	{
		return E_FAIL;
	}

	NUI_FUSION_BUFFER *destinationColorBuffer = colorImage->pFrameBuffer;

	if (nullptr == colorFrame || nullptr == destinationColorBuffer)
	{
		return E_NOINTERFACE;
	}

	// copy the color pixels so we can return the image frame
	hr = colorFrame->CopyConvertedFrameDataToArray(colorWidth * colorHeight * sizeof(RGBQUAD), destinationColorBuffer->pBits, ColorImageFormat_Bgra);

	if (FAILED(hr))
	{
		hr = E_FAIL;
	}

	return hr;
}

HRESULT KinectFacade::PostprocessInfrared(NUI_FUSION_IMAGE_FRAME *infraredImage, UINT16 *buffer)
{
	HRESULT hr = S_OK;

	if (buffer == nullptr || infraredImage == nullptr)
	{
		hr = E_FAIL;
	}

	const unsigned short* dataEnd = buffer + (infraredImage->width * infraredImage->height);
	BYTE* destination = infraredImage->pFrameBuffer->pBits;

	while (buffer < dataEnd)
	{
		// normalize the incoming infrared data (ushort) to a float ranging from 
		// [InfraredOutputValueMinimum, InfraredOutputValueMaximum] by
		// 1. dividing the incoming value by the source maximum value
		float intensityRatio = static_cast<float>(*buffer) / infraredSourceValueMaximum;

		// 2. dividing by the (average scene value * standard deviations)
		intensityRatio /= infraredSceneValueAverage * infraredSceneStandardDeviations;

		// 3. limiting the value to InfraredOutputValueMaximum
		intensityRatio = min(infraredOutputValueMaximum, intensityRatio);

		// 4. limiting the lower value InfraredOutputValueMinimym
		intensityRatio = max(infraredOutputValueMinimum, intensityRatio);

		// 5. converting the normalized value to a byte and using the result
		// as the RGB components required by the image
		byte intensity = static_cast<byte>(intensityRatio * 255.0f);
		*destination++ = intensity;
		*destination++ = intensity;
		*destination++ = intensity;
		*destination++ = 0xff;
		++buffer;
	}

	return hr;
}

HRESULT KinectFacade::ProcessInfrared(NUI_FUSION_IMAGE_FRAME *infraredImage, IInfraredFrame *infraredFrame)
{
	HRESULT hr = S_OK;

	if (nullptr == infraredFrame)
	{
		return E_FAIL;
	}

	NUI_FUSION_BUFFER *destinationInfrearedBuffer = infraredImage->pFrameBuffer;

	if (nullptr == infraredFrame || nullptr == destinationInfrearedBuffer)
	{
		return E_NOINTERFACE;
	}

	// copy the infrared pixels so we can return the image frame
	UINT bufferSize = 0;
	UINT16 *buffer;
	hr = infraredFrame->AccessUnderlyingBuffer(&bufferSize, &buffer);

	if (FAILED(hr))
	{
		return E_FAIL;
	}

	hr = PostprocessInfrared(infraredImage, buffer);

	return hr;
}

void KinectFacade::ProcessRawDepth(UINT16 *buffer, NUI_FUSION_IMAGE_FRAME **destination)
{
	HRESULT hr = CreateFrame(NUI_FUSION_IMAGE_TYPE_COLOR, width, height, destination);

	if (S_OK == hr)
	{
		const unsigned short* currentData = buffer;
		const unsigned short* dataEnd = buffer + (width*height);

		BYTE* destinationBuffer = (*destination)->pFrameBuffer->pBits;

		while (currentData < dataEnd)
		{
			USHORT depth = *currentData;
			BYTE intensity = static_cast<BYTE>((depth >= minimumReliableDistance) && (depth <= USHRT_MAX) ? (depth % 256) : 0);

			*destinationBuffer++ = intensity;
			*destinationBuffer++ = intensity;
			*destinationBuffer++ = intensity;
			*destinationBuffer++ = 0xFF;
			++currentData;
		}
	}
}

void KinectFacade::GetColorData(IMultiSourceFrame *frame, NUI_FUSION_IMAGE_FRAME **destination) {
	IColorFrame* colorFrame = nullptr;
	IColorFrameReference* frameReference = nullptr;
	frame->get_ColorFrameReference(&frameReference);

	ASSERT(frameReference);

	frameReference->AcquireFrame(&colorFrame);

	if (frameReference) frameReference->Release();

	if (!colorFrame) return;

	// fill data from frame
	if (S_OK == CreateFrame(NUI_FUSION_IMAGE_TYPE_COLOR, colorWidth, colorHeight, destination))
	{
		ASSERT(S_OK == CopyColor(*destination, colorFrame));
	}

	if (colorFrame) colorFrame->Release();
}

void KinectFacade::GetInfraredData(IMultiSourceFrame* frame, NUI_FUSION_IMAGE_FRAME **destination) {
	IInfraredFrame* infraredFrame;
	IInfraredFrameReference* frameReference = NULL;
	frame->get_InfraredFrameReference(&frameReference);

	ASSERT(frameReference);

	frameReference->AcquireFrame(&infraredFrame);

	if (frameReference) frameReference->Release();

	if (!infraredFrame) return;

	// fill data from frame
	if (S_OK == CreateFrame(NUI_FUSION_IMAGE_TYPE_COLOR, width, height, destination))
	{
		ASSERT(S_OK == ProcessInfrared(*destination, infraredFrame));
	}

	if (infraredFrame) infraredFrame->Release();
}

UINT16* KinectFacade::GetRawDepthData(IMultiSourceFrame *frame, UINT &capacity)
{
	IDepthFrame* depthFrame;
	IDepthFrameReference* frameReference = NULL;
	frame->get_DepthFrameReference(&frameReference);

	ASSERT(frameReference);

	frameReference->AcquireFrame(&depthFrame);

	if (frameReference) frameReference->Release();

	if (!depthFrame) return nullptr;

	// fill data from frame
	capacity = height * width;
	UINT16* buffer = new UINT16[capacity];
	depthFrame->CopyFrameDataToArray(capacity, buffer);

	if (depthFrame) depthFrame->Release();

	return buffer;
}

void KinectFacade::GetBodyData(IMultiSourceFrame *frame, IBody **bodies)
{
	IBodyFrame* bodyFrame;
	IBodyFrameReference* frameReference = NULL;
	frame->get_BodyFrameReference(&frameReference);

	ASSERT(frameReference);

	frameReference->AcquireFrame(&bodyFrame);

	if (frameReference) frameReference->Release();

	if (!bodyFrame) return;

	// store body data
	bodyFrame->GetAndRefreshBodyData(BODY_COUNT, bodies);

	if (bodyFrame) bodyFrame->Release();
}

void KinectFacade::GetFusionData(UINT16 *rawDepthBuffer, KinectParameters parameters, INuiFusionMesh **meshData, NUI_FUSION_IMAGE_FRAME **destination, KinectTypes types)
{
	HRESULT hr;

	// create reconstruction
	Matrix4 worldToCameraTransform;
	KinectHelper::SetIdentityMatrix(worldToCameraTransform);
	INuiFusionReconstruction* reconstruction;
	hr = NuiFusionCreateReconstruction(&parameters.reconstructionParameters, NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_AMP, -1, &worldToCameraTransform, &reconstruction);

	// create image frame
	NUI_FUSION_IMAGE_FRAME* depthFloatImageFrame;
	hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_FLOAT, width, height, nullptr, &depthFloatImageFrame);
	hr = reconstruction->DepthToDepthFloatFrame(rawDepthBuffer, width * height * sizeof(UINT16), depthFloatImageFrame, parameters.minimumDepth, parameters.maximumDepth, true);

	// remove noise
	NUI_FUSION_IMAGE_FRAME* smoothDepthFloatImageFrame;
	hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_FLOAT, width, height, nullptr, &smoothDepthFloatImageFrame);
	hr = reconstruction->SmoothDepthFloatFrame(depthFloatImageFrame, smoothDepthFloatImageFrame, 1, 0.04f);

	// reconstruction Process
	reconstruction->GetCurrentWorldToCameraTransform(&worldToCameraTransform);
	hr = reconstruction->ProcessFrame(smoothDepthFloatImageFrame, NUI_FUSION_DEFAULT_ALIGN_ITERATION_COUNT, NUI_FUSION_DEFAULT_INTEGRATION_WEIGHT, nullptr, &worldToCameraTransform);

	// calculate mesh
	reconstruction->CalculateMesh(parameters.voxelStep, meshData);

	// point cloud
	NUI_FUSION_IMAGE_FRAME* pointCloudImageFrame;
	hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_POINT_CLOUD, width, height, nullptr, &pointCloudImageFrame);
	hr = reconstruction->CalculatePointCloud(pointCloudImageFrame, &worldToCameraTransform);

	// surface
	hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_COLOR, width, height, nullptr, destination);

	// normals
	NUI_FUSION_IMAGE_FRAME* normalImageFrame;
	hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_COLOR, width, height, nullptr, &normalImageFrame);

	// shading point cloud
	Matrix4 worldToBGRTransform = { 0.0f };
	worldToBGRTransform.M11 = parameters.reconstructionParameters.voxelsPerMeter / parameters.reconstructionParameters.voxelCountX;
	worldToBGRTransform.M22 = parameters.reconstructionParameters.voxelsPerMeter / parameters.reconstructionParameters.voxelCountY;
	worldToBGRTransform.M33 = parameters.reconstructionParameters.voxelsPerMeter / parameters.reconstructionParameters.voxelCountZ;
	worldToBGRTransform.M41 = 0.5f;
	worldToBGRTransform.M42 = 0.5f;
	worldToBGRTransform.M43 = 0.0f;
	worldToBGRTransform.M44 = 1.0f;

	// process
	hr = NuiFusionShadePointCloud(pointCloudImageFrame, &worldToCameraTransform, &worldToBGRTransform, *destination, normalImageFrame);

	// release resources
	reconstruction->Release();
	NuiFusionReleaseImageFrame(depthFloatImageFrame);
	NuiFusionReleaseImageFrame(smoothDepthFloatImageFrame);
	NuiFusionReleaseImageFrame(pointCloudImageFrame);
	NuiFusionReleaseImageFrame(normalImageFrame);
}

void KinectFacade::GetKinectData(KinectData &kinectData, KinectTypes types, KinectParameters parameters)
{
	IMultiSourceFrame* multiSourceFrame = NULL;
	if (SUCCEEDED(multiSourceFrameReader->AcquireLatestFrame(&multiSourceFrame))) {
		UINT16 *rawDepthBuffer = nullptr;
		UINT rawDepthCapacity = 0;

		if ((types & KinectTypes::ColorData) != KinectTypes::NoData)
		{
			GetColorData(multiSourceFrame, &kinectData.colorData);
		}

		if ((types & KinectTypes::DepthData) != KinectTypes::NoData)
		{
			rawDepthBuffer = GetRawDepthData(multiSourceFrame, rawDepthCapacity);
			if (rawDepthBuffer) ProcessRawDepth(rawDepthBuffer, &kinectData.depthData);
		}

		if ((types & KinectTypes::BodyData) != KinectTypes::NoData)
		{
			GetBodyData(multiSourceFrame, kinectData.bodies);
		}

		if ((types & KinectTypes::InfraredData) != KinectTypes::NoData)
		{
			GetInfraredData(multiSourceFrame, &kinectData.infraredData);
		}

		if ((types & (KinectTypes::MeshData | KinectTypes::SurfaceData)) != KinectTypes::NoData)
		{
			if (rawDepthBuffer == nullptr) rawDepthBuffer = GetRawDepthData(multiSourceFrame, rawDepthCapacity);
			if (rawDepthBuffer) GetFusionData(rawDepthBuffer, parameters, &kinectData.meshData, &kinectData.surfaceData, types);
		}

		if (rawDepthBuffer) delete[] rawDepthBuffer;
	}
	if (multiSourceFrame) multiSourceFrame->Release();
}
