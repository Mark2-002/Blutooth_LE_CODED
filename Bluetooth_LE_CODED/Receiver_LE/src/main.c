
/*
   Atul_v
*/

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include "./headers/main_observer.h"
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>


LOG_MODULE_REGISTER(receiver,LOG_LEVEL_DBG);

static void set_random_static_address(void)
{

		int err; 
		bt_addr_le_t addr;
		//convert the string into a binary address and stores this address in a buffer whose address is addr.
		err=bt_addr_le_from_str("FA:17:DF:DF:4B:89","random" , &addr);

		// err= bt_addr_le_from_str("DE:AD:BE:AF:BA:11","random" , &addr);  //Only random static address can be given when the type is set to "random"
        if (err) {
			LOG_INF("Invalid BT address (err %d)\n", err);
		}
        //create a new Identity address. This must be done before enabling Bluetooth. Addr is the address used for the new identity
		err=bt_id_create(&addr, NULL);
                if (err < 0) {
			LOG_INF("Creating new ID failed (err %d)\n", err);
		}
            LOG_INF("Created new address\n");
 }


int main(void)
{
	int err;
	LOG_INF("Starting Observer For LE_CODED \n");
	set_random_static_address();

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err) {
		LOG_INF("Bluetooth init failed (err %d)\n", err);
		return;
	}
	else{
		LOG_INF("Bt Init succesfully");
	}
	observer_start();
	return 0;
}