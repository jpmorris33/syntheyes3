#include "Platform.hpp"

class TarFileIO : public FileIO {
	public:
		TarFileIO(const unsigned char *src, long length);
		~TarFileIO();
		bool open(const char *filename, const char *mode);
		void close();
		long read(void *buffer, long bytes);
		long write(const void *buffer, long bytes);
		char *readLine(char *buffer, long bytes);
		void seek(long offset);
		long tell();
		bool eof();
	private:
		const unsigned char *datasrc;
		long datalen;
};
