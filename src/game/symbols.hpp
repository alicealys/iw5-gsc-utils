#pragma once

/*
 * Trimmed from upstream iw5-gsc-utils: legacy `plutonium` namespace block
 * (gsc_ctx / function_table / method_table) REMOVED.
 * Everything below is unrelated to the old broken dispatch hook,
 * covering VM primitives like refcounting, array/object ops, and strings.
 */

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

	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{0x545680};
	WEAK symbol<void(const char* cmdName, void(), cmd_function_t* allocedCmd)> Cmd_AddCommandInternal{0x545DF0};
	WEAK symbol<void(const char* cmdName)> Cmd_RemoveCommand{0x545E20};

	WEAK symbol<const dvar_t*(const char*)> Dvar_FindVar{0x5BDCC0};

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
	WEAK symbol<unsigned int(unsigned int threadId)> Scr_GetSelf{0x5655E0};

	WEAK symbol<unsigned int(const char* str, unsigned int user)> SL_GetString{0x5649E0};
	WEAK symbol<unsigned int(const char* str)> SL_GetCanonicalString{0x5619A0};
	WEAK symbol<const char*(unsigned int stringValue)> SL_ConvertToString{0x564270};

	// C++ -> GSC direction: AllocThread reserves a VM thread bound to an entity
	// id, VM_Execute runs the bytecode at a SCRIPT_FUNCTION code position.
	WEAK symbol<unsigned int(unsigned int localId, const char* pos, unsigned int paramcount)> VM_Execute{0x56DFE0};
	WEAK symbol<void(unsigned int notifyListOwnerId, unsigned int stringValue, VariableValue* top)> VM_Notify{0x569720};

	WEAK symbol<void(int client)> ClientUserinfoChanged{0x4FADB0};
	WEAK symbol<void(int clientNum, int type, const char* command)> SV_GameSendServerCommand{0x573220};
	WEAK symbol<void(client_s* drop, const char* reason, bool tellThem)> SV_DropClient{0x570980};

	// Variables

	WEAK symbol<CmdArgs> cmd_args{0x1C978D0};

	WEAK symbol<unsigned int> levelEntityId{0x208E1A4};

	WEAK symbol<scrVmPub_t> scr_VmPub{0x20B4A80};
	WEAK symbol<scrVarGlob_t> scr_VarGlob{0x1E72180};

	WEAK symbol<gentity_s> g_entities{0x1A66E28};

	WEAK symbol<dvar_t> sv_maxclients{0x1BA0E4C};
	WEAK symbol<int> svs_clientCount{0x4B5CF8C};
	WEAK symbol<client_s> svs_clients{0x4B5CF90};
}
