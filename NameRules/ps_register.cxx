#include "ps_global.hxx"
#include "ps_defines.hxx"
#include "ps_register.hxx"
#include "ps_name_rules.hxx"

using namespace ps;

void ps::ps_register_name_rules()
{
	c_pptr<char>	nameRuleLines;
	TC_preference_search_scope_t oldScope;

	nameRuleLines.set_free_container_only(true);
	itk(PREF_ask_search_scope(&oldScope));
	itk(PREF_set_search_scope(TC_preference_site));
	try
	{
		itk(PREF_ask_char_values(PREF_NAME_RULES, nameRuleLines.plen(), nameRuleLines.pptr()));
	}
	catch (tcexception& e)
	{
		if (e.getstat() == PF_NOTFOUND)
		{
			return;
		}
	}
	itk(PREF_set_search_scope(oldScope));

	for (int i = 0; i < nameRuleLines.len(); i++)
	{
		const char			*nameRuleLine = nameRuleLines.val(i);
		string				propertyTypeMsg;
		vector<string>		nameRuleLinesSplitted; //!< 0 = Object Type, 1 = Property Type, 2 = Property Name, 3 = RegEx., 4 = Length, 5 = Error message
		METHOD_id_t 		method;
		c_ptr<TC_argument_list_t>	argument;

		split_string(nameRuleLine, ":", true, nameRuleLinesSplitted);

		// Don't register if incorrect setting
		if (nameRuleLinesSplitted.size() != 6)
			continue;

		if (nameRuleLinesSplitted[1] == "int")
		{
			propertyTypeMsg = PROP_set_value_int_msg;
		}
		else if (nameRuleLinesSplitted[1] == "float")
		{
			propertyTypeMsg = PROP_set_value_double_msg;
		}
		else if (nameRuleLinesSplitted[1] == "double")
		{
			propertyTypeMsg = PROP_set_value_double_msg;
		}
		else if (nameRuleLinesSplitted[1] == "string")
		{
			propertyTypeMsg = PROP_set_value_string_msg;
		}
		else
			throw psexception("Unsupported property type for naming rule validation.");

		// Allocate all memory on the heap
		argument.set_free(false);
		argument.alloc(1);
		argument.ptr()->number_of_arguments = 4;
		sm_alloc(argument.ptr()->arguments, TC_argument_t, argument.ptr()->number_of_arguments);
		argument.ptr()->arguments[0].type = POM_string;
		argument.ptr()->arguments[0].array_size = 1;
		sm_alloc(argument.ptr()->arguments[0].val_union.str_value, char, nameRuleLinesSplitted[1].length() + 1);
		tc_strcpy(argument.ptr()->arguments[0].val_union.str_value, nameRuleLinesSplitted[1].c_str());
		argument.ptr()->arguments[1].type = POM_string;
		argument.ptr()->arguments[1].array_size = 1;
		sm_alloc(argument.ptr()->arguments[1].val_union.str_value, char, nameRuleLinesSplitted[3].length() + 1);
		tc_strcpy(argument.ptr()->arguments[1].val_union.str_value, nameRuleLinesSplitted[3].c_str());
		argument.ptr()->arguments[2].type = POM_int;
		argument.ptr()->arguments[2].array_size = 1;
		argument.ptr()->arguments[2].val_union.int_value = atoi(nameRuleLinesSplitted[4].c_str());
		argument.ptr()->arguments[3].type = POM_string;
		argument.ptr()->arguments[3].array_size = 1;
		sm_alloc(argument.ptr()->arguments[3].val_union.str_value, char, nameRuleLinesSplitted[5].length() + 1);
		tc_strcpy(argument.ptr()->arguments[3].val_union.str_value, nameRuleLinesSplitted[5].c_str());

		itk(METHOD_find_prop_method(nameRuleLinesSplitted[0].c_str(), nameRuleLinesSplitted[2].c_str(), propertyTypeMsg.c_str(), &method));
		itk(METHOD_add_action(method, METHOD_pre_action_type, ps_validate_name_rule, argument.ptr()));
	}
}

int ps::libps_oninit(int *decision, va_list args)
{
	try
	{
		// Initialize logging mechanism
		if (initialize_logging())
			printf("Logging module enabled.\n");
		// If enabled, initialize HRTimer class
		if (hr_init())
			printf("HRTimer module enabled.\n");

		//char ch;
		//ch = getchar();

		// Register naming rules
		try
		{
			ps_register_name_rules();
		}
		catch (exception &e)
		{
			TC_write_syslog("%s\n", e.what());
		}
	}
	catch (tcexception& e)
	{
		TC_write_syslog("%s\n", e.what());

		return e.getstat();
	}
	catch (exception& e)
	{
		TC_write_syslog("%s\n", e.what());

		return 1;
	}

	return ITK_ok;
}

int ps::libps_onexit(int *decision, va_list args)
{
	try
	{
		// Print performance statistics when logging out
		hr_print_all();
	}
	catch (tcexception& e)
	{
		TC_write_syslog("%s\n", e.what());

		return e.getstat();
	}
	catch (exception& e)
	{
		TC_write_syslog("%s\n", e.what());

		return 1;
	}

	return ITK_ok;
}

int libpsadds_register_callbacks()
{ 
	printf("Installing PS User Exits Library - %s, "__DATE__" "__TIME__".\n", PS_LIBNAME);

	try
	{
		itk(CUSTOM_register_exit(PS_LIBNAME, "USER_init_module", (CUSTOM_EXIT_ftn_t) libps_oninit));
		itk(CUSTOM_register_exit(PS_LIBNAME, "USER_exit_module", (CUSTOM_EXIT_ftn_t) libps_onexit));
	}
	catch (tcexception& e)
	{
		TC_write_syslog(e.what());

		return e.getstat();
	}
	catch (exception& e)
	{
		TC_write_syslog(e.what());

		return 1;
	}

	return ITK_ok;
}