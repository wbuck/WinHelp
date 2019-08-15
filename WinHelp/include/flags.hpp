#pragma once
#include <windows.h>
#include <winnt.h>
#include <type_traits>

namespace wh
{
	enum class access_flag : DWORD
	{
		read = GENERIC_READ,
		write = GENERIC_WRITE,
		execute = GENERIC_EXECUTE,
		all = GENERIC_ALL
	};

	DEFINE_ENUM_FLAG_OPERATORS( access_flag );

	enum class share_flag : DWORD
	{
		share_none = 0,
		share_delete = FILE_SHARE_DELETE,
		share_read = FILE_SHARE_READ,
		share_write = FILE_SHARE_WRITE
	};

	DEFINE_ENUM_FLAG_OPERATORS( share_flag );

	enum class creation_option : DWORD
	{
		create_always = CREATE_ALWAYS,
		open_always = OPEN_ALWAYS,
		open_existing = OPEN_EXISTING,
		truncate_existing = TRUNCATE_EXISTING
	};

	enum class attr_flag : DWORD
	{
		attr_archive = FILE_ATTRIBUTE_ARCHIVE,
		attr_encrypted = FILE_ATTRIBUTE_ENCRYPTED,
		attr_hidden = FILE_ATTRIBUTE_HIDDEN,
		attr_normal = FILE_ATTRIBUTE_NORMAL,
		attr_offline = FILE_ATTRIBUTE_OFFLINE,
		attr_readonly = FILE_ATTRIBUTE_READONLY,
		attr_system = FILE_ATTRIBUTE_SYSTEM,
		attr_temp = FILE_ATTRIBUTE_TEMPORARY,
		flag_backup_semantics = FILE_FLAG_BACKUP_SEMANTICS,
		flag_delete_on_close = FILE_FLAG_DELETE_ON_CLOSE,
		flag_no_buffering = FILE_FLAG_NO_BUFFERING,
		flag_open_no_recall = FILE_FLAG_OPEN_NO_RECALL,
		flag_open_reparse_point = FILE_FLAG_OPEN_REPARSE_POINT,
		flag_overlapped = FILE_FLAG_OVERLAPPED,
		flag_posix_semantics = FILE_FLAG_POSIX_SEMANTICS,
		flag_random_access = FILE_FLAG_RANDOM_ACCESS,
		flag_session_aware = FILE_FLAG_SESSION_AWARE,
		flag_sequential_scan = FILE_FLAG_SEQUENTIAL_SCAN,
		flag_write_through = FILE_FLAG_WRITE_THROUGH,
		sec_anonymous = SECURITY_ANONYMOUS,
		sec_ctx_tracking = SECURITY_CONTEXT_TRACKING,
		sec_delegation = SECURITY_DELEGATION,
		sec_effective_only = SECURITY_EFFECTIVE_ONLY,
		sec_id = SECURITY_IDENTIFICATION,
		sec_impersonation = SECURITY_IMPERSONATION
	};

	DEFINE_ENUM_FLAG_OPERATORS( attr_flag );
}