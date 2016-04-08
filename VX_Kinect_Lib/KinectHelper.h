#pragma once
class KinectHelper
{
public:
	static HRESULT WriteAsciiObjMeshFile(INuiFusionMesh *mesh, const std::string& passedFilename, bool flipYZ);
	static void SetIdentityMatrix(Matrix4 &mat);
};

