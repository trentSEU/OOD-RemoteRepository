/////////////////////////////////////////////////////////////////////
// ServerPrototype.cpp - mock server                               //
// ver 2.0                                                         //
// Language:       C++, Visual Studio 2017                         //
// Platform:       Macbook pro, Windows 10 Home                    //
// Application:    Spring 2018 CSE687 Project#2                    //
// Author:         Yuan Liu, yliu219@syr.edu                       //
// Referrence:     Jim Fawcett, CSE687                             //
/////////////////////////////////////////////////////////////////////

#include "Server.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include <chrono>

namespace MsgPassComm = MsgPassingCommunication;

using namespace Repository;
using namespace FileSystem;
using Msg = MsgPassingCommunication::Message;

//----< get all the file name in the target path >-----------------

Files Server::getFiles(const Repository::SearchPath& path)
{
  return Directory::getFiles(path);
}
//----< get all the dir name in the target path >------------------

Dirs Server::getDirs(const Repository::SearchPath& path)
{
  return Directory::getDirectories(path);
}
//----< display the given message >--------------------------------

template<typename T>
void show(const T& t, const std::string& msg)
{
  std::cout << "\n  " << msg.c_str();
  for (auto item : t)
  {
    std::cout << "\n    " << item.c_str();
  }
}
//----< generate and send an echo message >------------------------

std::function<Msg(Msg)> echo = [](Msg msg) {
  Msg reply = msg;
  reply.to(msg.from());
  reply.from(msg.to());
  return reply;
};
//----< generate and send an replyPath message >------------------

std::function<Msg(Msg)> replyPath = [](Msg msg) {
	std::string path = serverStorage + "/" + msg.value("pacName");
	if (_access(path.c_str(), 0) == -1) {
		std::cout << "  " << path << " is not existing, making it now" << std::endl;
		int flag = _mkdir(path.c_str());
		if (flag != 0) {
			std::cout << "  fail to make" << std::endl;
		}
	}
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("replyPath");
	reply.attribute("receivePath", path);
	return reply;
};
//----< generate and send an checkInFile message >------------------

std::function<Msg(Msg)> checkInFile = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("");
	return reply;
};
//----< generate and send an message to reply getFiles >-----------

std::function<Msg(Msg)> getFiles = [](Msg msg) {
  Msg reply;
  reply.to(msg.from());
  reply.from(msg.to());
  reply.command("getFiles");
  std::string path = msg.value("path");
  if (path != "")
  {
    std::string searchPath = storageRoot;
    if (path != ".")
      searchPath = searchPath + "\\" + path;
    Files files = Server::getFiles(searchPath);
    size_t count = 0;
    for (auto item : files)
    {
      std::string countStr = Utilities::Converter<size_t>::toString(++count);
      reply.attribute("file" + countStr, item);
    }
  }
  else
  {
    std::cout << "\n  getFiles message did not define a path attribute";
  }
  return reply;
};
//----< generate and send a message to reply getDirs >-------------

std::function<Msg(Msg)> getDirs = [](Msg msg) {
  Msg reply;
  reply.to(msg.from());
  reply.from(msg.to());
  reply.command("getDirs");
  std::string path = msg.value("path");
  if (path != "")
  {
    std::string searchPath = storageRoot;
    if (path != ".")
      searchPath = searchPath + "\\" + path;
    Files dirs = Server::getDirs(searchPath);
    size_t count = 0;
    for (auto item : dirs)
    {
      if (item != ".." && item != ".")
      {
        std::string countStr = Utilities::Converter<size_t>::toString(++count);
        reply.attribute("dir" + countStr, item);
      }
    }
  }
  else
  {
    std::cout << "\n  getDirs message did not define a path attribute";
  }
  return reply;
};

int main()
{
	system("Title Server");
  std::cout << "\n  Testing Server";
  std::cout << "\n ==========================";
  std::cout << "\n";

	DbCore<PayLoad> db;
  Server server(serverEndPoint, "Server", db);
	server.clearDirs();
	server.checkDir();
  server.start();

  server.addMsgProc("echo", echo);
  server.addMsgProc("getFiles", getFiles);
  server.addMsgProc("getDirs", getDirs);
  server.addMsgProc("serverQuit", echo);
	server.addMsgProc("getPath", replyPath);
	server.addMsgProc("CheckInFile", checkInFile);
  server.processMessages();
  return 0;
}

