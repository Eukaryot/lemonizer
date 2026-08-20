/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#define RMX_LIB
#include "pch.h"
#include "helper/PlatformFunctions.h"
#include "builder/KnowledgeBase.h"
#include "builder/ProjectData.h"
#include "builder/RomContent.h"
#include "writer/HtmlWriter.h"


int main(int argc, char** argv)
{
	INIT_RMX;

	// Make sure we're in the correct working directory
	PlatformFunctions::changeWorkingDirectory(argc == 0 ? "" : argv[0]);

	// Read the config
	std::wstring projectName;
	{
		Json::Value root = rmx::JsonHelper::loadFile(L"config.json");
		if (root.isNull())
		{
			RMX_ERROR("Failed to load config.json, it's either not present or invalid JSON", );
			return 0;
		}

		rmx::JsonHelper rootHelper(root);
		if (!rootHelper.tryReadString("Project", projectName))
		{
			RMX_ERROR("No project name defined in config.json", );
			return 0;
		}
	}
	std::cout << "Using project: " << WString(projectName).toStdString() << std::endl;
	std::wstring projectPath = L"projects/" + projectName + L'/';

	// Load the project JSON
	ProjectData projectData;
	if (!projectData.loadFromProjectFile(projectPath + L"project.json"))
		return 0;
	projectData.mOutputPath = projectPath;

	// Load and analyze the ROM
	std::cout << "Loading ROM or binary '" << WString(projectPath + projectData.mRomFile).toStdString() << "'" << std::endl;
	RomContent romContent;
	if (!romContent.loadRom(projectPath + projectData.mRomFile))
		return 0;

	// Knowledge base stays empty for now (though we could fill it in with definitions loaded from lemonscript or a JSON)
	KnowledgeBase knowledgeBase;

	// Write output
	std::cout << "Writing output files into project directory '" << WString(projectPath).toStdString() << "'" << std::endl;
	FunctionBuilder functionBuilder(romContent);
	HtmlWriter htmlWriter(functionBuilder);
	htmlWriter.writeOutput();

	std::cout << "Done" << std::endl;
	return 0;
}
