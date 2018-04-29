#include <stdio.h>
#include <time.h>
#include "elev.h"

int main(){

	int button_signal, floor_current, time_current, emergency, at_floor, calibrated, buttons_to_stop_for, orders_ahead, orders_met, should_open, opened, motor_magnitude, i = -1, floor_last_sensed = -1, orders = 0, time_last_closed = 0, time_remaining = 0, direction = 1, floor_check_ahead_of = 0, initialized = elev_init();
	while (initialized){
		i 										= (i + 1) % 12 + (i == 0 || i == 8);
		button_signal 				= elev_get_button_signal(i % 3, i / 3);
		floor_current 				= elev_get_floor_sensor_signal();
		time_current					= time(NULL);
		emergency 						= elev_get_stop_signal();
		at_floor							= (floor_current > -1);
		floor_last_sensed			= (floor_current * at_floor) + (floor_last_sensed * !at_floor);
		calibrated						= (floor_last_sensed > -1);

		buttons_to_stop_for		= (0b101 * (direction > -1)) | (0b110 * (direction < 1));
		orders								= (orders | (button_signal << i)) * (!emergency && calibrated);
		orders_ahead					= (orders & ((0b111111111000 << (3 * floor_check_ahead_of)) >> (6 - (6 * direction))));
		orders_met						= (orders & (buttons_to_stop_for << (3 * floor_current))) * at_floor;

		should_open						= at_floor && (emergency || orders_met);
		opened								= should_open || (opened && time_remaining > 0);
		time_last_closed			= (time_current * should_open) + (time_last_closed * !should_open);
		time_remaining				= (3 + (time_last_closed - time_current));
		orders								= (orders & ~(opened * orders_met));

		motor_magnitude				= (!(opened || emergency || !orders) || !calibrated);
		direction							= (direction * (1 - (2 * !orders_ahead)));
		floor_check_ahead_of	= (at_floor * floor_current) + (!at_floor * (floor_check_ahead_of + (direction * !orders_ahead)));

		elev_set_button_lamp(i % 3, i / 3, (orders & (1 << i)));
		elev_set_motor_direction(motor_magnitude * direction);
		elev_set_stop_lamp(emergency && calibrated);
		elev_set_floor_indicator(floor_last_sensed * calibrated);
		elev_set_door_open_lamp(opened);
	}

	printf("Unable to initialize elevator hardware!\n");
	return 1;
}
