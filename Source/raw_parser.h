#include "parser.h"
#include <iostream>
#include <vector>
#include "parser_helper.h"
#include "PBRT/TextureParams.h"
#include "system.h"

#define API_MODUL(func) auto name = getString(cursor,end); ParamSet set; initParamSet(set, cursor,end); \
						scene.func(name,set);

/*!re2c re2c:define:YYCTYPE = "char"; */

void parse(const std::unique_ptr<char[]>& data, size_t length, PbrtScene& scene, std::string directory, const std::string& filename)
{
	System::setDirectory(directory);
	// special case for --nodirhierarchy
	directory = System::getCurrentDirectory();

	const char* cursor = data.get();
	const char* const end = data.get() + length;
	const char* marker = nullptr;
	System::runtimeInfo("parsing " + filename);
	System::runtimeInfo("removing comments");
	// remove comments
	{
		bool isComment = false;
		bool isString = false;
		char* c = data.get();
		while (c < end)
		{
			if(isComment)
			{
				if (*c == '\n')
					isComment = false;
				else
					*c = ' ';
			}
			else if(*c == '#' && !isString)
			{
				isComment = true;
				*c = ' ';
			}
			else if(*c == '"')
			{
				// dont comment out # in strings ("map #3")
				isString = !isString;
			}
			c++;
		}
	}
	System::runtimeInfo("parsing commands");
	try
	{
		while (cursor < end && *cursor)
		{
			marker = nullptr;
			/*!re2c
			re2c:define:YYCURSOR = cursor;
			re2c:define:YYMARKER = marker;
			re2c:define:YYLIMIT = end;
			re2c:yyfill:enable = 0;



			*	{													continue; }
			"#"	{ while(*cursor && *cursor != '\n') cursor++;		continue; }

			"Identity"	{ scene.apiIdentity();						continue; }
			"Translate"	{ scene.apiTranslate(getFloats(3,cursor,end));		continue; }
			"Scale"		{ scene.apiScale(getFloats(3,cursor,end));	continue; }
			"Rotate"	{ scene.apiRotate(getFloats(4,cursor,end));	continue; }
			"LookAt"	{ scene.apiLookAt(getFloats(9,cursor,end));	continue; }
			"Transform"	{ scene.apiTransform(getFloats(4 * 4,cursor,end));		continue; }
			"ConcatTransform"	{ scene.apiConcatTransfrom(getFloats(4 * 4,cursor,end));	continue; }
			"ActiveTransform StartTime" { scene.apiSetActiveTransform(true);	continue; }
			"ActiveTransform EndTime" { scene.apiSetActiveTransform(false);		continue; }
			"ActiveTransform All"	{ scene.apiSetActiveTransform(true);		continue; }
			"CoordinateSystem"		{ scene.apiCoordinateSystem(getString(cursor,end));	continue; }
			"CoordSysTransform"		{ scene.apiCoordSysTransform(getString(cursor,end)); continue;}

			"AttributeBegin"	{ scene.apiAttributeBegin();		continue; }
			"AttributeEnd"		{ scene.apiAttributeEnd();			continue; }
			"TransformBegin"	{ scene.apiTransformBegin();		continue; }
			"TransformEnd"		{ scene.apiTransformEnd();			continue; }
			"WorldBegin"		{ scene.apiWorldBegin();			continue; }
			"WorldEnd"			{ scene.apiWorldEnd();				break; }

			"Camera"			{ API_MODUL(apiCamera);				continue; }
			"Sampler"			{ API_MODUL(apiSampler);			continue; }
			"Film"				{ API_MODUL(apiFilm);				continue; }
			"Renderer"			{ API_MODUL(apiRenderer);			continue; }
			"SurfaceIntegrator"	{ API_MODUL(apiSurfaceIntegrator);	continue; }
			"VolumeIntegrator"	{ API_MODUL(apiVolumeIntegrator);	continue; }
			"Accelerator"		{ API_MODUL(apiAccelerator);		continue; }
			"PixelFilter"		{ API_MODUL(apiPixelFilter);		continue; }
			
			"Shape"				{ API_MODUL(apiShape);				continue; }
			"ObjectBegin"		{ scene.apiObjectBegin(getString(cursor,end)); continue; }
			"ObjectEnd"			{ scene.apiObjectEnd();				continue; }
			"ObjectInstance"	{ scene.apiObjectInstance(getString(cursor,end)); continue; }
			"LightSource"		{ API_MODUL(apiLightSource);		continue; }
			"AreaLightSource"	{ API_MODUL(apiAreaLightSource);	continue; }
			"ReverseOrientation"{ scene.apiReverseOrientation();	continue; }
			"Material"			{ API_MODUL(apiMaterial);			continue; }
			"MakeNamedMaterial"	{ API_MODUL(apiMakeNamedMaterial);	continue; }
			"NamedMaterial"		{ scene.apiNamedMaterial(getString(cursor,end)); continue; }
			"Volume"			{ API_MODUL(apiVolume);				continue; }
			"Texture"			{
									auto name = getString(cursor,end);
									auto type = getString(cursor,end);
									auto clas = getString(cursor,end);
									ParamSet set; initParamSet(set, cursor,end);
									scene.apiTexture(name,type,clas,set);
									continue;
								}

			"Include"			{	
									auto filename = getString(cursor, end);
									parseFile(filename,scene,directory); 
									System::setDirectory(directory);
									continue; 	
								}
			
			*/
		}
	}
	catch(const ParserException& e)
	{
		// determine line
		size_t line = 1;
		const char* cur = data.get();
		while (cur < e.where()) { if (*cur == '\n')++line;++cur; }
		System::error("line: " + std::to_string(line) + " : " + std::string(e.what()) + " in file: " + filename);
	}
	catch(const std::exception& e)
	{
		// determine line
		size_t line = 1;
		const char* cur = data.get();
		while (cur < cursor) { if (*cur == '\n')++line; ++cur; }
		System::error("line: " + std::to_string(line) + " : " + std::string(e.what()) + " in file: " + filename);
	}
}

pbrtParamType getParamTypeFromString(const std::string& s)
{
	const char* cursor = &(*s.begin());
	const char* end = cursor + s.length();
	const char* marker = nullptr;
	/*!re2c
		re2c:define:YYCURSOR = cursor;
		re2c:define:YYLIMIT = end;
		re2c:define:YYMARKER = marker;
		re2c:yyfill:enable = 0;
		
		*		{return pbrtParamType::ERROR;}
		"integer" {return pbrtParamType::Integer; }
		"float" {return pbrtParamType::Float; }
		"bool" {return pbrtParamType::Bool; }
		"point" {return pbrtParamType::Point; }
		"vector" {return pbrtParamType::Vector; }
		"normal" {return pbrtParamType::Normal; }
		"string" {return pbrtParamType::String; }
		"spectrum" {return pbrtParamType::Spectrum; }
		"color" {return pbrtParamType::Color; }
		"rgb" {return pbrtParamType::RGB; }
		"xyz" {return pbrtParamType::XYZ; }
		"texture" {return pbrtParamType::Texture; }
		"blackbody" {return pbrtParamType::Spectrum; }
	 */
	return pbrtParamType::ERROR;
}

PbrtScene::ShapeType getShapeTypeFromString(const std::string& s)
{
	const char* cursor = &(*s.begin());
	const char* end = cursor + s.length();
	const char* marker = nullptr;
	/*!re2c
	re2c:define:YYCURSOR = cursor;
	re2c:define:YYLIMIT = end;
	re2c:define:YYMARKER = marker;
	re2c:yyfill:enable = 0;

	*		{return PbrtScene::ShapeType::ERROR;}
	"cone" {return PbrtScene::ShapeType::Cone; }
	"cylinder" {return PbrtScene::ShapeType::Cylinder; }
	"disk" {return PbrtScene::ShapeType::Disk; }
	"hyperboloid" {return PbrtScene::ShapeType::Hyperboloid; }
	"heightfield" {return PbrtScene::ShapeType::Heightfield; }
	"loopsubdiv" {return PbrtScene::ShapeType::Loopsubdiv; }
	"nurbs" {return PbrtScene::ShapeType::Nurbs; }
	"paraboloid" {return PbrtScene::ShapeType::Paraboloid; }
	"sphere" {return PbrtScene::ShapeType::Sphere; }
	"trianglemesh" {return PbrtScene::ShapeType::Trianglemesh; }
	"plymesh" {return PbrtScene::ShapeType::Plymesh; }
	*/
	return PbrtScene::ShapeType::ERROR;
}

Light::Type getLightTypeFromString(const std::string& s)
{
	const char* cursor = &(*s.begin());
	const char* end = cursor + s.length();
	const char* marker = nullptr;
	/*!re2c
	re2c:define:YYCURSOR = cursor;
	re2c:define:YYLIMIT = end;
	re2c:define:YYMARKER = marker;
	re2c:yyfill:enable = 0;

	*		{return Light::Type::ERROR;}
	"distant" {return Light::Type::Distant; }
	"goniometric" {return Light::Type::Goniometric; }
	"infinite" {return Light::Type::Infinite; }
	"point" {return Light::Type::Point; }
	"projection" {return Light::Type::Projection; }
	"spot" {return Light::Type::Spot; }
	*/
	return Light::Type::ERROR;
}

Material::Type getMaterialTypeFromString(const std::string& s)
{
	if (s.length() == 0)
		return Material::None;

	const char* cursor = &(*s.begin());
	const char* end = cursor + s.length();
	const char* marker = nullptr;
	/*!re2c
	re2c:define:YYCURSOR = cursor;
	re2c:define:YYLIMIT = end;
	re2c:define:YYMARKER = marker;
	re2c:yyfill:enable = 0;

	*		{return Material::Type::ERROR;}
	"glass" {return Material::Type::Glass; }
	"kdsubsurface" {return Material::Type::Kdsubsurface; }
	"matte" {return Material::Type::Matte; }
	"measured" {return Material::Type::Measured; }
	"metal" {return Material::Type::Metal; }
	"mirror" {return Material::Type::Mirror; }
	"mix" {return Material::Type::Mix; }
	"plastic" {return Material::Type::Plastic; }
	"shinymetal" {return Material::Type::Shinymetal; }
	"substrate" {return Material::Type::Substrate; }
	"subsurface" {return Material::Type::Subsurface; }
	"translucent" {return Material::Type::Translucent; }
	"uber" {return Material::Type::Uber; }
	"hair" {return Material::Type::Hair; }
	"fourier" {return Material::Type::Fourier; }
	*/
	return Material::Type::ERROR;
}

size_t getTextureTypeFromString(const std::string& s)
{
	const char* cursor = &(*s.begin());
	const char* end = cursor + s.length();
	const char* marker = nullptr;
	/*!re2c
	re2c:define:YYCURSOR = cursor;
	re2c:define:YYLIMIT = end;
	re2c:define:YYMARKER = marker;
	re2c:yyfill:enable = 0;

	*			{return Texture<int>::Type::ERROR;}
	"bilerp"	{return Texture<int>::Type::Bilerp; }
	"checkerboard"	{return Texture<int>::Type::Checkerboard; }
	"constant"	{return Texture<int>::Type::Constant; }
	"dots"		{return Texture<int>::Type::Dots; }
	"fbm"		{return Texture<int>::Type::Fbm; }
	"imagemap"	{return Texture<int>::Type::Imagemap; }
	"marble"	{return Texture<int>::Type::Marble; }
	"mix"		{return Texture<int>::Type::Mix; }
	"scale"		{return Texture<int>::Type::Scale; }
	"uv"		{return Texture<int>::Type::Uv; }
	"windy"		{return Texture<int>::Type::Windy; }
	"wrinkled"	{return Texture<int>::Type::Wrinkled; }
	*/
	return Texture<int>::ERROR;
}