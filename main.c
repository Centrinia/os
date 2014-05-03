/* main.c */


void update() {
	vga_update();
}
int main()
{

    initialize_text_console();
    clear_console();

    print_string("Hello World!\n");
    setup_interrupts();
	setup_paging();

	vga_main();
    //enable_keyboard();
    //enable_rtc(10);
    //show_interrupts();
}
