#pragma once

// 4J Stu - Represents Java standard library class


#include "util/java/File.h"

#include "InputStream.h"

class FileInputStream : public InputStream {
public:
    FileInputStream(const File& file);
    virtual ~FileInputStream();
    virtual int  read();
    virtual int  read(byteArray b);
    virtual int  read(byteArray b, unsigned int offset, unsigned int length);
    virtual void close();
    virtual std::int64_t skip(std::int64_t n);

private:
    HANDLE m_fileHandle;
};
