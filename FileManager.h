#pragma once

#include <fstream>
#include <filesystem>
#include "DTypes.h"

class FileManager
{
	std::filesystem::path path;

public:

	std::fstream file;

	FileManager() {}
	~FileManager()
	{
		Close();
	}
	
	FileManager(const FileManager&) = delete;
	FileManager(FileManager&&) = delete;

	FileManager& operator=(const FileManager&) = delete;
	FileManager& operator=(FileManager&&) = delete;

	bool Open(int mode)
	{
		file.open(path.string(), mode);
		return file.is_open();
	}

	void Close()
	{
		if (file.is_open()) file.close();
	}

	void SetPath(const std::string& fname)
	{
		path = std::filesystem::current_path() / fname;
	}

	template <typename _Data>
	void ReadElem(Elem<_Data>& elem)
	{
		file.seekg(elem.pos);
		file.read(reinterpret_cast<char*>(&elem.data), sizeof(_Data));
		if (file.fail()) throw std::runtime_error("error occurred in [FileManager.h]FileManager::ReadElem(): reached end of file and tried to read at position out of range");
	}

	template <typename _Data>
	void WriteElem(const Elem<_Data>& elem)
	{
		file.seekp(elem.pos);
		file.write(reinterpret_cast<const char*>(&elem.data), sizeof(_Data));
		if (file.fail()) throw std::runtime_error("error occurred in [FileManager.h]FileManager::WriteElem(): can't write to file at expected position");
	}

	void SetWritePosToEnd()
	{
		file.seekp(0, std::ios::end);
	}

	void ReadByte(BYTE& b)
	{
		file.read(reinterpret_cast<char*>(&b), 1);
	}

	void WriteByte(const BYTE& b)
	{
		file.put(b);
	}

	void Resize(std::uintmax_t new_size)
	{
		std::filesystem::resize_file(path, new_size);
	}
};
