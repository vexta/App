#pragma once
enum class KinectTypes : int
{
	NoData = 0,
	ColorData = 1 << 0,
	DepthData = 1 << 1,
	InfraredData = 1 << 2,
	SurfaceData = 1 << 3,
	BodyData = 1 << 4,
	MeshData = 1 << 5
};

inline constexpr KinectTypes operator| (KinectTypes left, KinectTypes right)
{
	return static_cast<KinectTypes>(static_cast<int>(left) | static_cast<int>(right));
}

inline constexpr KinectTypes operator& (KinectTypes left, KinectTypes right)
{
	return static_cast<KinectTypes>(static_cast<int>(left) & static_cast<int>(right));
}

inline KinectTypes operator|= (KinectTypes left, KinectTypes right)
{
	return left | right;
}

inline KinectTypes operator&= (KinectTypes left, KinectTypes right)
{
	return left & right;
}

