#ifndef ASM_GPIO_H
#define ASM_GPIO_H


#include "asm/gpio.h"

#define GPIO2PORT(gpio)    (gpio / IO_GROUP_NUM)
#if 0
void gpio_port_lock(unsigned int port);

void gpio_port_unlock(unsigned int port);

int __gpio_direction_input(unsigned int gpio);
int gpio_direction_input(unsigned int gpio);

int __gpio_direction_output(unsigned int gpio, int value);
int gpio_direction_output(unsigned int gpio, int value);

int __gpio_set_pull_up(unsigned int gpio, int value);
int gpio_set_pull_up(unsigned int gpio, int value);

int __gpio_set_pull_down(unsigned int gpio, int value);
int gpio_set_pull_down(unsigned int gpio, int value);

int __gpio_set_hd(unsigned int gpio, int value);
int gpio_set_hd(unsigned int gpio, int value);

int __gpio_set_die(unsigned int gpio, int value);
int gpio_set_die(unsigned int gpio, int value);

int __gpio_set_output_clk(unsigned int gpio, int clk);
int gpio_set_output_clk(unsigned int gpio, int clk);

int __gpio_read(unsigned int gpio);
int gpio_read(unsigned int gpio);
#else
#define gpio_set_die(gpio, value)  			gpio_hw_set_die(IO_PORT_SPILT(gpio), value)
#define gpio_direction_output(gpio, value) 	gpio_hw_direction_output(IO_PORT_SPILT(gpio), value)
#define gpio_direction_input(gpio) 			gpio_hw_direction_input(IO_PORT_SPILT(gpio))
#define gpio_set_pull_up(gpio, value) 		gpio_hw_set_pull_up(IO_PORT_SPILT(gpio), value)
#define gpio_set_pull_down(gpio, value) 	gpio_hw_set_pull_down(IO_PORT_SPILT(gpio), value)
#define gpio_set_direction(gpio, value) 	gpio_hw_set_direction(IO_PORT_SPILT(gpio), value)
#endif




#endif
