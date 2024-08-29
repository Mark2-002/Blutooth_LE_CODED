/*
   Atul_v
*/

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


LOG_MODULE_REGISTER(Receiver,LOG_LEVEL_DBG);

// static uint8_t write_array[20]={0};
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


// static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,struct bt_data *data
// 					)
// {
	
// 	LOG_INF("Device found: %s, RSSI: %d, Type: %d\n", bt_addr_le_str(addr), rssi,adv_type);
// }	
#define n 3	 // Define a macro 'n' with the value 3
#define n1 3 // Define another macro 'n1' with the value 3
static int prev=0;
static int curr =0;
static int x =0;
static int max =0;
static int loss_per=0;

static int received =0;
#if defined(CONFIG_BT_EXT_ADV)


int packet_loss(){
	x=curr-prev-1;
	if (x>max&&prev>0){
		max=x;
	}
	return x;
}


static bool data_cb(struct bt_data *data, void *user_data)
{
	int total_write = pow(
		2, n); // total number of bytes to be written corresponding to one received packet
	int chunk_size = pow(
		2, n1); // total number of bytes to be written in one write operation [max.= 128]
	// int num_steps = pow(2, (n - n1)); // number of iterations in which the entire packet gets
	// written into the memory
	// char write_array[chunk_size];
	uint8_t write_array[chunk_size];

	

	/* data_cb is executed whenever some advertising packet comes. */
	LOG_INF(":::::::::Data From BLE:::::::::");

	// len is the length of the advertising packet.
	// uint8_t len;

	switch (data->type)
	{
	case BT_DATA_NAME_SHORTENED:	// Handle Bluetooth data of type BT_DATA_NAME_SHORTENED
	case BT_DATA_NAME_COMPLETE:		// Handle Bluetooth data of type BT_DATA_NAME_COMPLETE
	case BT_DATA_MANUFACTURER_DATA: // Handle Bluetooth data of type BT_DATA_MANUFACTURER_DATA

		int index = 0; // Initialize an integer variable 'index' with the value 0

		/* It is just a variable that runs from 0 to 128. */
		int j=0;
		prev=curr;
		// Storing chunk_size BYTES OF DATA AT ONCE //
		for (j = 0; j < chunk_size; j++)
		{
			/* Using char (char = 1 byte uint8_t type). */
			char number = data->data[j + index];
			/* Appending them in write array. */
			write_array[j] = number;
			curr=write_array[0];
			printk(" %d ", write_array[j]);
			

		
		}
		
		if(curr>240){
		loss_per=((float)(curr-received)/curr)*100;
		}
		
		printk("\n");
		packet_loss();
		received++;
		
		printk("Curr_packet lost ::  %d  || MAX_lost :: %d   ||  Packet received:: %d  || Total Loss %= %d %\n",x,max,received,loss_per);
		loss_per=0;
		if(curr==1){
			received=1;
		}
		gpio_pin_configure_dt(&led,GPIO_OUTPUT);
		gpio_pin_set_dt(&led,1);
		k_msleep(500);
		gpio_pin_set_dt(&led,0);
	}}


static const char *phy2str(uint8_t phy) // Define a function 'phy2str' that converts a PHY value to a string representation
{
	switch (phy)
	{ // Case when PHY is BT_GAP_LE_PHY_NONE
	case BT_GAP_LE_PHY_NONE:
		return "No packets";  // Return string "No packets" for this case
	case BT_GAP_LE_PHY_1M:	  // Case when PHY is BT_GAP_LE_PHY_1M
		return "LE 1M";		  // Return string "LE 1M" for this case
	case BT_GAP_LE_PHY_2M:	  // Case when PHY is BT_GAP_LE_PHY_2M
		return "LE 2M";		  // Return string "LE 2M" for this case
	case BT_GAP_LE_PHY_CODED: // Case when PHY is BT_GAP_LE_PHY_CODED
		return "LE Coded";	  // Return string "LE Coded" for this case
	default:				  // Default case for any other PHY value
		return "Unknown";	  // Return string "Unknown" for any other value
	}
}
static void scan_recv(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf)
{
	/* Executed when we recieve the packet from advertising node. */
	char le_addr[BT_ADDR_LE_STR_LEN];
	uint8_t data_status;
	uint16_t data_len;
	char name[30];
	//  (void)memset(name, 0, sizeof(name));

	data_len = buf->len; // Get the length of the data received in the network buffer 'buf'

	bt_data_parse(buf, data_cb, name); // Parse the Bluetooth data in the network buffer using the bt_data_parse function

	data_status = BT_HCI_LE_ADV_EVT_TYPE_DATA_STATUS(info->adv_props); // Extract the data status from the received scan information

	bt_addr_le_to_str(info->addr, le_addr, sizeof(le_addr)); // Convert the Bluetooth device address in 'info->addr' to a string format and store it in 'le_addr'
	LOG_INF ("Address:-%s ,RSSI: %d\n\n",le_addr,info->rssi);
}

static struct bt_le_scan_cb scan_callbacks = {
	// Define a structure named 'scan_callbacks' of type 'struct bt_le_scan_cb'
	.recv = scan_recv, // Assign the 'scan_recv' function as the callback for received scan data
};
#endif
int observer_start(void)
{

	// bt_addr_le_t addr1,addr2,addr3,addr4,addr5,addr6,addr7,addr8,addr9;
	bt_addr_le_t addr7;

	// convert the string into a binary address and stores this address in a buffer whose address is addr.
	// int err=bt_addr_le_from_str("F0:E7:0D:BF:92:05","random" , &addr1);
	// err=bt_addr_le_from_str("E0:34:2C:13:0C:08","random" , &addr2);
	// bt_addr_le_from_str("F4:45:C4:41:7A:86","random" , &addr5);
	// bt_addr_le_from_str("CF:40:B3:E2:5C:58","random" , &addr1);
	// bt_addr_le_from_str("FA:17:DF:DF:4B:89","random" , &addr2);
	// bt_addr_le_from_str("CD:2C:B8:88:9D:FB","random" , &addr3);
	// bt_addr_le_from_str("FF:AB:8A:03:6C:68","random" , &addr4);
	// bt_addr_le_from_str("F9:2E:8E:FB:CE:11","random" , &addr5);
	// bt_addr_le_from_str("D2:F0:F4:22:53:28","random" , &addr6);
	// bt_addr_le_from_str("DE:AC:BE:AF:BC:11","random" , &addr6);
	// bt_addr_le_from_str("CE:AD:BE:AF:BA:11", "random", &addr7);
	bt_addr_le_from_str("FA:17:DF:DF:4B:89", "random", &addr7);
	// bt_addr_le_from_str("DE:AC:BE:AF:BD:11","random" , &addr7);
	//  bt_addr_le_from_str("DB:AD:BE:AF:BA:11","random" , &addr8);
	// bt_addr_le_from_str("EC:86:AB:3B:B2:50","random" , &addr8);
	// bt_addr_le_from_str("DE:AC:BE:AF:BA:11","random" , &addr8);


	/*This will add address to accept filter list*/
	int err = bt_le_filter_accept_list_add(&addr7);
	if (err)
	{
		LOG_INF("Accept list not updated (err %d)\n", err);
		return;
	}
	


	// Parameters for bluetooth long range 
	struct bt_le_scan_param scan_param = {
		.type = BT_HCI_LE_SCAN_PASSIVE,
		.options = BT_LE_SCAN_OPT_FILTER_DUPLICATE | BT_LE_SCAN_OPT_FILTER_ACCEPT_LIST | BT_LE_SCAN_OPT_NO_1M | BT_LE_SCAN_OPT_CODED,
		.interval = 0x0060,
		.window = 0x0030,
		.interval_coded = 0x0060, // For coded PHY
		.window_coded = 0x0030,	  // For coded PHY
	};

	bt_le_scan_cb_register(&scan_callbacks); 
	//This will start scanning
	err = bt_le_scan_start(&scan_param,NULL);
	// err = bt_le_scan_start(&scan_param, scan_cb);  /* Use when you want to use scan_cb */
	if (err)
	{
		LOG_INF("Scanning failed to start (err %d)\n", err);
		return;
	}
	else {
		LOG_INF("Scanning .......");
	}
}
  