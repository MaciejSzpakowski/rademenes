#include <cstdio>
#include "types.h"
#include "stream.h"
#include <cstring>
#include "gl.h"

namespace ph
{
	void stream::openWriteFileStream(const char* filename)
	{
		this->f = fopen(filename, "wb");
		massert(f, "Could not open file");
	}

	bool stream::openReadFileStream(const char* filename)
	{
		this->f = fopen(filename, "rb");
		return !!f;
	}

	void stream::closeFileStream()
	{
		fclose(this->f);
	}

	void stream::writeInt32(int i)
	{
		fwrite(&i, 4, 1, this->f);
	}

	int stream::readInt32()
	{
		int i;
		fread(&i, 4, 1, this->f);
		return i;
	}

	void stream::writeUint32(uint i)
	{
		fwrite(&i, 4, 1, this->f);
	}

	uint stream::readUint32()
	{
		uint i;
		fread(&i, 4, 1, this->f);
		return i;
	}

	void stream::writeUint64(ulong i)
	{
		fwrite(&i, 8, 1, this->f);
	}

	ulong stream::readUint64()
	{
		ulong i;
		fread(&i, 8, 1, this->f);
		return i;
	}

	void stream::writeByte(byte i)
	{
		fwrite(&i, 1, 1, this->f);
	}

	byte stream::readByte()
	{
		byte i;
		fread(&i, 1, 1, this->f);
		return i;
	}

	void stream::write(void* src, uint len)
	{
		fwrite(src, len, 1, this->f);
	}

	void stream::read(void* dst, uint len)
	{
		fread(dst, len, 1, this->f);
	}

	uint stream::getpos()
	{
		return ftell(this->f);
	}

	void stream::setpos(uint pos)
	{
		fseek(this->f, pos, SEEK_SET);
	}
}