#pragma once
#include <vector>
#include <ei/vector.hpp>
#include <stack>
#include "ParamSet.h"
#include <map>
#include <memory>
#include "../geometry/Shape.h"
#include "Material.h"
#include "Light.h"
#include "TextureParams.h"

using Matrix = ei::Matrix<float, 4, 4>;
using Vector = ei::Vec3;

class PbrtScene
{
	enum Block
	{
		Scene,
		World,
		End
	};
	
public:
	enum class ShapeType
	{
		Cone,
		Cylinder,
		Disk,
		Hyperboloid,
		Heightfield,
		Loopsubdiv,
		Nurbs,
		Paraboloid,
		Sphere,
		Trianglemesh,
		Plymesh, // v3
		ERROR
	};
	struct SceneObject
	{
		std::string type;
		ParamSet set;
	};
	struct RenderOptions
	{
		Matrix cameraToWorld;
		SceneObject camera;
		Vector cameraPos;
		Vector cameraLookAt;
		SceneObject sampler;
		SceneObject film;
		SceneObject renderer;
		SceneObject surfaceIntegrator;
		SceneObject volumeIntegrator;
		SceneObject accelerator;
		SceneObject pixelFilter;
		std::vector<SceneObject> volumeRegions;
		std::vector<std::unique_ptr<Shape>> shapes;
		std::vector<Light> lights;
	};
	struct GraphicsState
	{
		std::string material = "matte";
		ParamSet materialParams;
		bool reverseOrientation = false;
		std::string areaLight;
		ParamSet areaLightParams;
		std::map<std::string, std::shared_ptr<Material>> namedMaterials;
		std::string currentNamedMaterial;
		std::map<std::string, std::shared_ptr<Texture<float>>> floatTextures;
		std::map<std::string, std::shared_ptr<Texture<Spectrum>>> spectrumTextures;

		std::shared_ptr<Material> createMaterial(ParamSet& set, const Matrix& toWorld);
		std::shared_ptr<Material> makeMaterial(const std::string& name, TextureParams& set, const Matrix& toWorld);
	};
public:
	PbrtScene();
	/***************
		API Calls
	***************/
#pragma region "TRANSFORM"
#define TRANS_ARGS const std::vector<float>& args
	// Transforms
	void apiIdentity();
	void apiTranslate(TRANS_ARGS);
	void apiScale(TRANS_ARGS);
	void apiRotate(TRANS_ARGS);
	void apiLookAt(TRANS_ARGS);
	void apiCoordinateSystem(const std::string& n);
	void apiCoordSysTransform(const std::string& n);
	void apiTransform(TRANS_ARGS);
	void apiConcatTransfrom(TRANS_ARGS);
	void apiSetActiveTransform(bool active);
	void apiTransformBegin();
	void apiTransformEnd();
#pragma endregion 
#pragma region "SCENE"
#define MODUL_ARGS const std::string& type, ParamSet& p
	// Scene
	void apiCamera(MODUL_ARGS);
	void apiSampler(MODUL_ARGS);
	void apiFilm(MODUL_ARGS);
	void apiRenderer(MODUL_ARGS);
	void apiSurfaceIntegrator(MODUL_ARGS);
	void apiVolumeIntegrator(MODUL_ARGS);
	void apiAccelerator(MODUL_ARGS);
	void apiPixelFilter(MODUL_ARGS);
#pragma endregion 

#pragma region "WORLD"
	void apiWorldBegin();
	void apiWorldEnd();
	void apiAttributeBegin();
	void apiAttributeEnd();
	
	void apiShape(MODUL_ARGS);
	void apiObjectBegin(const std::string& n);
	void apiObjectEnd();
	void apiObjectInstance(const std::string& n);
	void apiLightSource(MODUL_ARGS);
	void apiAreaLightSource(MODUL_ARGS);
	void apiReverseOrientation();
	void apiMaterial(MODUL_ARGS);
	void apiMakeNamedMaterial(MODUL_ARGS);
	void apiNamedMaterial(const std::string& n);
	void apiTexture(const std::string& name, const std::string& type, const std::string& clas, ParamSet& set);
	void apiVolume(MODUL_ARGS);
#pragma endregion 

	RenderOptions& getRenderOptions();
private:
	void useTrans(const Matrix& m, bool concat);
	void resetTransforms();

	template <class T>
	std::shared_ptr<Texture<T>> makeTexture(const std::string& name, const Matrix& toWorld, TextureParams& tp);
	template <class T>
	static void createImageMap(TextureParams& tp, Texture<T>& tex);
	template <class T>
	static void setTextureMapping(TextureParams& tp, Texture<T>& tex);
private:
	std::stack<Matrix> m_transforms;
	std::stack<bool> m_activeTransform; // indicates if first transform is active(because animation is not supported)
	Block m_block = Scene;
	std::map<std::string, Matrix> m_namedCoordSystems;

#pragma region "SCENE"
	RenderOptions m_renderOptions;
#pragma endregion 

#pragma region "WORLD"
	std::stack<GraphicsState> m_gfxStates;

	// instancing
	std::map<std::string, std::vector<std::unique_ptr<Shape>>> m_instances;
	std::vector<std::unique_ptr<Shape>>* m_pCurInstance = nullptr; // pointer to currently described Object Instance
	
#pragma endregion 
};


PbrtScene::ShapeType getShapeTypeFromString(const std::string&);
