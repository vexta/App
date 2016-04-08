#pragma once

#include "stdafx.h"
#include "KinectData.h"

KinectData::KinectData()
	: colorData(nullptr), depthData(nullptr), infraredData(nullptr), surfaceData(nullptr), bodies{ 0 }, meshData(nullptr)
{}

KinectData::~KinectData()
{
	NuiFusionReleaseImageFrame(colorData);
	NuiFusionReleaseImageFrame(depthData);
	NuiFusionReleaseImageFrame(infraredData);
	NuiFusionReleaseImageFrame(surfaceData);

	for (int i = 0; i < BODY_COUNT; i++)
	{
		if (bodies[i]) bodies[i]->Release();
	}

	if (meshData) meshData->Release();
}

Joint* KinectData::ExtractJointsForPerson(int personId)
{
	BOOLEAN isBodyTracked = false;
	bodies[personId]->get_IsTracked(&isBodyTracked);

	if (isBodyTracked)
	{
		Joint joints[JointType_Count];
		bodies[personId]->GetJoints(JointType_Count, joints);

		return joints;
	}
	else return nullptr;
}