#include "OmniversePrivatePCH.h"
#include "OVDebugLogs.h"


OVDebug::OVDebug()
{
	FDateTime Date;
	Date = FDateTime::Now();

	DirectoryCreated = false;
	
	VersionNumber = "v2.0.4";
	FileCreationDate = Date.GetDate().ToString();
	FileCreationDate.RemoveAt(10, FileCreationDate.Len() - 10);
	FileName = "OV_" + VersionNumber + "_" + FileCreationDate + ".txt";
	SaveDirectory = "C:\\Omniverse-Logs\\";
}


bool OVDebug::AddToFile(FString AddedContents)
{
	if (!DirectoryCreated)
		CreateFileDirectory();

	LoadCurrentFile();

	FString carriageReturn = "\n\r";

	FileContents += carriageReturn;
	FileContents += AddedContents;

	FString AbsoluteFilePath = SaveDirectory + FileName;

	return FFileHelper::SaveStringToFile(FileContents, *AbsoluteFilePath);
}


bool OVDebug::LoadCurrentFile()
{	
	return FFileHelper::LoadFileToString(FileContents, *FileName);
}


bool OVDebug::CreateFileDirectory()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	DirectoryCreated = true;

	// Directory Exists?
	if (!PlatformFile.DirectoryExists(*SaveDirectory))
	{
		PlatformFile.CreateDirectory(*SaveDirectory);

		if (!PlatformFile.DirectoryExists(*SaveDirectory))
		{
			DirectoryCreated = false;
		}
	}

	return DirectoryCreated;
}


bool OVDebug::InitializeDebugFile()
{
	FString carriageReturn = "\n\r";
	FString FileStart = "--------------------Version Number [" + VersionNumber + "]--------------------" + carriageReturn;

	return AddToFile(FileStart);
}


bool OVDebug::ParseCommandLineForDebug(FString CommandLineArgs)
{
	FString hyphenChar = "-";
	TArray<FString> ParseResults;

	CommandLineArgs.ParseIntoArray(ParseResults, *hyphenChar, true);

	for (int index = 0; index < ParseResults.Num(); index++)
	{
		FString DebugCommandLine = "PrintLogFile";

		if (ParseResults[index] == DebugCommandLine)
			return true;
	}

	return false;
}
