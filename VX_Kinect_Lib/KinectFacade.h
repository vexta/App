#pragma once

class KinectFacade
{
	static const float infraredSourceValueMaximum;
	static const float infraredOutputValueMinimum;
	static const float infraredOutputValueMaximum;
	static const float infraredSceneValueAverage;
	static const float infraredSceneStandardDeviations;
	static const USHORT minimumReliableDistance;

	IKinectSensor *kinectSensor;
	IMultiSourceFrameReader *multiSourceFrameReader;
	ICoordinateMapper *coordinateMapper;
	NUI_FUSION_CAMERA_PARAMETERS cameraParameters;

	HRESULT CopyColor(NUI_FUSION_IMAGE_FRAME *colorImageFrame, IColorFrame *pColorFrame);

	HRESULT PostprocessInfrared(NUI_FUSION_IMAGE_FRAME *infraredImage, UINT16 *buffer);
	HRESULT ProcessInfrared(NUI_FUSION_IMAGE_FRAME *infraredImage, IInfraredFrame *infraredFrame);
	void ProcessRawDepth(UINT16 *buffer, NUI_FUSION_IMAGE_FRAME **destination);
	void GetColorData(IMultiSourceFrame *frame, NUI_FUSION_IMAGE_FRAME **destination);
	void GetInfraredData(IMultiSourceFrame *frame, NUI_FUSION_IMAGE_FRAME **destination);
	UINT16 *GetRawDepthData(IMultiSourceFrame * frame, UINT &capacity);
	void GetBodyData(IMultiSourceFrame *frame, IBody **bodies);
	void GetFusionData(UINT16 *rawDepthBuffer, KinectParameters parameters, INuiFusionMesh **meshData, NUI_FUSION_IMAGE_FRAME **destination, KinectTypes types);
	HRESULT CreateFrame(NUI_FUSION_IMAGE_TYPE frameType, unsigned int imageWidth, unsigned int imageHeight, NUI_FUSION_IMAGE_FRAME **ppImageFrame);
public:
	static const int width;
	static const int height;
	static const int colorWidth;
	static const int colorHeight;

	KinectFacade();
	~KinectFacade();
	void GetKinectData(KinectData &kinectData, KinectTypes types, KinectParameters parameters);
};

