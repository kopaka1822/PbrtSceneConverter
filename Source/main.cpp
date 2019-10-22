#include <iostream>
#include <memory>
#include "PBRT/PbrtScene.h"
#include "parser.h"
#include "file.h"
#include "system.h"
#include "DialogOpenFile.h"
#include <chrono>
#include "ArgumentSet.h"

const auto g_helpstring = 
"arguments: input_pbrt output [optional args]\n" \
"e.g.: scenes/book.pbrt converted/book --pause\n"\
"optional arguments:\n"\
"		--pause (programm will pause after execution)\n"\
"		--noconvert (this will just try to read the file without converting)\n"\
"		--uiselect (this will ignore the input file and will let you choose the file via ui)\n"\
"		--errpause (this will pause after an error occured)\n"\
"		--silent (this will supress warnings, infos and runtime infos during conversion)\n"\
"		--dirhierarchy (assumes that filepaths are relative to the current .pbrt file)\n"\
"		--swapaxis [a1] [a2] ([a1] [a2]...) (swaps to the given axis: --swapaxis x z)\n"\
"       --autoedge [degree] uses the triangle normal for a vertex if the angle between triangle vertex and proposed normal is bigger than [degree]"\
"       --autoflat creates flat normals for a model if no normals are present";

void handleSwapAxisParam(const std::vector<std::string>& axis);
void doAxisSwap(PbrtScene& scene);

int main(int argc, char** argv)
{
	const char* decoLine = "*----------------------------------*\n";
	try
	{
		System::init();
		if (argc < 3)
		{
			System::error("insufficient arguments, please provide input file + output file name");
			System::runtimeInfo(g_helpstring);
			return 1;
		}

		System::args.init(argc - 2, argv + 2);
		System::argSilent = System::args.has("silent");
		std::cerr << std::boolalpha; // output bools as true or false

		if (System::args.has("swapaxis"))
			handleSwapAxisParam(System::args.getVector<std::string>("swapaxis"));

		System::setOutputDirectory(argv[2]);

		std::string sceneFile = argv[1];
		if(System::args.has("uiselect"))
		{
			DialogOpenFile dlg = DialogOpenFile("pbrt");
			dlg.Show();
			if(dlg.IsSuccess())
			{
				sceneFile = dlg.GetName();
			}
		}

		PbrtScene pbrtScene;
		
		std::cerr << decoLine << "INFO parsing scene\n" << decoLine;
		if(parseFile(sceneFile, pbrtScene, "") && !System::args.has("noconvert"))
		{
			// swap axis for scene geomentry if required
			if (System::hasAxisSwap())
				doAxisSwap(pbrtScene);

			// TODO convert to your own scene format here
			// pbrtScene: c++ scene description
			// argv[2] destination filename

			std::cerr << decoLine << "INFO finished converting\n" << decoLine;
		}
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	std::cerr << decoLine << "INFO SUMMARY\n" << decoLine;
	System::displayErrors();
	System::displayWarnings();
	System::displayInfos();

	if (System::args.has("pause"))
		system("pause");

	return 0;
}

void handleSwapAxisParam(const std::vector<std::string>& args)
{
	for (int i = 0; i + 1 < args.size(); i += 2)
	{
		auto getAxis = [](char c)
		{
			switch (c)
			{
			case 'x': return 0;
			case 'y': return 1;
			case 'z': return 2;
			}
			return -1;
		};
		auto a1 = getAxis(args.at(i).at(0));
		auto a2 = getAxis(args.at(i + 1).at(0));
		if (a1 == -1 || a2 == -1)
		{
			System::warning("invalid axis supplied. Only x y and z are valid.");
			continue;
		}
		System::setAxisSwap(a1, a2);
	}
}

void doAxisSwap(PbrtScene& scene)
{
	System::info("swapping axis");

	for (auto& o : scene.getRenderOptions().shapes)
		o->applyTransformFront(System::getAxisSwap());

	for (auto& l : scene.getRenderOptions().lights)
	{
		l.position = Shape::transformPoint(l.position, System::getAxisSwap());
		l.dir = Shape::transformVector(l.dir, System::getAxisSwap());
		l.from = Shape::transformPoint(l.from, System::getAxisSwap());
		l.to = Shape::transformPoint(l.to, System::getAxisSwap());
	}

	scene.getRenderOptions().cameraLookAt = Shape::transformPoint(scene.getRenderOptions().cameraLookAt, System::getAxisSwap());
	scene.getRenderOptions().cameraPos = Shape::transformPoint(scene.getRenderOptions().cameraPos, System::getAxisSwap());
}