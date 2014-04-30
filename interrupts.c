

__attribute__((naked)) void interrupt_handler()
{
	    asm ("pusha");
	     
	        /* do something */
	     
	        asm (
				        "popa" \
					        "iret"
		    );
}

