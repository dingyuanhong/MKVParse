#include "luaFunction.h"

extern "C"
{
#include "lua.h"  
#include "lauxlib.h"  
#include "lualib.h"  
}

#pragma comment(lib,"lua.lib")

const char * LUA_TypeName[] =
{
	"LUA_TNONE",
	"LUA_TNIL",
	"LUA_TBOOLEAN",
	"LUA_TLIGHTUSERDATA",
	"LUA_TNUMBER",
	"LUA_TSTRING",
	"LUA_TTABLE",
	"LUA_TFUNCTION",
	"LUA_TUSERDATA",
	"LUA_TTHREAD",
	"LUA_NUMTAGS"
};

inline const char * GetLuaTypeName(int type)
{
	return LUA_TypeName[type + 1];
}

#define LOGE printf

MKVValue *LuaGetMKVValue_(VINT id)
{
	lua_State *L = luaL_newstate();
	if (L == NULL)
	{
		return NULL;
	}
	luaL_openlibs(L);

	int nTop = lua_gettop(L);

	std::string luaFile = "./Script/mkv.lua";

	int bRet = luaL_loadfile(L, luaFile.c_str());
	if (bRet)
	{
		const char *errorString = lua_tostring(L, -1);
		LOGE((std::string("Error:") + errorString + "\n").c_str());
		lua_close(L);
		return NULL;
	}
	//3.����Lua�ļ�  
	bRet = lua_pcall(L, 0, 0, 0);
	if (bRet)
	{
		const char *errorString = lua_tostring(L, -1);
		LOGE((std::string("Error:") + errorString + "\n").c_str());
		lua_close(L);
		return NULL;
	}

	lua_getglobal(L, "parse");			// ��ȡ������ѹ��ջ��
	lua_pushinteger(L, id);			// ѹ���һ������
	int iRet = lua_pcall(L, 1, 1, 0);	// ���ú�������������Ժ󣬻Ὣ����ֵѹ��ջ�У�2��ʾ����������1��ʾ���ؽ��������  
	if (iRet)							// ���ó���  
	{
		const char *errorString = lua_tostring(L, -1);
		LOGE((std::string("Error:") + errorString + "\n").c_str());
		lua_close(L);
		return NULL;
	}

	MKVValue * value = NULL;

	if (lua_type(L, 1) == LUA_TTABLE)
	{
		lua_pushstring(L, "name");
		lua_gettable(L, -2);
		const char * string = lua_tostring(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "id");
		lua_gettable(L, -2);
		lua_Integer id = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "type");
		lua_gettable(L, -2);
		lua_Integer type = lua_tointeger(L, -1);
		lua_pop(L, 1);

		value = (MKVValue*)malloc(sizeof(MKVValue));
		value->name = strdup(string);
		value->id = id;
		value->type = type;
		value->free = 1;
	}
	else
	{
		int index = 1;
		int type = lua_type(L, index);
		if (type == LUA_TNUMBER)
		{
			lua_Number number = lua_tonumber(L, index);
			char buffer[64];
			sprintf(buffer, "%lf", number);
			LOGE((std::string("Error Result Type: ") + GetLuaTypeName(type) + " (" + buffer + ") ,Not LUA_TTABLE\n").c_str());
		}
		else if (type == LUA_TSTRING)
		{
			const char * str = lua_tostring(L, index);
			LOGE((std::string("Error Result Type: ") + GetLuaTypeName(type) + " (" + str + ") ,Not LUA_TTABLE\n").c_str());
		}
		else if (type == LUA_TBOOLEAN)
		{
			bool bl = lua_toboolean(L, index);
			LOGE((std::string("Error Result Type: ") + GetLuaTypeName(type) + " " + (bl ? "true" : "false") + " ,Not LUA_TTABLE\n").c_str());
		}
		else if (type == LUA_TNIL)
		{
		
		}
		else
		{
			LOGE((std::string("Error Result Type: ") + GetLuaTypeName(type) + " ,Not LUA_TTABLE\n").c_str());
		}
		lua_pop(L, 1);
	}
	nTop = lua_gettop(L);
	//7.�ر�state  
	lua_close(L);
	return value;
}