CUSTOM_UI_COMMAND_SIG(tc_toggle_debug)
CUSTOM_DOC("Invert a debug boolean for testing")
{
	tc_debug_toggle = !tc_debug_toggle;
}