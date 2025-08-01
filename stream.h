#pragma once
namespace ph
{
	struct stream
	{
		FILE* f;

		void openWriteFileStream(const char* filename);
		bool openReadFileStream(const char* filename);
		void closeFileStream();
		void writeInt32(int i);
		int readInt32();
		void writeUint32(uint i);
		uint readUint32();
		void writeUint64(ulong i);
		ulong readUint64();
		void writeByte(byte b);
		byte readByte();
		void write(void* src, uint len);
		void read(void* dst, uint len);
		uint getpos();
		void setpos(uint pos);
	};
}