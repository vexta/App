#pragma once

struct KinectData
{
	NUI_FUSION_IMAGE_FRAME *colorData;
	NUI_FUSION_IMAGE_FRAME *depthData;
	NUI_FUSION_IMAGE_FRAME *infraredData;
	NUI_FUSION_IMAGE_FRAME *surfaceData;
	IBody *bodies[BODY_COUNT]; // array of IBody pointers
	INuiFusionMesh *meshData;

	KinectData();
	~KinectData();
	Joint* ExtractJointsForPerson(int personId);
	Joint* ExtractJointsForFirstPerson(int& index);
};