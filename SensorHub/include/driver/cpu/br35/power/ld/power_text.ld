		. = ALIGN(4);
		PROVIDE(low_power_target_begin = .);
		*(.lp_target)
		PROVIDE(low_power_target_end = .);

		. = ALIGN(4);
		PROVIDE(low_power_callback_begin = .);
		*(.lp_callback)
		PROVIDE(low_power_callback_end = .);
	
		. = ALIGN(4);
        deepsleep_target_begin = .;
        PROVIDE(deepsleep_target_begin = .);
        KEEP(*(.deepsleep_target))
        deepsleep_target_end = .;
        PROVIDE(deepsleep_target_end = .);

		. = ALIGN(4);
		PROVIDE(m2p_msg_handler_begin = .);
		KEEP(*(.m2p_msg_handler))
		PROVIDE(m2p_msg_handler_end = .);
