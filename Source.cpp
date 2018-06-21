#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <vector>
#include <inttypes.h>

#define LOGE printf
#define LOGD printf

#include "EBML.h"
#include "luaFunction.h"

class MKVReader {
public:
	virtual uint64_t tell() = 0;
	virtual void seek(uint64_t pos) = 0;
	virtual uint64_t size() = 0;
	virtual VINT readVInt(bool dropOne = false) = 0;
	virtual uint8_t * read(uint64_t size) = 0;
};

class FileReader 
	:public MKVReader
{
public:
	FileReader(FILE * fp)
	{
		fp_ = fp;
		uint64_t tell = ftell(fp);
		fseek(fp,0,SEEK_END);
		uint64_t fsize = ftell(fp);
		fseek(fp,tell,SEEK_SET);
		size_ = fsize;
	}

	uint64_t tell()
	{
		return ftell(fp_);
	}

	void seek(uint64_t pos) {
		fseek(fp_,pos,SEEK_SET);
	}

	uint64_t size() {
		return size_;
	}

	VINT readVInt(bool dropOne = false) {
		uint8_t value = 0;
		int ret = fread(&value, sizeof(value), 1, fp_);
		if (ret != 1)
		{
			LOGE("read error.\n");
			return -1;
		}
		int count = 0;
		uint8_t utf_8v = value;
		while ((utf_8v & 0x80) == 0)
		{
			utf_8v <<= 1;
			count++;

			if (count > 7) break;
		}
		utf_8v <<= 1;
		utf_8v >>= (count + 1);
		if (count > 7) {
			LOGE("BufferReader VINT max count(%d).", count);
			return -1;
		}

		VINT rValue = 0;
		char * v = (char*)&rValue;
		if (dropOne)
		{
			v[0] = utf_8v;
		}
		else {
			v[0] = value;
		}
		
		if (count == 0) return rValue;
		ret = fread(v + 1, count, 1, fp_);
		if (ret != 1)
		{
			LOGE("read error.\n");
			return -1;
		}

		//反转大小
		if ((count + 1) % 2 == 0)
		{
			for (int i = 0; i < (count + 1) / 2; i++)
			{
				uint8_t tmp = v[i];
				v[i] = v[count - i];
				v[count - i] = tmp;
			}
		}
		else {
			for (int i = 0; i <= (count + 1) / 2; i++)
			{
				uint8_t tmp = v[i];
				v[i] = v[count - i];
				v[count - i] = tmp;
			}
		}

		return rValue;
	}

	uint8_t * read(uint64_t size)
	{
		if (size == 0) return NULL;
		uint8_t * buffer = (uint8_t*)malloc(size + 1);
		if (buffer == NULL) {
			LOGE("malloc error:%lld\n",size);
			return NULL;
		}
		memset(buffer, 0, size + 1);
		int ret = fread(buffer, size, 1, fp_);
		if (ret != 1)
		{
			LOGE("read error.\n");
			free(buffer);
			return NULL;
		}
		return buffer;
	}
private:
	FILE * fp_;
	uint64_t size_;
};

class BufferReader 
	:public MKVReader
{
public:
	BufferReader(uint8_t * buf, VINT len,uint64_t pos)
	{
		buffer_ = buf;
		len_ = len;
		cur_ = 0;
		pos_ = pos;
	}

	uint64_t tell()
	{
		return pos_ + cur_;
	}

	void seek(uint64_t pos) {
		cur_ = pos - pos_;
	}

	uint64_t size() {
		return pos_ + len_;
	}

	VINT readVInt(bool dropOne = false) {
		if (cur_ >= len_) return -1;
		uint8_t value = buffer_[cur_];
		cur_++;

		if (value == -1)
		{
			LOGE("BufferReader read error.\n");
			return -1;
		}

		int count = 0;
		uint8_t utf_8v = value;
		while ((utf_8v & 0x80) == 0)
		{
			utf_8v <<= 1;
			count++;

			if (count > 7) break;
		}
		utf_8v <<= 1;
		utf_8v >>= (count + 1);
		if (count > 7) {
			LOGE("BufferReader VINT max count(%d).", count);
			return -1;
		}

		VINT rValue = 0;
		char * v = (char*)&rValue;
		if (dropOne)
		{
			v[0] = utf_8v;
		}
		else {
			v[0] = value;
		}
		if (count == 0) return rValue;

		memcpy(v + 1, buffer_ + cur_, count);
		cur_ += count;

		//反转大小
		if ((count + 1) % 2 == 0)
		{
			for (int i = 0; i < (count + 1) / 2; i++)
			{
				uint8_t tmp = v[i];
				v[i] = v[count - i];
				v[count - i] = tmp;
			}
		}
		else {
			for (int i = 0; i <= (count + 1) / 2; i++)
			{
				uint8_t tmp = v[i];
				v[i] = v[count - i];
				v[count - i] = tmp;
			}
		}

		return rValue;
	}

	uint8_t * read(uint64_t size)
	{
		if (size == 0) return NULL;

		if (cur_ + size > len_) return NULL;
		
		uint8_t * buffer = (uint8_t*)malloc(size + 1);
		if (buffer == NULL) {
			LOGE("malloc error:%lld\n", size);
			return NULL;
		}

		memcpy(buffer, buffer_ + cur_, size);
		buffer[size] = 0x0;
		cur_ += size;

		return buffer;
	}
private:
	uint8_t * buffer_;
	size_t len_;
	size_t cur_;
	uint64_t pos_;
};

class EBML_ELEMENT;

class EBML_ELEMENT {
public:
	uint64_t pos;
	VINT ID; // EBML-ID
	VINT size; // size of element
	uint8_t * data; // data
	uint64_t pos_data;

	std::vector<EBML_ELEMENT*> child;
};

MKVValue *getMKVValue(VINT id)
{
	MKVValue * value = getMKVValue_(id);
	if (value == NULL) {
		value = LuaGetMKVValue_(id);
	}
	return value;
}

//打印元素数据
void printElementValue(EBML_ELEMENT * ele, MKVValue * mkvValue)
{
	printf("%s (%llx %llu %llu): ", mkvValue->name, ele->ID, ele->size, ele->pos);
	if (mkvValue->type == INT_TYPE) {
		if (ele->size == 8)
		{
			uint64_t value = *(uint64_t*)ele->data;
			char * v = (char*)&value;
			for (int i = 0; i < 4; i++)
			{
				uint8_t tmp = v[i];
				v[i] = v[8 - i - 1];
				v[8 - i - 1] = tmp;
			}
			printf("%llu\n", value);
		}
		else if (ele->size == 4) {
			uint32_t value = *(uint32_t*)ele->data;
			char * v = (char*)&value;
			for (int i = 0; i < 2; i++)
			{
				uint8_t tmp = v[i];
				v[i] = v[4 - i - 1];
				v[4 - i - 1] = tmp;
			}
			printf("%u\n", value);
		}
		else if (ele->size == 3) {
			uint32_t value = 0;
			char * v = (char*)&value;
			for (int i = 0; i < 3; i++)
			{
				v[i] = ele->data[2 - i];
			}
			printf("%u\n", value);
		}
		else if (ele->size == 2) {
			uint16_t value = *(uint16_t*)ele->data;
			char * v = (char*)&value;
			for (int i = 0; i < 1; i++)
			{
				uint8_t tmp = v[i];
				v[i] = v[2 - i - 1];
				v[2 - i - 1] = tmp;
			}
			printf("%u\n", value);
		}
		else if (ele->size == 1) {
			uint8_t value = *(uint8_t*)ele->data;
			printf("%d\n", value);
		}
		else {
			printf("unknow size(%llu)\n", ele->size);
		}
	}
	else if (mkvValue->type == STRING_TYPE) {
		printf("%s\n", ele->data);
	}
	else if (mkvValue->type == FLOAT_TYPE) {
		float value = *((float*)ele->data);
		printf("%f\n", value);
	}
	else if (mkvValue->type == HEXSTRING_TYPE)
	{
		for (int i = 0; i < ele->size; i++) {
			printf("%02x ", ele->data[i]);
		}
		printf("\n");
	}
	else if (mkvValue->type == BLOCK_TYPE)
	{
		printf("\n");
	}
	else {
		printf("\n");
	}
}

//打印元素
void printElement(EBML_ELEMENT * ele,int level) {
	int tabLevel = level;
	while (tabLevel-- > 0) {
		printf("\t");
	}

	MKVValue * mkvValue = getMKVValue(ele->ID);
	if(mkvValue != NULL){	
		printElementValue(ele,mkvValue);
	}
	else {
		printf("%llx %llu\n",ele->ID, ele->size);
	}

	freeMKVValue(mkvValue);

	if (ele->child.size() > 0)
	{
		for (int i = 0; i < ele->child.size(); i++)
		{
			printElement(ele->child[i],level+1);
		}
	}
}

//是否是容器
bool isContainer(VINT id) {
	MKVValue * value = getMKVValue(id);
	if (value == NULL) return false;
	bool ret = (value->type == CONTENT_TYPE);
	freeMKVValue(value);
	return ret;
}

//读取元素内容
EBML_ELEMENT *readElement(MKVReader * reader, uint64_t maxPos, int level);

//读取子元素
void readChild(EBML_ELEMENT * parent, MKVReader * reader,uint64_t maxPos,int level) {
	EBML_ELEMENT * ele = NULL;
	do {
		ele = readElement(reader, maxPos, level);
		if (ele != NULL) {
			parent->child.push_back(ele);
		}
		if (reader->tell() >= maxPos) break;
	} while (ele != NULL);
}

//读取元素内容
EBML_ELEMENT *readElement(MKVReader * reader,uint64_t maxPos,int level)
{
	uint64_t pos = reader->tell();
	if (pos >= maxPos) return NULL;
	VINT ID = reader->readVInt();
	if (ID == -1)
	{
		return NULL;
	}
	if (ID == EBML && level != 0)
	{
		reader->seek(pos);
		return NULL;
	}

	VINT size = reader->readVInt(true);
	if (size == -1)
	{
		return NULL;
	}
	uint64_t data_pos = reader->tell();

	EBML_ELEMENT * ele = new EBML_ELEMENT();
	ele->ID = ID;
	ele->pos = pos;
	ele->pos_data = data_pos;
	ele->size = size;
	ele->data = NULL;

	if (!isContainer(ele->ID)) {
		uint64_t avaliableSize = maxPos - reader->tell();
		if (size > avaliableSize)
		{
			size = avaliableSize;
		}

		uint8_t * data = reader->read(size);
		if (data == NULL)
		{
			delete ele;
			return NULL;
		}

		ele->size = size;
		ele->data = data;
	}
	else {
		uint64_t avaliableSize = maxPos - reader->tell();
		if (size > avaliableSize)
		{
			size = avaliableSize;
		}

		readChild(ele, reader, size + reader->tell(), level + 1);
	}

	return ele;
}

//解析MKV格式
int parseMKV(const char * url)
{
	FILE * fp = fopen(url, "rb");
	if (fp == NULL) return -1;

	std::vector<EBML_ELEMENT*> ebml;

	FileReader reader(fp);
	while (true)
	{
		EBML_ELEMENT * ele = readElement(&reader,reader.size(),0);
		if (ele == NULL)
		{
			if (reader.tell() != reader.size())
			{
				LOGE("tell:%lld size:%lld\n", reader.tell(),reader.size());
			}
			else {
				break;
			}
		}
		else {
			ebml.push_back(ele);
		}
	}

	for (int i = 0; i < ebml.size(); i++)
	{
		printElement(ebml[i],0);
	}
	return 0;
}

int main(int argc,char* argv[]){
	if (argc <= 1) return -1;
	char * file = argv[1];
	parseMKV(file);
	return -1;
}