#include "PbrtScene.h"
#include <iostream>
#include "../geometry/TriangleMesh.h"
#include "../geometry/Sphere.h"
#include <functional>
#include "volume.h"
#include "copper.h"
#include "../geometry/Plymesh.h"
#include "../geometry/LoopSubDiv.h"

#define ASSERT_TRANS_ARGS(number) if(args.size() != number) {System::error(std::string(__FUNCSIG__) + " invalid number of args\n");} else
#define ASSERT_BLOCK(block) if(m_block != block) {System::error(std::string(__FUNCSIG__) + " called in wrong block. will be ignored"); return;}

// TODO in eplsilon?
inline void CoordinateSystem(const Vector &v1, Vector *v2, Vector *v3) {
	if (fabsf(v1.x) > fabsf(v1.y)) {
		float invLen = 1.f / sqrtf(v1.x*v1.x + v1.z*v1.z);
		*v2 = Vector(-v1.z * invLen, 0.f, v1.x * invLen);
	}
	else {
		float invLen = 1.f / sqrtf(v1.y*v1.y + v1.z*v1.z);
		*v2 = Vector(0.f, v1.z * invLen, -v1.y * invLen);
	}
	*v3 = ei::cross(v1, *v2);
}

PbrtScene::PbrtScene()
{
	m_transforms.push(ei::identity4x4());
	m_activeTransform.push(true);

	m_renderOptions.cameraToWorld = ei::identity4x4();
	m_renderOptions.pixelFilter.type = "box";
	m_renderOptions.film.type = "image";
	m_renderOptions.sampler.type = "lowdiscrepancy";
	m_renderOptions.accelerator.type = "bvh";
	m_renderOptions.renderer.type = "sampler";
	m_renderOptions.surfaceIntegrator.type = "directlighting";
	m_renderOptions.volumeIntegrator.type = "emission";
	m_renderOptions.volumeIntegrator.type = "perspective";

	m_gfxStates.push(GraphicsState());
}

#pragma region "TRANSFORMS"
void PbrtScene::apiIdentity()
{
	useTrans(ei::identity4x4(), false);
}

void PbrtScene::apiTranslate(const std::vector<float>& args)
{
	ASSERT_TRANS_ARGS(3)
	{
		useTrans(ei::translation(Vector(args[0], args[1], args[2])),true);
	}
}

void PbrtScene::apiScale(const std::vector<float>& args)
{
	ASSERT_TRANS_ARGS(3)
	{
		useTrans(ei::scaling(ei::Vec4(args[0], args[1], args[2], 1.0f)),true);
	}
}

void PbrtScene::apiRotate(const std::vector<float>& args)
{
	ASSERT_TRANS_ARGS(4)
	{
		useTrans(Matrix(ei::rotation(Vector(args[1], args[2], args[3]), float(double(args[0]) / 360.0 * 2.0 * M_PI))),true);
	}
}

void PbrtScene::apiLookAt(TRANS_ARGS)
{
	ASSERT_TRANS_ARGS(9)
	{
		useTrans(ei::camera(Vector(args[0], args[1], args[2]), Vector(args[3], args[4], args[5]), Vector(args[6], args[7], args[8])),true);
	}
}

void PbrtScene::apiCoordinateSystem(const std::string& n)
{
	m_namedCoordSystems[n] = m_transforms.top();
}

void PbrtScene::apiCoordSysTransform(const std::string& n)
{
	auto elm = m_namedCoordSystems.find(n);
	if(elm != m_namedCoordSystems.end())
	{
		m_transforms.top() = elm->second;
	}
	else System::warning("couldn't find named coordinate system " + n);
}

void PbrtScene::apiTransform(const std::vector<float>& args)
{
	ASSERT_TRANS_ARGS(4 * 4)
	{
		useTrans(Matrix(
			args[0], args[4], args[8], args[12],
			args[1], args[5], args[9], args[13],
			args[2], args[6], args[10], args[14],
			args[3], args[7], args[11], args[15]), false);
	}
}

void PbrtScene::apiConcatTransfrom(const std::vector<float>& args)
{
	ASSERT_TRANS_ARGS(4 * 4)
	{
		useTrans(Matrix(
			args[0], args[4], args[8], args[12],
			args[1], args[5], args[9], args[13],
			args[2], args[6], args[10], args[14],
			args[3], args[7], args[11], args[15]), true);
	}
}

void PbrtScene::apiSetActiveTransform(bool active)
{
	m_activeTransform.top() = active;
}

void PbrtScene::apiTransformBegin()
{
	ASSERT_BLOCK(Block::World);
	m_transforms.push(m_transforms.top());
	m_activeTransform.push(m_activeTransform.top());
}

void PbrtScene::apiTransformEnd()
{
	ASSERT_BLOCK(Block::World);
	if (m_transforms.size() >= 2)
	{
		m_transforms.pop();
		m_activeTransform.pop();
	}
	else System::warning("TransformEnd stack underflow. Ignoring it");
}

void PbrtScene::useTrans(const Matrix& m, bool concat)
{
	if(m_activeTransform.top())
		if (concat)
			m_transforms.top() *= m;
		else
			m_transforms.top() = m;
}

void PbrtScene::resetTransforms()
{
	if (m_transforms.size() > 1)
		System::warning(std::to_string(m_transforms.size()) + " transforms on stack before resetting");
	while (m_transforms.size()) m_transforms.pop();
	m_transforms.push(ei::identity4x4());
}

#pragma endregion 

#pragma region "SCENE"
#define INIT_SCENE_MODUL(name) m_renderOptions.name.type = type; m_renderOptions.name.set = std::move(p)

void PbrtScene::apiCamera(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::Scene);
	// current ctm -> world to camera transformation
	INIT_SCENE_MODUL(camera);
	m_renderOptions.cameraPos = ei::transform(ei::Vec3(0.0f), m_transforms.top());
	
	m_renderOptions.cameraToWorld = ei::invert(m_transforms.top());
	m_renderOptions.cameraPos = ei::transform(ei::Vec3(0.0f), m_renderOptions.cameraToWorld);
	m_renderOptions.cameraLookAt = ei::transformDir(Vector(0.0f, 0.0f, 1.0f), m_renderOptions.cameraToWorld)
		+ m_renderOptions.cameraPos;
	apiCoordinateSystem("camera");
}

void PbrtScene::apiSampler(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::Scene);
	INIT_SCENE_MODUL(sampler);
}

void PbrtScene::apiFilm(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::Scene);
	INIT_SCENE_MODUL(film);
}

void PbrtScene::apiRenderer(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::Scene);
	INIT_SCENE_MODUL(renderer);
}

void PbrtScene::apiSurfaceIntegrator(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::Scene);
	INIT_SCENE_MODUL(surfaceIntegrator);
}

void PbrtScene::apiVolumeIntegrator(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::Scene);
	INIT_SCENE_MODUL(volumeIntegrator);
}

void PbrtScene::apiAccelerator(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::Scene);
	INIT_SCENE_MODUL(accelerator);
}

void PbrtScene::apiPixelFilter(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::Scene);
	INIT_SCENE_MODUL(pixelFilter);
}
#pragma endregion 

#pragma region "WORLD"
void PbrtScene::apiWorldBegin()
{
	ASSERT_BLOCK(Block::Scene);
	m_block = Block::World;
	apiSetActiveTransform(true);
	apiIdentity();
	apiCoordinateSystem("world");
}

void PbrtScene::apiWorldEnd()
{
	ASSERT_BLOCK(Block::World);
	m_block = Block::End;
	// output warnings concerning tranforms etc.
	resetTransforms(); // this will output warning when transforms on stack

	while (m_gfxStates.size() > 1)
	{
		m_gfxStates.pop();
		System::warning("missing end to AttributeBegin()");
	}
	if (m_gfxStates.size()) m_gfxStates.pop();
	assert(m_gfxStates.size() == 0);

	// clear instances
	m_instances.clear();
	m_pCurInstance = nullptr;

	m_namedCoordSystems.clear();
}

void PbrtScene::apiAttributeBegin()
{
	ASSERT_BLOCK(Block::World);
	// pushes: transform, material, reverse-orientation setting
	apiTransformBegin();
	m_gfxStates.push(m_gfxStates.top());
}

void PbrtScene::apiAttributeEnd()
{
	ASSERT_BLOCK(Block::World);
	if(m_gfxStates.size() < 2)
	{
		System::warning("AttributeEnd stack underflow. ignoring it");
		return;
	}
	m_gfxStates.pop();
	apiTransformEnd();
}

void PbrtScene::apiShape(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::World);
	auto stype = getShapeTypeFromString(type);
	std::unique_ptr<Shape> pShape;
	switch (stype)
	{
	case ShapeType::Trianglemesh:
		pShape = std::unique_ptr<Shape>(new TriangleMesh());
	{
			std::shared_ptr<Texture<float>> atex;
			auto alphaTexName = p.getTexture("alpha");
			if(alphaTexName != "")
			{
				auto it = m_gfxStates.top().floatTextures.find(alphaTexName);
				if (it != m_gfxStates.top().floatTextures.end())
					atex = it->second;
				else
					System::warning("couldn't find float texture " + alphaTexName + " for alpha parameter");
			}
			else if(p.getFloat("alpha",1.0f) == 0.0f)
			{
				atex = std::shared_ptr<Texture<float>>(new Texture<float>);
				atex->type = Texture<float>::Constant;
				atex->value = 0.0f;
				//atex->transform = m_transforms.top();
			}
			dynamic_cast<TriangleMesh*>(pShape.get())->setAlphaTexture(atex);
	}
		break;
	case ShapeType::Sphere:
		pShape = std::unique_ptr<Shape>(new Sphere());
		break;
	case ShapeType::Plymesh:
		pShape = std::unique_ptr<Shape>(new Plymesh());
		break;
	case ShapeType::Loopsubdiv:
		pShape.reset(new LoopSubDiv());
		break;
	case ShapeType::Cone:
	case ShapeType::Cylinder:
	case ShapeType::Disk:
	case ShapeType::Hyperboloid:
	case ShapeType::Heightfield:
	case ShapeType::Nurbs:
	case ShapeType::Paraboloid:
		System::warning("shape " + type + " not yet implemented. Will be ignored");
		return;
	default:
		System::error("unknow shape type " + type);
		return;
	}

	pShape->init(p);
	pShape->applyTransform(m_transforms.top());
	pShape->setMaterial(m_gfxStates.top().createMaterial(p, m_transforms.top()));
	if (m_gfxStates.top().reverseOrientation)
		pShape->flipNormals();

	std::shared_ptr<AreaLight> pArea;
	if(m_gfxStates.top().areaLight != "")
	{
		if(m_gfxStates.top().areaLight == "area" || m_gfxStates.top().areaLight == "diffuse")
		{
			pArea = std::shared_ptr<AreaLight>(new AreaLight);
			Spectrum L = m_gfxStates.top().areaLightParams.getSpectrum("L", Spectrum(1.0f));
			Spectrum sc = m_gfxStates.top().areaLightParams.getSpectrum("scale", Spectrum(1.0f));
			pArea->spectrum = L  * sc;
			pArea->nSamples = m_gfxStates.top().areaLightParams.getInt("nsamples", 1);
		}
		else System::warning("area light " + m_gfxStates.top().areaLight + " unknown. Will be ignored");
	}

	if(m_pCurInstance)
	{
		if (pArea)
			System::warning("Area lights not supported with object instancing");
		m_pCurInstance->push_back(move(pShape));
	}
	else
	{
		if (pArea)
			pShape->setAreaLight(pArea);
		m_renderOptions.shapes.push_back(move(pShape));
	}
}

void PbrtScene::apiObjectBegin(const std::string& n)
{
	ASSERT_BLOCK(Block::World);
	apiAttributeBegin();
	if (m_pCurInstance)
		std::exception("ObjectBegin called inside of instance definition");
	m_instances[n] = std::vector<std::unique_ptr<Shape>>();
	m_pCurInstance = &m_instances[n];
}

void PbrtScene::apiObjectEnd()
{
	ASSERT_BLOCK(Block::World);
	if (!m_pCurInstance)
		throw std::exception("ObjectEnd called outside of instance definition");
	m_pCurInstance = nullptr;
	apiAttributeEnd();
}

void PbrtScene::apiObjectInstance(const std::string& n)
{
	System::runtimeInfoSpam("object instance");
	if (m_gfxStates.top().reverseOrientation)
		System::info("reverse orientation is ignored in instancing");

	ASSERT_BLOCK(Block::World);
	if (m_pCurInstance)
		throw std::exception("ObjectInstance can't be called inside instance definition");
	auto in = m_instances.find(n);
	if (in == m_instances.end())
		throw std::exception(("Unable to find instance named " + n).c_str());

	const auto& vec = in->second;
	if (vec.size() == 0) return;

	// instanciate
	for(const auto& e : vec)
	{
		// make copy and instanciate
		auto copy = std::unique_ptr<Shape>(e->clone());
		copy->applyTransform(m_transforms.top());
		m_renderOptions.shapes.push_back(move(copy));
	}
}

void PbrtScene::apiLightSource(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::World);
	auto t = getLightTypeFromString(type);
	Light l;
	l.type = t;
	Spectrum scale = p.getSpectrum("scale", Spectrum(1.0f));
	l.spectrum = p.getSpectrum("L", Spectrum::FromRGB(1.0f, 1.0f, 1.0f)) * scale;
	// TODO is this always a scale factor?
	l.spectrum = p.getSpectrum("I", Spectrum::FromRGB(1.0f, 1.0f, 1.0f)) * l.spectrum;

	switch (t)
	{
	case Light::Spot:
		l.coneangle = p.getFloat("coneangle", 30.0f);
		l.conedeltaangle = p.getFloat("conedeltaangle", 5.0f);
	case Light::Distant: // + Spot
		l.to = p.getPoint("to", Vector(0.0f, 0.0f, 1.0f));
	case Light::Point: // + Distant + Spot
		l.from = p.getPoint("from", Vector(0.0f, 0.0f, 0.0f));
		break;
	case Light::Goniometric:
		l.mapname = p.getString("mapname", "");
		if (l.mapname.length() == 0)
			throw PbrtMissingParameter("mapname in goniometric light");
		break;
	case Light::Infinite: 
		l.nsamples = p.getInt("nsamples", 1);
		l.mapname = p.getString("mapname", "");
		break;
	case Light::Projection:
		l.fov = p.getFloat("fov", 45.0f);
		l.mapname = p.getString("mapname", "");
		break;
	default:
		System::error("light type unknown: " + type + ". Ignoring it");
		return;
	}
	// set position
	switch(t)
	{
	case Light::Distant:
		l.dir = Shape::transformVector(l.from - l.to, m_transforms.top());
		break;
	case Light::Projection:
	case Light::Goniometric:
		l.position = Shape::transformPoint(Vector(0.0f), m_transforms.top());
		break;
	case Light::Point: 
		l.position = Shape::transformPoint(l.from, m_transforms.top());
		break;
	case Light::Spot: 
	{
		// TODO examine
		Vector dir = ei::normalize(l.to - l.from);
		Vector du, dv;
		CoordinateSystem(dir, &du, &dv);
		Matrix dirToZ = Matrix(	du.x,  du.y,  du.z,  0.0f,
								dv.x,  dv.y,  dv.z,  0.0f,
								dir.x, dir.y, dir.z, 0.0f,
								0.0f,  0.0f,  0.0f,  1.0f);
		Matrix lightToWorld = m_transforms.top() * ei::translation(Vector(l.from.x, l.from.y, l.from.z)) * ei::invert(dirToZ);
		l.position = Shape::transformPoint(Vector(0.0f), lightToWorld);
		// TODO set direction?
	}
		break;
	default: break;
	}

	m_renderOptions.lights.push_back(l);
}

void PbrtScene::apiAreaLightSource(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::World);
	m_gfxStates.top().areaLight = type;
	m_gfxStates.top().areaLightParams = std::move(p);
}

void PbrtScene::apiReverseOrientation()
{
	ASSERT_BLOCK(Block::World);
	m_gfxStates.top().reverseOrientation = !m_gfxStates.top().reverseOrientation;
}

void PbrtScene::apiMaterial(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::World);
	m_gfxStates.top().material = type;
	m_gfxStates.top().materialParams = std::move(p);
	m_gfxStates.top().currentNamedMaterial = "";
}

void PbrtScene::apiMakeNamedMaterial(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::World);
	TextureParams mp(p, m_gfxStates.top().materialParams, m_gfxStates.top().floatTextures, m_gfxStates.top().spectrumTextures);
	auto matName = p.getString("type", "");
	if(matName == "")
	{
		System::error("no parameter string \"type\" found in MakeNamedMaterial");
		return;
	}

	auto mtl = m_gfxStates.top().makeMaterial(matName, mp, m_transforms.top());
	if (mtl)
	{
		m_gfxStates.top().namedMaterials[type] = mtl;
		System::runtimeInfo("made named material " + type);
	}
	else System::error("could not make material " + type);
}

void PbrtScene::apiNamedMaterial(const std::string& n)
{
	ASSERT_BLOCK(Block::World);
	m_gfxStates.top().currentNamedMaterial = n;
}

void PbrtScene::apiTexture(const std::string& name, const std::string& type, const std::string& clas, ParamSet& set)
{
	ASSERT_BLOCK(Block::World);
	TextureParams tp(set, set, m_gfxStates.top().floatTextures, m_gfxStates.top().spectrumTextures);

	if(type == "float")
	{
		if (m_gfxStates.top().floatTextures.find(name) != m_gfxStates.top().floatTextures.end())
			System::info("Texture " + name + " being redifined");

		auto tex = makeTexture<float>(clas, m_transforms.top(), tp);
		m_gfxStates.top().floatTextures[name] = tex;
		System::runtimeInfo("made float texture " + name);
	}
	else if(type == "color" || type == "spectrum")
	{
		if(m_gfxStates.top().spectrumTextures.find(name) != m_gfxStates.top().spectrumTextures.end())
			System::info("Texture " + name + " being redifined");

		auto tex = makeTexture<Spectrum>(clas, m_transforms.top(), tp);
		m_gfxStates.top().spectrumTextures[name] = tex;
		System::runtimeInfo("made spectrum texture " + name);
	}
	else System::error("Texture type " + type + " unknown");
}

void PbrtScene::apiVolume(MODUL_ARGS)
{
	ASSERT_BLOCK(Block::World);
	SceneObject obj;
	obj.type = type;
	obj.set = std::move(p);
	m_renderOptions.volumeRegions.push_back(obj);
}

PbrtScene::RenderOptions& PbrtScene::getRenderOptions()
{
	return m_renderOptions;
}
#pragma endregion 

#pragma region "TEXTURES AND MATERIALS"

template <class T>
std::shared_ptr<Texture<T>> PbrtScene::makeTexture(const std::string& name, const Matrix& toWorld, TextureParams& tp)
{
	static_assert(std::is_same<T, float>::value || std::is_same<T, Spectrum>::value, "only float and spectrum defined");

	// thats the tricky part ;)
	std::function<T(const std::string&, T)> getT = [&tp](const std::string& n, T def)
	{
		if (typeid(T) == typeid(float))
			return T(tp.getFloat(n, *reinterpret_cast<float*>(&def)));
		else
			return *reinterpret_cast<T*>(&tp.getSpectrum(n, def));
	};

	std::shared_ptr<Texture<T>> tex = std::shared_ptr<Texture<T>>(new Texture<T>);
	//tex->transform = toWorld;

	auto type = Texture<T>::Type(getTextureTypeFromString(name));
	tex->type = type;
	switch (type)
	{
	case Texture<T>::Bilerp: 
		setTextureMapping(tp, *tex);
		tex->v00 = getT("v00", 0.0f);
		tex->v01 = getT("v01", 1.0f);
		tex->v10 = getT("v10", 0.0f);
		tex->v11 = getT("v11", 1.0f);
		break;
	case Texture<T>::Checkerboard: 
		tex->tex1 = tp.getTexture<T>("tex1", T(1.0f));
		tex->tex2 = tp.getTexture<T>("tex2", T(0.0f));
		tex->dimension = tp.getInt("dimension", 2);
		if(tex->dimension == 2)
		{
			setTextureMapping(tp, *tex);
			auto mode = tp.getString("aamode", "closedform");
			tex->aamode = (mode == "closedform");
			if (!tex->aamode && mode != "none")
				System::error("Antialiasing mode \"" + mode + "\" not understood by "
				"Checkerboard2DTexture; using \"closedform\"");
		}
		else if (tex->dimension != 3)
			System::error(std::to_string(tex->dimension) + " dimensional checkerboard texture not supported\n");
		// use transform for 3d maps

		break;
	case Texture<T>::Constant: 
		tex->value = getT("value", 1.0f);
		break; // constant -> always value
	case Texture<T>::Dots: 
		setTextureMapping(tp, *tex);
		tex->inside = getT("inside", 1.0f);
		tex->outside = getT("outsude", 0.0f);
		break;
	case Texture<T>::Marble:
		if (typeid(T) == typeid(float))
		{
			System::error("marble texture float not implemented");
			return std::shared_ptr<Texture<T>>(); // Not implemented	
		}

		tex->scale = tp.getFloat("scale", 1.0f);
		tex->variation = tp.getFloat("variation", 0.2f);
	case Texture<T>::Wrinkled: // + Marble
	case Texture<T>::Fbm: // + Wrinkled + Marble
		// always 3d map -> use transform
		tex->octaves = tp.getInt("octaves", 8);
		tex->roughness = tp.getFloat("roughness", 0.5f);
		break;
	case Texture<T>::Imagemap: 
		createImageMap(tp, *tex);
		break;
	case Texture<T>::Mix:
		tex->tex1 = tp.getTexture<T>("tex1", T(0.0f));
		tex->tex2 = tp.getTexture<T>("tex2", T(1.0f));
		tex->amount = tp.getTexture<float>("amount", 0.5f);
		break;
	case Texture<T>::Scale:
		tex->tex1 = tp.getTexture<T>("tex1", T(1.0f));
		tex->tex2 = tp.getTexture<T>("tex2", T(1.0f));
		break;
	case Texture<T>::Uv:
		if (typeid(T) == typeid(float))
		{
			System::error("uv texture float not implemented");
			return std::shared_ptr<Texture<T>>();
		}
		
		setTextureMapping(tp, *tex);
		break;
	case Texture<T>::Windy: break; // just transform

	default:
		System::error("texture " + name + " unknown");
	}
	return tex;
}

template <class T>
void PbrtScene::createImageMap(TextureParams& tp, Texture<T>& tex)
{
	setTextureMapping(tp, tex);

	tex.maxAniso = tp.getFloat("maxanisotropy", 8.0f);
	tex.trilerp = tp.getBool("trilinear", false);
	auto wrap = tp.getString("wrap", "repeat");
	tex.wrapping = TexWrapMode::Repeat;
	if (wrap == "black") tex.wrapping = TexWrapMode::Black;
	if (wrap == "clamp") tex.wrapping = TexWrapMode::Clamp;
	tex.scale = tp.getFloat("scale", 1.0f);
	tex.gamma = tp.getFloat("gamma", 1.0f);
	tex.filename = tp.getFilename("filename", "");
}

template <class T>
void PbrtScene::setTextureMapping(TextureParams& tp, Texture<T>& tex)
{
	auto type = tp.getString("mapping", "uv");

	if (type == "spherical")
		tex.mapping = decltype(tex.mapping)::Spherical;
	else if (type == "cylindrical")
		tex.mapping = decltype(tex.mapping)::Cylindrical;
	else if (type == "planar")
	{
		tex.mapping = decltype(tex.mapping)::Planar;
		tex.v1 = tp.getVector("v1", Vector(1.0f, 0.0f, 0.0f));
		tex.v2 = tp.getVector("v2", Vector(0.0f, 1.0f, 0.0f));
		tex.udelta = tp.getFloat("udelta", 0.0f);
		tex.vdelta = tp.getFloat("vdelta", 0.0f);
	}
	else
	{
		if (type != "uv")
			System::error("2D texture mapping " + type + " unknown");
		// default uv map
		tex.mapping = decltype(tex.mapping)::Uv;
		tex.su = tp.getFloat("uscale", 1.0f);
		tex.sv = tp.getFloat("vscale", 1.0f);
		tex.du = tp.getFloat("udelta", 0.0f);
		tex.dv = tp.getFloat("vdelta", 0.0f);
	}
}

std::shared_ptr<Material> PbrtScene::GraphicsState::makeMaterial(const std::string& name, TextureParams& tp, const Matrix& toWorld)
{
	auto type = getMaterialTypeFromString(name);
	auto mtl = std::shared_ptr<Material>(new Material);
	mtl->type = type;
	mtl->bumpmap = tp.getTextureOrNull("bumpmap");

	switch (type)
	{
	case Material::Glass:
		mtl->Kt = tp.getTexture("Kt", Spectrum(1.0f));
		mtl->index = tp.getTexture("index", 1.5f);
	case Material::Mirror: // + Glass
		mtl->Kr = tp.getTexture("Kr", Spectrum(1.0f));
		break;
	case Material::Kdsubsurface: 
		mtl->Kd = tp.getTexture("Kd", Spectrum(0.5f));
		mtl->meanfreepath = tp.getTexture("meanfreepath", 1.0f);
		mtl->index = tp.getTexture("index", 1.3f);
		mtl->Kr = tp.getTexture("Kr", Spectrum(1.0f));
		break;
	case Material::Matte:
		mtl->Kd = tp.getTexture("Kd", Spectrum(0.5f));
		mtl->sigma = tp.getTexture("sigma", 0.0f);
		break;
	case Material::Measured:
		mtl->filename = tp.getFilename("filename", "");
		break;
	case Material::Metal:
	{
		static Spectrum copperN = Spectrum::FromSampled(CopperWavelengths, CopperN, CopperSamples);
		mtl->eta = tp.getTexture("eta", copperN);
		
		static Spectrum copperK = Spectrum::FromSampled(CopperWavelengths, CopperK, CopperSamples);
		mtl->k = tp.getTexture("k", copperK);

		mtl->roughness = tp.getTexture("roughness", 0.01f);
	}
		break;
	case Material::Mix:
	{
		auto m1 = tp.getString("namedmaterial1", "");
		auto m2 = tp.getString("namedmaterial2", "");
		std::shared_ptr<Material> mat1;
		std::shared_ptr<Material> mat2;
		auto itMat1 = namedMaterials.find(m1);
		auto itMat2 = namedMaterials.find(m2);
		if (itMat1 != namedMaterials.end())
			mat1 = itMat1->second;
		if (itMat2 != namedMaterials.end())
			mat2 = itMat2->second;

		if(!mat1)
		{
			System::error("named material " + m1 + " undefined. Using matte");
			mat1 = makeMaterial("matte", tp, toWorld);
		}
		if(!mat2)
		{
			System::error("named material " + m2 + " undefined. Using matte");
			mat2 = makeMaterial("matte", tp, toWorld);
		}

		mtl->amount = tp.getTexture("amount", Spectrum(0.5f));
		mtl->material1 = mat1;
		mtl->material2 = mat2;
	}
		break;
	case Material::Translucent:
		mtl->reflect = tp.getTexture("reflect", Spectrum(0.5f));
		mtl->transmit = tp.getTexture("transmit", Spectrum(0.5f));
	case Material::Plastic: // +Translucent
		mtl->Kd = tp.getTexture("Kd", Spectrum(0.25f)); 
		mtl->Ks = tp.getTexture("Ks", Spectrum(0.25f));
		mtl->roughness = tp.getTexture("roughness", 0.1f);
		break;
	case Material::Shinymetal:
		mtl->Kr = tp.getTexture("Kr", Spectrum(1.0f));
		mtl->Ks = tp.getTexture("Ks", Spectrum(1.0f));
		mtl->roughness = tp.getTexture("roughness", 0.1f);
		break;
	case Material::Substrate: 
		mtl->Kd = tp.getTexture("Kd", Spectrum(0.5f));
		mtl->Ks = tp.getTexture("Ks", Spectrum(0.5f));
		mtl->uroughness = tp.getTexture("uroughness", 0.1f);
		mtl->vroughness = tp.getTexture("vroughness", 0.1f);
		break;
	case Material::Subsurface:
	{
		Spectrum sa = Spectrum::FromRGB(.0011f, .0024f, .014f);
		Spectrum sps = Spectrum::FromRGB(2.55f, 3.21f, 3.77f);
		auto matName = tp.getString("name", "");
		bool found = GetVolumeScatteringProperties(name, &sa, &sps);
		if (name != "" && !found)
			System::warning("named material " + name + " not found. Using defaults");
		mtl->scale = tp.getFloat("scale", 1.0f);
		mtl->sigma_a = tp.getTexture("sigma_a", sa);
		mtl->sigma_prime_s = tp.getTexture("sigma_prime_s", sps);
		mtl->index = tp.getTexture("index", 1.3f);
		mtl->Kr = tp.getTexture("Kr", Spectrum(1.0f));
	}
		break;
	case Material::Uber:
		mtl->Kd = tp.getTexture("Kd", Spectrum(0.25f));
		mtl->Ks = tp.getTexture("Ks", Spectrum(0.25f));
		mtl->Kr = tp.getTexture("Kr", Spectrum(0.0f));
		mtl->Kt = tp.getTexture("Kt", Spectrum(0.0f));
		mtl->roughness = tp.getTexture("roughness", 0.1f);
		mtl->index = tp.getTexture("index", 1.5f);
		mtl->opacity = tp.getTexture("opacity", Spectrum(1.0f));
		break;
	case Material::Fourier:
		mtl->filename = tp.getFilename("bsdffile", "");
		break;
	case Material::Hair:
		mtl->sigma_a = tp.getTexture("sigma_a", Spectrum(1.0f));
		mtl->color = tp.getTexture("color", Spectrum(1.0f));
		mtl->eumelanin = tp.getTexture("eumelanin", 1.0f);
		mtl->pheomelanin = tp.getTexture("pheomelanin", 1.0f);
		mtl->eta_hair = tp.getTexture("eta", 1.55f);
		mtl->beta_m = tp.getTexture("bata_m", 0.3f);
		mtl->beta_n = tp.getTexture("beta_n", 0.3f);
		mtl->alpha = tp.getTexture("alpha", 2.0f);
		break;
	case Material::None:
		break;
	default:
		System::error("unable to create material " + name);
	}

	return mtl;
}

std::shared_ptr<Material> PbrtScene::GraphicsState::createMaterial(ParamSet& set, const Matrix& toWorld)
{
	std::shared_ptr<Material> mtl;
	TextureParams mp(set, materialParams, floatTextures, spectrumTextures);

	if (currentNamedMaterial != "")
	{
		auto it = namedMaterials.find(currentNamedMaterial);
		if (it != namedMaterials.end())
			mtl = it->second;
	}
	if (!mtl)
		mtl = makeMaterial(material, mp, toWorld);
	if (!mtl)
		mtl = makeMaterial("matte", mp, toWorld);
	if (!mtl)
		throw std::exception("unable to create matte material?!");

	return mtl;
}

#pragma endregion 