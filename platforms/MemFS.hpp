#include "Platform.hpp"

class MemFS : public FileIO {
	public:
		MemFS(unsigned char *src, long length);
		~MemFS();
		bool open(const char *filename, const char *mode);
		void close();
		long read(void *buffer, long bytes);
		long write(const void *buffer, long bytes);
		char *readLine(char *buffer, long bytes);
		void seek(long offset);
		long tell();
		bool eof();
	private:
		unsigned char *startptr;
		unsigned char *endptr;
		unsigned char *curptr;
		long filelen;
};
