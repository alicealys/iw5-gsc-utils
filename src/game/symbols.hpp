#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	// Functions

	WEAK symbol<void(int type, VariableUnion u)> AddRefToValue{0x5656E0};
	WEAK symbol<void(unsigned int id)> AddRefToObject{0x5655F0};
	WEAK symbol<unsigned int(unsigned int id)> AllocThread{0x565580};
	WEAK symbol<ObjectVariableValue*(unsigned int* id)> AllocVariable{0x565430};
	WEAK symbol<unsigned int()> AllocObject{0x565530};
	WEAK symbol<void(int type, VariableUnion u)> RemoveRefToValue{0x565730};
	WEAK symbol<void(unsigned int id)> RemoveRefToObject{0x5681E0};

	WEAK symbol<void(unsigned int weapon, bool isAlternate, char* output, unsigned int maxStringLen)> BG_GetWeaponNameComplete{0x42F760};

	WEAK symbol<void(int client)> ClientUserinfoChanged{0x4FADB0};
	WEAK symbol<const char*(int index)> ConcatArgs{0x502150};
	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{0x545680};
	WEAK symbol<void(const char* cmdName, void(), cmd_function_t* allocedCmd)> Cmd_AddCommandInternal{0x545DF0};
	WEAK symbol<void(const char* cmdName)> Cmd_RemoveCommand{0x545E20};
	WEAK symbol<const char*(int index)> Cmd_Argv{0x467600};

	WEAK symbol<const dvar_t*(const char*)> Dvar_FindVar{0x5BDCC0};

	WEAK symbol<char*(const char*)> I_CleanStr{0x0};

	WEAK symbol<VariableValue(unsigned int classnum, int entnum, int offset)> GetEntityFieldValue{0x56AF20};
	WEAK symbol<unsigned int(unsigned int parentId, unsigned int name)> FindVariable{0x5651F0};
	WEAK symbol<unsigned int(unsigned int parentId, unsigned int name)> FindObject{0x565BD0};
	WEAK symbol<unsigned int(unsigned int parentId, unsigned int name)> GetVariable{0x5663E0};
	WEAK symbol<unsigned int(unsigned int parentId, unsigned int name)> GetNewVariable{0x566390};
	WEAK symbol<unsigned int(unsigned int parentId, unsigned int unsignedValue)> GetNewArrayVariable{0x5668C0};
	WEAK symbol<void(unsigned int parentId, unsigned int id, VariableValue* value)> SetNewVariableValue{0x5658D0};
	WEAK symbol<void(unsigned int parentId, unsigned int index)> RemoveVariableValue{0x566500};

	WEAK symbol<const float* (const float* v)> Scr_AllocVector{0x565680};
	WEAK symbol<void()> Scr_ClearOutParams{0x569010};
	WEAK symbol<scr_entref_t(unsigned int entId)> Scr_GetEntityIdRef{0x565F60};
	WEAK symbol<void(unsigned int classnum, int entnum, int offset)> Scr_SetObjectField{0x52BCC0};
	WEAK symbol<void(int id, unsigned int stringValue, unsigned int paramcount)> Scr_NotifyId{0x56B5E0};
	WEAK symbol<int(const char* filename, unsigned int str)> Scr_GetFunctionHandle{0x5618A0};
	WEAK symbol<unsigned int(int handle, unsigned int objId, unsigned int paramcount)> Scr_ExecThreadInternal{0x56E1C0};
	WEAK symbol<unsigned int(int entnum, unsigned int classnum)> Scr_GetEntityId{0x567D80};
	WEAK symbol<unsigned int(int entnum, unsigned int classnum)> Scr_AddEntityNum{0x56ABC0};
	WEAK symbol<void()> Scr_AddArray{0x56AE30};
	WEAK symbol<unsigned int(unsigned int threadId)> Scr_GetSelf{0x5655E0};
	WEAK symbol<void()> Scr_MakeArray{0x56ADE0};
	WEAK symbol<void(unsigned int stringValue)> Scr_AddArrayStringIndexed{0x56AE70};
	WEAK symbol<void(unsigned int classnum, unsigned int name, unsigned int canonicalString, unsigned int offset)> Scr_AddClassField{0x567CD0};

	WEAK symbol<unsigned int(const char* str, unsigned int user)> SL_GetString{0x5649E0};
	WEAK symbol<unsigned int(const char* str)> SL_GetCanonicalString{0x5619A0};
	WEAK symbol<const char*(unsigned int stringValue)> SL_ConvertToString{0x564270};

	WEAK symbol<void(int clientNum, int type, const char* command)> SV_GameSendServerCommand{0x573220};
	WEAK symbol<void(int arg, char* buffer, int bufferLength)> SV_Cmd_ArgvBuffer{0x5459F0};

	WEAK symbol<void(unsigned int notifyListOwnerId, unsigned int stringValue, VariableValue* top)> VM_Notify{0x569720};
	WEAK symbol<unsigned int(unsigned int localId, const char* pos, unsigned int paramcount)> VM_Execute{0x56DFE0};

	WEAK symbol<void* (jmp_buf* Buf, int Value)> longjmp{0x7363BC};
	WEAK symbol<int(jmp_buf* Buf, int a2, int a3, int a4)> _setjmp{0x734CF8};

	// Variables

	WEAK symbol<CmdArgs> cmd_args{0x1C978D0};

	WEAK symbol<int> g_script_error_level{0x20B21FC};
	WEAK symbol<jmp_buf> g_script_error{0x20B4218};

	WEAK symbol<scrVmPub_t> scr_VmPub{0x20B4A80};
	WEAK symbol<scrVarGlob_t> scr_VarGlob{0x1E72180};

	WEAK symbol<scr_classStruct_t*> g_classMap{0x8B4300};

	WEAK symbol<gentity_s> g_entities{0x1A66E28};
	WEAK symbol<unsigned int> levelEntityId{0x208E1A4};

	WEAK symbol<client_s> svs_clients{0x4B5CF90};

	namespace plutonium
	{
		WEAK symbol<std::unordered_map<std::string, std::uint16_t>> function_map_rev{0x206964D0};
		WEAK symbol<std::unordered_map<std::string, std::uint16_t>> method_map_rev{0x206964F0};
		WEAK symbol<std::unordered_map<std::string, std::uint16_t>> token_map_rev{0x20696530};
		WEAK symbol<int(const char* fmt, ...)> printf{0x20887840};
		WEAK symbol<void*> function_table{0x2068F210};
		WEAK symbol<void*> method_table{0x2068F9E0};
	}
}