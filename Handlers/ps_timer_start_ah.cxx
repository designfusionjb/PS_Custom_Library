#include "ps_global.hxx"
#include "ps_defines.hxx"
#include "ps_action_handlers.hxx"

using namespace ps;

int ps_timer_start_ah(EPM_action_message_t msg)
{
	const char			*debug_name = "PS-timer-start-AH";
	char				*pszArg = NULL;
	string				marker;
	int					result = ITK_ok;
	EPM_decision_t		decision = EPM_go;
	h_args				args(msg.arguments);
	
	log_debug("[START] %s", debug_name);

	try
	{
		if (args.size() == 0)
			throw psexception("Missing mandatory arguments.");

		if (!args.getStr("marker", marker))
			throw psexception("Missing mandatory argument 'marker'.");

		hr_start(marker.c_str());
	}
	catch (tcexception& e)
	{
		decision = EPM_nogo;
		EMH_store_error_s1(EMH_severity_error, ACTION_HANDLER_DEFAULT_IFAIL, e.what());
		log_error(e.what());
	}
	catch (psexception& e)
	{
		result = ACTION_HANDLER_DEFAULT_IFAIL;
		EMH_store_error_s1(EMH_severity_error, ACTION_HANDLER_DEFAULT_IFAIL, e.what());
		log_error(e.what());
	}

	log_debug("[STOP] %s", debug_name);

	return result;
}