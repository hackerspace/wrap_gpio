all: wrap_read_gpio wrap_write_gpio wrap_setup_gpio

wrap_read_gpio: wrap_read_gpio.c
	gcc -m32 -static -o wrap_read_gpio wrap_read_gpio.c

wrap_write_gpio: wrap_write_gpio.c
	gcc -m32 -static -o wrap_write_gpio wrap_write_gpio.c

wrap_setup_gpio: wrap_setup_gpio.c
	gcc -m32 -static -o wrap_setup_gpio wrap_setup_gpio.c

clean:
	rm wrap_read_gpio wrap_write_gpio wrap_setup_gpio

install:
	cp wrap_read_gpio wrap_write_gpio wrap_setup_gpio /usr/bin

uninstall:
	rm /usr/bin/wrap_read_gpio /usr/bin/wrap_write_gpio /usr/bin/wrap_setup_gpio
