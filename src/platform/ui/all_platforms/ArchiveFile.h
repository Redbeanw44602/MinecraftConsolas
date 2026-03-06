#pragma once

#include <unordered_map>
#include <vector>

#include "util/console/ArrayWithLength.h"
#include "util/java/File.h"

class DataInputStream;

class ArchiveFile {
protected:
    File  m_sourcefile;
    BYTE* m_cachedData;

    typedef struct _MetaData {
        std::wstring filename;
        int          ptr;
        int          filesize;
        bool         isCompressed;

    } MetaData, *PMetaData;

    std::unordered_map<std::wstring, PMetaData> m_index;

public:
    void _readHeader(DataInputStream* dis);

    ArchiveFile(File file);
    ~ArchiveFile();

    std::vector<std::wstring>* getFileList();
    bool                       hasFile(const std::wstring& filename);
    int                        getFileSize(const std::wstring& filename);
    byteArray                  getFile(const std::wstring& filename);
};
