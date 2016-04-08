#pragma once

struct KinectParameters
{
	NUI_FUSION_RECONSTRUCTION_PARAMETERS reconstructionParameters;
	int voxelStep;
	float minimumDepth;
	float maximumDepth;

	KinectParameters()
	{
		reconstructionParameters.voxelsPerMeter = 256;
		reconstructionParameters.voxelCountX = 256;
		reconstructionParameters.voxelCountY = 256;
		reconstructionParameters.voxelCountZ = 256;
		voxelStep = 4;
		minimumDepth = NUI_FUSION_DEFAULT_MINIMUM_DEPTH;
		maximumDepth = NUI_FUSION_DEFAULT_MAXIMUM_DEPTH;
	}
};