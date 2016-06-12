#include <iostream>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
std::pair<int,int> ParseRevisions(std::string revisions)
{
	revisions.replace(revisions.find(":"),1," ");
	int start,finish;
	std::istringstream(revisions)>>start>>finish;
	return {start,finish};
}
void Execute(const std::string &command)
{
	system(("\""+command+"\"").c_str());
}
std::vector<std::string> EnumerateDirectoryEntries(
	const std::string &directory)
{
	using TFileIterator=std::tr2::sys::directory_iterator;
	std::vector<std::string> entries;
	for(auto file=TFileIterator(directory);file!=TFileIterator();++file)
	{
		std::string fileName=file->path().filename().string();
		if(fileName.find('"')==0)
			fileName=fileName.substr(1,fileName.length()-2);
		entries.push_back(fileName);
	}
	return entries;
}
void UpdateWorkingCopy(const std::string &location,int revision)
{
	Execute("svn update \""+location+"\" -r "+std::to_string(revision));
}
void UpdateWorkingCopyToHeadRevision(const std::string &location)
{
	Execute("svn update \""+location+"\"");
}
std::string GetLogMessage(const std::string &location,int revision)
{
	const std::string tempFile="temp";
	Execute(
		"svn log \""+location+"\" -r "+std::to_string(revision)
		+" > "+tempFile);
	std::string logMessage;
	{
		std::ifstream reader(tempFile);
		std::string trash;
		for(int c=0;c<3;++c)
			std::getline(reader,trash);
		std::getline(reader,logMessage);
	}
	std::remove(tempFile.c_str());
	return logMessage;
}
void Commit(const std::string &location,const std::string &message)
{
	Execute("svn commit \""+location+"\" --message \""+message+"\"");
}
void DeleteDirectoryContent(const std::string &path)
{
	for(const auto &fileName:EnumerateDirectoryEntries(path))
		std::remove((path+"\\"+fileName).c_str());
}
void CopyDirectoryContent(
	const std::string &source,
	const std::string &destination)
{
	for(const auto &fileName:EnumerateDirectoryEntries(source))
	{
		if(fileName==".svn")
			continue;
		std::tr2::sys::copy_file(
			source+"\\"+fileName,
			destination+"\\"+fileName);
	}
}
void AddAllFilesToSvn(const std::string &workingCopy)
{
	for(const auto &fileName:EnumerateDirectoryEntries(workingCopy))
		Execute("svn add \""+workingCopy+"\\"+fileName+"\"");
}
int main(int argc,char *argv[])
{
	if(argc!=4)
	{
		std::cout
			<<"usage: svn-copy source destination start-revision:finish-revision"<<std::endl
			<<"\t"<<"source and destination should be clean working copies of the"<<std::endl
			<<"\t"<<"corresponding locations"<<std::endl;
		return 0;
	}
	const std::string source=argv[1];
	const std::string destination=argv[2];
	const auto revisions=ParseRevisions(argv[3]);
	UpdateWorkingCopyToHeadRevision(destination);
	for(int revision=revisions.first;revision<=revisions.second;++revision)
	{
		std::clog<<revision<<std::endl;
		UpdateWorkingCopy(source,revision);
		DeleteDirectoryContent(destination);
		CopyDirectoryContent(source,destination);
		AddAllFilesToSvn(destination);
		Commit(destination,GetLogMessage(source,revision));
	}
    return 0;
}
