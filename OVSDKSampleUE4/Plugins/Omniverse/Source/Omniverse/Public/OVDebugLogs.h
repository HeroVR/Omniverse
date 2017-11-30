#pragma once
/*
 This class is used to print the Debug strings in the Plugin.
 This will help CPs (content providers) see what the Plugin is doing,
 and if it is broken.
*/


class OVDebug
{
public:

	OVDebug();
	~OVDebug() {};



	//Functions
	bool AddToFile(FString AddedContents);
	bool LoadCurrentFile();
	bool InitializeDebugFile();
	bool CreateFileDirectory();
	bool ParseCommandLineForDebug(FString CommandLineArgs);

	//Variables
	FString VersionNumber;
	FString FileCreationDate;
	FString FileName;
	FString SaveDirectory;
	FString FileContents;

	bool DirectoryCreated = false;
};
