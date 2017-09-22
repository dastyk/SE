#include <PathParsing.h>
#include "HLSLParser.h"
#include <fstream>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "TextureDesc.h"

namespace fs = std::experimental::filesystem;
using namespace SE;
using namespace std;

int ImageParse(const char* filename, const char* outputFilename);

int main(int argc, char* argv[])
{

	if (argc < 3)
	{
		printf("Usage: %s <source directory> [target directory]\n", argv[0]);
	}
	else
	{
		Parsers::HLSLParser hlslp;
		vector<Utilz::File> files;
		Utilz::get_all_files_names_within_folder(argv[1], files);
		for (auto& f : files)
		{
			if (Utilz::getExtension(f.name) == "hlsl")
			{
				printf("Parsing file: %s...\n", f.name.c_str());
				
				if (hlslp.Parse(Utilz::utf8ToUtf16(f.fullPath).c_str(), (std::string(argv[2]) + "\\"+  Utilz::removeRoot(f.fullPath)).c_str()))
					printf("Could not parse: %s\n", f.name.c_str());
			}
			else if(Utilz::getExtension(f.name) == "png" || Utilz::getExtension(f.name) == "jpg" || Utilz::getExtension(f.name) == "tga")
			{
				printf("Parsing file: %s...\n", f.name.c_str());
				if (ImageParse(f.fullPath.c_str(), (std::string(argv[2]) + "\\" + Utilz::removeRoot(f.fullPath.substr(0, f.fullPath.size() - Utilz::getExtension(f.name).size()) + "sei")).c_str()))
					printf("Could not parse: %s\n", f.name.c_str());
				else
					f.fullPath.replace(f.fullPath.size() - Utilz::getExtension(f.name).size(), Utilz::getExtension(f.name).size(), "sei");
			}
			else
			{
				std::ifstream  src(f.fullPath, std::ios::binary);

				std::string path = (std::string(argv[2]) + "/" + Utilz::removeRoot(f.fullPath)).c_str();
				auto idx = path.find_last_of("\\/");
				auto path2 = path.substr(0, idx);
				fs::create_directories(path2);

				std::ofstream  dst(path, std::ios::binary | std::ios::trunc);

				dst << src.rdbuf();
				src.close();
				dst.close();
			}
		}


		std::ofstream gE;
		gE.open("rawLoaderEntries.txt", std::ios::trunc);
		if (gE.is_open())
		{
			for (auto& f : files)
				gE << (std::string(argv[2]) + "/" + Utilz::removeRoot(f.fullPath)) << std::endl;
		}


	}
	
	return 0;
}


int ImageParse(const char* filename, const char* outputFilename)
{
	Graphics::TextureDesc texDesc;
	int x, y, n;
	stbi_uc* pixelData = stbi_load(filename, &x, &y, &n, 4);
	texDesc.width = x;
	texDesc.height = y;

	std::ofstream outfile;
	std::string path = outputFilename;
	auto idx = path.find_last_of("\\/");
	path = path.substr(0, idx);
	fs::create_directories(path);
	outfile.open(outputFilename, std::ios::binary | std::ios::trunc | std::ios::out);
	if (!outfile.is_open())
	{
		stbi_image_free(pixelData);
		printf("Wow, %s\n", outputFilename);
		return -1;
	}

	outfile.write((char*)&texDesc, sizeof(texDesc));
	outfile.write((char*)pixelData, x * y * 4);

	outfile.close();

	stbi_image_free(pixelData);

	return 0;
}