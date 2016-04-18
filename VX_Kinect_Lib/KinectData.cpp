#pragma once

#include "stdafx.h"
#include "KinectData.h"

KinectData::KinectData()
	: colorData(nullptr), depthData(nullptr), infraredData(nullptr), surfaceData(nullptr), bodies{ 0 }, meshData(nullptr)
{}

KinectData::~KinectData()
{
	if (colorData) NuiFusionReleaseImageFrame(colorData);
	if (depthData) NuiFusionReleaseImageFrame(depthData);
	if (infraredData) NuiFusionReleaseImageFrame(infraredData);
	if (surfaceData) NuiFusionReleaseImageFrame(surfaceData);

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

Joint* KinectData::ExtractJointsForFirstPerson(int& index)
{
	BOOLEAN isBodyTracked = false;
	Joint joints[JointType_Count];
	HandState leftHandState;
	for (int i = 0; i < BODY_COUNT; i++) {
		if (bodies[i]) {
			bodies[i]->get_IsTracked(&isBodyTracked);
			bodies[i]->get_HandLeftState(&leftHandState);
			if (isBodyTracked) {
				Joint joints[JointType_Count];
				bodies[i]->GetJoints(JointType_Count, joints);

				index = i;
				return joints;
			}
		}
	}

	return nullptr;
}

