#include "ts_env.h"

TsEnv g_env;

TsEnv::TsEnv()
{}

TsEnv::~TsEnv()
{}

bool TsEnv::init(bool for_web)
{
	EXLOG_LEVEL(EX_LOG_LEVEL_INFO);

	ex_exec_file(m_exec_file);

	m_exec_path = m_exec_file;
	ex_dirname(m_exec_path);

	if (!for_web)
		return true;

    // check development flag file, if exists, run in development mode for trace and debug.
    ex_wstr dev_flag_file = m_exec_path;
    ex_path_join(dev_flag_file, false, L"dev_mode", NULL);

	ex_wstr base_path = m_exec_path;
    ex_wstr log_path;
    ex_wstr conf_file;

    if (ex_is_file_exists(dev_flag_file.c_str()))
	{
		EXLOGW("===== DEVELOPMENT MODE =====\n");

        ex_path_join(base_path, true, L"..", L"..", L"..", L"..", L"server", NULL);
        conf_file = base_path;
        ex_path_join(conf_file, false, L"share", L"etc", L"web.ini.in", NULL);

        log_path = base_path;
		ex_path_join(log_path, false, L"share", L"log", NULL);
	}
	else	// not in development mode
	{
		base_path = m_exec_path;
		ex_path_join(base_path, true, L"..", NULL);

#ifdef EX_OS_WIN
		conf_file = base_path;
		ex_path_join(conf_file, false, L"etc", L"web.ini", NULL);
		log_path = base_path;
		ex_path_join(log_path, false, L"log", NULL);
#else
		conf_file = L"/etc/teleport/web.ini";
		log_path = L"/var/log/teleport";

#endif
	}

	m_www_path = base_path;
	ex_path_join(m_www_path, false, L"www", NULL);

//	// 定位 log, etc, www 路径
//	// 默认情况下，以上三个目录均位于本可执行程序的 ../ 相对位置，
//	// 如果不存在，则可能是开发调试模式，则尝试从源代码仓库根目录下的share目录中查找。
//	ex_wstr base_path = m_exec_path;
//	ex_path_join(base_path, true, L"..", NULL);
//
//	ex_wstr conf_file = base_path;
//	ex_path_join(conf_file, false, L"etc", L"web.ini", NULL);
//
//	if (ex_is_file_exists(conf_file.c_str()))
//	{
//		m_www_path = base_path;
//		ex_path_join(m_www_path, false, L"www", NULL);
//	}
//	else
//	{
//		EXLOGW("===== DEVELOPMENT MODE =====\n");
//		base_path = m_exec_path;
//		ex_path_join(base_path, true, L"..", L"..", L"..", L"..", L"server", L"share", NULL);
//
//		conf_file = base_path;
//		ex_path_join(conf_file, false, L"etc", L"web.ini", NULL);
//
//		m_www_path = m_exec_path;
//		ex_path_join(m_www_path, true, L"..", L"..", L"..", L"..", L"server", L"www", NULL);
//	}

	if (!ex_is_file_exists(conf_file.c_str()))
	{
		EXLOGE("[tpweb] web.ini not found.\n");
		return false;
	}

	ExIniFile cfg;
	if (!cfg.LoadFromFile(conf_file))
	{
		EXLOGE("[tpweb] can not load web.ini.\n");
		return false;
	}

	ex_wstr log_file;
	ExIniSection* ps = cfg.GetDumySection();
	if (!ps->GetStr(L"log_file", log_file))
	{
//		ex_wstr log_path = base_path;
//		ex_path_join(log_path, false, L"log", NULL);
		EXLOG_FILE(L"tpweb.log", log_path.c_str());
	}
	else
	{
		ex_remove_white_space(log_file);
		if (log_file[0] == L'"' || log_file[0] == L'\'')
			log_file.erase(0, 1);
		if (log_file[ log_file.length() - 1 ] == L'"' || log_file[log_file.length() - 1] == L'\'')
			log_file.erase(log_file.length() - 1, 1);

//		ex_wstr log_path = log_file;
		log_path = log_file;
		ex_dirname(log_path);
		ex_wstr file_name;
		file_name.assign(log_file, log_path.length() + 1, log_file.length());

		EXLOG_FILE(file_name.c_str(), log_path.c_str());
	}

	int log_level = EX_LOG_LEVEL_INFO;
	if (ps->GetInt(L"log_level", log_level))
	{
		EXLOGV("[tpweb] log-level: %d\n", log_level);
		EXLOG_LEVEL(log_level);
	}

//	EXLOGI("==============================\n");
//	EXLOGI("[tpweb] start...\n");

	return true;
}
