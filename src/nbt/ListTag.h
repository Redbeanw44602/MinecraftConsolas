#pragma once

#include "util/java/InputOutputStream/DataInput.h"
#include "util/java/InputOutputStream/DataOutput.h"

#include "Tag.h"

template <class T>
class ListTag : public Tag {
private:
    std::vector<Tag*> list;
    byte              type;

public:
    ListTag() : Tag(L"") {}
    ListTag(const std::wstring& name) : Tag(name) {}

    void write(DataOutput* dos) {
        if (list.size() > 0) type = (list[0])->getId();
        else type = 1;

        dos->writeByte(type);
        dos->writeInt((int)list.size());

        auto itEnd = list.end();
        for (auto it = list.begin(); it != itEnd; it++) (*it)->write(dos);
    }

    void load(DataInput* dis) {
        type     = dis->readByte();
        int size = dis->readInt();

        list.clear();
        for (int i = 0; i < size; i++) {
            Tag* tag = Tag::newTag(type, L"");
            tag->load(dis);
            list.push_back(tag);
        }
    }

    byte getId() { return TAG_List; }

    std::wstring toString() {
        static wchar_t buf[64];
        swprintf(
            buf,
            64,
            L"%d entries of type %ls",
            list.size(),
            Tag::getTagName(type)
        );
        return std::wstring(buf);
    }

    void print(char* prefix, std::ostream out) {
        Tag::print(prefix, out);

        out << prefix << "{" << std::endl;

        char* newPrefix = new char[strlen(prefix) + 4];
        strcpy(newPrefix, prefix);
        strcat(newPrefix, "   ");
        auto itEnd = list.end();
        // FIXME: Incompatible ostream -> wostream.
        // for (auto it = list.begin(); it != itEnd; it++)
        //     (*it)->print(newPrefix, out);
        delete[] newPrefix;
        out << prefix << "}" << std::endl;
    }

    void add(T* tag) {
        type = tag->getId();
        list.push_back(tag);
    }

    T* get(int index) { return (T*)list[index]; }

    int size() { return (int)list.size(); }

    virtual ~ListTag() {
        auto itEnd = list.end();
        for (auto it = list.begin(); it != itEnd; it++) {
            delete *it;
        }
    }

    Tag* copy() {
        ListTag<T>* res = new ListTag<T>(getName());
        res->type       = type;
        auto itEnd      = list.end();
        for (auto it = list.begin(); it != itEnd; it++) {
            T* copy = (T*)(*it)->copy();
            res->list.push_back(copy);
        }
        return res;
    }

#if 0
	bool equals(Object obj)
	{
		if (Tag::equals(obj))
		{
			ListTag *o = (ListTag *) obj;
			if (type == o->type)
			{
				bool equal = false;
				if(list.size() == o->list.size())
				{
					equal = true;
					auto itEnd = list.end();
					// 4J Stu - Pretty inefficient method, but I think we can live with it give how often it will happen, and the small sizes of the data sets
					for (auto it = list.begin(); it != itEnd; it++)
					{
						bool thisMatches = false;
						for(auto it2 = o->list.begin(); it != o->list.end(); ++it2)
						{
							if((*it)->equals(*it2))
							{
								thisMatches = true;
								break;
							}
						}
						if(!thisMatches)
						{
							equal = false;
							break;
						}
					}
				}

				//return list->equals(o->list);
				return equal;
			}
		}
		return false;
	}
#endif
};
